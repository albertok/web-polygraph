
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_GADGETS_H
#define POLYGRAPH__XSTD_GADGETS_H

#include "xstd/h/iostream.h"
#include "xstd/h/stdint.h"
#include <limits>

// abs, min, and max
template <class T> inline T Abs(const T &a) { return a > 0 ? a : -a; }
template <class T> inline T Min(const T &a, const T &b) { return (a <= b) ? a : b; }
template <class T> inline T Max(const T &a, const T &b) { return (a >= b) ? a : b; }
template <class T> inline T Min(const T &a, const T &b, const T &c) { return Min(Min(a,b), c); }
template <class T> inline T Max(const T &a, const T &b, const T &c) { return Max(Max(a,b), c); }
template <class T> inline T MiniMax(const T &l, const T &v, const T &h) { return Min(Max(l,v), h); }

// equality for floating point numbers
template <class T> inline bool Equal(const T &x, const T &y) { return Abs(x - y) < std::numeric_limits<T>::epsilon(); }

// safe division
template <class T>
inline double Ratio(const T n, const double d) { return d ? (n ? n/d : 0) : -1; }
template <class T>
inline double Percent(const T n, const double d) { return Ratio(n, d/100); }

// try "ceil(700/0.7)" to see why xceil is needed
extern double xceil(double nom, double denom);

// the greatest common divider
extern int XGCD(int a, int b);

// to avoid problems with signed chars as indices into arrays
inline int xord(char c) { return (int)(unsigned char)c; }

// strNchr() and friends, sort of
extern const char *StrBoundChr(const char *s, char c, const char *eos);
extern const char *StrBoundStr(const char *s, const char *embed, const char *eos);
// search for char from end of string
extern const char *StrBoundRChr(const char *s, char c, const char *eos);
// finds next white space
extern const char *StrBoundSpace(const char *s, const char *eos);
// finds next white space and skips it
extern const char *StrBoundAfterSpace(const char *s, const char *eos);
// finds next non white space
extern const char *StrNotSpace(const char *s);

// a better atoi, returns def on error
extern int xatoi(const char *s, int def = 0);

// similar to strto*-like functions
// note: isDbl fails on integers, isNum does not
extern bool isInt(const char *s, int &i, const char **p = 0, int base = 0);
extern bool isInt64(const char *s, int64_t &i, const char **p = 0, int base = 0);
extern bool isDbl(const char *s, double &d, const char **p = 0);
extern bool isNum(const char *s, double &d, const char **p = 0);

// sends cout, clog, and cerr to fname
extern void redirectOutput(const char *fname);
// set ``default'' format for a stream and custom precision
extern void configureStream(ostream &os, int prec);

// returns the number of bytes put into outBuf
extern int DecodeBase64(const char *inBuf, int inLen, char *outBuf, int outLen);
extern ostream &PrintBase64(ostream &os, const char *buf, int len);

// returns FD limits
extern int GetCurFD(); // current maximum
extern int GetMaxFD(); // absolute maximum
// attempts to set max number of FDs; returns actual maxFD set or -1 on failure
extern int SetMaxFD(int maxFD);

#endif
