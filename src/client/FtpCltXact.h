
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_FTPCLTXACT_H
#define POLYGRAPH__CLIENT_FTPCLTXACT_H

#include "client/CltXact.h"
#include "runtime/FtpMsg.h"

class FtpCltXact: public CltXact {
	public:
		FtpCltXact();

		virtual void reset();

		// setup and execution
		virtual PipelinedCxm *getPipeline();
		virtual void pipeline(PipelinedCxm *aMgr);
		virtual void freezeProxyAuth() {} // our scheme does not change
		void exec(Connection *const aConn);
		void finish(Error err);

		virtual bool writeFirst() const;
		bool wantsToWrite() const; // false iff writing should pause

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

		virtual bool controlledPostRead(bool &needMore);
		virtual bool controlledFill(bool &needMore);
		virtual bool controlledPostWrite(Size &size, bool &needMore);
		virtual bool controlledMasterRead();

		virtual AuthPhaseStat::Scheme proxyStatAuth() const;

	protected:
		virtual void makeReq(WrBuf &buf);

		Error transition(FtpReq::Command from, FtpReq::Command to);
		void makeUser(ostream &os);
		void makePass(ostream &os);
		void makeType(ostream &os);
		void makePasv(ostream &os);
		void makePort(ostream &os);
		void makeRetr(ostream &os);
		void makeStor(ostream &os);
		void makeQuit(ostream &os);

		Error handleReplies();
		Error interpretReply();
		Error interpretPasv();
		Error kickDataChannel();

		void stopCtrlChannel(const Error &err);
		Error startDataChannel(const NetAddr &addr);
		Error startDataListen();
		void stopDataChannel();
		void stopDataListen();

		void acceptDataConnection();
		void noteDataChannelReadReady();
		void noteDataChannelWriteReady();

		bool finishIfDone();

	protected:
		FtpRep theRep;
		FtpReq::Command theReqCmd; // last FTP command requested

		Connection theDataConn; // data connection
		int theDataPort; // port to listen for incoming data connections
		Socket theSock; // data channel listening socket
		FileScanReserv theReserv;

		enum DataChannelState {
			dcsNone,
			dcsConnecting,
			dcsListening,
			dcsConnected,
			dcsInProgress,
			dcsAborted,
			dcsDone
		};
		DataChannelState theDataChannelState;
};

#endif
