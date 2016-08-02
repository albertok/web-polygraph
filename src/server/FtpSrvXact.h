
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_FTPSRVXACT_H
#define POLYGRAPH__SERVER_FTPSRVXACT_H

#include "runtime/FtpMsg.h"
#include "server/SrvXact.h"

class FtpSrvXact: public SrvXact {
	public:
		FtpSrvXact();

		virtual void reset();
		virtual void finish(Error err);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

	protected:
		virtual void doStart();
		virtual void noteConnReady();

		virtual void noteHdrDataReady();
		virtual void noteRepSent();
		virtual void makeRep(WrBuf &buf);

		Error interpretCmd();
		Error interpretPort();
		Error startDataListen();
		Error startDataChannel();
		void stopDataListen();
		void closeDataChannel();
		void acceptDataConnection();
		void noteDataChannelWriteReady();
		void noteDataChannelReadReady();
		Error processXferRequest();
		bool prepareContent();
		Error startDataXfer();
		bool parseUrl();

		void makeSimpleResponse(ostream &os, const String &reply);
		void makeMdtmResponse(ostream &os);
		void makeSizeResponse(ostream &os);
		void makePasvResponse(ostream &os);
		void makeXferResponse(ostream &os);

		FtpReq theReq;
		String theDir; // current directory, controlled by CWD commands
		String theFilename; // last RETR command argument
		Socket theSock; // data channel listening socket
		FileScanReserv theReserv;
		MessageSize theWrittenSize; // accumulates, then updates theRepSize
		Connection theDataConn; // data connection
		int theDataPort; // port to listen for incoming data connections
		NetAddr thePortAddr; // PORT address for active FTP

		// data transfer state
		enum DataXferState {
			dxsNone, // transfer was not requested or was fully completed
			dxsConnWaiting, // xfer requested, but waiting for a data conn
			dxsInProgress, // we are writing data
			dxsComplete, // wrote everything
			dxsAborted, // could not write everything due to I/O errors
			dxsBadUrl // could not start writing because bad URL was requested
		};
		// data I/O state; a combination of listen/connect and data socket states
		enum DataIoState {
			disNone, // not listening or writing
			disListening, // listening
			disConnecting, // connecting
			disWait, // waiting for RETR to start writing
			disBusy // writing
		};
		DataXferState theDataXferState;
		DataIoState theDataIoState;
		bool sent1xx;
};

#endif
