
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "xstd/Clock.h"
#include "runtime/Agent.h"
#include "runtime/CompoundXactInfo.h"
#include "runtime/Connection.h"
#include "runtime/httpHdrs.h"
#include "runtime/Xaction.h"
#include "runtime/IcpXaction.h"
#include "runtime/PageInfo.h"
#include "base/StatIntvlRec.h"
#include "runtime/StatIntvl.h"

#include "runtime/globals.h"
#include "xstd/gadgets.h"
#include "runtime/polyBcastChannels.h"
#include "base/polyLogCats.h"


int StatIntvl::TheReportCat = lgcAll;
Array<bool> StatIntvl::IsActiveCat(lgcEnd);


void StatIntvl::ActiveCat(int cat) {
	Assert(0 <= cat && cat < lgcEnd);
	IsActiveCat.put(true, cat);
}

StatIntvl::StatIntvl() {
	Assert(TheConnOpenChannel);
	theChannels <<
		TheAgentBegChannel << TheAgentEndChannel <<
		TheConnOpenChannel << TheConnEstChannel << TheConnCloseChannel <<
		TheConnIdleBegChannel << TheConnIdleEndChannel <<
		TheConnSslActiveChannel << TheConnSslInactiveChannel <<
		TheConnTunnelEstablishedChannel <<
		TheXactBegChannel << TheXactEndChannel <<
		TheCompoundXactBegChannel << TheCompoundXactEndChannel <<
		TheXactErrChannel << TheXactRetrChannel <<
		TheIcpXactBegChannel << TheIcpXactEndChannel << TheIcpXactErrChannel <<
		ThePageEndChannel <<
		TheWaitBegChannel << TheWaitEndChannel
		;
}

void StatIntvl::setDuration(Time start) {
	const Time dur = TheClock.time() >= start ? TheClock - start : Time(0,0);
	for (int i = 0; i < IsActiveCat.count(); ++i) {
		if (IsActiveCat[i])
			getRec(i).theDuration = dur;
	}
}

void StatIntvl::storeAll(OLog &ol, int tag) const {
	for (int i = 0; i < IsActiveCat.count(); ++i) {
		if (IsActiveCat[i]) {
			ol << bege(tag, i);
			getRec(i).store(ol);
			ol << ende;
		}
	}
}

void StatIntvl::noteAgentEvent(BcastChannel *ch, const Agent *a) {
	Assert(a);
	StatIntvlRec &rec = getRec(a->logCat());

	if (ch == TheAgentBegChannel) {
		++rec.thePopulusLvl;
	} else
	if (ch == TheAgentEndChannel) {
		--rec.thePopulusLvl;
	} else {
		Assert(false);
	}

	checkpoint();
}

void StatIntvl::noteConnEvent(BcastChannel *ch, const Connection *conn) {
	Assert(conn);
	StatIntvlRec &rec = getRec(conn->logCat());

	if (ch == TheConnOpenChannel) {
		++rec.theOpenLvl;
		if (conn->socksProxy())
			++rec.theSocksStat.connLevel();
		if (conn->tunneling()) // TODO: move if an https-proxy is used
			++rec.theConnectStat.connLevel();
		if (ProtoIntvlPtr p = conn->protoStat)
			++(rec.*p).connLevel();
		// SSL has TheConnSslActiveChannel
	} else 
	if (ch == TheConnSslActiveChannel) {
		Should(conn->sslActive());
		++rec.theSslStat.connLevel();
	} else 
	if (ch == TheConnSslInactiveChannel) {
		Should(conn->usedSsl);
		--rec.theSslStat.connLevel();
	} else 
	if (ch == TheConnEstChannel) {
		++rec.theEstbLvl;
	} else
	if (ch == TheConnIdleBegChannel) {
		++rec.theIdleLvl;
	} else
	if (ch == TheConnIdleEndChannel) {
		--rec.theIdleLvl;
	} else
	if (ch == TheConnTunnelEstablishedChannel) {
		ShouldUs(conn->tunneling());
		--rec.theConnectStat.connLevel(); // CONNECT, not just tunnel stats
	} else
	if (ch == TheConnCloseChannel) {
		if (!conn->bad()) {
			rec.theConnLifeTm.record((TheClock - conn->openTime()).msec());
			rec.theConnUseCnt.record(conn->useCnt());

			// record greater-than-1 depths only to get pipelining probability later
			const int depth = conn->useLevelMax();
			if (depth > 1)
				rec.theConnPipelineDepth.record(depth);
		}
		if (conn->ioCnt())
			--rec.theEstbLvl;
		--rec.theOpenLvl;
		if (conn->socksProxy())
			--rec.theSocksStat.connLevel();
		if (ProtoIntvlPtr p = conn->protoStat)
			--(rec.*p).connLevel();
		if (conn->tunneling() && !conn->tunnelEstablished())
			--rec.theConnectStat.connLevel();
		// SSL has TheConnSslInactiveChannel
	} else
		Assert(ch == TheConnSslEstablishedChannel);

	checkpoint();
}

