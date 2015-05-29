
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STATINTVLREC_H
#define POLYGRAPH__BASE_STATINTVLREC_H

#include "base/LogObj.h"
#include "base/AuthStat.h"
#include "base/IcpStat.h"
#include "base/ProtoIntvlStat.h"


// holds basic statistics for a given interval
class StatIntvlRec: public LogObj {
	public:
		StatIntvlRec();

		void restart();

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &);
		bool sane() const;

		double reqRate() const; // req/sec
		double repRate() const; // rep/sec
		double reqBwidth() const; // bytes/sec
		double repBwidth() const; // bytes/sec
		AggrStat repTime() const; // hit + miss
		AggrStat repSize() const; // hit + miss

		int xactCnt() const { return theXactCnt + theXactErrCnt; }
		BigSize totFillSize() const;
		int totFillCount() const;
		double errRatio() const;
		double errPercent() const;
		double recurrenceRatio() const;
		TmSzStat reps() const; // all successful xactions
		TmSzStat projectedReps() const; // projected xactions
		TmSzStat customProjectedReps() const; // custom projected xactions

		void keepLevels(const StatIntvlRec &prevIntvl);
		void concat(const StatIntvlRec &r); // sequential intervals
		void merge(const StatIntvlRec &r);  // concurrent intervals

		ostream &print(ostream &os, const String &pfx) const;
		void linePrintAll(ostream &os, bool includeLevels) const;
		void linePrintProtos(ostream &os, int offset, bool includeLevels) const;

	protected:
		void join(const StatIntvlRec &r);

	public: /* read only, except for kids */
		LevelStat thePopulusLvl; // number of agents alive

		/* HTTP */
		LevelStat theWaitLvl; // resources waiting requests
		LevelStat theXactLvl; // active xactions
		LevelStat theOpenLvl; // open connections
		LevelStat theEstbLvl; // estanlished connections
		LevelStat theIdleLvl; // idle connections

		AggrStat theConnLifeTm; // to get mean life time
		AggrStat theConnUseCnt; // xactions per connection
		AggrStat theConnPipelineDepth; // max pipeline depth per piped conn

		/* these are only for basic xactions */
		HRStat theIdealHR;    // offered HR
		HRStat theRealHR;     // measured HR
		HRStat theChbR;       // cachablity ratio
		TmSzStat theFill;     // cachable misses

		/* special xactions */
		TmSzStat theRediredReq; // transactions due to redir replies
		TmSzStat theRepToRedir; // transactions resulted in redir replies
		TmSzStat theIms;      // IMS transactions
		TmSzStat theReload;   // reload transactions
		TmSzStat theRange;    // range transactions
		TmSzStat theHead;     // transactions using HEAD request method
		TmSzStat thePost;     // transactions using POST
		TmSzStat thePut;      // transactions using PUT
		TmSzStat theConnect;  // transactions using CONNECT
		TmSzStat theAbort;    // PGL-aborted transactions

		TmSzStat thePage;     // page "download" time and cumulative size

		TmSzStat theCustom;   // user-selected transactions

		int theXactCnt;       // successful xactions 
		int theXactErrCnt;    // xaction errors
		int theXactRetrCnt;   // xaction retries
		int theUniqUrlCnt;    // transactions with unique Request-URIs

		/* other protocols */
		IcpStat theIcpStat;      // Internet Cache Protocol
		ProtoIntvlStat theSocksStat; // SOCKS
		ProtoIntvlStat theSslStat; // Secure Sockets Layer
		ProtoIntvlStat theFtpStat; // File Transfer Protocol

		AggrStat theContinueMsg; // 100 Continue messages

		TmSzStat theProxyValidations; // proxy validation

		AuthStat theAuth; // response size/time for authIng and authEd
		TmSzStat theAuthNone; // response size/time for transactions with no auth
		TmSzStat theTunneled; // response size/time for tunneled transactions

		Time theDuration;     // actual intvl length, computed for us

		// Will be non-static when we start reporting live stats.
		// Will be custom class when we add more live stats.
		static TmSzStat TheLiveReps; // live xactions reply time/size
		static TmSzStat TheCustomLive; // user-selected live transactions

		// warning: ltrace stores us in a Ring which uses memmove
};

struct EmbedStats {
	int tagSeen;
	int tagMatched;
	int attrSeen;
	int attrMatched;
	int urlSeen;
	int foreignUrlRequested;
	int foreignUrlReceived;

	EmbedStats() { tagSeen = tagMatched = attrSeen = attrMatched =
		urlSeen = foreignUrlRequested = foreignUrlReceived = 0; }
};

extern EmbedStats TheEmbedStats;

#endif
