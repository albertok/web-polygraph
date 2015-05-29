/***************************************************************************
 * This software is licensed as described in the file NOTICE, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * Derived from: 'libcurl/http_ntlm.c,v 1.55 2006-10-17 21:32:56 bagder'
 ***************************************************************************/

/* NTLM details:

   http://davenport.sourceforge.net/ntlm.html
   http://www.innovation.ch/java/ntlm.html

   Another implementation:
   http://lxr.mozilla.org/mozilla/source/security/manager/ssl/src/nsNTLMAuthModule.cpp

*/

#include "base/polygraph.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "client/NtlmAuth.h"
#include "client/SpnegoCodec.h"

#include "xstd/Assert.h"
#include "xstd/gadgets.h"
#include "xstd/Rnd.h"
#include "xstd/String.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#if OPENSSL_ENABLED

#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>

#if OPENSSL_VERSION_NUMBER < 0x00907001L
#define DES_key_schedule des_key_schedule
#define DES_cblock des_cblock
#define DES_set_odd_parity des_set_odd_parity
#define DES_set_key des_set_key
#define DES_ecb_encrypt des_ecb_encrypt

/* This is how things were done in the old days */
#define DESKEY(x) x
#define DESKEYARG(x) x
#else
/* Modern version */
#define DESKEYARG(x) *x
#define DESKEY(x) &x
#endif


/* Define this to make the type-3 message include the NT response message */
#define USE_NTRESPONSES 1

/* Define this to make the type-3 message include the NTLM2Session response
   message, requires USE_NTRESPONSES. */
#define USE_NTLM2SESSION 1

/* Flag bits definitions based on http://davenport.sourceforge.net/ntlm.html */

#define NTLMFLAG_NEGOTIATE_UNICODE               (1<<0)
/* Indicates that Unicode strings are supported for use in security buffer
   data. */

#define NTLMFLAG_NEGOTIATE_OEM                   (1<<1)
/* Indicates that OEM strings are supported for use in security buffer data. */

#define NTLMFLAG_REQUEST_TARGET                  (1<<2)
/* Requests that the server's authentication realm be included in the Type 2
   message. */

/* unknown (1<<3) */
#define NTLMFLAG_NEGOTIATE_SIGN                  (1<<4)
/* Specifies that authenticated communication between the client and server
   should carry a digital signature (message integrity). */

#define NTLMFLAG_NEGOTIATE_SEAL                  (1<<5)
/* Specifies that authenticated communication between the client and server
   should be encrypted (message confidentiality). */

#define NTLMFLAG_NEGOTIATE_DATAGRAM_STYLE        (1<<6)
/* unknown purpose */

#define NTLMFLAG_NEGOTIATE_LM_KEY                (1<<7)
/* Indicates that the LAN Manager session key should be used for signing and
   sealing authenticated communications. */

#define NTLMFLAG_NEGOTIATE_NETWARE               (1<<8)
/* unknown purpose */

#define NTLMFLAG_NEGOTIATE_NTLM_KEY              (1<<9)
/* Indicates that NTLM authentication is being used. */

/* unknown (1<<10) */
/* unknown (1<<11) */

#define NTLMFLAG_NEGOTIATE_DOMAIN_SUPPLIED       (1<<12)
/* Sent by the client in the Type 1 message to indicate that a desired
   authentication realm is included in the message. */

#define NTLMFLAG_NEGOTIATE_WORKSTATION_SUPPLIED  (1<<13)
/* Sent by the client in the Type 1 message to indicate that the client
   workstation's name is included in the message. */

#define NTLMFLAG_NEGOTIATE_LOCAL_CALL            (1<<14)
/* Sent by the server to indicate that the server and client are on the same
   machine. Implies that the client may use a pre-established local security
   context rather than responding to the challenge. */

#define NTLMFLAG_NEGOTIATE_ALWAYS_SIGN           (1<<15)
/* Indicates that authenticated communication between the client and server
   should be signed with a "dummy" signature. */

