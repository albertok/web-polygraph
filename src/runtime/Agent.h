
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_AGENT_H
#define POLYGRAPH__RUNTIME_AGENT_H

#include "xstd/Socket.h"
#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "base/UniqId.h"
#include "runtime/AgentCfg.h"
#include "runtime/HttpVersion.h"

class AgentSym;
class RndDistr;
class Cache;
class SslCtx;
class OBStream;
class IBStream;
class PortMgr;


// common stuff for servers and clients
class Agent {
	public:
		enum Protocol { pUnknown, pFTP, pHTTP };
		enum ProtoVersion { protoNone, protoHttp1p0, protoHttp1p1 };

	public:
		Agent();
		virtual ~Agent();

		void cache(Cache *aCache);

		const UniqId &id() const { return theId; }
		int seqvId() const { return theSeqvId; }
		const NetAddr &host() const { return theHost; }
		const HttpVersion &httpVersion() const { return theHttpVersion; }
		Cache *cache() { return theCache; }
		Time selectLifetime() const;

		virtual AgentCfg *cfg() = 0;
		virtual void loadWorkingSet(IBStream &is);
		virtual void storeWorkingSet(OBStream &os);
		virtual void missWorkingSet(); // no stored info for this agent

		virtual void start() = 0;
		virtual void stop() = 0;

		virtual int logCat() const = 0;

		virtual void describe(ostream &os) const;

		Socket makeListenSocket(const NetAddr &addr);
		Socket makeListenSocket(PortMgr *const portMgr);

	public:
		bool isCookieSender;

	protected:
		void configure(const AgentSym *cfg, const NetAddr &aHost);
		Socket makeSocket(const NetAddr &addr);
		void selectHttpVersion();

	protected:
		UniqId theId;            // unique agent id (random struct)
		int theSeqvId;           // unique agent id (sequential int)

		String theKind;          // user specified label
		NetAddr theHost;         // the [host:]port we live on
		RndDistr *theLifetimeDistr; // lifetime for xactions
		RndDistr *theThinkDistr; // think time for xactions
		HttpVersion theHttpVersion;
		SslCtx *theSslCtx;
		SockOpt theSockOpt;

	private:
		static int TheLastSeqvId;
		Cache *theCache;         // shared cache for proxy agents
};

#endif
