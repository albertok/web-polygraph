
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_SCRIPT_H
#define POLYGRAPH__RUNTIME_SCRIPT_H

#include "pgl/PglSemx.h"

class SynSym;
class StatIntvl;
class DutWatchdog;

// run-time in-core PGL interpreter
class Script: public PglSemx {
	public:
		typedef DutWatchdog Sampler;

	public:
		Script(const SynSym &aCode);
		virtual ~Script();
		
		void statsSampler(Sampler *aSampler);

		const SynSym &code() const;

		void run();

		ExpressionSym *callFunc(const String &cname, const ListSym &args);
		virtual void callProc(const String &cname, const ListSym &args);
		virtual void execEveryCode(const EveryCodeSym &ecode);

	protected:
		int logCat() const;

		SynSym *theCode;
		Sampler *theSampler;
};

#endif