#define NTLMFLAG_TARGET_TYPE_DOMAIN              (1<<16)
/* Sent by the server in the Type 2 message to indicate that the target
   authentication realm is a domain. */

#define NTLMFLAG_TARGET_TYPE_SERVER              (1<<17)
/* Sent by the server in the Type 2 message to indicate that the target
   authentication realm is a server. */

#define NTLMFLAG_TARGET_TYPE_SHARE               (1<<18)
/* Sent by the server in the Type 2 message to indicate that the target
   authentication realm is a share. Presumably, this is for share-level
   authentication. Usage is unclear. */

#define NTLMFLAG_NEGOTIATE_NTLM2_KEY             (1<<19)
/* Indicates that the NTLM2 signing and sealing scheme should be used for
   protecting authenticated communications. */

#define NTLMFLAG_REQUEST_INIT_RESPONSE           (1<<20)
/* unknown purpose */

#define NTLMFLAG_REQUEST_ACCEPT_RESPONSE         (1<<21)
/* unknown purpose */

#define NTLMFLAG_REQUEST_NONNT_SESSION_KEY       (1<<22)
/* unknown purpose */

#define NTLMFLAG_NEGOTIATE_TARGET_INFO           (1<<23)
/* Sent by the server in the Type 2 message to indicate that it is including a
   Target Information block in the message. */

/* unknown (1<24) */
/* unknown (1<25) */
/* unknown (1<26) */
/* unknown (1<27) */
/* unknown (1<28) */

#define NTLMFLAG_NEGOTIATE_128                   (1<<29)
/* Indicates that 128-bit encryption is supported. */

#define NTLMFLAG_NEGOTIATE_KEY_EXCHANGE          (1<<30)
/* unknown purpose */

#define NTLMFLAG_NEGOTIATE_56                    (1<<31)
/* Indicates that 56-bit encryption is supported. */

/* this function converts from the little endian format used in the incoming
   package to whatever endian format we're using natively */
static unsigned int readint_le(unsigned char *buf) /* must point to a
                                                      4 bytes buffer*/
{
  return ((unsigned int)buf[0]) | ((unsigned int)buf[1] << 8) |
    ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
}


/*
  (*) = A "security buffer" is a triplet consisting of two shorts and one
  long:

  1. a 'short' containing the length of the buffer in bytes
  2. a 'short' containing the allocated space for the buffer in bytes
  3. a 'long' containing the offset to the start of the buffer from the
     beginning of the NTLM message, in bytes.
*/

static bool ParseT2(
		unsigned int & flags,
		unsigned char * challenge,
		/* NTLM T2, base64 decoded if necesary*/
		const char *header, 
		/* size of NTLM data */
		const unsigned int size) 
{
  /* point to the correct struct with this */
  static const char type2_marker[] = { 0x02, 0x00, 0x00, 0x00 };

  flags = 0;

      /* We got a type-2 message here:

         Index   Description         Content
         0       NTLMSSP Signature   Null-terminated ASCII "NTLMSSP"
                                     (0x4e544c4d53535000)
         8       NTLM Message Type   long (0x02000000)
         12      Target Name         security buffer(*)
         20      Flags               long
         24      Challenge           8 bytes
         (32)    Context (optional)  8 bytes (two consecutive longs)
         (40)    Target Information  (optional) security buffer(*)
         32 (48) start of data block
      */

      if((size < 32) ||
         (memcmp(header, "NTLMSSP", 8) != 0) ||
         (memcmp(header+8, type2_marker, sizeof(type2_marker)) != 0)) {
        /* This was not a good enough type-2 message */
        return false;
      }

      flags = readint_le((unsigned char*)header+20);
      memcpy(challenge, header+24, 8);
      return true;
}

/*
 * Turns a 56 bit key into the 64 bit, odd parity key and sets the key.  The
 * key schedule ks is also set.
 */
