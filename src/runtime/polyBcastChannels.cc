
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/polyBcastChannels.h"

BcastChannel *TheAgentBegChannel = 0;
BcastChannel *TheAgentEndChannel = 0;

BcastChannel *TheWaitBegChannel = 0;
BcastChannel *TheWaitEndChannel = 0;

BcastChannel *TheSessionBegChannel = 0;
BcastChannel *TheSessionCntChannel = 0;
BcastChannel *TheSessionEndChannel = 0;

BcastChannel *TheConnOpenChannel = 0;
BcastChannel *TheConnEstChannel = 0;
BcastChannel *TheConnCloseChannel = 0;
BcastChannel *TheConnIdleBegChannel = 0;
BcastChannel *TheConnIdleEndChannel = 0;
BcastChannel *TheConnSslActiveChannel = 0;
BcastChannel *TheConnSslInactiveChannel = 0;

BcastChannel *TheXactBegChannel = 0;
BcastChannel *TheXactEndChannel = 0;
BcastChannel *TheXactErrChannel = 0;
BcastChannel *TheXactRetrChannel = 0;

BcastChannel *TheIcpXactBegChannel = 0;
BcastChannel *TheIcpXactEndChannel = 0;
BcastChannel *TheIcpXactErrChannel = 0;

BcastChannel *ThePageEndChannel = 0;

BcastChannel *TheErrChannel = 0;
BcastChannel *TheInfoChannel = 0;

BcastChannel *TheLogCfgChannel = 0;
BcastChannel *TheLogStateChannel = 0;

BcastChannel *ThePhasesEndChannel = 0;


/* initialization */

int PolyBcastChannelsInit::TheUseCount = 0;

void PolyBcastChannelsInit::init() {
	TheAgentBegChannel = new BcastChannel("agent_beg");
	TheAgentEndChannel = new BcastChannel("agent_end");

	TheWaitBegChannel = new BcastChannel("wait_beg");
	TheWaitEndChannel = new BcastChannel("wait_end");

	TheSessionBegChannel = new BcastChannel("session_beg");
	TheSessionCntChannel = new BcastChannel("session_cnt");
	TheSessionEndChannel = new BcastChannel("session_end");

	TheConnOpenChannel = new BcastChannel("conn_open");
	TheConnEstChannel = new BcastChannel("conn_est");
	TheConnCloseChannel = new BcastChannel("conn_close");
	TheConnIdleBegChannel = new BcastChannel("conn_idle_beg");
	TheConnIdleEndChannel = new BcastChannel("conn_idle_end");
	TheConnSslActiveChannel = new BcastChannel("conn_ssl_active");
	TheConnSslInactiveChannel = new BcastChannel("conn_ssl_inactive");

	TheXactBegChannel = new BcastChannel("xact_beg");
	TheXactEndChannel = new BcastChannel("xact_end");
	TheXactErrChannel = new BcastChannel("xact_err");
	TheXactRetrChannel = new BcastChannel("xact_retr");

	TheIcpXactBegChannel = new BcastChannel("icp_xact_beg");
	TheIcpXactEndChannel = new BcastChannel("icp_xact_end");
	TheIcpXactErrChannel = new BcastChannel("icp_xact_err");

	ThePageEndChannel = new BcastChannel("page_end");

	TheErrChannel = new BcastChannel("error");
	TheInfoChannel = new BcastChannel("info");

	TheLogCfgChannel = new BcastChannel("log_cfg");
	TheLogStateChannel = new BcastChannel("log_state");

	ThePhasesEndChannel = new BcastChannel("shutdown");
}

void PolyBcastChannelsInit::clean() {
	delete TheAgentBegChannel; TheAgentBegChannel = 0;
	delete TheAgentEndChannel; TheAgentEndChannel = 0;

	delete TheWaitBegChannel; TheWaitBegChannel = 0;
	delete TheWaitEndChannel; TheWaitEndChannel = 0;

	delete TheSessionBegChannel; TheSessionBegChannel = 0;
	delete TheSessionCntChannel; TheSessionCntChannel = 0;
	delete TheSessionEndChannel; TheSessionEndChannel = 0;

	delete TheConnOpenChannel; TheConnOpenChannel = 0;
	delete TheConnEstChannel; TheConnEstChannel = 0;
	delete TheConnCloseChannel; TheConnCloseChannel = 0;
	delete TheConnIdleBegChannel; TheConnIdleBegChannel = 0;
	delete TheConnIdleEndChannel; TheConnIdleEndChannel = 0;
	delete TheConnSslActiveChannel; TheConnSslActiveChannel = 0;
	delete TheConnSslInactiveChannel; TheConnSslInactiveChannel = 0;

	delete TheXactBegChannel; TheXactBegChannel = 0;
	delete TheXactEndChannel; TheXactEndChannel = 0;
	delete TheXactErrChannel; TheXactErrChannel = 0;
	delete TheXactRetrChannel; TheXactRetrChannel = 0;

	delete TheIcpXactBegChannel; TheIcpXactBegChannel = 0;
	delete TheIcpXactEndChannel; TheIcpXactEndChannel = 0;
	delete TheIcpXactErrChannel; TheIcpXactErrChannel = 0;

	delete ThePageEndChannel; ThePageEndChannel = 0;

	delete TheErrChannel; TheErrChannel = 0;
	delete TheInfoChannel; TheInfoChannel = 0;

	delete TheLogCfgChannel; TheLogCfgChannel = 0;
	delete TheLogStateChannel; TheLogStateChannel = 0;

	delete ThePhasesEndChannel; ThePhasesEndChannel = 0;
}

