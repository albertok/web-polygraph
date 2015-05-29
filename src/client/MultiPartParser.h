
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_MULTIPARTPARSER_H
#define POLYGRAPH__CLIENT_MULTIPARTPARSER_H

#include "client/ParseBuffer.h"
#include "client/BodyParserFarm.h"
#include "client/BodyParser.h"

class CltXact;
class Error;

// parser for multipart HTTP body
// passes parsed content further down the chain,
// and reports every part headers, if any, back to the owner
//
// Note that at present only multipart/byteranges can be parsed.
class MultiPartParser: public BodyParser {
	public:
		static BodyParser *GetOne(CltXact *anOwner, BodyParser *aNextParser, const String &aBoundary);

	public:
		MultiPartParser();
		virtual ~MultiPartParser();
		virtual BodyParserFarm &farm() const;

		void configure(CltXact *anOwner, BodyParser *aNextParser, const String &aBoundary);
		virtual void reset();

		virtual void noteLeftovers(const ParseBuffer &leftovers);
		virtual void noteOverflow(const ParseBuffer &buf);

	protected:
		typedef void (MultiPartParser::*Step)();

	protected:
		virtual Size parse(const ParseBuffer &buf);

		bool mayContinue() const;
		void noteError(const Error &e);

		void parseMessageBeg();
		void parseBoundary();
		void parsePartBeg();
		void parsePartHdrs();
		void parsePartBody();
		void parsePartEnd();
		void parseMessageEnd();

	protected:
		static Step psMessageBeg;
		static Step psBoundary;
		static Step psPartHdrs;
		static Step psPartBody;
		static Step psPartEnd;
		static Step psMessageEnd;

		BodyParser *theNextParser;

		ParseBuffer theBuf;
		Step theStep;
		Size theLeftBodySize;
		bool needMoreData;
		bool theError;
		String theBoundary;
		RepHdr theRepHdr;

	private:
		void resetSelf();

	private:
		static BodyParserFarmT<MultiPartParser> TheParsers;
};

#endif