static void setup_des_key(unsigned char *key_56,
                          DES_key_schedule DESKEYARG(ks))
{
  DES_cblock key;

  key[0] = key_56[0];
  key[1] = (unsigned char)(((key_56[0] << 7) & 0xFF) | (key_56[1] >> 1));
  key[2] = (unsigned char)(((key_56[1] << 6) & 0xFF) | (key_56[2] >> 2));
  key[3] = (unsigned char)(((key_56[2] << 5) & 0xFF) | (key_56[3] >> 3));
  key[4] = (unsigned char)(((key_56[3] << 4) & 0xFF) | (key_56[4] >> 4));
  key[5] = (unsigned char)(((key_56[4] << 3) & 0xFF) | (key_56[5] >> 5));
  key[6] = (unsigned char)(((key_56[5] << 2) & 0xFF) | (key_56[6] >> 6));
  key[7] = (unsigned char) ((key_56[6] << 1) & 0xFF);

  DES_set_odd_parity(&key);
  DES_set_key(&key, ks);
}

 /*
  * takes a 21 byte array and treats it as 3 56-bit DES keys. The
  * 8 byte plaintext is encrypted with each key and the resulting 24
  * bytes are stored in the results array.
  */
static void lm_resp(unsigned char *keys,
                    const unsigned char *plaintext,
                      unsigned char *results)
{
  DES_key_schedule ks;

  setup_des_key(keys, DESKEY(ks));
  DES_ecb_encrypt((DES_cblock*) plaintext, (DES_cblock*) results,
                  DESKEY(ks), DES_ENCRYPT);

  setup_des_key(keys+7, DESKEY(ks));
  DES_ecb_encrypt((DES_cblock*) plaintext, (DES_cblock*) (results+8),
                  DESKEY(ks), DES_ENCRYPT);

  setup_des_key(keys+14, DESKEY(ks));
  DES_ecb_encrypt((DES_cblock*) plaintext, (DES_cblock*) (results+16),
                  DESKEY(ks), DES_ENCRYPT);
}


/*
 * Set up lanmanager hashed password
 */
static void mk_lm_hash(const char * password, unsigned char *lmbuffer /* 21 bytes */)
{
  unsigned char pw[14];
  static const unsigned char magic[] = {
    0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25 /* i.e. KGS!@#$% */
  };
  unsigned int i;
  size_t len = strlen(password);

  if (len > 14)
    len = 14;

  for (i=0; i<len; i++)
    pw[i] = (unsigned char)toupper(password[i]);

  for (; i<14; i++)
    pw[i] = 0;

  {
    /* Create LanManager hashed password. */

    DES_key_schedule ks;

    setup_des_key(pw, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock *)magic, (DES_cblock *)lmbuffer,
                    DESKEY(ks), DES_ENCRYPT);

    setup_des_key(pw+7, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock *)magic, (DES_cblock *)(lmbuffer+8),
                    DESKEY(ks), DES_ENCRYPT);

    memset(lmbuffer + 16, 0, 21 - 16);
  }
}

static void utf8_to_unicode_le(unsigned char *dest, const char *src,
                               size_t srclen)
{
  size_t i;
  for (i=0; i<srclen; i++) {
    dest[2*i]   = (unsigned char)src[i];
    dest[2*i+1] =   '\0';
  }
}

/*
 * Set up nt hashed passwords
 */
static void mk_nt_hash(const char* password, unsigned char *ntbuffer /* 21 bytes */)
{
  size_t len = strlen(password);
  unsigned char *pw = (unsigned char*)malloc(len*2);

  utf8_to_unicode_le(pw, password, len);

  {
    /* Create NT hashed password. */
    MD4_CTX MD4;

    MD4_Init(&MD4);
    MD4_Update(&MD4, pw, 2*len);
    MD4_Final(ntbuffer, &MD4);

    memset(ntbuffer + 16, 0, 21 - 16);
  }

  free(pw);
}

#define SHORTPAIR(x) ((x) & 0xff), (((x) >> 8) & 0xff)
#define LONGQUARTET(x) ((x) & 0xff), (((x) >> 8)&0xff), \
  (((x) >>16)&0xff), (((x)>>24) & 0xff)

// 1024 chars minus one char for the zero at the end
#define HOSTNAME_MAX 1024-1

