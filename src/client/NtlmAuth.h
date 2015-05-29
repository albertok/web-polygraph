#ifndef __HTTP_NTLM_AUTH_H
#define __HTTP_NTLM_AUTH_H

// create last part of the NTLM header value
extern void NtlmAuthPrintT1(ostream &os, bool useNTLM2 = true);

// create last part of the NTLM header value based on header from T2
extern bool NtlmAuthPrintT3(ostream &os,
	const char *headerT2, // base64-encoded T2 header
	const char *userp, // domain name included if available
	const char *passwdp);

// decode base64-encoded spnego packet and confirm that this is spnego
// init token with NTLM mechanism in the list of supported mechanisms
extern bool isSpnegoNtlm(const char * header);

// create last part of the Negotiate header value
extern void NegoNtlmAuthPrintT1(ostream &os, bool useSpnego, bool useNTLM2 = true);


// create last part of the Negotiate header value
extern bool NegoNtlmAuthPrintT3(ostream &os,
	const char *headerT2, // T2 header, base64-encoded NTLMSSP or NTLMSSP inside spnego
	const char *userp, // domain name included if available
	const char *passwdp,
	bool useSpnego);

#endif
