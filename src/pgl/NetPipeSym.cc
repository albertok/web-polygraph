
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include <limits.h>

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglBwidthSym.h"
#include "pgl/PglDistrSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/NetPipeSym.h"

#include "xstd/gadgets.h"



String NetPipeSym::TheType = "NetPipe";

static String strBandwidth = "bandwidth";
static String strKind = "kind";
static String strOutgoing = "outgoing";
static String strPacket_delay = "packet_delay";
static String strPacket_loss = "packet_loss";
static String strQueue_byte_size = "queue_byte_size";
static String strQueue_packet_size = "queue_packet_size";
static String strTime_distr = "time_distr";


NetPipeSym::NetPipeSym(): HostsBasedSym(TheType) {
	theRec->bAdd(StringSym::TheType, strKind, 0);
	theRec->bAdd(strTime_distr, strPacket_delay, 0);
	theRec->bAdd(BwidthSym::TheType, strBandwidth, 0);
	theRec->bAdd(NumSym::TheType, strPacket_loss, 0);
	theRec->bAdd(IntSym::TheType, strQueue_packet_size, 0);
	theRec->bAdd(SizeSym::TheType, strQueue_byte_size, 0);
	theRec->bAdd(BoolSym::TheType, strOutgoing, 0);
}

NetPipeSym::NetPipeSym(const String &aType, PglRec *aRec): HostsBasedSym(aType, aRec) {
}

bool NetPipeSym::isA(const String &type) const {
	return HostsBasedSym::isA(type) || type == TheType;
}

SynSym *NetPipeSym::dupe(const String &type) const {
	if (isA(type))
		return new NetPipeSym(this->type(), theRec->clone());
	return HostsBasedSym::dupe(type);
}

String NetPipeSym::kind() const {
	return getString(strKind);
}

bool NetPipeSym::bandwidthBitsps(int &bitps) const {
	Bwidth byteps = 0;
	if (getBwidth(strBandwidth, byteps)) {
		bitps = (int)MiniMax(0.0, 8*byteps, (double)INT_MAX);
		return true;
	}
	return false;
}

RndDistr *NetPipeSym::packetDelay() const {
	return getDistr(strPacket_delay);
}

bool NetPipeSym::packetLoss(double &plr) const {
	return getDouble(strPacket_loss, plr);
}

bool NetPipeSym::queuePacketSize(int &size) const {
	return getInt(strQueue_packet_size, size);
}

bool NetPipeSym::queueByteSize(int &size) const {
	BigSize sz;
	if (getSize(strQueue_byte_size, sz)) {
		size = sz.byte();
		return true;
	}
	return false;
}

bool NetPipeSym::outgoing() const {
	bool outg;
	if (getBool(strOutgoing, outg))
		return outg;
	else
		return true;
}

bool NetPipeSym::incoming() const {
	bool outg;
	if (getBool(strOutgoing, outg))
		return !outg;
	else
		return true;
}
