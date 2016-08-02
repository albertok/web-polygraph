
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/ProxySym.h"



const String ProxySym::TheType = "Proxy";


ProxySym::ProxySym(): AgentSym(TheType) {
}

ProxySym::ProxySym(const String &aType, PglRec *aRec): AgentSym(aType, aRec) {
}

bool ProxySym::isA(const String &type) const {
	return type == TheType || AgentSym::isA(type);
}

SynSym *ProxySym::dupe(const String &type) const {
	if (isA(type))
		return new ProxySym(this->type(), theRec->clone());
	return AgentSym::dupe(type);
}

String ProxySym::msgTypesField() const {
	Assert(false); // should not be called
	return String();
}
