
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_FTPMSG_H
#define POLYGRAPH__RUNTIME_FTPMSG_H

#include "xstd/Size.h"
#include "xstd/String.h"
#include "xstd/PrefixIdentifier.h"

class OLog;

// common interface for parsing FTP requests and responses
class FtpMsg {
	public:
		FtpMsg();
		virtual ~FtpMsg() {}

		virtual void reset();

		virtual bool parse(const char *buf, Size sz) = 0;
		virtual void store(OLog &log) const;

		Size size() const { return theImage.len(); }
		const String &image() const { return theImage; }

		static ostream &PrintAddr(ostream &os, const NetAddr &addr);
		static NetAddr ParseAddr(const String &str);

	protected:
		String theImage;
};

class FtpReq: public FtpMsg {
	public:
		enum Command {
			frcUnknown,
			frcSYST,
			frcUSER,
			frcPASS,
			frcTYPE,
			frcCWD,
			frcPWD,
			frcMDTM,
			frcSIZE,
			frcPASV,
			frcPORT,
			frcEPSV,
			frcRETR,
			frcSTOR,
			frcQUIT,
			frcOther
		};

		static void Configure();

		FtpReq();

		virtual void reset();

		virtual bool parse(const char *buf, Size sz);
		virtual void store(OLog &log) const;

		Command cmd() const;
		const String &argument() const;

	protected:
		Command theCmd;
		String theArgument;

		static PrefixIdentifier TheCmdTable;
};

class FtpRep: public FtpMsg {
	public:
		FtpRep();

		virtual void reset();

		virtual bool parse(const char *buf, Size sz);
		virtual void store(OLog &log) const;

		int code() const;
		const String &data() const;

	protected:
		int theCode;
		String theData;
};

#endif