void StatIntvl::noteXactEvent(BcastChannel *ch, const Xaction *x) {
	Assert(x);
	StatIntvlRec &rec = getRec(x->logCat());
	const Time repTime = x->lifeTime();
	const Size repSize = x->repSize().actual();

	if (ch == TheXactBegChannel) {
		++rec.theXactLvl;
		if (x->startedXactSequence())
			++rec.theBaseLvl;
		if (x->socksConfigured())
			++rec.theSocksStat.xactLevel();
		if (ProtoIntvlPtr p = x->protoStat)
			++(rec.*p).xactLevel();
		if (x->oid().connect())
			++rec.theConnectStat.xactLevel();
		else // this and similar exclusions do not work for https-proxy
		if (x->sslConfigured())
			++rec.theSslStat.xactLevel();
		// a beginning xact cannot be in the authing state?
	} else
	if (ch == TheXactEndChannel) {
		const ObjId &oid = x->oid();
		const Size reqSize = x->reqSize().actual();
		const bool authing = x->authing();

		// stats must be recorded in only one category for totals to work!
		if (oid.aborted())
			rec.theAbort.record(repTime, repSize);
		else
		if (authing) {
			Assert(x->logCat() == lgcCltSide);
			rec.theAuthingStat.recordXact(repTime, repSize, false);
			rec.theAuth.recordAuthIng(x->proxyStatAuth(), repTime, repSize);
		} else
		if (oid.basic()) {
			// XXX: calculate and use "ideal" time here
			rec.theIdealHR.record(Time(0,0), repSize, oid.offeredHit());
			rec.theRealHR.record(repTime, repSize, oid.hit());
			rec.theChbR.record(repTime, repSize, oid.cachable());
			if (oid.fill())
				rec.theFill.record(repTime, repSize);
		} else
		if (oid.active() || oid.passive()) {
			; // do nothing, protoStats handles these
		} else
		if (oid.connect())
			rec.theConnectStat.recordXact(repTime, repSize, false);
		else
		if (oid.repToRedir())
			rec.theRepToRedir.record(repTime, repSize);
		else
		if (oid.rediredReq())
			rec.theRediredReq.record(repTime, repSize);
		else
		if (oid.imsAny()) { // should we count 304s separately?
			rec.theIms.record(repTime, repSize);
			if (x->logCat() == lgcSrvSide &&
				x->reqFlags() >= 0 &&
				!(x->reqFlags() & Xaction::xfValidation))
				rec.theProxyValidations.record(repTime, repSize);
		} else
		if (oid.reload())
			rec.theReload.record(repTime, repSize);
		else
		if (oid.range())
			rec.theRange.record(repTime, repSize);
		else
		if (oid.head())
			rec.theHead.record(repTime, repSize);
		else
		if (oid.post())
			rec.thePost.record(repTime, repSize);
		else
		if (oid.put())
			rec.thePut.record(repTime, repSize);
		else {
			Should(false); // all categories should be accounted for
		}

		rec.theRepContType.record(x->actualRepType(), repTime, repSize);
		// XXX: request content type is not determined on server side
		rec.theReqContType.record(x->reqOid().type(), repTime, reqSize);

		rec.theXactCnt++;
		--rec.theXactLvl;
		if (x->logCat() == lgcCltSide && !oid.repeat())
			++rec.theUniqUrlCnt;

		if (x->socksConfigured()) {
			--rec.theSocksStat.xactLevel();

			// comparison with pure HTTP misses will not be accurate
			// comparison with pure HTTP hits and not-hits will be accurate
			if (x->usedSocks())
			rec.theSocksStat.recordXact(repTime, repSize,
				oid.basic() && oid.hit());
		}

		if (x->sslConfigured() && !oid.connect()) {
			--rec.theSslStat.xactLevel();
			// see comparison accuracy comment above
			if (x->usedSsl())
			rec.theSslStat.recordXact(repTime, repSize,
				oid.basic() && oid.hit());
		}

		if (ProtoIntvlPtr p = x->protoStat) {
			--(rec.*p).xactLevel();
			// see comparison accuracy comment above
			(rec.*p).recordXact(repTime, repSize,
				oid.basic() && oid.hit());
		}

		if (oid.connect())
			--rec.theConnectStat.xactLevel();
		if (authing) {
			++rec.theAuthingStat.xactLevel();
			--rec.theAuthingStat.xactLevel();
		}

		if (x->continueMsgTime() > 0)
			rec.theContinueMsg.record(x->continueMsgTime().msec());

		if (x->logCat() == lgcCltSide && !authing) {
			// record auth stats
			// Note: authing stats are record above
			if (x->conn()->tunneling() && !oid.connect()) {
				ShouldUs(x->usedSsl());
				rec.theTunneled.record(repTime, repSize);
			} else if (x->conn()->socksAuthed())
				rec.theAuth.recordAuthEd(AuthPhaseStat::sSocksUserPass, repTime, repSize);
			else if (x->proxyStatAuth() != AuthPhaseStat::sNone ||
				x->conn()->theOriginNtlmState.state != ntlmNone ||
				oid.authCred())
				rec.theAuth.recordAuthEd(x->proxyStatAuth(), repTime, repSize);
			else
				rec.theAuthNone.record(repTime, repSize);
		}

		if (!x->partOf()) {
			rec.theBaseline.recordCompound(x->lifeTime(), reqSize, repSize, 1);
			ShouldUs(x->startedXactSequence());
			--rec.theBaseLvl;
			if (rec.theSocksStat.updateProgress) // any ProtoIntvlStat would do
				TheProgress.success();
		}
	} else
	if (ch == TheWaitBegChannel) {
		++rec.theWaitLvl;
	} else
	if (ch == TheWaitEndChannel) {
		--rec.theWaitLvl;
	} else
	if (ch == TheXactErrChannel) {
		rec.theXactErrCnt++;
		if (x && x->started()) {
			--rec.theXactLvl;
			if (x->socksConfigured()) {
				--rec.theSocksStat.xactLevel();
				if (x->usedSocks())
				rec.theSocksStat.recordXactError();
			}
			if (x->sslConfigured() && !x->oid().connect()) {
				--rec.theSslStat.xactLevel();
				if (x->usedSsl())
				rec.theSslStat.recordXactError();
			}
			if (ProtoIntvlPtr p = x->protoStat) {
				--(rec.*p).xactLevel();
				(rec.*p).recordXactError();
			}
			if (x->oid().connect()) {
				--rec.theConnectStat.xactLevel();
				rec.theConnectStat.recordXactError();
			}
			if (x->authing()) {
				++rec.theAuthingStat.xactLevel();
				--rec.theAuthingStat.xactLevel();
				rec.theAuthingStat.recordXactError();
			}
			if (!x->partOf()) {
				ShouldUs(x->startedXactSequence());
				--rec.theBaseLvl;
				if (rec.theSocksStat.updateProgress)
					TheProgress.failure();
			}
		}
	} else
	if (ch == TheXactRetrChannel) {
		rec.theXactRetrCnt++;
		if (x && x->started()) {
			--rec.theXactLvl;
			if (x->socksConfigured())
				--rec.theSocksStat.xactLevel();
			if (ProtoIntvlPtr p = x->protoStat)
				--(rec.*p).xactLevel();
			if (x->authing()) {
				++rec.theAuthingStat.xactLevel();
				--rec.theAuthingStat.xactLevel();
			}
			if (x->oid().connect())
				--rec.theConnectStat.xactLevel();
			else
			if (x->sslConfigured())
				--rec.theSslStat.xactLevel();
			if (!x->partOf()) {
				ShouldUs(x->startedXactSequence());
				--rec.theBaseLvl;
			}
			// TODO: reduce per-channel duplication of level-maintenance code
		}
	} else {
		Assert(false);
	}

	if (ch == TheXactEndChannel ||
		ch == TheXactRetrChannel ||
		ch == TheXactErrChannel) {
		if (x->inCustomStatsScope())
			rec.theCustom.record(repTime, repSize);
        }

	checkpoint();
}

