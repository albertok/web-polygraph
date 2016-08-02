
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLPP_H
#define POLYGRAPH__PGL_PGLPP_H

#include "xstd/Array.h"
#include "xparser/Lexer.h"

class String;
class GLexer;

// PGL preprocessor

class PglPp: public Lexer {
	public:
		PglPp(const String &fname);
		virtual ~PglPp();

		const String image() const { return theImage; }

	protected:
		virtual LexToken scan();
		void advance();

		GLexer *lexer() { return theLexers[theDepth-1]; }
		const GLexer *lexer() const { return theLexers[theDepth-1]; }

		bool filtered();
		void ignoreLineCmnt();
		void ignoreBlockCmnt();
		void ppdInclude();
		void ppDirective();
		void system();

		void open(const String &fname);
		void open(istream &is, const String &fname);
		void close();

		void printLexers(ostream &os) const;

		void syncImage();
		bool spaceAfter(char c) const;

	public:
		static Array<String*> TheDirs;
	
	protected:
		GLexer *theLexers[256]; // replace with Array
		int theDepth;           // nesting depth
		
		String theImage;  // pre-processed image buffer
		String theIndent; // indentation when building theImage
};

#endif
