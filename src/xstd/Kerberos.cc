
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <stdlib.h>

#include "xstd/Assert.h"
#include "xstd/Kerberos.h"

namespace Kerberos {

#ifdef KERBEROS_ENABLED
const bool Supported = true;
#else
const bool Supported = false;
#endif


// Context

Context::Context(): theData(0) {
}

Context::Context(const Context &context): theData(0) {
	use(context.theData);
}

Context &Context::operator =(const Context &context) {
	abandon();
	use(context.theData);
	return *this;
}

ErrorCode Context::init() {
#if KERBEROS_ENABLED
	abandon();
	use(new Data);
	return krb5_init_context(&theData->krb5Context);
#else
	return -1;
#endif
}

Context::operator krb5_context() const {
	return theData ? theData->krb5Context : 0;
}

void Context::use(Data *const anImpl) {
	Must(!theData);
	Must(anImpl);
	theData = anImpl;
	++theData->useCount;
}

void Context::abandon() {
	if (theData) {
		if (!--theData->useCount)
			delete theData;
		theData = 0;
	}
}


// Context::Data

Context::Data::Data(): krb5Context(0), useCount(0) {
}

Context::Data::~Data() {
#if KERBEROS_ENABLED
	Must(!useCount);
	if (krb5Context)
		krb5_free_context(krb5Context);
#endif
}


// Data

Data::Data(char *const aData, const unsigned int aLength) {
	set(aData, aLength);
}

void Data::set(char *const aData, const unsigned int aLength) {
	krb5_data::data = aData;
	krb5_data::length = aLength;
}


#if KERBEROS_STRING_IS_NEEDED
// String

void String2::init(const Context &aContext, char *const val) {
	free();
	theContext = aContext;
	theObject = val;
}

void String2::free() {
#if KERBEROS_ENABLED
	krb5_free_string(theContext, theObject); // requires MIT Kerberos v1.10+
	Object<char *>::free();
#endif
}
#endif /* KERBEROS_STRING_IS_NEEDED */

// Error

Error::Error(const Context &aContext, const ErrorCode aCode):
	theCode(aCode) {
#if KERBEROS_ENABLED
	theContext = aContext;
	theObject = krb5_get_error_message(theContext, theCode);
#else
	theObject = "Kerberos support was disabled at build time";
#endif
}

void Error::free() {
#if KERBEROS_ENABLED
	if (theObject)
		krb5_free_error_message(theContext, theObject);
	Object<const char *>::free();
#endif
}

ostream &Error::print(ostream &os) const {
	return os << "Kerberos error: " << message() << " (" << theCode << ")";
}


// Principal

ErrorCode Principal::init(const Context &aContext, const ::String &name) {
#if KERBEROS_ENABLED
	free();
	theContext = aContext;
	return krb5_parse_name(theContext, name.cstr(), &theObject);
#else
	return -1;
#endif
}

void Principal::free() {
#if KERBEROS_ENABLED
	krb5_free_principal(theContext, theObject);
	Object<krb5_principal>::free();
#endif
}


// Creds

ErrorCode Creds::init(const InitCredsContext &initCredsContext) {
#if KERBEROS_ENABLED
	free();
	theContext = initCredsContext.context();
	return krb5_init_creds_get_creds(theContext, initCredsContext,
		&theObject);
#else
	return -1;
#endif
}

ErrorCode Creds::init(const TktCredsContext &tktCredsContext) {
#if KERBEROS_ENABLED
	free();
	theContext = tktCredsContext.context();
	return krb5_tkt_creds_get_creds(theContext, tktCredsContext,
		&theObject);
#else
	return -1;
#endif
}

ErrorCode Creds::init(const Principal &client, const Principal &server) {
#if KERBEROS_ENABLED
	free();
	theContext = client.context();

	if (const ErrorCode e = krb5_copy_principal(theContext, client,
		&theObject.client))
		return e;

	return krb5_copy_principal(theContext, server, &theObject.server);
#else
	return -1;
#endif
}

void Creds::free() {
#if KERBEROS_ENABLED
	krb5_free_cred_contents(theContext, &theObject);
	Object<krb5_creds>::free();
#endif
}


// CCache

const ::String CCache::TheCcTypeMemory = "MEMORY";

ErrorCode CCache::init(const Context &aContext, const ::String &path) {
#if KERBEROS_ENABLED
	free();
	theContext = aContext;
	return path ? krb5_cc_resolve(theContext, path.cstr(),
		&theObject) : krb5_cc_new_unique(theContext,
		TheCcTypeMemory.cstr(), 0, &theObject);
#else
	return -1;
#endif
}

void CCache::free() {
#if KERBEROS_ENABLED
	if (theObject) {
		if (TheCcTypeMemory == krb5_cc_get_type(theContext, theObject))
			Must(!krb5_cc_destroy(theContext, theObject));
		else
			Must(!krb5_cc_close(theContext, theObject));
	}
	Object<krb5_ccache>::free();
#endif
}

ErrorCode CCache::initialize(const Principal &principal) {
#if KERBEROS_ENABLED
	return krb5_cc_initialize(theContext, theObject, principal);
#else
	return -1;
#endif
}

::String CCache::name() const {
#if KERBEROS_ENABLED
	return ::String(krb5_cc_get_type(theContext, theObject)) + ':' +
		krb5_cc_get_name(theContext, theObject);
#else
	return "";
#endif
}

ErrorCode CCache::store(Creds &creds) {
#if KERBEROS_ENABLED
	return krb5_cc_store_cred(theContext, theObject,
		&creds.krb5Creds());
#else
	return -1;
#endif
}


// DefaultCCache

DefaultCCache::DefaultCCache(const CCache &ccache) {
	Set(ccache.name());
}

DefaultCCache::~DefaultCCache() {
	Set("");
}

void DefaultCCache::Set(const ::String &id) {
	Must(!setenv("KRB5CCNAME", id.cstr(), 1));
}


// InitCredsContext

ErrorCode InitCredsContext::init(const Principal &client) {
#if KERBEROS_ENABLED
	free();
	theContext = client.context();
	return krb5_init_creds_init(theContext, client, 0, 0, 0, 0, &theObject);
#else
	return -1;
#endif
}

ErrorCode InitCredsContext::password(const ::String &pass) {
#if KERBEROS_ENABLED
	return krb5_init_creds_set_password(theContext, theObject, pass.cstr());
#else
	return -1;
#endif
}

ErrorCode InitCredsContext::step(Data &in, Data &out) {
#if KERBEROS_ENABLED
	Data realm;
	unsigned int flags;
	return krb5_init_creds_step(theContext, theObject, &in, &out,
		&realm, &flags);
#else
	return -1;
#endif
}

void InitCredsContext::free() {
#if KERBEROS_ENABLED
	krb5_init_creds_free(theContext, theObject);
	Object<krb5_init_creds_context>::free();
#endif
}

Time InitCredsContext::endTime() const {
#if KERBEROS_ENABLED
	krb5_ticket_times times;
	if (!krb5_init_creds_get_times(theContext, theObject, &times))
		return Time::Sec(times.endtime);
	else
		return Time();
#else
	return Time();
#endif
}

// TktCredsContext

ErrorCode TktCredsContext::init(const Principal &client, const Principal &server,
	const CCache &ccache) {
#if KERBEROS_ENABLED
	free();
	theContext = client.context();

	Creds inCreds;
	inCreds.init(client, server);
	return krb5_tkt_creds_init(theContext, ccache, &inCreds.krb5Creds(), 0, &theObject);
#else
	return -1;
#endif
}

ErrorCode TktCredsContext::step(Data &in, Data &out) {
#if KERBEROS_ENABLED
	Data realm;
	unsigned int flags;
	return krb5_tkt_creds_step(theContext, theObject, &in, &out,
		&realm, &flags);
#else
	return -1;
#endif
}

void TktCredsContext::free() {
#if KERBEROS_ENABLED
	krb5_tkt_creds_free(theContext, theObject);
	Object<krb5_tkt_creds_context>::free();
#endif
}

}; // namespace Kerberos