void StatIntvl::noteCompoundXactEvent(BcastChannel *ch, const CompoundXactInfo *compound) {
	if (ch == TheCompoundXactEndChannel) {
		StatIntvlRec &rec = getRec(compound->logCat);

		// compound->completed() may be false here if the transaction sequence
		// was aborted prematurely (e.g., our CONNECT attempt was rejected).

		compound->record(rec.theBaseline);
		--rec.theBaseLvl;
		if (rec.theSocksStat.updateProgress) // any ProtoIntvlStat would do
			TheProgress.success();
	}
}

void StatIntvl::noteIcpXactEvent(BcastChannel *ch, const IcpXaction *x) {
	Assert(x);
	StatIntvlRec &rec = getRec(x->logCat());

	if (ch == TheIcpXactEndChannel) {
		if (x->timedout())
			rec.theIcpStat.recordTimeout();
		else
			rec.theIcpStat.record(x->lifeTime(), x->repSize(), x->hit());
	}

	checkpoint();
}

void StatIntvl::notePageEvent(BcastChannel *ch, const PageInfo *p) {
	Assert(p);
	StatIntvlRec &rec = getRec(lgcCltSide);
	if (ch == ThePageEndChannel)
		rec.thePage.record(p->lifeTime, p->size);
	checkpoint();
}

bool StatIntvl::checkpoint() {
	return false;
}

void StatIntvl::UpdateLiveStats() {
	static Time lastUpdateTime;
	if (lastUpdateTime == TheClock)
		return;

	lastUpdateTime = TheClock;
	StatIntvlRec::TheLiveReps.reset();
	StatIntvlRec::TheCustomLive.reset();
	for (const Xaction *x = TheLiveXacts.begin();
		x != TheLiveXacts.end();
		x = TheLiveXacts.next(x)) {
		Must(x->started());
		const Time repTime = TheClock - x->startTime();
		const Size repSize = x->repSize().actual();
		StatIntvlRec::TheLiveReps.record(repTime, repSize);
		if (x->inCustomStatsScope())
			StatIntvlRec::TheCustomLive.record(repTime, repSize);
	}
}
