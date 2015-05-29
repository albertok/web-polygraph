
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_URIBODYPARSER_H
#define POLYGRAPH__CLIENT_URIBODYPARSER_H

#include "client/BodyParserFarm.h"
#include "client/BodyParser.h"

class CltXact;
class CltCfg;

// searches for anything that looks like a URI and
// forwards them to the owner
class UriBodyParser: public BodyParser {
	public:
		static BodyParser *GetOne(CltXact *anOwner, const CltCfg *aCfg);

	public:
		UriBodyParser();

		void configure(CltXact *anOwner, const CltCfg *aCfg);

		virtual void reset();
		virtual BodyParserFarm &farm() const;

		virtual void noteLeftovers(const ParseBuffer &data);
		virtual void noteOverflow(const ParseBuffer &data);

	protected:
		virtual Size parse(const ParseBuffer &data);
		void parseTag(const char *tagB, const char *tagE);

	protected:
		const CltCfg *theCfg;

	private:
		void resetSelf();

	private:
		static BodyParserFarmT<UriBodyParser> TheParsers;
};

#endif
