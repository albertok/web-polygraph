
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#if OPENSSL_ENABLED
#include <openssl/err.h>
#include <openssl/rand.h>
#endif

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/Ssl.h"

int Ssl::TheLevel = 0;

// XXX: we should set the [SSL] error when SSL library is not found
// XXX: we should set the global error to an SSL error when an SSL call fails

/* SslCtx class */

SslCtx::SslCtx(SslProtocol protocol, const String &cipher): theCtx(0) {
#if OPENSSL_ENABLED
	Must(IsProtocolSupported(protocol));
	/* Pass *_method() result directly to SSL_CTX_new(3) instead
	 * of storing it in a variable and calling SSL_CTX_new(3)
	 * once. These functions use const types in OpenSSL 1.0 and
	 * non-const types in older OpenSSL versions and it breaks the
	 * build. */
	switch(protocol) {
		case SSLv2:
#ifndef OPENSSL_NO_SSL2
			theCtx = ::SSL_CTX_new(::SSLv2_method());
#endif
			break;
		case TLSv1:
			theCtx = ::SSL_CTX_new(::TLSv1_method());
			break;
		case SSLv3:
			theCtx = ::SSL_CTX_new(::SSLv3_method());
			break;
		case SSLv23:
			theCtx = ::SSL_CTX_new(::SSLv23_method());
			break;
		default:
			theCtx = ::SSL_CTX_new(::SSLv23_method());
			Should(false);
	}
	Must(theCtx);
	Must(::SSL_CTX_set_cipher_list(theCtx, cipher.cstr()));
#endif
}

SslCtx::~SslCtx() {
#if OPENSSL_ENABLED
	if (theCtx)
		::SSL_CTX_free(theCtx);
#endif
}

bool SslCtx::IsProtocolSupported(const int protocol) {
#if OPENSSL_ENABLED
	switch(protocol) {
		case SSLv2:
#ifdef OPENSSL_NO_SSL2
			return false;
#endif
		case TLSv1:
		case SSLv3:
		case SSLv23:
			break;
		default:
			return false;
	}
	return true;
#endif
	return false;
}

bool SslCtx::IsCompressionConfigurable() {
#if OPENSSL_ENABLED
#	ifdef SSL_OP_NO_COMPRESSION
		return true;
#	endif
#endif
	return false;
}

bool SslCtx::useCertificateChainFile(const String &fname) {
#if OPENSSL_ENABLED
	return ::SSL_CTX_use_certificate_chain_file(theCtx, fname.cstr()) > 0;
#endif
	return false;
}

bool SslCtx::usePrivateKeyFile(const String &fname) {
#if OPENSSL_ENABLED
	return ::SSL_CTX_use_PrivateKey_file(theCtx, fname.cstr(), SSL_FILETYPE_PEM) > 0;
#endif
	return false;
}

bool SslCtx::checkPrivateKey() {
#if OPENSSL_ENABLED
	return ::SSL_CTX_check_private_key(theCtx) > 0;
#endif
	return false;
}

void SslCtx::setDefaultPasswdCb(pem_password_cb *cb) {
#if OPENSSL_ENABLED
	::SSL_CTX_set_default_passwd_cb(theCtx, cb);
#endif
}

bool SslCtx::loadVerifyLocations(const String &fname, const String &dirName) {
#if OPENSSL_ENABLED
	const char *dname = dirName.len() ? dirName.cstr() : 0;
	return ::SSL_CTX_load_verify_locations(theCtx, fname.cstr(), dname) > 0;
#endif
	Assert(sizeof(fname) && sizeof(dirName));
	return false;
}

long SslCtx::sessionCacheMode(long mode) {
#if OPENSSL_ENABLED
	return SSL_CTX_set_session_cache_mode(theCtx, mode);
#endif
	Assert(sizeof(mode));
	return 0;
}

long SslCtx::sessionCacheSize(long count) {
#if OPENSSL_ENABLED
	return SSL_CTX_sess_set_cache_size(theCtx, count);
#endif
	Assert(sizeof(count));
	return 0;
}

