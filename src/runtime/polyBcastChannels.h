
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_POLYBCASTCHANNELS_H
#define POLYGRAPH__RUNTIME_POLYBCASTCHANNELS_H

#include "xstd/LibInit.h"
#include "runtime/BcastChannel.h"

extern BcastChannel *TheAgentBegChannel;
extern BcastChannel *TheAgentEndChannel;

extern BcastChannel *TheWaitBegChannel;
extern BcastChannel *TheWaitEndChannel;

extern BcastChannel *TheSessionBegChannel;
extern BcastChannel *TheSessionCntChannel;
extern BcastChannel *TheSessionEndChannel;

extern BcastChannel *TheConnOpenChannel;
extern BcastChannel *TheConnCloseChannel;
extern BcastChannel *TheConnEstChannel;
extern BcastChannel *TheConnIdleBegChannel;
extern BcastChannel *TheConnIdleEndChannel;
extern BcastChannel *TheConnSslActiveChannel;
extern BcastChannel *TheConnSslInactiveChannel;
extern BcastChannel *TheConnSslEstablishedChannel;

extern BcastChannel *TheConnTunnelEstablishedChannel;

extern BcastChannel *TheXactBegChannel;
extern BcastChannel *TheXactEndChannel;
extern BcastChannel *TheXactErrChannel;
extern BcastChannel *TheXactRetrChannel;

extern BcastChannel *TheCompoundXactBegChannel;
extern BcastChannel *TheCompoundXactEndChannel;

extern BcastChannel *TheIcpXactBegChannel;
extern BcastChannel *TheIcpXactEndChannel;
extern BcastChannel *TheIcpXactErrChannel;

extern BcastChannel *ThePageEndChannel;

extern BcastChannel *TheErrChannel;
extern BcastChannel *TheInfoChannel;

extern BcastChannel *TheLogCfgChannel;
extern BcastChannel *TheLogStateChannel;

extern BcastChannel *ThePhasesEndChannel;

LIB_INITIALIZER(PolyBcastChannelsInit)

#endif
