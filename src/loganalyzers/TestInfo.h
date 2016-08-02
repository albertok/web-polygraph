
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

		// executive summary scope; empty if no executive summary is possible
		const Scope &execScope() const;

		const String &label() const;
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

		bool hasScope(const Scope &scope) const;
		int scopes(InfoScopes &res) const;

		Counter repCount(const Scope &scope) const;
		Counter hitCount(const Scope &scope) const;
		Counter uselessProxyValidationCount(const Scope &scope) const;
		BigSize repVolume(const Scope &scope) const;
		BigSize hitVolume(const Scope &scope) const;
		BigSize uselessProxyValidationVolume(const Scope &scope) const;
		AggrStat lastReqByteWritten(const Scope &scope) const;
		AggrStat lastReqByteRead(const Scope &scope) const;
		AggrStat firstRespByteWritten(const Scope &scope) const;
		AggrStat firstRespByteRead(const Scope &scope) const;

		void checkConsistency();
		void compileExecScope(BlobDb &db);
		void compileStats(BlobDb &db);

	protected:
		void checkCommonBenchmarkVersion();
		void checkCommonStartTime();

		void cmplExecSumVars(BlobDb &db);
		void cmplExecSum(BlobDb &db);
		void cmplExecSumTable(BlobDb &db);
		void cmplExecSumPhases(BlobDb &db);
		void cmplWorkload(BlobDb &db);
		void cmplWorkloadBlob(ReportBlob &blob, const String &pfx, const String &key, const String &pglCfg);

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

		void cmplTraffic(BlobDb &db);
		void cmplRptm(BlobDb &db);
		void cmplSavings(BlobDb &db);
		void cmplLevels(BlobDb &db);
		void cmplAuthentication(BlobDb &db);
		void cmplErrors(BlobDb &db);
		void cmplNotes(BlobDb &db);

	protected:
		String theLabel;
		String theBenchmarkVersion;
		Time theStartTime;

		Array<SideInfo*> theSides;
		Array<InfoScope*> theScopes;
		String theOneSideWarn;
		Scope theExecScope;
};

#endif
