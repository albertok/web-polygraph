
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_KERBEROS_H
#define POLYGRAPH__XSTD_KERBEROS_H

#include "xstd/h/iosfwd.h"
#include "xstd/h/string.h"
#include "xstd/String.h"
#include "xstd/Time.h"

#if KERBEROS_ENABLED

#if HAVE_KRB5_H
#if HAVE_BROKEN_KRB5_H
extern "C" {
#include <krb5.h>
}
#else
#include <krb5.h>
#endif
#endif

#endif

namespace Kerberos {

#if !KERBEROS_ENABLED

typedef int krb5_error_code;

struct krb5_data {
	unsigned int length;
	char * data;
};

typedef void *krb5_ccache;
typedef void *krb5_context;
typedef void *krb5_creds;
typedef void *krb5_init_creds_context;
typedef void *krb5_principal;
typedef void *krb5_tkt_creds_context;

#define KRB5KRB_ERR_RESPONSE_TOO_BIG 1

#endif

extern const bool Supported;

// Kerberos

typedef krb5_error_code ErrorCode;

class InitCredsContext;
class TktCredsContext;

class Context {
	public:
		Context();
		Context(const Context &context);
		~Context() { free(); }

		Context &operator =(const Context &context);

		ErrorCode init();
		void free() { abandon(); }

		operator krb5_context() const;

	private:
		struct Data {
			Data();
			~Data();

			krb5_context krb5Context;
			unsigned useCount;
		};

		void use(Data *const aData);
		void abandon();

		Data *theData;
};

template <typename Krb5Object>
class Object {
	public:
		Object();
		virtual ~Object() {}

		const Context &context() const { return theContext; }
		operator const Krb5Object &() const { return theObject; }

	protected:
		virtual void free() = 0;

		// not implemented
		Object(const Object &);
		Object &operator =(const Object &);

		Context theContext;
		Krb5Object theObject;
};

class Data: public krb5_data {
	public:
		Data() { reset(); }
		Data(char *const aData, const unsigned int aLength);

		void reset() { set(0, 0); }
		void set(char *const aData, const unsigned int aLength);

		bool empty() const { return !length(); }
		char *const data() const { return reinterpret_cast<char *>(krb5_data::data); }
		unsigned int length() const { return krb5_data::length; }
};

/* Kerberos::String uses krb5_free_string() introduced in MIT Kerberos v1.10 */
#if KERBEROS_STRING_IS_NEEDED
class String: public Object<char *> {
	public:
		virtual ~String() { free(); }

		void init(const Context &aContext, char *const val);
		virtual void free();
};
#endif /* KERBEROS_STRING_IS_NEEDED */

class Error: public Object<const char *> {
	public:
		Error(const Context &aContext, const ErrorCode aCode);
		virtual ~Error() { free(); }

		ErrorCode code() const { return theCode; }
		const char *message() const { return theObject; }

		ostream &print(ostream &os) const;

	private:
		virtual void free();

		const ErrorCode theCode;
};

class Principal: public Object<krb5_principal> {
	public:
		virtual ~Principal() { free(); }

		ErrorCode init(const Context &aContext, const ::String &name);
		virtual void free();
};

class Creds: public Object<krb5_creds> {
	public:
		virtual ~Creds() { free(); }

		ErrorCode init(const InitCredsContext &initCredsContext);
		ErrorCode init(const TktCredsContext &tktCredsContext);
		ErrorCode init(const Principal &client, const Principal &server);
		virtual void free();

		krb5_creds &krb5Creds() { return theObject; }
};

class CCache: public Object<krb5_ccache> {
	public:
		virtual ~CCache() { free(); }

		ErrorCode init(const Context &aContext, const ::String &path = "");
		virtual void free();

		ErrorCode initialize(const Principal &principal);
		::String name() const;
		ErrorCode store(Creds &creds);

	private:
		static const ::String TheCcTypeMemory;
};

class DefaultCCache {
	public:
		DefaultCCache(const CCache &ccache);
		~DefaultCCache();

	protected:
		static void Set(const ::String &id);
};

class InitCredsContext: public Object<krb5_init_creds_context> {
	public:
		virtual ~InitCredsContext() { free(); }

		ErrorCode init(const Principal &client);
		ErrorCode password(const ::String &pass);
		ErrorCode step(Data &in, Data &out);
		Time endTime() const;
		virtual void free();

};

class TktCredsContext: public Object<krb5_tkt_creds_context> {
	public:
		virtual ~TktCredsContext() { free(); }

		ErrorCode init(const Principal &client, const Principal &server, const CCache &ccache);
		ErrorCode step(Data &in, Data &out);
		virtual void free();
};


// Object

template <typename Krb5Object>
Object<Krb5Object>::Object() {
	memset(&theObject, 0, sizeof(theObject));
}

template <typename Krb5Object>
void Object<Krb5Object>::free() {
	memset(&theObject, 0, sizeof(theObject));
	theContext.free();
}


// Error

inline
ostream &operator <<(ostream &os, const Error &err) {
	return err.print(os);
}

}; // namespace Kerberos

#endif
