
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CHUNKEDCODINGPARSER_H
#define POLYGRAPH__CLIENT_CHUNKEDCODINGPARSER_H

#include "client/ParseBuffer.h"
#include "client/BodyParserFarm.h"
#include "client/BodyParser.h"

class CltXact;
class Error;

// parsers chunked coding from RFC 2616,
// passes parsed content further down the chain,
// and reports trailer headers, if any, back to the owner
class ChunkedCodingParser: public BodyParser {
	public:
		static BodyParser *GetOne(CltXact *anOwner, BodyParser *aNextParser);

	public:
		ChunkedCodingParser();
		virtual ~ChunkedCodingParser();
		virtual BodyParserFarm &farm() const;

		void configure(CltXact *anOwner, BodyParser *aNextParser);
		virtual void reset();

		virtual void noteLeftovers(const ParseBuffer &leftovers);
		virtual void noteOverflow(const ParseBuffer &buf);

	protected:
		typedef void (ChunkedCodingParser::*Step)();

	protected:
		virtual Size parse(const ParseBuffer &buf);

		bool mayContinue() const;
		void noteError(const Error &e);

		void parseChunkBeg();
		void parseChunkBody();
		void parseChunkEnd();
		void parseTrailer();
		void parseTrailerHeader();
		void parseMessageEnd();

	protected:
		static Step psChunkBeg;
		static Step psChunkBody;
		static Step psChunkEnd;
		static Step psTrailer;
		static Step psMessageEnd;

		BodyParser *theNextParser;

		ParseBuffer theBuf;
		Step theStep;
		Size theChunkSize;
		Size theLeftBodySize;
		bool needMoreData;
		bool theError;

	private:
		void resetSelf();

	private:
		static BodyParserFarmT<ChunkedCodingParser> TheParsers;
};

#endif
