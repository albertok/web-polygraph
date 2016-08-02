
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_GZIPENCODER_H
#define POLYGRAPH__CSM_GZIPENCODER_H

#include "csm/BodyIter.h"

namespace zlib {
class Deflator;
}

class GzipEncoder: public BodyIter {
	public:
		GzipEncoder(int aLevel, BodyIter *aProducer);
		virtual ~GzipEncoder();
		virtual GzipEncoder *clone() const;

		virtual void start(WrBuf *aBuf);
		virtual void stop();

		virtual bool pour();

	protected:
		const int theLevel;    // compression level: 0-9
		BodyIter *theProducer; // produces unencoded content
		zlib::Deflator *theEncoder;  // compresses content

		WrBuf theProducerBuf;
};

#endif
