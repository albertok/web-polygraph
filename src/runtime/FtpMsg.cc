
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/OLog.h"
#include "runtime/FtpMsg.h"

PrefixIdentifier FtpReq::TheCmdTable;

/* FtpMsg */

FtpMsg::FtpMsg() {
	FtpMsg::reset();
}

void FtpMsg::reset() {
	theImage = String();
}

void FtpMsg::store(OLog &log) const {
	log << theImage;
}

// XXX: IPv4 only
ostream &FtpMsg::PrintAddr(ostream &os, const NetAddr &addr) {
	for (int i = 0; i < 4; ++i)
		os << addr.octet(i) << ',';
	const int p1 = addr.port() / 256;
	const int p2 = addr.port() - 256 * p1;
	return 	os << p1 << ',' << p2;
}

// XXX: IPv4 only
NetAddr FtpMsg::ParseAddr(const String &str) {
	if (str.len() < 1*6 + 1*5) // digits and commas in a,b,c,d,p1,p2
		return NetAddr();

	// find the first digit
	const char *s = str.cstr();
	const char *const end = s + str.len();
	while (*s && !isdigit(*s)) ++s;
	if (!*s)
		return NetAddr();

	const char *p;
	struct in_addr ip_addr;
	for (int i = 0; i < 4; ++i) {
		int n;
		if (!isInt(s, n, &p) ||
			p + 1 >= end ||
			*p != ',')
			return NetAddr();
		(reinterpret_cast< unsigned char *>(&ip_addr))[i] = n;
		s = p + 1;
	}

	int n1, n2;
	if (!isInt(s, n1, &p) ||
		p + 1 >= end ||
		*p != ',')
		return NetAddr();
	s = p + 1;
	if (!isInt(s, n2))
		return NetAddr();

	return NetAddr(InAddress(ip_addr), n1 * 256 + n2);
}


/* FtpReq */

void FtpReq::Configure() {
	TheCmdTable.add("SYST", frcSYST);
	TheCmdTable.add("USER", frcUSER);
	TheCmdTable.add("PASS", frcPASS);
	TheCmdTable.add("TYPE", frcTYPE);
	TheCmdTable.add("CWD" , frcCWD );
	TheCmdTable.add("PWD" , frcPWD );
	TheCmdTable.add("MDTM", frcMDTM);
	TheCmdTable.add("SIZE", frcSIZE);
	TheCmdTable.add("PASV", frcPASV);
	TheCmdTable.add("PORT", frcPORT);
	TheCmdTable.add("EPSV", frcEPSV);
	TheCmdTable.add("RETR", frcRETR);
	TheCmdTable.add("STOR", frcSTOR);
	TheCmdTable.add("QUIT", frcQUIT);
	TheCmdTable.optimize();
}

FtpReq::FtpReq() {
	FtpReq::reset();
}

void FtpReq::reset() {
	FtpMsg::reset();
	theCmd = frcUnknown;
	theArgument = String();
}

bool FtpReq::parse(const char *buf, Size sz) {
	reset();
	const char *sp = 0;
	const char *nl = 0;
	for (const char *i = buf; !nl && i < buf + sz; ++i) {
		if (*i == '\n')
			nl = i;
		else
		if (*i == ' ' && !sp)
			sp = i;
	}

	if (!nl)
		return false;

	theImage = String(buf, nl - buf + 1);
	if (nl > buf && nl[-1] == '\r')
		--nl;
	const String cmdStr = String(buf, sp ? sp - buf : nl - buf);
	if (const int id = TheCmdTable.lookup(cmdStr))
		theCmd = static_cast<Command>(id);
	else 
		theCmd = frcOther;
	if (sp)
		theArgument = String(sp + 1, nl - sp - 1 );

	return true;
}

void FtpReq::store(OLog &log) const {
	FtpMsg::store(log);
}

FtpReq::Command FtpReq::cmd() const {
	return theCmd;
}

const String &FtpReq::argument() const {
	return theArgument;
}

/* FtpReq */

FtpRep::FtpRep() {
	FtpRep::reset();
}

void FtpRep::reset() {
	FtpMsg::reset();
	theCode = -1;
	theData = String();
}

void FtpRep::store(OLog &log) const {
	FtpMsg::store(log);
	log
		<< theCode
		;
}

bool FtpRep::parse(const char *buf, Size sz) {
	reset();

	const char *nl = buf;
	while (nl < buf + sz) {
		const char *cl = nl;
		for (nl = cl; nl < buf + sz; ++nl) {
			if (*nl == '\n')
				break;
		}
		if (nl < buf + sz &&
			cl + 4 < buf + sz &&
			cl[3] == ' ') {
			int n;
			const char *p;
			if (isInt(cl, n, &p) && p == cl + 3) {
				theCode = n;
				theData = String(p + 1, nl - p);
				theImage = String(buf, nl - buf + 1);
				return true;
			}
		}
		++nl;
	}
	return false;
}

int FtpRep::code() const {
	return theCode;
}

const String &FtpRep::data() const {
	return theData;
}