void NtlmAuthPrintT1(ostream & os, bool useNTLM2Session) {
	NegoNtlmAuthPrintT1(os, /* useSpnego */ false, useNTLM2Session);
}

void NegoNtlmAuthPrintT1(ostream & os, bool useSpnego, bool useNTLM2Session)
{
  unsigned char ntlmbuf[1024]; /* enough, unless the user+host+domain is very
                                  long */

  /* Create and send a type-1 message:

  Index Description          Content
  0     NTLMSSP Signature    Null-terminated ASCII "NTLMSSP"
                             (0x4e544c4d53535000)
  8     NTLM Message Type    long (0x01000000)
  12    Flags                long
  16    Supplied Domain      security buffer(*)
  24    Supplied Workstation security buffer(*)
  32    start of data block

  */

  snprintf((char *)ntlmbuf, sizeof(ntlmbuf), "NTLMSSP%c"
           "\x01%c%c%c" /* 32-bit type = 1 */
           "%c%c%c%c"   /* 32-bit NTLM flag field */
           "%c%c"  /* domain length */
           "%c%c"  /* domain allocated space */
           "%c%c"  /* domain name offset */
           "%c%c"  /* 2 zeroes */
           "%c%c"  /* host length */
           "%c%c"  /* host allocated space */
           "%c%c"  /* host name offset */
           "%c%c"  /* 2 zeroes */
           "%s"   /* host name */
           "%s",  /* domain string */
           0,     /* trailing zero */
           0,0,0, /* part of type-1 long */

           LONGQUARTET(
             NTLMFLAG_NEGOTIATE_OEM|
             NTLMFLAG_REQUEST_TARGET|
             NTLMFLAG_NEGOTIATE_NTLM_KEY|
             (useNTLM2Session ? NTLMFLAG_NEGOTIATE_NTLM2_KEY : 0) |
             NTLMFLAG_NEGOTIATE_ALWAYS_SIGN
             ),
           //remember that host and domain are empty
           SHORTPAIR(0 /*domlen*/),
           SHORTPAIR(0 /*domlen*/),
           SHORTPAIR(0 /*domoff*/),
           0,0,
           SHORTPAIR(0 /*hostlen*/),
           SHORTPAIR(0 /*hostlen*/),
           SHORTPAIR(0 /*hostoff*/),
           0,0,
           "" /* host is empty */, "" /* domain is empty */);

  if (useSpnego)
  {
      char buff[1024];
      const unsigned int buff_size = sizeof(buff)/sizeof(buff[0]);
      int spnego_size;
      spnego_size = spnegoWrapNtlmMaterial((const char*)ntlmbuf, 32, buff, buff_size, ACCEPT_INCOMPLETE);
      if ( spnego_size == 0)
      {
          return;
      }
        /* convert the binary blob into base64 */
        PrintBase64(os, (const char*) buff, spnego_size);
  } else {
      /* packet length is 32 + hostlen + domlen == 32*/
      PrintBase64(os, (const char *)ntlmbuf, 32);
  }
}

/* this is for creating ntlm header output for plain NTLM. */
/* it is the same as for Negotiate/NTLMSSP w/o spnego */
bool NtlmAuthPrintT3(ostream & os,
		    const char * headerT2,
                    const char* userp,
                    const char * passwdp)
{
    return NegoNtlmAuthPrintT3(os, headerT2, userp, passwdp, /* useSpnego */ false);
}

