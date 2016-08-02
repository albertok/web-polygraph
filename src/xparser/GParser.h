
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_GPARSER_H
#define POLYGRAPH__XPARSER_GPARSER_H

#include "xstd/Array.h"
#include "xparser/SynSym.h"
#include "xparser/Lexer.h"

struct ReduceInfo;


// The GParser class is the base class from which generated parser classes
// are derived from.

// The GParser class provides the general bottom-up driver routine
// for LR(1) parsing.  It includes two stacks: the first is the
// push-down stack for parsing, while the second is for semantic
// attributes.

// It also continues hooks for 3 other tables: the actionTable
// merges parsing action and transition information.  A position
// value indicates a shift to a destination state.  A negative
// value indicates a reduction whose details are found in the
// reduceTable.  Conditionals are indicated by the condTable
// pointer.

class GParser {
	public:
		GParser(Lexer *);
		virtual ~GParser();

		const SynSym *parse();

	protected:
		virtual int actionTable(int, int) = 0;
		virtual ReduceInfo *reduceTable(int) = 0;

		int top() { return theParseStack.last(); }
		void push(int x) { theParseStack.push(x); }
		void throwAway(int x) { theParseStack.pop(x); }

		SynSym **sem_top(const int len);
		void sem_push(SynSym *x);
		void sem_throwAway(int x);
		void peek();
		const TokenLoc &tokenLoc() const { return lexer->token().loc(); }

	protected:
		Lexer *lexer;

		int maxState;
		int acceptState;
		int *condTable;

		Array<SynSym*> theSemStack;
		Array<int> theParseStack;
};


struct ReduceInfo {
	int head;
	int parseItems;
	int semItems;
	SynSym *(*action)(SynSym **);
	int   (*conditional)(SynSym **);
};

#endif
