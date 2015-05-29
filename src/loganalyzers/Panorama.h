
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_PANORAMA_H
#define POLYGRAPH__LOGANALYZERS_PANORAMA_H

#include "xstd/Map.h"
#include "loganalyzers/Sample.h"

class Formatter;

// horizontal merge of composite samples with similar structure
// all kids should have the same ID
class Panorama: public CompositeSample {
	public:
		static String TheId;
		static void LabelLocation(const String &location, const String &label);
		static String LocationLabel(const String &location);

	public:
		Panorama *findSlice(const String &key, int idxHint);

		virtual Panorama *genDiff() const;
		virtual void report(Formatter &form) const;

	protected:
		virtual const String &typeId() const { return TheId; }
		const Panorama *panKid(int idx) const;
		Panorama *panKid(int idx);

	private:
		typedef Map<String> LocationLabels;
		static LocationLabels TheLocationLabels;
};

// a leaf panorama node
// all kids should be AtomSamples
class PanAtom: public Panorama {
	public:
		static String TheId;

	public:
		virtual Panorama *genDiff() const;
		virtual void report(Formatter &form) const;

	protected:
		virtual const String &typeId() const { return TheId; }
};

#endif
