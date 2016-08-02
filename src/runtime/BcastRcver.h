
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_BCASTRCVER_H
#define POLYGRAPH__RUNTIME_BCASTRCVER_H

#include "xstd/Array.h"

class BcastChannel;
class Agent;
class Client;
class Connection;
class Xaction;
class CompoundXactInfo;
class IcpXaction;
class PageInfo;
class OLog;

// broadcast channel(s) subscriber

class BcastRcver {
	public:
		typedef enum { ieNone, ieWssFill, ieWssFreeze, ieReportProgress } InfoEvent;

	public:
		virtual ~BcastRcver();

		inline void noteEvent(BcastChannel *ch, const Agent *c);
		inline void noteEvent(BcastChannel *ch, const Client *c);
		inline void noteEvent(BcastChannel *ch, const Connection *c);
		inline void noteEvent(BcastChannel *ch, const Xaction *x);
		inline void noteEvent(BcastChannel *ch, const CompoundXactInfo *cx);
		inline void noteEvent(BcastChannel *ch, const IcpXaction *x);
		inline void noteEvent(BcastChannel *ch, const PageInfo *p);
		inline void noteEvent(BcastChannel *ch, const char *msg);
		inline void noteEvent(BcastChannel *ch, const Error &err);
		inline void noteEvent(BcastChannel *ch, InfoEvent ev);
		inline void noteEvent(BcastChannel *ch, OLog *log);

		void startListen();
		void stopListen();

	protected:
		virtual void noteAgentEvent(BcastChannel *ch, const Agent *c);
		virtual void noteClientEvent(BcastChannel *ch, const Client *c);
		virtual void noteConnEvent(BcastChannel *ch, const Connection *c);
		virtual void noteXactEvent(BcastChannel *ch, const Xaction *x);
		virtual void noteCompoundXactEvent(BcastChannel *ch, const CompoundXactInfo *x);
		virtual void noteIcpXactEvent(BcastChannel *ch, const IcpXaction *x);
		virtual void notePageEvent(BcastChannel *ch, const PageInfo *p);
		virtual void noteMsgStrEvent(BcastChannel *ch, const char *msg);
		virtual void noteErrEvent(BcastChannel *ch, const Error &err);
		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);
		virtual void noteLogEvent(BcastChannel *ch, OLog &log);

	protected:
		Array<BcastChannel*> theChannels; // channels we are subscribed to
};


/* inlined methods */

inline
void BcastRcver::noteEvent(BcastChannel *ch, const Agent *a) {
	noteAgentEvent(ch, a);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const Client *c) {
	noteClientEvent(ch, c);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const Connection *c) {
	noteConnEvent(ch, c);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const Xaction *x) {
	noteXactEvent(ch, x);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const CompoundXactInfo *cx) {
	noteCompoundXactEvent(ch, cx);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const IcpXaction *x) {
	noteIcpXactEvent(ch, x);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const PageInfo *x) {
	notePageEvent(ch, x);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const char *msg) {
	noteMsgStrEvent(ch, msg);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, const Error &err) {
	noteErrEvent(ch, err);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, InfoEvent ev) {
	noteInfoEvent(ch, ev);
}

inline
void BcastRcver::noteEvent(BcastChannel *ch, OLog *log) {
	noteLogEvent(ch, *log);
}

#endif
