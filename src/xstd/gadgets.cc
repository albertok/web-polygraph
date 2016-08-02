
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <limits>
#include <limits.h>
#include <ctype.h>
#include <fstream>

#include "xstd/h/math.h"
#include "xstd/h/string.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sys/types.h"
#include "xstd/h/sys/resource.h"
#include "xstd/Time.h"

// see SSI_FD_NEWMAX below
#ifdef HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif

#include "xstd/Assert.h"
#include "xstd/gadgets.h"


// try "ceil(700/0.7)" to see why xceil is needed
// also useful as "xceil(m1*m2, 1)" because "ceil(10*~0.9)" == 10 on Linux
double xceil(double nom, double denom) {
#if HAVE_CEILF
	return (double)ceilf((float)(nom/denom));
#else
	const double cex = ceil(nom/denom);
	const double cm1 = cex-1;
	const double cp1 = cex+1;

	const double dm1 = Abs(nom - cm1*denom);
	const double dex = Abs(nom - cex*denom);
	const double dp1 = Abs(nom - cp1*denom);

	if (dm1 <= dex && nom <= cm1*denom )
		return dm1 <= dp1 || !(nom <= cp1*denom) ? cm1 : cp1;
	else
		return dex <= dp1 || !(nom <= cp1*denom) ? cex : cp1;
#endif
}

int XGCD(int a, int b) {
	return b ? XGCD(b, a % b) : a;
}

// strNstr(), sort of
const char *StrBoundStr(const char *s, const char *embed, const char *eos) {
	if (!Should(embed))
		return 0;
	if (!*embed)
		return 0; // or s?

	const char head = *embed++; // skip first char
	const int tailLen = strlen(embed);
	// find first character first
	while (const char *p = StrBoundChr(s, head, eos)) {
		const char *tail = p + 1;
		if (eos - tail >= tailLen && strncmp(tail, embed, tailLen) == 0)
			return p;
		s = tail;
	}
	return 0;
}

// strNchr(), sort of
const char *StrBoundChr(const char *s, char c, const char *eos) {
	while (s < eos) {
		if (*s == c)
			return s;
		else
			++s;
	}
	return 0;
}

// find char from end of string
const char *StrBoundRChr(const char *s, char c, const char *eos) {
	while (s < eos--) {
		if (*eos == c)
			return eos;
	}
	return 0;
}

// finds next space char
const char *StrBoundSpace(const char *s, const char *eos) {
	while (s < eos) {
		if (isspace(*s))
			return s;
		else
			++s;
	}
	return 0;
}

const char *StrBoundAfterSpace(const char *s, const char *eos) {
	while (s < eos && !isspace(*s))
		s++;
	while (s < eos && isspace(*s))
		s++;
	return s < eos ? s : 0;
}

const char *StrNotSpace(const char *s) {
	while (*s) {
		if (!isspace(*s))
			return s;
		++s;
	}
	return 0;
}

bool isInt(const char *s, int &i, const char **p, int base) {
	const int min = std::numeric_limits<int>::min();
	const int max = std::numeric_limits<int>::max();
	int64_t i64;
	if (isInt64(s, i64, p, base) && Should(min <= i64 && i64 <= max)) {
		i = static_cast<int>(i64);
		return true;
	}
	return false;
}

bool isInt64(const char *s, int64_t &i, const char **p, int base) {
	if (s) {
		char *ptr = 0;
		const int64_t h = strtoll(s, &ptr, base);
		if (ptr != s && ptr) {
			i = h;
			if (p) *p = ptr;
			return true;
		}
	}
	return false;
}

bool isNum(const char *s, double &d, const char **p) {
	if (s) {
		char *ptr = 0;
		const double h = strtod(s, &ptr);
		if (ptr != s) {
			d = h;
			if (p) *p = ptr;
			return true;
		}
	}
	return false;
}

bool isDbl(const char *s, double &d, const char **p) {
	double dd;
	const char *pp;
	if (!isNum(s, dd, &pp))
		return false; 

	Assert(pp);

	// check that it is not an integer (has '.' or 'e')
	const char *h = strchr(s, '.');
	bool dbl = h && h < pp;
	if (!dbl) {
		h = strchr(s, 'e');
		dbl = h && h < pp;
	}

	if (dbl) {
		d = dd;
		if (p) *p = pp;
	}
	return dbl;
}

int xatoi(const char *s, int def) { 
	isInt(s, def);
	return def;
}

void redirectOutput(const char *fname) {
	if (Should(fname)) {
		ofstream *f = new ofstream(fname);
		cout.rdbuf(f->rdbuf());
		cerr.rdbuf(f->rdbuf());
		clog.rdbuf(f->rdbuf());
	}
}

