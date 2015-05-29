
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_ERRNO_H
#define POLYGRAPH__XSTD_H_ERRNO_H

#include <errno.h>

// define Unix-stile errors via WSA macros
#if defined(HAVE_WINSOCK2_H) 
#	ifndef _WINSOCK2API_
#		include <winsock2.h>
#	endif

#	ifndef EWOULDBLOCK
#	define EWOULDBLOCK             WSAEWOULDBLOCK
#	endif

#	ifndef EINPROGRESS
#	define EINPROGRESS             WSAEINPROGRESS        
#	endif

#	ifndef EALREADY
#	define EALREADY                WSAEALREADY                            
#	endif

#	ifndef ENOTSOCK
#	define ENOTSOCK                WSAENOTSOCK                            
#	endif

#	ifndef EDESTADDRREQ
#	define EDESTADDRREQ            WSAEDESTADDRREQ
#	endif

#	ifndef EMSGSIZE
#	define EMSGSIZE                WSAEMSGSIZE    
#	endif

#	ifndef EPROTOTYPE
#	define EPROTOTYPE              WSAEPROTOTYPE             
#	endif

#	ifndef ENOPROTOOPT
#	define ENOPROTOOPT             WSAENOPROTOOPT  
#	endif

#	ifndef EPROTONOSUPPORT
#	define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#	endif

#	ifndef ESOCKTNOSUPPORT
#	define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT           
#	endif

#	ifndef EOPNOTSUPP
#	define EOPNOTSUPP              WSAEOPNOTSUPP   
#	endif

#	ifndef EPFNOSUPPORT
#	define EPFNOSUPPORT            WSAEPFNOSUPPORT     
#	endif

#	ifndef EAFNOSUPPORT
#	define EAFNOSUPPORT            WSAEAFNOSUPPORT
#	endif

#	ifndef EADDRINUSE
#	define EADDRINUSE              WSAEADDRINUSE      
#	endif

#	ifndef EADDRNOTAVAIL
#	define EADDRNOTAVAIL           WSAEADDRNOTAVAIL                      
#	endif

#	ifndef ENETDOWN
#	define ENETDOWN                WSAENETDOWN        
#	endif

#	ifndef ENETUNREACH
#	define ENETUNREACH             WSAENETUNREACH  
#	endif

#	ifndef ENETRESET
#	define ENETRESET               WSAENETRESET                          
#	endif

#	ifndef ECONNABORTED
#	define ECONNABORTED            WSAECONNABORTED
#	endif

#	ifndef ECONNRESET
#	define ECONNRESET              WSAECONNRESET      
#	endif

#	ifndef ENOBUFS
#	define ENOBUFS                 WSAENOBUFS                             
#	endif

#	ifndef EISCONN
#	define EISCONN                 WSAEISCONN                           
#	endif

#	ifndef ENOTCONN
#	define ENOTCONN                WSAENOTCONN        
#	endif

#	ifndef ESHUTDOWN
#	define ESHUTDOWN               WSAESHUTDOWN                           
#	endif

#	ifndef ETOOMANYREFS
#	define ETOOMANYREFS            WSAETOOMANYREFS                      
#	endif

#	ifndef ETIMEDOUT
#	define ETIMEDOUT               WSAETIMEDOUT       
#	endif

#	ifndef ECONNREFUSED
#	define ECONNREFUSED            WSAECONNREFUSED                        
#	endif

#	ifndef ELOOP
#	define ELOOP                   WSAELOOP                               
#	endif

#	ifndef ENAMETOOLONG
#	define ENAMETOOLONG            WSAENAMETOOLONG
#	endif

#	ifndef EHOSTDOWN
#	define EHOSTDOWN               WSAEHOSTDOWN       
#	endif

#	ifndef EHOSTUNREACH
#	define EHOSTUNREACH            WSAEHOSTUNREACH                
#	endif

#	ifndef ENOTEMPTY
#	define ENOTEMPTY               WSAENOTEMPTY
#	endif

#	ifndef EPROCLIM
#	define EPROCLIM                WSAEPROCLIM              
#	endif

#	ifndef EUSERS
#	define EUSERS                  WSAEUSERS   
#	endif

#	ifndef EDQUOT
#	define EDQUOT                  WSAEDQUOT          
#	endif

#	ifndef ESTALE
#	define ESTALE                  WSAESTALE
#	endif

#	ifndef EREMOTE
#	define EREMOTE                 WSAEREMOTE
#	endif

#endif                                                               


#endif
