
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_SIDEINFO_H
#define POLYGRAPH__LOGANALYZERS_SIDEINFO_H

#include "xstd/Time.h"
#include "xstd/String.h"
#include "xstd/BigSize.h"
#include "xstd/Array.h"
#include "base/ProtoStatPtr.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/SomeInfo.h"

#include <list>

class InfoScopes;
class TestInfo;
class ProcInfo;
class PhaseInfo;
class TmsSzStat;
class StatPhaseRec;
class ErrorRec;
class ErrorStat;
class StatTable;
class XmlTable;
class XmlTag;
class BlobDb;
class Stex;


// aggregate stats and other logged information about a test side 
// (e.g., client- or server-side); 
// manages info about all processes that belong to one side
class SideInfo: public SomeInfo {
	public:
		static void Configure();

	public:
		SideInfo(int aLogCat);
		~SideInfo();

		int logCat() const;
		const String &name() const; // "client" or "server"
		const String &otherName() const; // "server" or "client"
		const String &benchmarkVersion() const;
		const String &pglCfg() const;
		Time startTime() const;
		Counter repCount(const Scope &scope) const;
		Counter hitCount(const Scope &scope) const;
		Counter offeredHitCount(const Scope &scope) const;
		Counter uselessProxyValidationCount(const Scope &scope) const;
		BigSize repVolume(const Scope &scope) const;
		BigSize hitVolume(const Scope &scope) const;
		BigSize offeredHitVolume(const Scope &scope) const;
		BigSize uselessProxyValidationVolume(const Scope &scope) const;
		AggrStat lastReqByteWritten(const Scope &scope) const;
		AggrStat lastReqByteRead(const Scope &scope) const;
		AggrStat firstRespByteWritten(const Scope &scope) const;
		AggrStat firstRespByteRead(const Scope &scope) const;

		void test(TestInfo *t);
		TestInfo *test();

		void add(ProcInfo *info); // absorbs
		ProcInfo &proc(int idx);
		int procCount() const;

		const PhaseInfo &phase(const Scope &scope) const;
		const PhaseInfo &phase(const String &name) const;
		const PhaseInfo &phase(int idx) const;
		int phaseCount() const;
		bool hasPhase(const String &name) const { return findPhase(name); }

		const Scope &scope() const { return theScope; } // all phases
		int scopes(InfoScopes &res) const;
		const Scope &execScope() const;
		const StatPhaseRec &execScopeStats() const;

		void checkConsistency();
		void compileExecScope(const InfoScope *other);
		void compileStats(BlobDb &db);
		void cmplLoadFigure(BlobDb &db, XmlTag &blob, const Scope &scope, const bool small = false) const;
		String cmplRptmFigure(BlobDb &db, const Scope &scope, const bool small = false) const;

	protected:
		typedef const SslPhaseStat::Stat &(SslPhaseStat::*SslStatsPtr)() const;

		static void AddProtoStexes(ProtoIntvlPtr protoPtr);
		static void AddStex(Array<Stex*> &stexes, Stex *stex, const Stex *parent);

		void configureExecScope(const char *reqSrc, const Array<String*> &reqPhases);
		void guessExecScope();

		void addPhase(const PhaseInfo &phase);
		const PhaseInfo *findPhase(const String &name) const;
		PhaseInfo *findPhase(const String &name);

		void checkCommonBenchmarkVersion();
		void checkCommonPglCfg();
		void checkCommonStartTime();
		void checkCommonPhases();

