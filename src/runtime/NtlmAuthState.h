
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_NTLM_AUTH_STATE_H
#define POLYGRAPH__RUNTIME_NTLM_AUTH_STATE_H

typedef enum { ntlmNone, // No NTML is used or not initiated
		ntlmSentT1, ntlmSentT3, ntlmDone, ntlmError } NtlmAuthState;

#endif
