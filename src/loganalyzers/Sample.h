
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_SAMPLE_H
#define POLYGRAPH__LOGANALYZERS_SAMPLE_H

#include "xstd/String.h"
#include "xstd/Array.h"
#include "xstd/h/iosfwd.h"

class Panorama;
class Formatter;

// a data point, measurement, or spiecemen extracted from the logs
// and tagged with a unique key and descriptive title
class Sample {
	public:
		typedef String Key;

	public:
		virtual ~Sample() {}

		const Key &key() const { return theKey; }
		void key(const Key &aKey) { theKey = aKey; }

		const String &title() const { return theTitle; }
		void title(const String &aTitle) { theTitle = aTitle; }

		const String &location() const { return theLocation; }
		void location(const String &aLocation) { theLocation = aLocation; }
		virtual void propagateLocation(const String &aLocation);

		bool similar(const Sample &s) const;

		virtual Panorama *makePanoramaSkeleton() const = 0;
		virtual void fillPanorama(Panorama *p) const = 0;

		virtual void print(ostream &os) const = 0;
		virtual void reportDifferences(const Sample &, Formatter &form) const;

		virtual const String &typeId() const = 0;
		virtual Sample *clone() const = 0;

	protected:
		virtual bool selfSimilar(const Sample &s) const = 0; // type matches
		void copy(const Sample &s);

	protected:
		Key theKey;
		String theLocation;
		String theTitle;
};

class CompositeSample: public Sample {
	public:
		static String TheId;

	public:
		virtual ~CompositeSample();

		int kidCount() const { return theKids.count(); }
		void add(Sample *aKid);

		virtual Panorama *makePanoramaSkeleton() const;
		virtual void fillPanorama(Panorama *p) const;

		virtual void propagateLocation(const String &aLocation);

		virtual void print(ostream &os) const;

		virtual const String &typeId() const { return TheId; }
		virtual Sample *clone() const;

	protected:
		virtual bool selfSimilar(const Sample &s) const;
		void copy(const CompositeSample &c);

	protected:
		Array<Sample*> theKids;
};

class AtomSample: public Sample {
	public:
		virtual void print(ostream &os) const;

		virtual void setImage(const String &image);
		const String &image() const { return theImage; }

		virtual Panorama *makePanoramaSkeleton() const;

	protected:
		virtual bool selfSimilar(const Sample &s) const;

	private:
		String theImage;
};

class NumberSample: public AtomSample {
	public:
		static String TheId;
		static double TheDelta; // 0 = exact comparison, 1 = 100% difference OK

	public:
		NumberSample();

		virtual void setImage(const String &image);

		virtual void fillPanorama(Panorama *p) const;

		virtual const String &typeId() const { return TheId; }
		virtual Sample *clone() const;

		virtual void reportDifferences(const Sample &, Formatter &form) const;

	protected:
		virtual bool selfSimilar(const Sample &s) const;

	private:
		typedef double Value;
		double theValue;
};

class TextSample: public AtomSample {
	public:
		static String TheId;

		virtual void propagateLocation(const String &aLocation);
		virtual void fillPanorama(Panorama *p) const;

		virtual const String &typeId() const { return TheId; }
		virtual Sample *clone() const;
};

#endif
