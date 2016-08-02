/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */

#ifndef POLYGRAPH__CONFIG_H
#define POLYGRAPH__CONFIG_H


/* here are some typedefs that configure script might be missing */
/* #undef rlim_t */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* host type from configure */
#define CONFIG_HOST_TYPE "x86_64-unknown-linux-gnu"

/* negative or zero if we failed to detect it */
#define DEFAULT_FD_SETSIZE 1024

/* htobe64 and be64toh */
#define HAVE_64BIT_BYTE_ORDER_CONVERSION 1

/* altzone global is supported (Solaris only?) */
/* #undef HAVE_ALTZONE */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if krb5.h is broken for C++. */
/* #undef HAVE_BROKEN_KRB5_H */

/* Define to 1 if ldns/ldns.h needs extern "C" wrapppers. */
/* #undef HAVE_BROKEN_LDNS_H */

/* Define to 1 if you have the `ceilf' function. */
#define HAVE_CEILF 1

/* Define to 1 if C ldns/ldns.h uses a "class" keyword */
/* #undef HAVE_CLASS_IN_LDNS */

/* Define to 1 if you have the `closesocket' function. */
/* #undef HAVE_CLOSESOCKET */

/* Support setting CPU affinity for workers */
#define HAVE_CPU_AFFINITY 1

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define if you have the dlopen/sum/error/close. */
#define HAVE_DLOPEN 1

/* Define to 1 if the system has the type `dn_pipe'. */
/* #undef HAVE_DN_PIPE */

/* Define to 1 if you have the <endian.h> header file. */
#define HAVE_ENDIAN_H 1

/* Define to 1 if you have the `epoll_create' function. */
#define HAVE_EPOLL_CREATE 1

/* must explicitly declare timezone global as extern */
/* #undef HAVE_EXTERN_TIMEZONE */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the `getifaddrs' function. */
#define HAVE_GETIFADDRS 1

/* Define to 1 if you have the `GetLastError' function. */
/* #undef HAVE_GETLASTERROR */

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getrlimit' function. */
#define HAVE_GETRLIMIT 1

/* Define to 1 if you have the `getrusage' function. */
#define HAVE_GETRUSAGE 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <gssapi/gssapi.h> header file. */
/* #undef HAVE_GSSAPI_GSSAPI_H */

/* Define to 1 if you have the <gssapi.h> header file. */
/* #undef HAVE_GSSAPI_H */

/* Define to 1 if you have the <ifaddrs.h> header file. */
#define HAVE_IFADDRS_H 1

/* Define to 1 if you have the `inet_lnaof' function. */
#define HAVE_INET_LNAOF 1

/* Define to 1 if you have the `inet_makeaddr' function. */
#define HAVE_INET_MAKEADDR 1

/* Define to 1 if you have the `inet_netof' function. */
#define HAVE_INET_NETOF 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `ioctl' function. */
#define HAVE_IOCTL 1

/* Define to 1 if you have the `ioctlsocket' function. */
/* #undef HAVE_IOCTLSOCKET */

/* Define to 1 if you have the <iomanip> header file. */
#define HAVE_IOMANIP 1

/* Define to 1 if you have the <iomanip.h> header file. */
/* #undef HAVE_IOMANIP_H */

/* Define to 1 if you have the <iosfwd> header file. */
#define HAVE_IOSFWD 1

/* Define to 1 if you have the <iosfwd.h> header file. */
/* #undef HAVE_IOSFWD_H */

/* Define to 1 if you have the <iostream> header file. */
#define HAVE_IOSTREAM 1

/* Define to 1 if you have the <iostream.h> header file. */
/* #undef HAVE_IOSTREAM_H */

/* Define to 1 if you have the <krb5.h> header file. */
/* #undef HAVE_KRB5_H */

/* Define to 1 if the system has the type `krb5_init_creds_context'. */
/* #undef HAVE_KRB5_INIT_CREDS_CONTEXT */

/* Define to 1 if the system has the type `krb5_tkt_creds_context'. */
/* #undef HAVE_KRB5_TKT_CREDS_CONTEXT */

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define if you have the ncurses library (-lncurses). */
/* #undef HAVE_LIBNCURSES */

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `ssl' library (-lssl). */
#define HAVE_LIBSSL 1

/* Define to 1 if you have `z' library (-lz) */
#define HAVE_LIBZ 1

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ncurses.h> header file. */
/* #undef HAVE_NCURSES_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/in_var.h> header file. */
/* #undef HAVE_NETINET_IN_VAR_H */

/* Define to 1 if you have the <netinet/ip_dummynet.h> header file. */
/* #undef HAVE_NETINET_IP_DUMMYNET_H */

/* Define to 1 if you have the <netinet/ip_fw.h> header file. */
/* #undef HAVE_NETINET_IP_FW_H */

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#define HAVE_NETINET_TCP_H 1

/* Define to 1 if you have the <net/if.h> header file. */
#define HAVE_NET_IF_H 1

/* Define to 1 if you have the <net/if_var.h> header file. */
/* #undef HAVE_NET_IF_VAR_H */

/* Define to 1 if you have the <openssl/err.h> header file. */
#define HAVE_OPENSSL_ERR_H 1

/* Define to 1 if you have the <openssl/rand.h> header file. */
#define HAVE_OPENSSL_RAND_H 1

/* Define to 1 if you have the <openssl/ssl.h> header file. */
#define HAVE_OPENSSL_SSL_H 1

/* Define to 1 if you have the `pclose' function. */
#define HAVE_PCLOSE 1

/* Define to 1 if you have the `poll' function. */
#define HAVE_POLL 1

