
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef SPNEGOCODEC_H_
#define SPNEGOCODEC_H_

enum spnego_auth_result {
    ACCEPT_COMPLETED = 0,
    ACCEPT_INCOMPLETE = 1,
    REJECTED = 2
};

// initialized in implementation file
extern const char * const GSSOID_KERBEROS_MECHANISM; // = "1.2.840.113554.1.2.2";
extern const char * const GSSOID_MS_KERBEROS_MECHANISM; // = "1.2.840.48018.1.2.2";
extern const char * const GSSOID_NTLMSSP_MECHANISM; // = "1.3.6.1.4.1.311.2.2.10";
extern const char * const GSSOID_SPNEGO_MECHANISM; // = "1.3.6.1.5.5.2";

// return true of spnego packet is InitToken packet and
// is parsed correctly. isMechanismPresent indicates whether
// support for the specified mechanism is indicated by this packet.
extern bool spnegoCheckInitMsg (
        const char *spnegoPacket, // already decoded with base64 if necessary
        const unsigned int size,
        bool & isMechanismPresent,
        const char * mechanism);

// return true of spnego packet is InitToken packet and
// is parsed correctly. isMechanismPresent indicates whether
// support for the NTLMSSP mechanism is indicated by this packet.
extern bool spnegoCheckNtlmInitMsg (
        const char *spnegoPacket, // already decoded with base64 if necessary
        const unsigned int size,
        bool & isMechanismPresent);

// return value is the size of output buffer; zero == error
extern unsigned int spnegoUnwrapMaterial(
        const char * spnegoPacket,
        const unsigned int size,
        const char * mechanism, // just to verify
        char * output, // where to put extracted auth blob
        unsigned int max_output, // max size of buffer
        spnego_auth_result & rc // auth result
    );

extern unsigned int spnegoUnwrapNtlmMaterial(
    const char * spnegoPacket,
    const unsigned int size,
    char * output, // where to put extracted ntlm blob
    unsigned int max_output, // max size of buffer
    spnego_auth_result & rc // auth result
);

// return value is the size of output buffer; zero == error
extern unsigned int spnegoWrapMaterial(
        const char *material, // auth material
        const unsigned int size,
        const char *mechanism, // one of auth mechanisms known to the lib
        char * output, // where to put the result (not yet base64 encoded)
        unsigned int max_output,
        spnego_auth_result rc = ACCEPT_INCOMPLETE // auth result
    );

extern unsigned int spnegoWrapNtlmMaterial(
    const char *material, // auth material
    const unsigned int size,
    char * output, // where to put the result (not yet base64 encoded)
    unsigned int max_output,
    spnego_auth_result rc = ACCEPT_INCOMPLETE // auth result
);

#endif /*SPNEGOCODEC_H_*/
