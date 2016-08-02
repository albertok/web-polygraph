
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/LogComment.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/ObjUniverse.h"
#include "client/WssFreezer.h"

int WssFreezer::TheStartCount = 0;

WssFreezer::WssFreezer() {
	theChannels.append(TheInfoChannel);
}

void WssFreezer::start() {
	Assert(!TheStartCount); // multiple wss freezers are not supported
	TheStartCount++;
	startFill();
	startListen();
}

void WssFreezer::startFill() {
	printGoal(Comment(5) << "fyi: working set size goal: ");
	Comment << endc;

	if (ObjUniverse::Frozen()) {
		Comment(5) << "warning: working set is already frozen" << endc;
		ObjUniverse::ReportWss(5);
	} else {
		Broadcast(TheInfoChannel, BcastRcver::ieWssFill);
	}
}

void WssFreezer::startFreeze() {
	stopListen();
	Comment(5) << "fyi: started freezing working set size" << endc;
	ObjUniverse::FreezeWss();
	Broadcast(TheInfoChannel, BcastRcver::ieWssFreeze);
	if (ObjUniverse::Frozen())
		Comment(5) << "fyi: working set size is now frozen" << endc;
}
