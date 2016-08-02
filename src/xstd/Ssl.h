
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_SSL_H
#define POLYGRAPH__XSTD_SSL_H

#if OPENSSL_ENABLED
#include <openssl/ssl.h>
#else
	class SSL_CTX {};
	class SSL {};
	class SSL_SESSION {};
	typedef int pem_password_cb(char *buf, int size, int rwflag, void *data);

#	define SSL_ERROR_ZERO_RETURN 3
#	define SSL_ERROR_WANT_READ 4
#	define SSL_ERROR_WANT_WRITE 5
#	define SSL_ERROR_WANT_X509_LOOKUP 6
#	define SSL_ERROR_SYSCALL 7

#	define SSL_VERIFY_NONE 8
#	define SSL_VERIFY_PEER 9
#	define SSL_VERIFY_FAIL_IF_NO_PEER 10
#	define SSL_VERIFY_CLIENT_ONCE 11

#	define SSL_SESS_CACHE_OFF 12
#	define SSL_SESS_CACHE_CLIENT 13
#	define SSL_SESS_CACHE_SERVER 14
#	define SSL_SESS_CACHE_BOTH 15
#	define SSL_SESS_CACHE_NO_AUTO_CLEAR 16
#	define SSL_SESS_CACHE_NO_INTERNAL_LOOKUP 17

#endif

#include "xstd/Size.h"

class String;
class Ssl;
class SslSession;


// SSL_CTX wrapper; the context is used to create SSL connections
class SslCtx {
	public:
		enum SslProtocol { SSLv2 = 1, SSLv3, TLSv1, SSLv23 };

	public:
		SslCtx(SslProtocol protocol, const String &cipher);
		~SslCtx();

		static bool IsProtocolSupported(const int protocol);
		static bool IsCompressionConfigurable();

		bool useCertificateChainFile(const String &fname);
		bool usePrivateKeyFile(const String &fname);
		bool checkPrivateKey();
		void setDefaultPasswdCb(pem_password_cb *cb);
		bool loadVerifyLocations(const String &fname, const String &dirName);
		void setVerify(int mode) const;
		bool disableCompression();

		long sessionCacheMode(long mode);
		long sessionCacheSize(long count);
		bool sessionId(const String &id);

		Ssl *makeConnection() const; // never fails

	private:
		SslCtx(const SslCtx &anSslCtx); // disable copying
		SslCtx &operator =(const SslCtx &anSslCtx);

	private:
		SSL_CTX *theCtx;
};

// SSL wrapper; should have been called SslConnection
class Ssl {
	public:
		enum Role { rlClient, rlServer };

	public:
		static int Level(); // the number of connections in existence

	public:
		Ssl(const SSL_CTX *aCtx); // use SslCtx::makeConnection() instead
		~Ssl();

		bool setFd(int);
		bool setBIO(Ssl *ssl);
		void playRole(int role);
		void playClientRole();
		void playServerRole();
		bool enablePartialWrite();
		bool acceptMovingWriteBuffer();
		bool enableAutoRetry();
		bool resumeSession(SslSession *session);

		bool connect(int &err);
		bool accept(int &err);
		Size read(char *buf, Size size);
		Size write(const char *, Size);
		bool shutdown(int &err);

		bool dataPending() const;
		bool reusedSession() const;

		SslSession *refCountedSession() const;

		const char *getCipher();
		int getError(int);
		const char *getErrorString(int);

	protected:
		bool addMode(long mode);

	private:
		Ssl(const Ssl &anSsl); // not implemented
		Ssl &operator =(const Ssl &anSsl); // not implemented

	private:
		static int TheLevel;
		SSL *theConn;
};

// a wrapper wroung _refcounted_ session
class SslSession {
	public:
		SslSession(SSL_SESSION *aSession);
		~SslSession();

		SSL_SESSION *raw();

	private:
		SSL_SESSION *theSession;
};

// wrapper for openssl funcs
class SslMisc {
	public:
		static unsigned long ErrGetErrorLine(const char **fname, int *lineno);
		static const char *ErrErrorString(unsigned long e);
		static bool LibraryInit();
		static void SeedRng(double seed);
};

#endif
