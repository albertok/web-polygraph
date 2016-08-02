
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_URISCRIPTBODYPARSER_H
#define POLYGRAPH__CLIENT_URISCRIPTBODYPARSER_H

#include "xstd/RegEx.h"
#include "client/BodyParserFarm.h"
#include "client/BodyParser.h"

class CltXact;
class CltCfg;

// searches for anything that looks like a URI_SCRIPT and
// forwards them to the owner
class UriScriptBodyParser: public BodyParser {
	public:
		static BodyParser *GetOne(CltXact *anOwner, const CltCfg *aCfg);

	public:
		UriScriptBodyParser();

		void configure(CltXact *anOwner, const CltCfg *aCfg);

		virtual void reset();
		virtual BodyParserFarm &farm() const;

		virtual void noteLeftovers(const ParseBuffer &data);
		virtual void noteOverflow(const ParseBuffer &data);

	protected:
		static void ConfigureScriptVarRe(RegEx &re, const String &name);

		virtual Size parse(const ParseBuffer &data);

		void parseOpen(const ParseBuffer &buf);
		void parseClose(const ParseBuffer &buf);
		void parseBody();
		Size parseAny(const ParseBuffer &buf);

		void parseVars();
		bool parseVar(const char *name, const RegEx &re, String &value);
		void forwardUri(const String &uri);

	protected:
		static RegEx TheWre;
		static RegEx TheXre;
		static RegEx TheYre;
		static RegEx TheZre;

		const CltCfg *theCfg;

		const char *theScriptBeg;
		const char *theScriptEnd;

		enum { stNone = 0, stOpen, stClose, stBody, stDone } theState;

	private:
		void resetSelf();

	private:
		static BodyParserFarmT<UriScriptBodyParser> TheParsers;
};

#endif