		void compileStats(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplLoadBlob(BlobDb &db, const Scope &scope);
		void cmplLoadTable(BlobDb &db, ReportBlob &parent, const Scope &scope);
		void cmpProtoStats(BlobDb &db, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope);
		void cmplProtoLoadBlob(BlobDb &db, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope);
		void cmplProtoLoadTable(BlobDb &db, ReportBlob &parent, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope);
		void cmplProtoLoadFigure(BlobDb &db, ReportBlob &blob, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope);
		void cmplRptmVsLoadFigure(BlobDb &db, const PhaseInfo &phase, const Scope &scope);

		void cmplHitRatioTable(BlobDb &db, const Scope &scope);
		void cmplHrTraces(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplBhrTrace(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplDhrTrace(BlobDb &db, ReportBlob &blob, const Scope &scope);

		void cmplXactLevelTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplReplyStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplRequestStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplCompoundReplyStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplCompoundRequestStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplAuthStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplReplyStatusStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplXactLevelFigure(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplConnLevelTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplConnLevelFigure(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplConnPipelineBlob(BlobDb &db, const Scope &scope);
		void cmplConnPipelineTable(BlobDb &db, ReportBlob &parent, const Scope &scope);
		void cmplConnPipelineTrace(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplConnPipelineHist(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplPopulLevelTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplPopulLevelFigure(BlobDb &db, ReportBlob &blob, const Scope &scope);
		void cmplReplyObjectTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplRequestObjectTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplCompoundReplyObjectTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplCompoundRequestObjectTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplAuthObjectTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplReplyStatusObjectTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplValidationTable(BlobDb &db, const PhaseInfo &s, const Scope &scope);
		void cmplErrorTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplCookieTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplSslSessionTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope);
		void cmplSslSessionStats(BlobDb &db, const PhaseInfo &phase, const Scope &scope, const String &pfx);
		void cmplSslSessionCommonStats(BlobDb &db, const PhaseInfo &phase, const Scope &scope, const String &pfx, const String &titlePfx, const SslStatsPtr sslStatsPtr);
		void cmplSslSessionTableRec(BlobDb &db, XmlTable &table, const Scope &scope, const String &pfx, const String &name);
		void cmplObjectBlobs(BlobDb &db, const PhaseInfo &phase, const Scope &scope, const Array<Stex*> &stexes);
		void cmplUnseenObjectsBlob(BlobDb &db, const Scope &scope);
		void cmplSideSum(BlobDb &db);

		void cmplLevelTableRec(BlobDb &db, const String &pfx, const String &state, const LevelStat &stats, const Scope &scope, XmlTable &table);
		void cmplObjectTableRec(BlobDb &db, StatTable &table, const Stex &stex, const PhaseInfo &phase, const Scope &scope);
		void cmplStreamTableRec(BlobDb &db, StatTable &table, const Stex &stex, const PhaseInfo &phase, const Scope &scope, const Stex *const topStex);
		void cmplValidationTableRec(BlobDb &db, XmlTable &table, const Stex &stex, const PhaseInfo &phase, const Scope &scope, const String &pfx, const String &name);
		void cmplErrorTableRec(BlobDb &db, XmlTable &table, const ErrorStat &errors, const ErrorRec &error, const Scope &scope);
		void cmplCookieTableRec(BlobDb &db, XmlTable &table, const Stex &stex, const Stex &allStex, const PhaseInfo &phase, const Scope &scope, const String &pfx, const String &name);
		void cmplObjectBlob(BlobDb &db, const Stex &stex, const PhaseInfo &phase, const Scope &scope);

		static XmlTable makeStreamTableHdr(const bool hasParts = false);
		static XmlTable makeObjectTableHdr(const bool hasParts = false);

		static void SortStexes(const PhaseInfo &phase, const Array<Stex*> &in, Array<Stex*> &out);

	protected:
		static Stex *TheAllReps; // points to the top most reply stex
		static Stex *TheAllReqs; // points to the top most request stex
		static Stex *TheUsefulProxyValidation; // points to useful proxy validation stex
		static Array<Stex*> TheReplyStex; // array of reply related stexes
		static Array<Stex*> TheRequestStex; // array of request related stexes
		static Array<Stex*> TheCompoundReplyStex; // array of compound reply stexes
		static Array<Stex*> TheCompoundRequestStex; // array of compound request stexes
		static Array<Stex*> TheAuthStex; // array of auth related stexes
		static Array<Stex*> TheReplyStatusStex; // array of reply status code stexes

	protected:
		Scope theScope;
		mutable Scope theExecScope;
		int theLogCat;
		String theBenchmarkVersion;
		String thePglCfg;
		Time theStartTime;
		TestInfo *theTest;

		Array<ProcInfo*> theProcs;
		Array<PhaseInfo*> thePhases;
		PhaseInfo theExecScopePhase;
		PhaseInfo theAllPhasesPhase;

		std::list<String> theUnseenObjects;
};

#endif
