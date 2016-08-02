
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_GSS_H
#define POLYGRAPH__XSTD_GSS_H

#include "xstd/h/stdint.h"

#include "xstd/Array.h"
#include "xstd/String.h"
#include "xstd/NetAddr.h"

#if KERBEROS_ENABLED

#if HAVE_GSSAPI_GSSAPI_H
#include <gssapi/gssapi.h>
#elif HAVE_GSSAPI_H
#include <gssapi.h>
#endif

#endif

namespace Kerberos {
class CCache;
};

namespace Gss {

#if !KERBEROS_ENABLED

const uint32_t GSS_S_COMPLETE = 0;

inline bool GSS_ERROR(const uint32_t) { return true; }

struct gss_buffer_desc {
	size_t length;
	void *value;
};

typedef void *gss_OID;
typedef void *gss_ctx_id_t;
typedef void *gss_name_t;

const gss_ctx_id_t GSS_C_NO_CONTEXT = 0;

#endif

class Error {
	public:
		Error(const uint32_t aMajor = GSS_S_COMPLETE);

		uint32_t majorCode() const { return theMajor; }
		uint32_t minorCode() const { return theMinor; }

		void minorCode(const uint32_t aMinor);

		operator void *() const { return GSS_ERROR(majorCode()) ? (void*)-1 : (void*)0; }

		ostream &print(ostream &os) const;

	protected:
		static void Print(ostream &os, const uint32_t status, const int type);

		uint32_t theMajor;
		uint32_t theMinor;
};

template <typename GssObject>
class Object {
	public:
		Object() {}

		void moveFrom(Object &other);
		virtual void free() = 0;
		const GssObject &rawObject() const { return theObject; }

	protected:
		// not implemented
		Object(const Object &);
		Object &operator =(const Object &);

		virtual void reset() = 0;

		GssObject theObject;
};

class Buffer: public Object<gss_buffer_desc> {
	public:
		Buffer() { reset(); }
		~Buffer() { free(); }

		bool empty() const { return theObject.length <= 0; }
		const char *data() const { return reinterpret_cast<const char *>(theObject.value); }
		size_t length() const { return theObject.length; }

		virtual void free();

		gss_buffer_desc &rawObject() { Must(empty()); return theObject; }

		ostream &print(ostream &os) const;

	protected:
		virtual void reset();
};

class Name: public Object<gss_name_t> {
	public:
		Name() { reset(); }
		~Name() { free(); }

		Error init(const String &name);
		virtual void free();

	protected:
		virtual void reset();
};

class Context: public Object<gss_ctx_id_t> {
	public:
		Context() { reset(); }
		~Context() { free(); }

		void moveFrom(Context &other);
		Error init(const Kerberos::CCache &aKerberosCCache, const String &target);
		Error process(const String &input);
		virtual void free();

		operator void *() const { return theObject == GSS_C_NO_CONTEXT ? (void*)0 : (void*)-1; }
		ostream &printToken(ostream &os) const;
		void kdcAddr(const NetAddr &aKdcAddr) { theKdc = aKdcAddr; }
		const NetAddr &kdcAddr() const { return theKdc; }

	protected:
		virtual void reset();

		static gss_OID SelectMech();

		const Kerberos::CCache *theKerberosCCache;
		Name theTarget;
		Array<char> theInputToken;
		Buffer theOutputToken;
		NetAddr theKdc;
};


// Error

inline
ostream &operator <<(ostream &os, const Error &err) {
	return err.print(os);
}


// Object

template <typename GssObject>
void Object<GssObject>::moveFrom(Object &other) {
	free();
	theObject = other.theObject;
	other.reset();
}


// Buffer

inline
ostream &operator <<(ostream &os, const Buffer &buf) {
	return buf.print(os);
}

}; // namespace Gss

#endif
