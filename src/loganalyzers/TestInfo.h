
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_TESTINFO_H
#define POLYGRAPH__LOGANALYZERS_TESTINFO_H

#include "xstd/Array.h"
#include "xstd/Time.h"
#include "xstd/String.h"
#include "xstd/BigSize.h"
#include "loganalyzers/SomeInfo.h"

class InfoScopes;
class SideInfo;
class BlobDb;

// aggregate stats and other logged information about a test 
// manages info about all sides of the test
class TestInfo: public SomeInfo {
	public:
		TestInfo(const String &aLabel);
		~TestInfo();

		// get/set scope for executive summary
		const Scope &execScope() const;
		void execScope(const Scope &aScope);
		const Scope &guessExecScope();

		const String &label() const;
		const String &pglCfg() const;
		Time startTime() const;

		bool twoSided() const { return cltSideExists() && srvSideExists(); }
		const SideInfo *cltSideExists() const;
		const SideInfo *srvSideExists() const;
		const SideInfo &aSide() const;
		const SideInfo &cltSide() const;
		const SideInfo &srvSide() const;
		SideInfo &cltSide();
		SideInfo &srvSide();
		const SideInfo &side(int logCat) const;
		SideInfo &side(int logCat);

		int scopes(InfoScopes &res) const;

		int repCount(const Scope &scope) const;
		int hitCount(const Scope &scope) const;
		int uselessProxyValidationCount(const Scope &scope) const;
		BigSize repVolume(const Scope &scope) const;
		BigSize hitVolume(const Scope &scope) const;
		BigSize uselessProxyValidationVolume(const Scope &scope) const;
		AggrStat lastReqByteWritten(const Scope &scope) const;
		AggrStat lastReqByteRead(const Scope &scope) const;
		AggrStat firstRespByteWritten(const Scope &scope) const;
		AggrStat firstRespByteRead(const Scope &scope) const;

		void checkConsistency();

		void compileStats(BlobDb &db);

	protected:
		void checkCommonPglCfg();
		void checkCommonBenchmarkVersion();
		void checkCommonStartTime();

		void cmplExecSumVars(BlobDb &db);
		void cmplExecSum(BlobDb &db);
		void cmplExecSumTable(BlobDb &db, const Scope &cltScope);
		void cmplExecSumPhases(BlobDb &db, const Scope &cltScope);
		void cmplWorkload(BlobDb &db);
		void cmplWorkloadBlob(ReportBlob &blob, const String &side, const String &pglCfg);

		void cmplSynonyms(BlobDb &db, const Scope &scope);
		void cmplHitRatioVars(BlobDb &db, const Scope &scope);
		void cmplHitRatio(BlobDb &db, const Scope &scope);
		void cmplHitRatioTable(BlobDb &db, XmlTag &parent, const Scope &scope) ;
		void cmplCheapProxyValidationVars(BlobDb &db, const Scope &scope);
		void cmplCheapProxyValidation(BlobDb &db, const Scope &scope);
		void cmplCheapProxyValidationTable(BlobDb &db, XmlTag &parent, const Scope &scope) ;

		void cmplByteLatencyVars(BlobDb &db, const Scope &scope);
		void cmplByteLatency(BlobDb &db, const Scope &scope);
		void cmplByteLatencyTable(BlobDb &db, XmlTag &parent, const Scope &scope);
		void cmplByteLatencyHist(BlobDb &db, XmlTag &parent, const Scope &scope);

		void cmplBaseStats(BlobDb &db, const Scope &scope);
		void cmplTraffic(BlobDb &db, const Scope &scope);
		void cmplRptm(BlobDb &db, const Scope &scope);
		void cmplSavings(BlobDb &db, const Scope &scope);
		void cmplLevels(BlobDb &db, const Scope &scope);
		void cmplAuthentication(BlobDb &db, const Scope &scope);
		void cmplErrors(BlobDb &db, const Scope &scope);
		void cmplNotes(BlobDb &db);

	protected:
		String theLabel;
		String theBenchmarkVersion;
		String thePglCfg;
		Time theStartTime;

		Array<SideInfo*> theSides;
		Array<InfoScope*> theScopes;
		String theOneSideWarn;
		Scope theExecScope;
};

#endif