bool SslCtx::sessionId(const String &id) {
#if OPENSSL_ENABLED
	return SSL_CTX_set_session_id_context(theCtx, (unsigned char*)id.data(), id.len()) > 0;
#endif
	Assert(sizeof(id));
	return false;
}

Ssl *SslCtx::makeConnection() const {
	return new Ssl(theCtx);
}

void SslCtx::setVerify(int mode) const {
#if OPENSSL_ENABLED
	return ::SSL_CTX_set_verify(theCtx, mode, 0);
#endif
}

bool SslCtx::disableCompression() {
#if OPENSSL_ENABLED
#	ifdef SSL_OP_NO_COMPRESSION
		return ::SSL_CTX_set_options(theCtx, SSL_OP_NO_COMPRESSION) & SSL_OP_NO_COMPRESSION;
#	endif
#endif
	return false;
}


/* Ssl Class */

int Ssl::Level() {
	return TheLevel;
}

Ssl::Ssl(const SSL_CTX *ctx): theConn(0) {
#if OPENSSL_ENABLED
	// cast to non-const context because OpenSSL does not use "const"
	theConn = ::SSL_new((SSL_CTX*)ctx);
	if (Should(theConn))
		TheLevel++;
#else
	Assert(sizeof(ctx));
#endif
}

Ssl::~Ssl() {
#if OPENSSL_ENABLED
	if (Should(theConn)) {
		::SSL_free(theConn);
		TheLevel--;
	}
#endif
}

bool Ssl::shutdown(int &res) {
#if OPENSSL_ENABLED
	res = ::SSL_shutdown(theConn);
	return res > 0;
#endif
	res = -1;
	return false;
}

bool Ssl::setFd(int fd) {
#if OPENSSL_ENABLED
	return ::SSL_set_fd(theConn, fd) > 0;
#endif
	return false && !sizeof(fd);
}

bool Ssl::setBIO(Ssl *ssl) {
#if OPENSSL_ENABLED
	BIO *const bio = BIO_new(BIO_f_ssl());
	if (!bio)
		return false;
	BIO_set_ssl(bio, ssl->theConn, false);
	SSL_set_bio(theConn, bio, bio);
	return true;
#endif
	return false && !sizeof(ssl);
}

void Ssl::playRole(int role) {
	switch (role) {
		case rlClient:
			playClientRole();
			break;
		case rlServer:
			playServerRole();
			break;
		default:
			Should(false);
	}
}

void Ssl::playClientRole() {
#if OPENSSL_ENABLED
	::SSL_set_connect_state(theConn);
#endif
}

void Ssl::playServerRole() {
#if OPENSSL_ENABLED
	::SSL_set_accept_state(theConn);
#endif
}

bool Ssl::enablePartialWrite() {
#if OPENSSL_ENABLED
#	ifdef SSL_MODE_ENABLE_PARTIAL_WRITE
		return addMode(SSL_MODE_ENABLE_PARTIAL_WRITE);
#	else
		return false; // unsupported mode
#	endif
#endif
	return false;
}

bool Ssl::enableAutoRetry() {
#if OPENSSL_ENABLED
#	ifdef SSL_MODE_AUTO_RETRY
		return addMode(SSL_MODE_AUTO_RETRY);
#	else
		return false; // unsupported mode
#	endif
#endif
	return false;
}

