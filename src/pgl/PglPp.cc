
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include <fstream>
#include "xstd/CommandToBuffer.h"
#include "pgl/PglLexer.h"
#include "pgl/PglPp.h"
#include "pgl/PglParser.h"


Array<String*> PglPp::TheDirs;


/* PglPp */

PglPp::PglPp(const String &fname): theDepth(0) {
	open(fname);
}

PglPp::~PglPp() {
	while (theDepth > 0) close();
}

LexToken PglPp::scan() {
	Assert(lexer());

	advance();
	while (!filtered() && theDepth);

	syncImage();

	return token();
}

void PglPp::advance() {
	theToken = lexer()->nextToken();
}

// returns true if it is OK to pass current symbol to the parser
bool PglPp::filtered() {
	Assert(lexer());
	switch (symbol()) {
		case CMNT_LINE_TOKEN:
			ignoreLineCmnt(); 
			advance();
			return false;
		case CMNT_OPEN_TOKEN:
			ignoreBlockCmnt();
			advance();
			return false;
		case BQW_STR_TOKEN:
			system();
			return false;
		case POUND_TOKEN: {
			// pp-directives must be the first token on a line
			if (!token().firstOnLine())
				return true;
			ppDirective();
			return false;
		}
		case _EOF_TOKEN: {
			close();
			if (theDepth)
				advance();
			return false;
		}
		default:
			return true;
	}
}

void PglPp::ignoreLineCmnt() {
	while (true) {
		const char c = lexer()->tokenChar().ch;
		if (c == EOF || c == '\n')
			break;
		lexer()->nextCh();
	}
}

void PglPp::ignoreBlockCmnt() {
	const TokenLoc startLoc = token().loc();
	char prev = '\0';
	while (true) {
		const char curr = lexer()->tokenChar().ch;
		if (prev == '/' && curr == '*') {
			cerr << lexer()->tokenChar().loc
				<< "warning: nested /* comments */" << endl
				<< startLoc
				<< "warning: start of a surrounding /* comment */"
				<< endl;
			lexer()->nextCh();
			ignoreBlockCmnt();
		}
		lexer()->nextCh();
		if (curr == EOF || (prev == '*' && curr == '/'))
			break;
		prev = curr;
	}
}

void PglPp::ppdInclude() {
	const String fname = spelling();

	if (symbol() != DQW_STR_TOKEN)
		cerr << token().loc() << "`#include' expects \"FILENAME\", got `" << fname << "'" << endl << xexit;

	open(fname);
	advance();
}

void PglPp::ppDirective() {
	LexToken pound = theToken;

	advance();
	LexToken name = theToken;

	if (name.id() != ID_TOKEN)
		cerr << token().loc() << "malformed preprocessor directive near `" << name.spell() << "'" << endl << xexit;

	if (!pound.loc().sameLine(name.loc()))
		return; // empty pp directive is ignored

	advance();

	if (name.spell() == "include")
		ppdInclude();
	else
		cerr << name.loc() << "unknown preprocessor #directive `" << name.spell() << "'" << endl << xexit;
}

void PglPp::system() {
	const String cmd = spelling();

	if (cmd) { // ignore empty commands?
		if (stringstream *const sbuf = xstd::CommandToBuffer(cmd)) {
			const String h = cmd.len() > 13 ?
				cmd(0, 10) + "..." : cmd;
			open(*sbuf, "`" + h + "`");
		} else {
			cerr << token().loc() << "command '" << cmd
				<< "' probably failed" << endl << xexit;
		}
		advance();
	}
}

void PglPp::open(const String &fname) {
	if (fname == "-") {
		open(*new istream(cin.rdbuf()), fname);
		return;
	}

	for (int j = -1; j < TheDirs.count(); ++j) {
		const String fn = j >= 0 ? *TheDirs[j] + "/" + fname : fname;
		if (istream &is = *new ifstream(fn.cstr(), ios::in)) {
			open(is, fn);
			return;
		}
	}

	printLexers(cerr);
	cerr << token().loc() << "cannot open `" << fname << "';"
		<< " tried " << 1+TheDirs.count() << " location(s):"
		<< endl;
	for (int i = -1; i < TheDirs.count(); ++i) {
		const String fn = i >= 0 ? *TheDirs[i] + "/" + fname : fname;
		cerr << "\t " << fn << endl;
	}
	cerr << xexit;
}

void PglPp::open(istream &is, const String &fname) {
	if (theDepth >= (int)(sizeof(theLexers)/sizeof(*theLexers))) {
		printLexers(cerr);
		cerr << token().loc() << "nesting is too deep in preprocessor" << endl << xexit;
	}

	theLexers[theDepth++] = new PglLexer(is, fname);
}

void PglPp::close() {
	if (theDepth <= 0 || !lexer())
		cerr << here << "internal error, lost lexer?" << endl << xabort;
	istream &is = lexer()->source();
	delete lexer();
	delete &is;
	theLexers[--theDepth] = 0;
}

void PglPp::printLexers(ostream &os) const {
	for (int i = theDepth-1; i > 0; --i) {
		os << theLexers[i-1]->token().loc() << "inclusion of `" << theLexers[i]->fname() << "'" << endl;
	}
}

bool PglPp::spaceAfter(char c) const {
	return isalnum(c) || c == ',' || c == ']' || c == ':';
}

void PglPp::syncImage() {
	if (symbol() <= _EOF_TOKEN)
		return;

	if (symbol() != RIGHTBRACE_TOKEN && theImage && theImage.last() == '\n' && theIndent)
		theImage += theIndent;

	switch (symbol()) {
		case BQW_STR_TOKEN:
			theImage += '`';
			theImage += token().spell();
			theImage += '`';
			break;
		case SQW_STR_TOKEN:
			theImage += '\'';
			theImage += token().spell();
			theImage += '\'';
			break;
		case DQW_STR_TOKEN:
			theImage += '"';
			theImage += token().spell();
			theImage += '"';
			break;
		case LEFTBRACE_TOKEN:
			theImage += token().spell();
			theImage += '\n';
			theIndent += '\t';
			break;
		case RIGHTBRACE_TOKEN:
			theIndent = theIndent.len() <= 1 ?
				String() : theIndent(0, theIndent.len()-1);
			theImage += theIndent;
			theImage += token().spell();
			break;
		case SEMICOLON_TOKEN:
			theImage += token().spell();
			theImage += '\n';
			break;
		case ASGN_TOKEN:
			theImage += " = ";
			break;
		default:
			if (theImage && spaceAfter(theImage.last()) && isalnum(token().spell()[0]))
				theImage += ' ';
			else
			if (!theIndent && theImage && theImage.last() == '\n')
				theImage += '\n';
			theImage += token().spell();
	}
}

