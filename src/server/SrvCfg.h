
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_SRVCFG_H
#define POLYGRAPH__SERVER_SRVCFG_H

#include "xstd/Array.h"
#include "csm/ContentTypeIdx.h"
#include "runtime/AgentCfg.h"

class XactAbortCoord;
class ServerSym;
class PopModel;
class RndDistr;
class Area;

// Server configuration items that can be shared among multiple servers
class SrvCfg: public AgentCfg {
	public:
		SrvCfg();
		~SrvCfg();

		void configure(const ServerSym *aServer);

		bool hasContType(int id) const { return theTypes.hasContType(id); }

		// whether SSL is configured and supported for protocol
		bool sslActive(const int protocol) const;

		int selectProtocol();
		void selectAbortCoord(XactAbortCoord &coord);
		bool setEmbedContType(ObjId &oid, const Area &category) const;

	protected:
		void configureRepTypes();
		void configureProtocols();
		void configureCookies();

	public:
		const ServerSym *theServer;  // used to identify/share configs
		ContentTypeIdx theTypes;  // IDs of all known content types
		PopModel *thePopModel;    // popularity model to repeat oids
		RndDistr *theRepTypeSel;  // selects reply types	
		RndDistr *theProtocolSel; // selects protocol

		RndDistr *theCookieCounts; // cookies per response
		RndDistr *theCookieSizes;  // individual cookie value sizes
		double theCookieSendProb;  // probability of sending a cookie

		double theAbortProb;
		double theReqBodyAllowed;
};


class SrvSharedCfgs: protected Array<SrvCfg*> {
	public:
		~SrvSharedCfgs();
		SrvCfg *getConfig(const ServerSym *cfg);

	protected:
		SrvCfg *addConfig(const ServerSym *cfg);
};

extern SrvSharedCfgs TheSrvSharedCfgs;

#endif
