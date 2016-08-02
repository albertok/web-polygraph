
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_BCASTSENDER_H
#define POLYGRAPH__RUNTIME_BCASTSENDER_H

#include "runtime/BcastChannel.h"
#include "runtime/BcastRcver.h"

void StopTrafficWaiting();

template<class EvType>
inline
void Broadcast(BcastChannel *ch, EvType ev) {
	if (ch->stopsTrafficWaiting)
		StopTrafficWaiting();
	int i = ch->startBcast();
	while (--i >= 0)
		ch->rcver(i)->noteEvent(ch, ev);
}

#endif
