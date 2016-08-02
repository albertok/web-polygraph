
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_CONTENTSYM_H
#define POLYGRAPH__PGL_CONTENTSYM_H

#include "xstd/Array.h"
#include "pgl/PglRecSym.h"

class MimeHeaderSym;
class ClientBehaviorSym;
class MimeSym;
class RndDistr;
class ObjLifeCycleSym;

// parameters for object life cycle model
class ContentSym: public RecSym {
	public:
		static String TheType;

	public:
		ContentSym();
		ContentSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool hasEmbed() const;

		String kind() const;
		MimeSym *mime() const;
		RndDistr *size() const;
		ObjLifeCycleSym *objLifeCycle() const;
		bool cachable(double &ratio) const;
		bool unique(double &ratio) const;
		bool checksum(double &ratio) const;
		bool contains(Array<ContentSym*> &ccfgs, RndDistr *&selector, Array<double> &cprobs) const;
		RndDistr *embedCount() const;
		bool choiceSpace(int &space) const;
		ClientBehaviorSym *clientBehavior() const;
		String generator() const;
		String cdb() const;

		String injectObject() const;
		String injectDb() const;
		bool infectProb(double &prob) const;
		RndDistr *injectGap() const;

		bool encodings(Array<String*> &codings) const;
		const ArraySym *mimeHeaders() const;
		String documentRoot() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