/* this is for creating ntlm header output */
bool NegoNtlmAuthPrintT3(ostream & os,
		    const char * headerT2,
                    const char* userp,
                    const char * passwdp,
		    bool useSpnego)
{
    unsigned int flags = 0;
    unsigned char challenge[8];

    const char *domain=""; /* empty */
    char host [HOSTNAME_MAX+ 1] = ""; /* empty */
    unsigned int domlen = strlen(domain);
    unsigned int hostlen = strlen(host);
    unsigned int hostoff; /* host name offset */
    unsigned int domoff;  /* domain name offset */
    size_t size;
    unsigned char ntlmbuf[1024]; /* enough, unless the user+host+domain is very
                                  long */

    const char * header = headerT2;
    /* skip initial whitespaces */
    while(*header && isspace((unsigned char)*header))
      header++;

    if (!*header)
      return false;

    if (useSpnego)
    {
        char base64buff[1024];
        size = DecodeBase64(header, strlen(header), (char*)base64buff, sizeof(base64buff)/sizeof(base64buff[0]));
        spnego_auth_result rc;
        if (!spnegoUnwrapNtlmMaterial(base64buff, size, (char*)ntlmbuf, sizeof(ntlmbuf)/sizeof(ntlmbuf[0]), rc))
	{
	    return false;
	}
	if (rc != ACCEPT_INCOMPLETE)
	{
	    return false;
	}
    } else {
        size = DecodeBase64(header, strlen(header), (char*)ntlmbuf, sizeof(ntlmbuf)/sizeof(ntlmbuf[0]));
    }

    // Parse T2 message
    if ( ! ParseT2(flags, challenge, (const char*)ntlmbuf, size)) {
	return false;
    }

    /* not set means empty */
    if(!userp)
	userp="";

    if(!passwdp)
	passwdp="";

    /* We received the type-2 message already, create a type-3 message:

    Index   Description            Content
    0       NTLMSSP Signature      Null-terminated ASCII "NTLMSSP"
                                   (0x4e544c4d53535000)
    8       NTLM Message Type      long (0x03000000)
    12      LM/LMv2 Response       security buffer(*)
    20      NTLM/NTLMv2 Response   security buffer(*)
    28      Domain Name            security buffer(*)
    36      User Name              security buffer(*)
    44      Workstation Name       security buffer(*)
    (52)    Session Key (optional) security buffer(*)
    (60)    Flags (optional)       long
    52 (64) start of data block

    */

    int lmrespoff;
    unsigned char lmresp[24]; /* fixed-size */
#if USE_NTRESPONSES
    int ntrespoff;
    unsigned char ntresp[24]; /* fixed-size */
#endif
    unsigned int useroff;
    const char *user;
    unsigned int userlen;

    user = strchr(userp, '\\');
    if(!user)
      user = strchr(userp, '/');

    if (user) {
      domain = userp;
      domlen = (user - domain);
      user++;
    }
    else
      user = userp;
    userlen = strlen(user);

    hostlen = 0;
#ifdef GET_NTLM_HOSTNAME_FROM_GETHOSTNAME
#ifdef HAVE_UNISTD_H
    if (! gethostname(host, HOSTNAME_MAX)) {
#else
	if (0) {
#endif
#else
    if (const char *at = strrchr(user, '@')) {
      strncpy(host, at+1, sizeof(host));
	  host[sizeof(host)-1] = '\0';
      userlen = at - user;
#endif
      /* If the workstation is configured with a full DNS name (i.e.
       * workstation.somewhere.net) gethostname() returns the fully qualified
       * name, which NTLM doesn't like.
       */
      char *dot = strchr(host, '.');
      if (dot)
        *dot = '\0';
      hostlen = strlen(host);
    }


#if USE_NTLM2SESSION
    /* We don't support NTLM2 if we don't have USE_NTRESPONSES */
    if (flags & NTLMFLAG_NEGOTIATE_NTLM2_KEY) {
      unsigned char ntbuffer[0x18];
      unsigned char tmp[0x18];
      unsigned char md5sum[MD5_DIGEST_LENGTH];
      MD5_CTX MD5;
      unsigned char random[8];

      /* Need to create 8 bytes random data */
      RndGen rnd;
      random[0] = (unsigned char) rnd(0,256);
      random[1] = (unsigned char) rnd(0,256);
      random[2] = (unsigned char) rnd(0,256);
      random[3] = (unsigned char) rnd(0,256);
      random[4] = (unsigned char) rnd(0,256);
      random[5] = (unsigned char) rnd(0,256);
      random[6] = (unsigned char) rnd(0,256);
      random[7] = (unsigned char) rnd(0,256);

      /* 8 bytes random data as challenge in lmresp */
      memcpy(lmresp,random,8);
      /* Pad with zeros */
      memset(lmresp+8,0,0x10);

      /* Fill tmp with challenge + random */
      memcpy(tmp,challenge,8);
      memcpy(tmp+8,random,8);

      MD5_Init(&MD5);
      MD5_Update(&MD5, tmp, 16);
      MD5_Final(md5sum, &MD5);
      /* We shall only use the first 8 bytes of md5sum,
         but the des code in lm_resp only encrypt the first 8 bytes */
      mk_nt_hash(passwdp, ntbuffer);
      lm_resp(ntbuffer, md5sum, ntresp);

      /* End of NTLM2 Session code */
    }
    else
#endif
    {

#if USE_NTRESPONSES
      unsigned char ntbuffer[0x18];
#endif
      unsigned char lmbuffer[0x18];

#if USE_NTRESPONSES
      mk_nt_hash(passwdp, ntbuffer);
      lm_resp(ntbuffer, challenge, ntresp);
#endif

      mk_lm_hash(passwdp, lmbuffer);
      lm_resp(lmbuffer, challenge, lmresp);
      /* A safer but less compatible alternative is:
       *   lm_resp(ntbuffer, &ntlm->nonce[0], lmresp);
       * See http://davenport.sourceforge.net/ntlm.html#ntlmVersion2 */
    }

    lmrespoff = 64; /* size of the message header */
#if USE_NTRESPONSES
    ntrespoff = lmrespoff + 0x18;
    domoff = ntrespoff + 0x18;
#else
    domoff = lmrespoff + 0x18;
#endif
    useroff = domoff + domlen;
    hostoff = useroff + userlen;

    /* Create the big type-3 message binary blob */
    size = snprintf((char *)ntlmbuf, sizeof(ntlmbuf),
                    "NTLMSSP%c"
                    "\x03%c%c%c" /* type-3, 32 bits */

                    "%c%c" /* LanManager length */
                    "%c%c" /* LanManager allocated space */
                    "%c%c" /* LanManager offset */
                    "%c%c" /* 2 zeroes */

                    "%c%c" /* NT-response length */
                    "%c%c" /* NT-response allocated space */
                    "%c%c" /* NT-response offset */
                    "%c%c" /* 2 zeroes */

                    "%c%c"  /* domain length */
                    "%c%c"  /* domain allocated space */
                    "%c%c"  /* domain name offset */
                    "%c%c"  /* 2 zeroes */

                    "%c%c"  /* user length */
                    "%c%c"  /* user allocated space */
                    "%c%c"  /* user offset */
                    "%c%c"  /* 2 zeroes */

                    "%c%c"  /* host length */
                    "%c%c"  /* host allocated space */
                    "%c%c"  /* host offset */
                    "%c%c"  /* 2 zeroes */

                    "%c%c"  /* session key length (unknown purpose) */
                    "%c%c"  /* session key allocated space (unknown purpose) */
                    "%c%c"  /* session key offset (unknown purpose) */
                    "%c%c"  /* 2 zeroes */

                    "%c%c%c%c" /* flags */

                    /* domain string */
                    /* user string */
                    /* host string */
                    /* LanManager response */
                    /* NT response */
                    ,
                    0, /* zero termination */
                    0,0,0, /* type-3 long, the 24 upper bits */

                    SHORTPAIR(0x18),  /* LanManager response length, twice */
                    SHORTPAIR(0x18),
                    SHORTPAIR(lmrespoff),
                    0x0, 0x0,

#if USE_NTRESPONSES
                    SHORTPAIR(0x18),  /* NT-response length, twice */
                    SHORTPAIR(0x18),
                    SHORTPAIR(ntrespoff),
                    0x0, 0x0,
#else
                    0x0, 0x0,
                    0x0, 0x0,
                    0x0, 0x0,
                    0x0, 0x0,
#endif
                    SHORTPAIR(domlen),
                    SHORTPAIR(domlen),
                    SHORTPAIR(domoff),
                    0x0, 0x0,

                    SHORTPAIR(userlen),
                    SHORTPAIR(userlen),
                    SHORTPAIR(useroff),
                    0x0, 0x0,

                    SHORTPAIR(hostlen),
                    SHORTPAIR(hostlen),
                    SHORTPAIR(hostoff),
                    0x0, 0x0,

                    0x0, 0x0,
                    0x0, 0x0,
                    0x0, 0x0,
                    0x0, 0x0,

                    LONGQUARTET(flags));

    /* We append the binary hashes */
    if(size < (sizeof(ntlmbuf) - 0x18)) {
      memcpy(&ntlmbuf[size], lmresp, 0x18);
      size += 0x18;
    }


#if USE_NTRESPONSES
    if(size < (sizeof(ntlmbuf) - 0x18)) {
      memcpy(&ntlmbuf[size], ntresp, 0x18);
      size += 0x18;
    }

#endif

    /* Make sure that the domain, user and host strings fit in the target
       buffer before we copy them there. */
    if(size + userlen + domlen + hostlen >= sizeof(ntlmbuf)) {
//      failf(conn->data, "user + domain + host name too big");
      return false;
    }

    Assert(size == domoff);
    memcpy(&ntlmbuf[size], domain, domlen);
    size += domlen;

    Assert(size == useroff);
    memcpy(&ntlmbuf[size], user, userlen);
    size += userlen;

    Assert(size == hostoff);
    memcpy(&ntlmbuf[size], host, hostlen);
    size += hostlen;

    if (useSpnego)
    {
        int spnego_size;
        char buff[1024];
        spnego_size = spnegoWrapNtlmMaterial((const char*)ntlmbuf, size, buff, sizeof(buff)/sizeof(buff[0]), ACCEPT_INCOMPLETE);
	if ( spnego_size == 0)
	{
	    return false;
	}
        /* convert the binary blob into base64 */
        PrintBase64(os, (const char*) buff, spnego_size);
    } else {
        /* convert the binary blob into base64 */
        PrintBase64(os, (const char*) ntlmbuf, size);
    }

//  the result of base64 encoding should be used like this:
//       *header = aprintf("%sAuthorization: %s %s\r\n",
//                               proxy?"Proxy-":"",
//                               (isNtlm ? "NTML" : "Negotiate"),
//                               base64encodedstr);

  return true;
}

bool isSpnegoNtlm(const char * header)
{
    const char * hdr = header;
    /* skip initial whitespaces */
    while(*hdr && isspace((unsigned char)*hdr))
      hdr++;

    if (!*hdr)
      return false;

    unsigned int size;
    char buf[1024];
    size = DecodeBase64(hdr, strlen(hdr), buf, sizeof(buf)/sizeof(buf[0]));
    bool isSupported = false;
    if (spnegoCheckNtlmInitMsg(buf, size, isSupported)) {
        return isSupported;
    } else {
        return false;
    }
}

#else /* OPENSSL_ENABLED */

void NtlmAuthPrintT1(ostream &os, bool useNTLM2) {
	Assert(false);
}

bool NtlmAuthPrintT3(ostream &os,
        const char *headerT2, // base64-encoded T2 header
        const char *userp, // domain name included if available
        const char *passwdp) {
	Assert(false);
	return false;
}

// decode base64-encoded spnego packet and confirm that this is spnego
// init token with NTLM mechanism in the list of supported mechanisms
bool isSpnegoNtlm(const char * header) {
	Assert(false);
	return false;
}

// create last part of the Negotiate header value
void NegoNtlmAuthPrintT1(ostream &os, bool useSpnego, bool useNTLM) {
	Assert(false);
}

// create last part of the Negotiate header value
bool NegoNtlmAuthPrintT3(ostream &os,
        const char *headerT2, // T2 header, base64-encoded NTLMSSP or NTLMSSP inside spnego
        const char *userp, // domain name included if available
        const char *passwdp,
        bool useSpnego) {
	Assert(false);
	return false;
}


#endif /* OPENSSL_ENABLED */
