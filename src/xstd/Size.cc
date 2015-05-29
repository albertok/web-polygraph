
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/Size.h"

Size Size::Bit(int n) {
	Assert(n % 8 == 0);
	return Byte(n/8);
}

ostream &Size::print(ostream &os) const {

	if (theSize < 1024) {
		os << theSize << "Bytes";
	} else {
		const int osprec = os.precision();
		os.precision(3);
		
		if (theSize < 1024*1024)
			os << theSize/1024. << "KB";
		else
		if (theSize < 1024*1024*1024)
			os << theSize/(1024*1024.) << "MB";
		else
			os << theSize/(1024*1024*1024.) << "GB";
		
		os.precision(osprec);
	}

	return os;
}