bool Ssl::acceptMovingWriteBuffer() {
#if OPENSSL_ENABLED
#	ifdef SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
		return addMode(SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
#	else
		return false; // unsupported mode
#	endif
#endif
	return false;
}

bool Ssl::resumeSession(SslSession *session) {
#if OPENSSL_ENABLED
	return ::SSL_set_session(theConn, session->raw()) > 0;
#endif
	return false;
}

bool Ssl::connect(int &res) {
#if OPENSSL_ENABLED
	res = ::SSL_connect(theConn);
	return res > 0;
#endif
	res = -1;
	return false;
}

bool Ssl::accept(int &res) {
#if OPENSSL_ENABLED
	res = ::SSL_accept(theConn);
	return res > 0;
#endif
	res = -1;
	return false;
}

Size Ssl::read(char *buf, Size sz) {
#if OPENSSL_ENABLED
	return ::SSL_read(theConn, buf, sz);
#endif
	Assert(sizeof(buf) && sizeof(sz));
	return -1;
}

Size Ssl::write(const char *buf, Size sz) {
#if OPENSSL_ENABLED
	return ::SSL_write(theConn, buf, sz);
#endif
	Assert(sizeof(buf) && sizeof(sz));
	return -1;
}

bool Ssl::addMode(long modeBit) {
#if OPENSSL_ENABLED
	const long newMode = SSL_set_mode(theConn, modeBit);
	return (newMode & modeBit) != 0;
#endif
	Assert(sizeof(modeBit));
	return false;
}

bool Ssl::dataPending() const {
#if OPENSSL_ENABLED
	return ::SSL_pending((SSL*)theConn);
#endif
	return false;
}

bool Ssl::reusedSession() const {
#if OPENSSL_ENABLED
	return ::SSL_session_reused((SSL*)theConn) > 0;
#endif
	return false;
}

SslSession *Ssl::refCountedSession() const {
#if OPENSSL_ENABLED
	SSL_SESSION *session = SSL_get1_session((SSL*)theConn);
	return session ? new SslSession(session) : 0;
#endif
	return 0;
}

const char *Ssl::getCipher() {
#if OPENSSL_ENABLED
	return ::SSL_get_cipher(theConn);
#else
	return "You don't have libssl";
#endif
}

int Ssl::getError(int e) {
#if OPENSSL_ENABLED
	return ::SSL_get_error(theConn, e);
#endif
	Assert(sizeof(e));
	return -1;
}

const char *Ssl::getErrorString(int e) {
#if OPENSSL_ENABLED
	switch(::SSL_get_error(theConn, e)) {
		case SSL_ERROR_NONE:
			return "SSL_ERROR_NONE";
		case SSL_ERROR_ZERO_RETURN:
			return "SSL_ERROR_ZERO_RETURN";
		case SSL_ERROR_WANT_READ:
			return "SSL_ERROR_WANT_READ";
		case SSL_ERROR_WANT_WRITE:
			return "SSL_ERROR_WANT_WRITE";
		case SSL_ERROR_WANT_X509_LOOKUP:
			return "SSL_ERROR_WANT_X509_LOOKUP";
		case SSL_ERROR_SYSCALL:
			return "SSL_ERROR_SYSCALL";
		case SSL_ERROR_SSL:
			return "SSL_ERROR_SSL";
		default:
			return "UNKNOWN";
	}
#endif
	return "NO_LIBSSL";
}


/* SslSession */

SslSession::SslSession(SSL_SESSION *aSession): theSession(aSession) {
}

SslSession::~SslSession() {
#if OPENSSL_ENABLED
	if (theSession)
		SSL_SESSION_free(theSession);
#endif
}

SSL_SESSION *SslSession::raw() {
	return theSession;
}



/* SslMisc class : MISC functions, unlreated to SSL_CTX or SSL */

const char *SslMisc::ErrErrorString(unsigned long e) {
#if OPENSSL_ENABLED
	return ::ERR_error_string(e, NULL);
#else
	return "You do not have libssl";
#endif
}

unsigned long SslMisc::ErrGetErrorLine(const char **fname, int *line) {
#if OPENSSL_ENABLED
	return ::ERR_get_error_line(fname, line);
#else
	return 0;
#endif
}

bool SslMisc::LibraryInit() {
#if OPENSSL_ENABLED
	::SSL_load_error_strings(); // optional, uses extra memory, but faster?
	(void)::SSL_library_init();
	return true;
#endif
	return false;
}

void SslMisc::SeedRng(double seed) {
#if OPENSSL_ENABLED
#	if HAVE_RAND_STATUS
		if (RAND_status() > 0)
			return; // enough random data (e.g., library used /dev/urandom)
#	endif
	RAND_seed(&seed, sizeof(seed));
#	if HAVE_RAND_STATUS
		Must(RAND_status());
#	endif
#endif
}
