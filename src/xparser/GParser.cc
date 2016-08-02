
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include "xparser/TokenSym.h"
#include "xparser/GParser.h"

#ifndef DEBUGPRINT
#define DEBUGPRINT 0
#endif

GParser::GParser(Lexer *aLexer): lexer(aLexer) {
	peek();
}

GParser::~GParser() {
}

const SynSym *GParser::parse() {
	int nextState;
	ReduceInfo *p;
	SynSym *n_sym;

	push(1);
#if DEBUGPRINT
	cerr << here "push 1" << endl;
#endif
	for (;;) {

		// A transition to a state number greater than the maximum number of
		// states indicates dynamic conflict resolution.
		
	    nextState = actionTable(top(), lexer->symbol());

#if DEBUGPRINT
		cerr << tokenLoc() << "state: " << nextState
			<< ", symbol " << lexer->symbol() << endl;
#endif

	    if ((nextState = actionTable(top(), lexer->symbol())) >= maxState) {

			// Where conditionals exist, each predicate is attempted in
			// sequence.  Where none of the predicates evaluate to true,
			// the parsing action might also default to a normal shift
			// or reduce.

			int a, i = nextState-maxState;
			while ((a = condTable[i])) {
			    if (a < 0) {
				nextState = a;
				// normal reduce action found
				goto to_reduce;
			    } else if (a >= maxState) {
				p = reduceTable(a-maxState);
				if ((*(p->conditional))(sem_top(p->semItems))) {
				    nextState = maxState-a;
				    // conflict resolved, production to be used
				    // has been decided
				    goto to_reduce;
				} else
				    i++;
			    } else {
				nextState = a;
				// normal shift action found
				goto to_shift;
			    }
			}
			// since we cannot resolve the conflict, and no
			// normal actions are available, we report error
			goto to_error;
	    } else 
		if (nextState > 0) {

			// For a conventional shift action, shifting to the accept
			// state indicates a successful parser.

		to_shift:
			if (nextState == acceptState)
				return theSemStack.empty() ? 0 : *sem_top(0);
#if DEBUGPRINT
			cerr << tokenLoc() << "term: " << lexer->symbol()
				<< " goto " << nextState << endl;
#endif
			push(nextState);
			sem_push(new TokenSym(lexer->spelling(), lexer->symbol()));
			peek();
	    } else
		if (nextState < 0) {

		to_reduce:
			p = reduceTable(-nextState);

			// A negative number of parse items to be removed indicates
			// the evaluation of inherited attributes.
			if (p->parseItems < 0) {
				cerr << here << "inherited symbols are not supported" << endl << xabort;
			} else {

				// Where reductions involve attribute evaluation, these are 
				// performed before stack rearrangement is effected.

			    if (p->action) {
					// normal reduce with action routine
					SynSym **base = sem_top(p->semItems);
					n_sym = (*(p->action))(base);
					if (!n_sym)
						cerr << here << "null symbol on parsing stack :(" << endl << xabort;
					if (!n_sym->loc() && p->semItems > 0 && base[1])
						n_sym->loc(base[1]->loc());
				} else {
					// normal reduce without action routine
					// should not happened in this version!
					n_sym = 0;
				}
			    sem_throwAway(p->semItems);
			    sem_push(n_sym);

			    if (p->parseItems - p->semItems)
					cerr << here << "inherited symbols are not supported" << endl << xabort;
#if DEBUGPRINT
				cerr << tokenLoc() << "pop " << p->parseItems << endl;
#endif
				throwAway(p->parseItems);
			}
#if DEBUGPRINT
			cerr << tokenLoc() << "state: " << top()
				<< " NT " << p->head
				<< " goto " << actionTable(top(), p->head) << endl;
#endif
			push(actionTable(top(), p->head));
		} else {

		to_error:
			// are we printing current location here?
			cerr << tokenLoc() << "syntax error";
			if (lexer->token().spell())
				cerr << " near `" << lexer->spelling() << "'";
			cerr << endl << xexit;
	    }
	}
}

SynSym **GParser::sem_top(int len) {
	// Cannot use Array::item() method here because idx can be negative.
	const int idx = theSemStack.count() - len - 1;
	return &theSemStack[idx];
}

void GParser::sem_push(SynSym *x) {
	if (!x)
		cerr << here << "null symbol on parsing stack :(" << endl << xabort;

	if (!x->loc())
		x->loc(tokenLoc());

	if (theSemStack.count() >= 4096)
		cerr << tokenLoc() << "parsing stack grew suspiciously large (" << theSemStack.count() << " symbols)" << endl << xexit;

	theSemStack.push(x);
}

void GParser::sem_throwAway(int x) {
	while (x-- > 0)
		delete theSemStack.pop();
}

void GParser::peek() {
	lexer->nextToken();
}