void configureStream(ostream &os, int prec) {
	os.precision(prec);
	os.setf(ios::fixed, ios::floatfield);
	os.setf(ios::showpoint, ios::basefield);
}

// XXX: move into a separate file
static bool base64_initialized = false;
static int base64_value[256];
const char base64_code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz0123456789+/";

static
void base64_init() {
	const int count = (int)(sizeof(base64_value)/sizeof(*base64_value));
	{for (int i = 0; i < count; ++i)
		base64_value[i] = -1;
	}

	{for (int i = 0; i < 64; i++)
		base64_value[(int) base64_code[i]] = i;
	}
	base64_value[(int)'='] = 0;

	base64_initialized = true;
}

// XXX: should check for garbage at the end of inBuf if we ran out of outBuf
int DecodeBase64(const char *inBuf, int inLen, char *outBuf, int outLen) {
	Assert(inBuf && outBuf);

	if (!base64_initialized)
		base64_init();

	int c = 0;
	long val = 0;
	int outPos = 0;
	for (int inPos = 0; inPos < inLen && outPos < outLen; ++inPos) {
		unsigned int k = ((unsigned int) (unsigned char) inBuf[inPos]) % 256;
		if (base64_value[k] < 0)
			continue;
		val <<= 6;
		val += base64_value[k];
		if (++c < 4)
			continue;

		if (outPos < outLen)
			outBuf[outPos++] = val >> 16;         // High 8 bits
		if (outPos < outLen)
			outBuf[outPos++] = (val >> 8) & 0xff; // Mid 8 bits
		if (outPos < outLen)
			outBuf[outPos++] = val & 0xff;        // Low 8 bits
		val = c = 0;
	}
	return outPos;
}


// adopted from http://ftp.sunet.se/pub2/gnu/vm/base64-encode.c with adjustments
ostream &PrintBase64(ostream &os, const char *buf, int len) {
	Assert(buf);

	if (!base64_initialized)
		base64_init();

	int bits = 0;
	int char_count = 0;
	while (len--) {
		const int c = (unsigned char)*buf++;
		bits += c;
		char_count++;
		if (char_count == 3) {
			os
				<< base64_code[bits >> 18]
				<< base64_code[(bits >> 12) & 0x3f]
				<< base64_code[(bits >> 6) & 0x3f]
				<< base64_code[bits & 0x3f];
			bits = 0;
			char_count = 0;
		} else {
			bits <<= 8;
		}
	}

	if (char_count) {
		bits <<= 16 - (8 * char_count);
		os << base64_code[bits >> 18] << base64_code[(bits >> 12) & 0x3f];
		if (char_count == 1)
			os.write("==", 2);
		else
			os << base64_code[(bits >> 6) & 0x3f] << '=';
	}
	return os;
}

		
#ifdef HAVE_GETRLIMIT

static
int rlimit2int(rlim_t rl) {
#ifdef RLIM_INFINITY
	if (rl == RLIM_INFINITY) {
#ifdef __APPLE__
		// On Mac OS X setrlimit(2) does not accept RLIM_INFINITY or
		// INT_MAX values. This causes assertion when FileScanner sets
		// max FD to hard limit. See compatibility section in
		// getrlimit(2).
		return OPEN_MAX;
#else
		return INT_MAX;
#endif
	}
#endif
	// rlim_t can be unsigned so we are trying to avoid comparisons
	// that generate warnings and casting that changes signs
	if (!rl)
		return 0;
	if (rl <= 0)
		return -1;
	Should(rl <= ((rlim_t)INT_MAX));
	return MiniMax(0, (int)rl, INT_MAX);
}

int GetCurFD() {
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		return -1;
	return rlimit2int(rl.rlim_cur);
}

int GetMaxFD() {
	// True64 requires this to by-pass 4K hard limit
#ifdef SSI_FD_NEWMAX
	static bool beenThere = false;
	if (!beenThere) {
		Should(setsysinfo(SSI_FD_NEWMAX, 0, 0, 0, 1) == 0);
		beenThere = true;
	}
#endif
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		return -1;
	return rlimit2int(rl.rlim_max);
}

int SetMaxFD(int maxFD) {
	struct rlimit rl;
	rl.rlim_max = Min(maxFD, GetMaxFD());
	rl.rlim_cur = rl.rlim_max;
	if (rl.rlim_cur > 0)
		Assert(setrlimit(RLIMIT_NOFILE, &rl) >= 0);
	return rlimit2int(rl.rlim_cur);
}

#else /* HAVE_GETRLIMIT */

int GetCurFD() { return -1; }
int GetMaxFD() { return 0xFFFF; }
int SetMaxFD(int maxFD) { return maxFD; }

#endif /* HAVE_GETRLIMIT */
