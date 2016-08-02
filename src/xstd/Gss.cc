
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iomanip.h"
#include "xstd/Assert.h"
#include "xstd/Gss.h"
#include "xstd/Kerberos.h"
#include "xstd/gadgets.h"
#if KERBEROS_ENABLED
#include <gssapi/gssapi_krb5.h>
#endif

namespace Gss {

// Call

template <typename F, typename P1>
Error Call(const F f, P1 p1) {
	uint32_t minorStatus = 0;
	Error e = f(&minorStatus, p1);
	e.minorCode(minorStatus);
	return e;
}

template <typename F, typename P1, typename P2>
Error Call(const F f, P1 p1, P2 p2) {
	uint32_t minorStatus = 0;
	Error e = f(&minorStatus, p1, p2);
	e.minorCode(minorStatus);
	return e;
}

template <typename F, typename P1, typename P2, typename P3>
Error Call(const F f, P1 p1, P2 p2, P3 p3) {
	uint32_t minorStatus = 0;
	Error e = f(&minorStatus, p1, p2, p3);
	e.minorCode(minorStatus);
	return e;
}

template <typename F, typename P1, typename P2, typename P3, typename P4, typename P5>
Error Call(const F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
	uint32_t minorStatus = 0;
	Error e = f(&minorStatus, p1, p2, p3, p4, p5);
	e.minorCode(minorStatus);
	return e;
}

template <typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12>
Error Call(const F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12) {
	uint32_t minorStatus = 0;
	Error e = f(&minorStatus, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
	e.minorCode(minorStatus);
	return e;
}


// Error

Error::Error(const uint32_t aMajor): theMajor(aMajor), theMinor(0) {
}

void Error::minorCode(const uint32_t aMinor) {
	Must(!minorCode());
	theMinor = aMinor;
}

ostream &Error::print(ostream &os) const {
#if KERBEROS_ENABLED
	if (majorCode() || minorCode())
		os << endl << "Kerberos/GSS error stack:" << endl;
	if (majorCode())
		Print(os, majorCode(), GSS_C_GSS_CODE);
	if (minorCode())
		Print(os, minorCode(), GSS_C_MECH_CODE);
	return os;
#else
	return os << "Kerberos support was disabled at build time";
#endif
}

#if KERBEROS_ENABLED
void Error::Print(ostream &os, const uint32_t status, const int type) {
	uint32_t context = 0;
	do {
		Buffer msg;
		Must(!Call(gss_display_status, status, type, GSS_C_NO_OID,
			&context, &msg.rawObject()));
		os << "\t* " << msg << endl;
	} while (context);
}
#endif


// Buffer

void Buffer::free() {
#if KERBEROS_ENABLED
	if (!empty()) {
		Must(!Call(gss_release_buffer, &theObject));
		reset();
	}
#endif
}

void Buffer::reset() {
	theObject.value = 0;
	theObject.length = 0;
}

ostream &Buffer::print(ostream &os) const {
	return os.write(data(), length());
}


// Name

Error Name::init(const String &name) {
#if KERBEROS_ENABLED
	free();
	const void *const data = name.data();
	gss_buffer_desc buf;
	buf.value = const_cast<void *>(data);
	buf.length = name.len();
	return Call(gss_import_name, &buf, GSS_C_NO_OID, &theObject);
#else
	return Error();
#endif
}

void Name::free() {
#if KERBEROS_ENABLED
	Must(!Call(gss_release_name, &theObject));
#endif
}

void Name::reset() {
#if KERBEROS_ENABLED
	theObject = GSS_C_NO_NAME;
#endif
}


// Context

void Context::moveFrom(Context &other) {
	Must(other);
	Should(!*this);
	free();
	theObject = other.theObject;
	theKerberosCCache = other.theKerberosCCache;
	theTarget.moveFrom(other.theTarget);
	theOutputToken.moveFrom(other.theOutputToken);
	theKdc = other.theKdc;
	other.reset();
}

Error Context::init(const Kerberos::CCache &aKerberosCCache, const String &target) {
	theKerberosCCache = &aKerberosCCache;

	if (const Error e = theTarget.init(target))
		return e;

	return process(String());
}

Error Context::process(const String &input) {
#if KERBEROS_ENABLED
	gss_buffer_desc inputBuf;
	if (input) {
		Should(*this);
		theInputToken.resize(input.len());
		theInputToken.resize(DecodeBase64(input.data(), input.len(),
			theInputToken.items(), theInputToken.count()));
		const void *const inputData = theInputToken.items();
		inputBuf.value = const_cast<void *>(inputData);
		inputBuf.length = theInputToken.count();
	} else {
		Should(!*this);
		inputBuf.value = 0;
		inputBuf.length = 0;
	}

	theOutputToken.free();

	// Force usage of the right ccache since our GSS_C_NO_CREDENTIAL below
	// will result in the library looking for the cached credential.
	// TODO: Is there a way to initialize the credentials explicitly while
	// tying them to our cache? gss_acquire_cred_cache() is not available.
	const char **oldNameP = 0;
	if (const Error err = Call(gss_krb5_ccache_name, theKerberosCCache->name().cstr(), oldNameP))
		return err;

	gss_OID *const actualMech = 0;
	uint32_t *const retFlags = 0;
	uint32_t *const timeRec = 0;
	return Call(gss_init_sec_context, GSS_C_NO_CREDENTIAL, &theObject,
		theTarget.rawObject(), SelectMech(), 0, 0,
		GSS_C_NO_CHANNEL_BINDINGS, &inputBuf, actualMech,
		&theOutputToken.rawObject(), retFlags, timeRec);
#else
	return Error();
#endif
}

void Context::free() {
#if KERBEROS_ENABLED
	if (*this)
		Must(!Call(gss_delete_sec_context, &theObject, GSS_C_NO_BUFFER));

	theOutputToken.free();
	theTarget.free();
	theKerberosCCache = 0;
#endif
}

void Context::reset() {
	theObject = GSS_C_NO_CONTEXT;
	theKerberosCCache = 0;
	theKdc = NetAddr();
	// Does not reset theTarget and theOutputToken!
}

ostream &Context::printToken(ostream &os) const {
	if (Should(!theOutputToken.empty()))
		PrintBase64(os, theOutputToken.data(), theOutputToken.length());
	return os;
}

gss_OID Context::SelectMech() {
#if KERBEROS_ENABLED
	static gss_OID Mech = GSS_C_NO_OID;

	static bool didOnce = false;
	if (!didOnce) {
		static const String SpnegoMechStr = "\x2b\x06\x01\x05\x05\x02";
		static const gss_OID_desc SpnegoMechDesc  = {
			static_cast<uint32_t>(SpnegoMechStr.len()),
			const_cast<char *>(SpnegoMechStr.data())
		};
		static const gss_OID SpnegoMech =
			const_cast<gss_OID>(&SpnegoMechDesc);

		gss_OID_set mechSet = GSS_C_NO_OID_SET;
		int present;
		if (Should(!Call(gss_indicate_mechs, &mechSet)) &&
			Should(!Call(gss_test_oid_set_member, SpnegoMech,
			mechSet, &present)) && present)
			Mech = SpnegoMech;

		Must(!Call(gss_release_oid_set, &mechSet));

		didOnce = true;
	}

	return Mech;
#else
	return 0;
#endif
}

}; // namespace Gss