/* Define to 1 if you have the `popen' function. */
#define HAVE_POPEN 1

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* Define to 1 if you have the `RAND_egd' function. */
#define HAVE_RAND_EGD 1

/* Define to 1 if you have the `RAND_screen' function. */
/* #undef HAVE_RAND_SCREEN */

/* Define to 1 if you have the `RAND_status' function. */
#define HAVE_RAND_STATUS 1

/* Define to 1 if you have the <regex.h> header file. */
#define HAVE_REGEX_H 1

/* Define to 1 if you have the `rint' function. */
#define HAVE_RINT 1

/* sockaddr structure has sa_len member */
/* #undef HAVE_SA_LEN */

/* Define to 1 if you have the `sched_getaffinity' function. */
#define HAVE_SCHED_GETAFFINITY 1

/* Define to 1 if you have the <sched.h> header file. */
#define HAVE_SCHED_H 1

/* Define to 1 if you have the `sched_setaffinity' function. */
#define HAVE_SCHED_SETAFFINITY 1

/* Define to 1 if you have the `SetLastError' function. */
/* #undef HAVE_SETLASTERROR */

/* have set_new_handler or std::set_new_handler */
#define HAVE_SET_NEW_HANDLER 1

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `sleep' function. */
#define HAVE_SLEEP 1

/* Define to 1 if you have the <sstream> header file. */
#define HAVE_SSTREAM 1

/* Define to 1 if you have the `stat' function. */
#define HAVE_STAT 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `stricmp' function. */
/* #undef HAVE_STRICMP */

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strnicmp' function. */
/* #undef HAVE_STRNICMP */

/* Define to 1 if you have the <strstream> header file. */
#define HAVE_STRSTREAM 1

/* Define to 1 if you have the <strstream.h> header file. */
/* #undef HAVE_STRSTREAM_H */

/* Define to 1 if you have the <strstrea.h> header file. */
/* #undef HAVE_STRSTREA_H */

/* Define to 1 if you have the <sys/endian.h> header file. */
/* #undef HAVE_SYS_ENDIAN_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysinfo.h> header file. */
#define HAVE_SYS_SYSINFO_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `timegm' function. */
#define HAVE_TIMEGM 1

/* how time zone global variable looks like (timezone, _timezone, etc.) */
#define HAVE_TIMEZONE timezone

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* tm structure has tm_gmtoff member */
#define HAVE_TM_GMTOFF 1

/* */
/* #undef HAVE_TYPE_IFALIASREQ */

/* */
#define HAVE_TYPE_IFREQ 1

/* */
/* #undef HAVE_TYPE_IN6_ALIASREQ */

/* */
/* #undef HAVE_TYPE_IN6_IFREQ */

/* */
#define HAVE_TYPE_IOS_BASE_FMTFLAGS 1

/* */
/* #undef HAVE_TYPE_IOS_FMTFLAGS */

/* */
#define HAVE_TYPE_RLIM_T 1

/* */
#define HAVE_TYPE_RUSAGE 1

/* Some systems use socklen_t typedef for some socket operations. Socklen_t
   may conflict with "int" that is also used. */
#define HAVE_TYPE_SOCKLEN_T 1

/* */
/* #undef HAVE_TYPE_STREAMPOS */

/* */
#define HAVE_TYPE_TIMEVAL 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unlink' function. */
#define HAVE_UNLINK 1

/* Define to 1 if you have the <winbase.h> header file. */
/* #undef HAVE_WINBASE_H */

/* Define to 1 if you have the <winsock2.h> header file. */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if you have the `WSACleanup' function. */
/* #undef HAVE_WSACLEANUP */

/* Define to 1 if you have the `WSAIoctl' function. */
/* #undef HAVE_WSAIOCTL */

/* Define to 1 if you have the `WSAStartup' function. */
/* #undef HAVE_WSASTARTUP */

/* Define to 1 if you have the `_ftime' function. */
/* #undef HAVE__FTIME */

/* Define to 1 if you have the `_pclose' function. */
/* #undef HAVE__PCLOSE */

/* Define to 1 if you have the `_popen' function. */
/* #undef HAVE__POPEN */

/* Define if Kerberos support is enabled. */
/* #undef KERBEROS_ENABLED */

/* Define if libldns support is enabled. */
/* #undef LDNS_ENABLED */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Mac OS X 10.6 (at least) defines NCURSES_OPAQUE to 1 by default. Some
   structs we use (e.g. WINDOW) are defined only if NCURSES_OPAQUE is 0. */
/* #undef NCURSES_OPAQUE */

/* Define if OpenSSL support is enabled */
#define OPENSSL_ENABLED 1

/* Name of package */
#define PACKAGE "polygraph"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "Web Polygraph"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Web Polygraph 4.9.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "polygraph"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.9.0"

/* negative or zero if no probing has been done or probing failed */
#define PROBED_MAXFD 65536

/* sighandler prototype (e.g. "void SignalHandler(...)" on IRIX */
#define SIGNAL_HANDLER_TYPE void SignalHandler(int)

/* The size of `char', as computed by sizeof. */
/* #undef SIZEOF_CHAR */

/* The size of `int', as computed by sizeof. */
/* #undef SIZEOF_INT */

/* The size of `long', as computed by sizeof. */
/* #undef SIZEOF_LONG */

/* The size of `short', as computed by sizeof. */
/* #undef SIZEOF_SHORT */

/* The size of `void*', as computed by sizeof. */
/* #undef SIZEOF_VOIDP */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Defined if supported DummyNet version was found */
/* #undef USE_DUMMYNET */

/* Version number of package */
#define VERSION "4.9.0"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

#include "post-config.h"

#endif /* POLYGRAPH__CONFIG_H */
