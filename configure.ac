dnl
dnl  @configure_input@
dnl
dnl  Configuration input file for Web Polygraph
dnl

AC_INIT([Web Polygraph],[4.9.0],[],[polygraph])
AC_CONFIG_SRCDIR([src/base/polygraph.h])
AC_CONFIG_AUX_DIR(cfgaux)

AM_INIT_AUTOMAKE([1.5 nostdinc])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([src/config.h])
AC_PREFIX_DEFAULT(/usr/local)

AH_TOP(
[#ifndef POLYGRAPH__CONFIG_H
#define POLYGRAPH__CONFIG_H


/* here are some typedefs that configure script might be missing */
#undef rlim_t])

AH_BOTTOM(
[#include "post-config.h"

#endif /* POLYGRAPH__CONFIG_H */])

dnl general stuff used by other checks
AH_VERBATIM(CONFIG_HOST_TYPE,
[/* host type from configure */
#define CONFIG_HOST_TYPE "-unknown-"])
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(CONFIG_HOST_TYPE, "$host")

dnl add polygraph- prefix and strip .pl extension during installation
program_transform_name="s/^/polygraph-/; s/\\.pl\$$//; $program_transform_name"

AC_LANG([C++])

dnl save pre-set values (if any) for variables that are
dnl created by autoconf
PRESET_CXXFLAGS="$CXXFLAGS"
PRESET_LDFLAGS="$LDFLAGS"

dnl AC_CONFIG_SUBDIRS(src/Hapy)

dnl check for programs
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AM_PROG_AR

dnl user can overwrite this default with --enable-shared
AC_DISABLE_SHARED
LT_INIT

AC_C_BIGENDIAN

AC_PATH_PROG(HELP2MAN, help2man)
AM_CONDITIONAL(ENABLE_MANPAGES_GEN, [test -n "$HELP2MAN"])

# change default extention for source files
ac_ext=cc

dnl check if alleged C++ compiler understands C++
AC_MSG_CHECKING(whether the C++ compiler ($CXX) is a C++ compiler)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
	#define Concat(a,b) a ## b
	struct T { 
		virtual ~T();
		virtual int m() const = 0;
		mutable bool a;
	};

	template <class P>
	struct C: public P {
		C(int): c(0) {}
		int m() const;
		int c;
	};

	template <class P>
	int C<P>::m() const { Concat(re,turn) c; }

	inline int test_inline() { return 0; }
]], [[
	// this is main()'s body
	static C<T> ct(1);
	return 0;
]])],[
	AC_MSG_RESULT(yes)
],[
	AC_MSG_RESULT(no)
	AC_MSG_ERROR(the compiler ($CXX) failed to pass a simple C++ test; check config.log for details)
])


dnl check for optional features
AC_SUBST(std_include)
AC_ARG_WITH(std-include,
AS_HELP_STRING([--with-std-include=DIR],[where to find standard C++ headers]),
	[
		std_include=`echo -I$withval | sed 's/:/ -I/g'`
		CPPFLAGS="$CPPFLAGS $std_include"
	],
	[ 	
		std_include='' 
	]
)


dnl adjust program options

OLD_CXXFLAGS=$CXXFLAGS
if test -z "$PRESET_CXXFLAGS"
then
	if test "x$GXX" = xyes
	then [

		# reset to preferred options
		# replace -O? with -O3
		CXXFLAGS=`echo $CXXFLAGS | sed 's/-O[0-9]*/-O3/'`;
		# enable useful warnings
		CXXFLAGS="$CXXFLAGS -Wall -Wwrite-strings -Woverloaded-virtual"

		# custom host-dependent tuning
		case "$host" in
		alpha-*)
			# we get random coredumps with g++ -O on alpha-dec-osf4
			# g++ -O3 warns about being buggy on alpha-*-freebsd4.*
			echo deleting -On on $host
			CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-O[0-9]* *//'`
			;;
		*-linux-*)
			# -O2,3 seems to produce coredumps on RH and MDK linux
			echo enforcing -O1 on $host
			CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-O[0-9]* */-O1 /'`
			;;
		esac
	] fi
fi

if test "x$PRESET_CXXFLAGS" != "x$CXXFLAGS"
then
	AC_MSG_CHECKING(whether custom $CXX flags work)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		]], [[
			return 0;
		]])],[
			AC_MSG_RESULT(probably)
			echo "changing $CXX flags to $CXXFLAGS"
		],[
			AC_MSG_RESULT(no)
			CXXFLAGS=$OLD_CXXFLAGS
			echo "leaving  $CXX flags at $CXXFLAGS"
		
	])
fi

dnl check whether compiler is ok with passing -rdynamic to linker
dnl that flag is required with GCC for linking executables that call dlopen
if test "x$GXX" = "xyes" 
then
	AC_MSG_CHECKING(whether -rdynamic $CXX flag works)
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		]], [[
			return 0;
		]])],[
			AC_MSG_RESULT(probably)
			LDFLAG_RDYNAMIC=-rdynamic
		],[
			AC_MSG_RESULT(no)
			LDFLAG_RDYNAMIC=""
		
	])
fi


dnl check for libraries
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_CHECK_LIB(socket, main)
AC_CHECK_LIB(m, main)

AC_MSG_CHECKING(whether to link statically)
AC_ARG_WITH(static-linking,
[  --with-static-linking   link binaries statically],
[
	case "$withval" in
	no)
		AC_MSG_RESULT(no)
		;;
	*)
		AC_MSG_RESULT(yes)
		LDFLAGS="-static $LDFLAGS"
	esac
],[
	AC_MSG_RESULT(no)
]
)

dnl check if compiler can find C++ headers
AC_CHECK_HEADER(iostream, [],
	AC_MSG_WARN([
		Failure to find 'iostream' header file indicates
		a compiler installation problem; You may want to
		use --enable-std-include option to help your 
		compiler to find directories with standard C++
		include files.])
)

AC_CACHE_CHECK(whether C++ std:: namespace works,
	ac_cv_std_namespace_works, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <iostream>
			#include <iomanip>
		]], [[
			std::cout << std::endl;
			return 0;
		]])],[
			ac_cv_std_namespace_works=yes
		],[
			ac_cv_std_namespace_worls=no
		])
	]
)


dnl Check for the presence of SSL libraries and headers
dnl This code was inspired by curl's configure.in

dnl Default to compiler/linker defaults for SSL files and libraries.
OPT_SSL=off
AC_ARG_WITH(ssl,dnl
AS_HELP_STRING([--with-ssl=PATH],[where to look for OpenSSL libs and headers])
AS_HELP_STRING([--without-ssl],[disable SSL support]),
	[OPT_SSL=$withval])

if test X"$OPT_SSL" = Xno
then
	AC_MSG_WARN([
		Support for Polygraph SSL features is
		explicitly prohibited by the user.])
	sleep 1;
else
	dnl default to disable; the checks below may enable it
	OPENSSL_ENABLED=0

	dnl save the pre-ssl check flags for a while
	CLEANLDFLAGS="$LDFLAGS"
	CLEANCPPFLAGS="$CPPFLAGS"

	dnl Check for and handle argument to --with-ssl.
	case "$OPT_SSL" in
	yes)
		EXTRA_SSL=/usr/local/ssl
		;;
	off)
		EXTRA_SSL=
		;;
	*)
		dnl user-specified PATH
		EXTRA_SSL=$OPT_SSL
		LDFLAGS="$LDFLAGS -L$EXTRA_SSL/lib"
		CPPFLAGS="$CPPFLAGS -I$EXTRA_SSL/include"
		;;
	esac

	AC_CHECK_LIB(crypto, CRYPTO_lock,[
		 HAVECRYPTO="yes"
		 ],[
		 OLDLDFLAGS="$LDFLAGS"
		 OLDCPPFLAGS="$CPPFLAGS"
		 LDFLAGS="$CLEANLDFLAGS -L$EXTRA_SSL/lib"
		 CPPFLAGS="$CLEANCPPFLAGS -I$EXTRA_SSL/include"
		 AC_CHECK_LIB(crypto, CRYPTO_add_lock,[
			 HAVECRYPTO="yes" ], [
			 LDFLAGS="$OLDLDFLAGS"
			 CPPFLAGS="$OLDCPPFLAGS"
		 ])
	])


	if test "x$HAVECRYPTO" = "xyes"
	then
		dnl Check for OpenSSL headers
		dnl OPENSSL_ENABLED=1 may still be overwritten later
		AC_CHECK_HEADERS(openssl/ssl.h openssl/err.h openssl/rand.h,
			OPENSSL_ENABLED=1)
	fi

	if test "$OPENSSL_ENABLED" = "1"
	then
		dnl check for SSL libs
		dnl this is only reasonable to do if crypto actually is there
		dnl it is important to do this _after_ the crypto lib

		AC_CHECK_LIB(crypto, CRYPTO_add_lock)
		AC_CHECK_LIB(ssl, SSL_connect)

		if test "$ac_cv_lib_ssl_SSL_connect" != yes
		then
			dnl we didn't find the SSL lib, try the RSAglue/rsaref stuff
			AC_MSG_CHECKING(for ssl with RSAglue/rsaref libs in use)
			OLIBS=$LIBS
			LIBS="$LIBS -lRSAglue -lrsaref"
			AC_CHECK_LIB(ssl, SSL_connect)
			if test "$ac_cv_lib_ssl_SSL_connect" != yes
			then
				dnl still no SSL_connect
				OPENSSL_ENABLED=0
				AC_MSG_RESULT(no)
				LIBS=$OLIBS
			else
				AC_MSG_RESULT(yes)
			fi
		fi

		dnl If the ENGINE library seems to be around, 
		dnl check for the OpenSSL engine header
		dnl it is kind of "separated" from the main SSL check
		dnl AC_CHECK_FUNC(ENGINE_init, 
		dnl	AC_CHECK_HEADERS(openssl/engine.h))
	fi

	if test X"$OPT_SSL" != Xoff -a "$OPENSSL_ENABLED" != "1"
	then
		AC_MSG_ERROR([OpenSSL libraries and/or directories were not found in $EXTRA_SSL])
	fi

	dnl these can only exist if openssl exists
	AC_CHECK_FUNCS( RAND_status \
		RAND_screen \
		RAND_egd )

	AC_DEFINE_UNQUOTED(OPENSSL_ENABLED, $OPENSSL_ENABLED, 
		[Define if OpenSSL support is enabled ])
fi

dnl Set zlib_places to fix AX_CHECK_ZLIB serial number 8
if test "x${with_zlib}" = "x"
then
	zlib_places="/usr/local /usr /opt/local /sw"
fi
AX_CHECK_ZLIB
if test "$zlib_cv_libz" = "yes"
then
	AC_CACHE_CHECK(whether zlib supports HTTP gzip encoding natively,
		ac_cv_zlib_gzip, [
			AC_RUN_IFELSE([AC_LANG_SOURCE([[
				#include <zlib.h>
				
				int main() {
					z_stream z;
					z.zalloc = Z_NULL;
					z.zfree = Z_NULL;
					z.opaque = 0;
					// the magic constants below are taken from zlib.h to force
					// gzip header and footer for the deflated stream
					int res = deflateInit2(&z, Z_DEFAULT_COMPRESSION, 
						Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
					return (res == Z_OK) ? 0 : -1;
				}
			]])],[
				ac_cv_zlib_gzip="yes";
			],[
				AC_MSG_ERROR([
		Your zlib library does not seem to support 
		HTTP gzip encoding natively.
		Upgrade to zlib version 1.2.0.4 (or later) OR
		disable zlib usage with --without-zlib])
			],[
				dnl TODO: this warning disrupts output and
				dnl should not be conditional;
				dnl move it after the AC_CACHE_CHECK call.
				AC_MSG_WARN([
		Skipping checks for native HTTP gzip encoding support in
		zlib due to cross compiling.])
			])
		]
	)
else
	AC_MSG_WARN([
		Polygraph HTTP compression features are
		explicitly disabled by the user.])
	sleep 1;
fi

AC_MSG_NOTICE([checking for Kerberos support])
AX_SAVE_FLAGS
AC_ARG_WITH([kerberos],
AS_HELP_STRING([--with-kerberos=PATH],[path to krb5-config (default=detect)])
AS_HELP_STRING([--without-kerberos],[disable Kerberos support]),
	[test "x$with_kerberos" != "xno" && KERBEROS_WANTED=1],
	[with_kerberos=yes])
if test "x$with_kerberos" != "xno"; then
	if test "x$with_kerberos" = "xyes"; then
		AC_PATH_PROG([KRB5CONFIG], [krb5-config])
	else
		AC_MSG_CHECKING([for krb5-config])
		AC_MSG_RESULT([$with_kerberos])
		KRB5CONFIG="$with_kerberos"
	fi
	AC_MSG_CHECKING([if krb5-config is executable])
	if test -x "$KRB5CONFIG"; then
		AC_MSG_RESULT([yes])
		AC_MSG_CHECKING([for gssapi support in krb5-config])
		if "$KRB5CONFIG" | grep -q gssapi; then
			AC_MSG_RESULT([yes])
			KRB5CFLAGS=`"$KRB5CONFIG" --cflags gssapi`
			CPPFLAGS="$CPPFLAGS $KRB5CFLAGS"
			KRB5LIBS=`"$KRB5CONFIG" --libs gssapi`
			LIBS="$LIBS $KRB5LIBS"
		else
			AC_MSG_RESULT([no])
		fi
	else
		AC_MSG_RESULT([no])
	fi
fi
if test "x$KRB5LIBS" != "x"; then
	KERBEROS_ENABLED=1 # may be overridden below

	AC_CHECK_HEADERS([krb5.h],
		[AC_MSG_CHECKING([if krb5.h is broken for C++])
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM([[#include <krb5.h>]],
				[[krb5_init_context(0)]])],
			[AC_MSG_RESULT([not broken])],
			[AC_MSG_RESULT([broken])
			AC_MSG_CHECKING([if we can work around broken krb5.h])
			AC_LINK_IFELSE(
				[AC_LANG_PROGRAM(
					[[extern "C" {
					#include <krb5.h>
					}]],
					[[krb5_init_context(0)]])],
				[AC_MSG_RESULT([yes])
				AC_DEFINE(HAVE_BROKEN_KRB5_H, 1,
					[Define to 1 if krb5.h is broken for C++.])],
				[AC_MSG_RESULT([no])
				KERBEROS_ENABLED=0])])],
		[KERBEROS_ENABLED=0])

	AC_CHECK_TYPES([krb5_init_creds_context, krb5_tkt_creds_context], [],
		[KERBEROS_ENABLED=0],
		[[#if HAVE_KRB5_H
		#if HAVE_BROKEN_KRB5_H
		extern "C" {
		#include <krb5.h>
		}
		#else
		#include <krb5.h>
		#endif
		#endif]])

	AC_CHECK_HEADERS([gssapi/gssapi.h], [],
		[AC_CHECK_HEADERS([gssapi.h], [], [KERBEROS_ENABLED=0])])
fi
if test "x$KERBEROS_ENABLED" = "x1"; then
	AC_DEFINE(KERBEROS_ENABLED, 1, [Define if Kerberos support is enabled.])
	AC_MSG_NOTICE([Kerberos support enabled])
else
	if test "x$KERBEROS_WANTED" = "x1"; then
		AC_MSG_ERROR([Kerberos support requested but could not be enabled])
	else
		AC_MSG_WARN([Kerberos support disabled.])
	fi
	AX_RESTORE_FLAGS
fi

AC_MSG_NOTICE([checking for libldns support])
AX_SAVE_FLAGS
AC_ARG_WITH([ldns],
AS_HELP_STRING([--with-ldns=PATH],[path to ldns-config (default=detect)])
AS_HELP_STRING([--without-ldns],[disable ldns support]),
	[test "x$with_ldns" != "xno" && LDNS_WANTED=1],
	[with_ldns=yes])
if test "x$with_ldns" != "xno"; then
	if test "x$with_ldns" = "xyes"; then
		AC_PATH_PROG([LDNSCONFIG], [ldns-config])
	else
		AC_MSG_CHECKING([for ldns-config])
		AC_MSG_RESULT([$with_ldns])
		LDNSCONFIG="$with_ldns"
	fi
	AC_MSG_CHECKING([if ldns-config is executable])
	if test -x "$LDNSCONFIG"; then
		AC_MSG_RESULT([yes])
		LDNSCFLAGS=`"$LDNSCONFIG" --cflags`
		CPPFLAGS="$CPPFLAGS $LDNSCFLAGS"
		LDNSLIBS=`"$LDNSCONFIG" --libs`
		LIBS="$LIBS $LDNSLIBS"
	else
		AC_MSG_RESULT([no])
	fi
fi
if test "x$LDNSLIBS" != "x"; then
	LDNS_ENABLED=1 # may be overridden below
	AC_MSG_CHECKING([whether ldns/ldns.h compiles])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[#include <ldns/ldns.h>]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])
		AC_MSG_CHECKING([whether we can fix ldns/ldns.h compilation])
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(
				[[#define class classC
				#include <ldns/ldns.h>]])],
			[AC_MSG_RESULT([yes])
			AC_DEFINE(HAVE_CLASS_IN_LDNS, 1,
				[Define to 1 if C ldns/ldns.h uses a "class" keyword])],
			[AC_MSG_RESULT([no])
			LDNS_ENABLED=0]
		)]
	)
fi
if test "x$LDNS_ENABLED" = "x1"; then
	AC_MSG_CHECKING([whether ldns/ldns.h links])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM([[
			#if HAVE_CLASS_IN_LDNS
			#define class classC
			#endif
			#include <ldns/ldns.h>]],
			[[ldns_buffer_new(0)]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])
		AC_MSG_CHECKING([whether we can fix ldns/ldns.h linking])
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM(
				[[
				#if HAVE_CLASS_IN_LDNS
				#define class classC
				#endif
				extern "C" {
				#include <ldns/ldns.h>
				}]],
				[[ldns_buffer_new(0)]])],
			[AC_MSG_RESULT([yes])
			AC_DEFINE(HAVE_BROKEN_LDNS_H, 1,
				[Define to 1 if ldns/ldns.h needs extern "C" wrapppers.])],
			[AC_MSG_RESULT([no])
			LDNS_ENABLED=0])])
fi
if test "x$LDNS_ENABLED" = "x1"; then
	AC_DEFINE(LDNS_ENABLED, 1, [Define if libldns support is enabled.])
	AC_MSG_NOTICE([libldns support enabled])
else
	if test "x$LDNS_WANTED" = "x1"; then
		AC_MSG_ERROR([libldns support requested but could not be enabled])
	else
		AC_MSG_NOTICE([libldns support disabled])
	fi
	if test "x$with_ldns" == "xno"; then
		AC_MSG_WARN([libldns support is explicitly disabled by the user. PGL zoneDomains() function will not work.])
	else
		AC_MSG_WARN([libldns is not found. PGL zoneDomains() function will not work.])
	fi
	AX_RESTORE_FLAGS
fi


dnl check whether loganalizers/comparator should be built
AC_MSG_CHECKING(whether to build comparator)
AC_ARG_ENABLE(comparator,
	AS_HELP_STRING([--enable-comparator],[build comparator [[default=no]]]),
	[case "${enableval}" in
		yes) result=yes ;;
		no)  result=no ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --enable-comparator <yes|no>]) ;;
	esac],
	[result="no (by default)"]
)
AC_MSG_RESULT($result)
AM_CONDITIONAL(ENABLE_COMPARATOR, test "x$result" = xyes)


dnl library checks below do not use default actions to 
dnl avoid adding found libraries to all executables via LIBS


dnl dlopen may be in libc or in libdl
HAVE_DLOPEN_SOMEWHERE=no
AC_CHECK_LIB(c, dlopen,
	[
		HAVE_DLOPEN_SOMEWHERE=yes
		LIB_DL=""
	],[
		AC_CHECK_LIB(dl, dlopen, [
			HAVE_DLOPEN_SOMEWHERE=yes
			LIB_DL="-ldl"
		],[
			LIB_DL=""
		])
	]
)

if test $HAVE_DLOPEN_SOMEWHERE = yes
then
	AC_DEFINE(HAVE_DLOPEN, 1,
		[Define if you have the dlopen/sum/error/close.])
fi

AC_CHECK_LIB(ncurses, main,
	[
		AC_DEFINE(HAVE_LIBNCURSES, 1,
			[Define if you have the ncurses library (-lncurses).])
		AC_DEFINE(NCURSES_OPAQUE, 0,
			[Mac OS X 10.6 (at least) defines NCURSES_OPAQUE to 1 by
			default. Some structs we use (e.g. WINDOW) are defined
			only if NCURSES_OPAQUE is 0.])
		LIB_CURSES="-lncurses"
	],[
		LIB_CURSES=""
	]
)

dnl checks for header files
AC_CHECK_HEADERS(\
	arpa/inet.h \
	dirent.h \
	dlfcn.h \
	endian.h \
	fcntl.h \
	ifaddrs.h \
	iomanip \
	iomanip.h \
	iosfwd \
	iosfwd.h \
	iostream \
	iostream.h \
	math.h \
	ncurses.h \
	netdb.h \
	netinet/in.h \
	netinet/in_var.h \
	netinet/ip_dummynet.h \
	netinet/ip_fw.h \
	netinet/tcp.h \
	net/if.h \
	net/if_var.h \
	process.h \
	signal.h \
	sched.h \
	string.h \
	strings.h \
	sstream \
	strstrea.h \
	strstream \
	strstream.h \
	sys/endian.h \
	sys/ioctl.h \
	sys/param.h \
	sys/resource.h \
	sys/select.h \
	sys/socket.h \
	sys/sockio.h \
	sys/stat.h \
	sys/sysinfo.h \
	sys/time.h \
	sys/types.h \
	time.h \
	unistd.h \
	regex.h \
	winbase.h \
	winsock2.h,,,-)


dnl check for function parameters

AC_CACHE_CHECK(for signal handler type,
	ac_cv_signal_handler_type, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <signal.h>
			extern void my_sig_handler(int signo);
		]], [[
			signal(SIGINT, my_sig_handler);
			return 0;
		]])],[
			ac_cv_signal_handler_type="void SignalHandler(int)";
		],[
			# best we can do without checking further
			ac_cv_signal_handler_type="void SignalHandler(...)";
		])
	]
)
AC_DEFINE_UNQUOTED(SIGNAL_HANDLER_TYPE, $ac_cv_signal_handler_type,
	[sighandler prototype (e.g. "void SignalHandler(...)" on IRIX])


dnl check for types

dnl check for rlim_t type in sys/socket.h
AH_TEMPLATE(HAVE_TYPE_RLIM_T)
AC_CACHE_CHECK(for rlim_t, ac_cv_type_rlim_t, [
	AC_EGREP_CPP(
		[rlim_t[^a-zA-Z_0-9]],
		[
			#include <sys/types.h>
			#include <sys/resource.h>
			#if STDC_HEADERS
			#include <stdlib.h>
			#include <stddef.h>
			#endif
		],
		ac_cv_type_rlim_t=yes,
		ac_cv_type_rlim_t=no
	)
])
if test "x$ac_cv_type_rlim_t" = xyes; then
  AC_DEFINE(HAVE_TYPE_RLIM_T, 1)
fi

AC_CACHE_CHECK(for socklen_t, ac_cv_type_socklen_t, [
	AC_EGREP_CPP(
		[socklen_t[^a-zA-Z_0-9]],
		[
			#include <sys/types.h>
			#include <sys/socket.h>
			#if STDC_HEADERS
			#include <stdlib.h>
			#include <stddef.h>
			#endif
		],
		ac_cv_type_socklen_t=yes,
		ac_cv_type_socklen_t=no
	)
])
if test "x$ac_cv_type_socklen_t" = xyes; then
  AC_DEFINE(HAVE_TYPE_SOCKLEN_T, 1, [Some systems use socklen_t typedef for
                                     some socket operations. Socklen_t may
                                     conflict with "int" that is also used.])
fi

AH_TEMPLATE(HAVE_TYPE_RUSAGE)
AC_CACHE_CHECK(for rusage, ac_cv_have_type_rusage, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <sys/time.h>
		#include <sys/resource.h>
	]], [[
		struct rusage R;
		return sizeof(R) == 0;
	]])],[
		ac_cv_have_type_rusage="yes"
	],[
		ac_cv_have_type_rusage="no"
	])
])
if test "x$ac_cv_have_type_rusage" = xyes ; then
  AC_DEFINE(HAVE_TYPE_RUSAGE)
fi


dnl tm.tm_gmtoff
AH_TEMPLATE(HAVE_TYPE_TIMEVAL)
AC_CACHE_CHECK(for timeval, ac_cv_have_type_timeval, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <time.h>
		#include <sys/time.h>
	]], [[
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = 0;
		return 0;
	]])],[
		ac_cv_have_type_timeval="yes";
	],[
		ac_cv_have_type_timeval="no";
	])
])
if test "x$ac_cv_have_type_timeval" = xyes; then
	AC_DEFINE(HAVE_TYPE_TIMEVAL, 1)
fi

AH_TEMPLATE(HAVE_TYPE_IFREQ)
AC_CACHE_CHECK(for ifreq, ac_cv_have_type_ifreq, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <sys/types.h>
		#include <sys/socket.h>
		#ifdef HAVE_NET_IF_H
		#include <net/if.h>
		#endif
	]], [[
		struct ifreq r;
		return sizeof(r) == 0;
	]])],[
		ac_cv_have_type_ifreq="yes"
	],[
		ac_cv_have_type_ifreq="no"
	])
])
if test "x$ac_cv_have_type_ifreq" = xyes ; then
  AC_DEFINE(HAVE_TYPE_IFREQ)
fi

AH_TEMPLATE(HAVE_TYPE_IFALIASREQ)
AC_CACHE_CHECK(for ifaliasreq, ac_cv_have_type_ifaliasreq, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <sys/types.h>
		#include <sys/socket.h>
		#ifdef HAVE_NET_IF_H
		#include <net/if.h>
		#endif
	]], [[
		struct ifaliasreq r;
		return sizeof(r) == 0;
	]])],[
		ac_cv_have_type_ifaliasreq="yes"
	],[
		ac_cv_have_type_ifaliasreq="no"
	])
])
if test "x$ac_cv_have_type_ifaliasreq" = xyes ; then
  AC_DEFINE(HAVE_TYPE_IFALIASREQ)
fi

AH_TEMPLATE(HAVE_TYPE_IN6_ALIASREQ)
AC_CACHE_CHECK(for in6_aliasreq, ac_cv_have_type_in6_aliasreq, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <sys/types.h>
		#include <sys/socket.h>
		#ifdef HAVE_NET_IF_H
		#include <net/if.h>
		#endif
		#ifdef HAVE_NET_IF_VAR_H
		#include <net/if_var.h>
		#endif
		#ifdef HAVE_NETINET_IN_H
		#include <netinet/in.h>
		#endif
		#ifdef HAVE_NETINET_IN_VAR_H
		#include <netinet/in_var.h>
		#endif
	]], [[
		struct in6_aliasreq r;
		return sizeof(r) == 0;
	]])],[
		ac_cv_have_type_in6_aliasreq="yes"
	],[
		ac_cv_have_type_in6_aliasreq="no"
	])
])
if test "x$ac_cv_have_type_in6_aliasreq" = xyes ; then
  AC_DEFINE(HAVE_TYPE_IN6_ALIASREQ)
fi

AH_TEMPLATE(HAVE_TYPE_IN6_IFREQ)
AC_CACHE_CHECK(for in6_ifreq, ac_cv_have_type_in6_ifreq, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <sys/types.h>
		#include <sys/socket.h>
		#ifdef HAVE_NET_IF_H
		#include <net/if.h>
		#endif
		#ifdef HAVE_NET_IF_VAR_H
		#include <net/if_var.h>
		#endif
		#ifdef HAVE_NETINET_IN_H
		#include <netinet/in.h>
		#endif
		#ifdef HAVE_NETINET_IN_VAR_H
		#include <netinet/in_var.h>
		#endif
	]], [[
		struct in6_ifreq r;
		return sizeof(r) == 0;
	]])],[
		ac_cv_have_type_in6_ifreq="yes"
	],[
		ac_cv_have_type_in6_ifreq="no"
	])
])
if test "x$ac_cv_have_type_in6_ifreq" = xyes ; then
  AC_DEFINE(HAVE_TYPE_IN6_IFREQ)
fi

AH_TEMPLATE(HAVE_TYPE_STREAMPOS)
AC_CACHE_CHECK(for streampos, ac_cv_have_streampos, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <iostream.h>
	]], [[
		const streampos pos = 0;
		return sizeof(pos) == 0;
	]])],[
		ac_cv_have_type_streampos="yes"
	],[
		ac_cv_have_type_streampos="no"
	])
])
if test "x$ac_cv_have_type_streampos" = xyes ; then
  AC_DEFINE(HAVE_TYPE_STREAMPOS)
fi

AH_TEMPLATE(HAVE_TYPE_IOS_BASE_FMTFLAGS)
AC_CACHE_CHECK(for ios_base::fmtflags, ac_cv_have_type_ios_base_fmtflags, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <ios>
	]], [[
		std::ios_base::fmtflags flags;
		return sizeof(flags) == 0;
	]])],[
		ac_cv_have_type_ios_base_fmtflags="yes"
	],[
		ac_cv_have_type_ios_base_fmtflags="no"
	])
])
if test "x$ac_cv_have_type_ios_base_fmtflags" = xyes ; then
  AC_DEFINE(HAVE_TYPE_IOS_BASE_FMTFLAGS)
fi

AH_TEMPLATE(HAVE_TYPE_IOS_FMTFLAGS)
AC_CACHE_CHECK(for ios::fmtflags, ac_cv_have_type_ios_fmtflags, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <iostream.h>
	]], [[
		ios::fmtflags flags;
		return sizeof(flags) == 0;
	]])],[
		ac_cv_have_type_ios_fmtflags="yes"
	],[
		ac_cv_have_type_ios_fmtflags="no"
	])
])
if test "x$ac_cv_have_type_ios_fmtflags" = xyes ; then
  AC_DEFINE(HAVE_TYPE_IOS_FMTFLAGS)
fi


dnl sockaddr.sa_len
AC_CACHE_CHECK(whether sockaddr has sa_len,
	ac_cv_sockaddr_has_sa_len, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <sys/types.h>
			#include <sys/socket.h>
		]], [[
			// this is main()'s body
			struct sockaddr addr;
			addr.sa_len = 0;
			return 0;
		]])],[
			ac_cv_sockaddr_has_sa_len="yes";
		],[
			ac_cv_sockaddr_has_sa_len="no";
		])
	]
)	
if test "x$ac_cv_sockaddr_has_sa_len" = xyes; then
	AC_DEFINE(HAVE_SA_LEN, 1, [sockaddr structure has sa_len member])
fi


dnl tm.tm_gmtoff
AC_CACHE_CHECK(whether tm has tm_gmtoff,
	ac_cv_tm_has_tm_gmtoff, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <time.h>
			#include <sys/time.h>
		]], [[
			struct tm t;
			t.tm_gmtoff = 0;
			return 0;
		]])],[
			ac_cv_tm_has_tm_gmtoff="yes";
		],[
			ac_cv_tm_has_tm_gmtoff="no";
		])
	]
)
if test "x$ac_cv_tm_has_tm_gmtoff" = xyes; then
	AC_DEFINE(HAVE_TM_GMTOFF, 1, [tm structure has tm_gmtoff member])
fi


dnl check for global variables

dnl timezone(s)
AC_DEFUN([AC_TMP], [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
	#include <time.h>
	#include <sys/time.h>
]], [[
	return (int)_timezone;
]])],[
	ac_cv_var_timezone="_timezone";
],[
	AC_TRY_COMPILE([
		#include <time.h>
		#include <sys/time.h>
		extern time_t timezone;
	],[
		return (int)timezone;
	],[
		ac_cv_var_timezone="extern";
	],[
		ac_cv_var_timezone="none";
	])
])])


AC_CACHE_CHECK(for global timezone variable,
	ac_cv_var_timezone, [
		AC_RUN_IFELSE([AC_LANG_SOURCE([[
			#include <time.h>
			#include <sys/time.h>
			
			int main() {
				/* function name or a variable name? */
				return (((void*)&timezone) == ((void*)timezone)) ? -1 : 0;
			}
		]])],[
			ac_cv_var_timezone="timezone";
		],[
			AC_TMP
		],[
			AC_TMP
		])
	]
)

AH_TEMPLATE(HAVE_TIMEZONE, [how time zone global variable looks like (timezone, _timezone, etc.)])
if test "x$ac_cv_var_timezone" = xnone; then
	:;
else
	if test "x$ac_cv_var_timezone" = xextern; then
		AC_DEFINE(HAVE_EXTERN_TIMEZONE, 1, [must explicitly declare timezone global as extern])
		AC_DEFINE(HAVE_TIMEZONE, timezone)
	else
		AC_DEFINE_UNQUOTED(HAVE_TIMEZONE, $ac_cv_var_timezone)
	fi
fi

AH_TEMPLATE(HAVE_ALTZONE, [altzone global is supported (Solaris only?)])
AC_CACHE_CHECK(for altzone,
	ac_cv_have_altzone, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <time.h>
			#include <sys/time.h>
		]], [[
			return (int)altzone;
		]])],[
			ac_cv_have_altzone="yes";
		],[
			ac_cv_have_altzone="no";
		])
	]
)
if test "x$ac_cv_have_altzone" = xyes; then
	AC_DEFINE(HAVE_ALTZONE, 1)
fi

AC_CACHE_CHECK([for htobe64 and be64toh],
	ac_cv_have_64bit_byte_order_conversion, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#if HAVE_ENDIAN_H
			#include <endian.h>
			#elif HAVE_SYS_ENDIAN_H
			#include <sys/endian.h>
			#endif
		]], [[
			htobe64(0);
			be64toh(0);
		]])],[
			ac_cv_have_64bit_byte_order_conversion="yes";
		],[
			ac_cv_have_64bit_byte_order_conversion="no";
		])
	]
)
if test "x$ac_cv_have_64bit_byte_order_conversion" = xyes; then
	AC_DEFINE(HAVE_64BIT_BYTE_ORDER_CONVERSION, 1, [htobe64 and be64toh])
fi

dnl check for compiler characteristics

dnl check for functions and methods
AC_CHECK_FUNCS(\
	rint \
	ceilf \
	getifaddrs \
	gettimeofday \
	getpagesize \
	getrlimit \
	getrusage \
	ioctl \
	poll \
	epoll_create \
	signal \
	unlink \
	sleep \
	fork \
	strcasecmp \
	strncasecmp \
	timegm \
	pclose \
	popen \
	inet_makeaddr \
	inet_lnaof \
	inet_netof \
	sched_getaffinity \
	sched_setaffinity \
	\
	_ftime \
	_pclose \
	_popen \
	ioctlsocket \
	stat \
	stricmp \
	strnicmp \
	GetLastError \
	SetLastError \
	WSAStartup \
	WSACleanup \
	WSAIoctl \
	closesocket
)

AH_TEMPLATE(HAVE_SET_NEW_HANDLER)
AC_CACHE_CHECK(for set_new_handler, ac_cv_have_set_new_handler, [
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
			#include <new>
			using namespace std;
			static void myHandler() {}
			int main() {
				set_new_handler(&myHandler);
				return 0;
			}
		]])],[ac_cv_have_set_new_handler="yes"],[ac_cv_have_set_new_handler="no"],[ac_cv_have_set_new_handler="no"])
])
if test "x$ac_cv_have_set_new_handler" = xyes ; then
  AC_DEFINE(HAVE_SET_NEW_HANDLER, 1, [have set_new_handler or std::set_new_handler])
fi


if test "x$ac_cv_func_sched_getaffinity" = "xyes" -a "x$ac_cv_func_sched_setaffinity" = "xyes" ; then
	AC_DEFINE(HAVE_CPU_AFFINITY,1,[Support setting CPU affinity for workers])
fi


dnl check for system services

AH_VERBATIM(DEFAULT_FD_SETSIZE,
[/* negative or zero if we failed to detect it */
#define DEFAULT_FD_SETSIZE -1])
AC_MSG_CHECKING(Default FD_SETSIZE value)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/types.h>
	int main() {
		fprintf(fopen("conftestval", "w"), "%d\n", FD_SETSIZE);
		return 0;
	}
	]])],[DEFAULT_FD_SETSIZE=`cat conftestval`],[DEFAULT_FD_SETSIZE=-1],[DEFAULT_FD_SETSIZE=-1])
AC_MSG_RESULT($DEFAULT_FD_SETSIZE)
AC_DEFINE_UNQUOTED(DEFAULT_FD_SETSIZE, $DEFAULT_FD_SETSIZE)

AH_VERBATIM(PROBED_MAXFD,
[/* negative or zero if no probing has been done or probing failed */
#define PROBED_MAXFD -1])
AC_MSG_CHECKING(Maximum number of filedescriptors we can open)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
		/* this ingenuous check is derived from uncopyrighted Squid/configure.in */
		#include <stdio.h>
		#include <unistd.h>
		
		#include <sys/time.h>	/* needed on FreeBSD */
		#include <sys/param.h>
		#include <sys/resource.h>

		// see SSI_FD_NEWMAX below
		#ifdef HAVE_SYS_SYSINFO_H
		#include <sys/sysinfo.h>
		#endif
		
		int main() {
		#ifdef SSI_FD_NEWMAX
			if (setsysinfo(SSI_FD_NEWMAX, 0, 0, 0, 1) != 0)
				perror("setsysinfo(SSI_FD_NEWMAX)");
		#endif

		#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
		#if !defined(RLIMIT_NOFILE)
		#define RLIMIT_NOFILE RLIMIT_OFILE
		#endif
			struct rlimit rl;
			if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
				perror("getrlimit: RLIMIT_NOFILE");
			} else {
			rl.rlim_cur = rl.rlim_max; /* set it to the max */
			if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
				perror("setrlimit: RLIMIT_NOFILE");
			}
		#endif /* RLIMIT_NOFILE || RLIMIT_OFILE */

			/* by starting at 2^15, we will never exceed 2^16 */
			int i,j;
			i = j = 1<<15;
			while (j) {
				j >>= 1;
				if (dup2(0, i) < 0) { 
					i -= j;
				} else {
					close(i);
					i += j;
				}
			}
			i++;
			FILE *fp = fopen("conftestval", "w");
			fprintf (fp, "%d\n", i);
			return 0;
		}
	]])],[PROBED_MAXFD=`cat conftestval`],[PROBED_MAXFD=-1],[PROBED_MAXFD=-2
])
AC_MSG_RESULT($PROBED_MAXFD)
AC_DEFINE_UNQUOTED(PROBED_MAXFD, $PROBED_MAXFD)

dnl various aftershock warnings
	
if test "x$ac_cv_header_regex_h" = xno
then
	AC_MSG_WARN([
		No Regular Expression library header file found (regex.h);
		RE-dependent features such as ACL checking will not work.])
fi

if test "x$ac_cv_header_netinet_ip_dummynet_h" = xyes
then
	AC_CHECK_TYPES(dn_pipe,
		[AC_DEFINE(USE_DUMMYNET, 1, [Defined if supported DummyNet version was found])],
		[AC_MSG_WARN([unsupported DummyNet version])], [
			#include <sys/types.h>
			#include <sys/socket.h>
			#include <net/if.h>
			#include <netinet/in.h>
			#include <netinet/ip_fw.h>
			#include <netinet/ip_dummynet.h>
		])
else
	AC_MSG_WARN([
		No DummyNet header files found;
		piper tool will not be available.])
fi

if test "x$ac_cv_header_dlfcn_h" = xno -o $HAVE_DLOPEN_SOMEWHERE != yes
then
	AC_MSG_WARN([
		No dl library or dlfcn.h header file found;
		dynamically loadable modules will not be supported.])
fi

if test "x$ac_cv_header_ncurses_h" = xno -o "x$ac_cv_lib_ncurses_main" = xno
then
	AC_MSG_WARN([
		No ncurses library or header files found;
		polymon tool will not be available.])
fi

AX_CREATE_STDINT_H(src/xstd/h/stdint.h)

dnl these variables will be substituted in Makefiles
AC_SUBST(LIB_DL)
AC_SUBST(LIB_CURSES)
AC_SUBST(LDFLAG_RDYNAMIC)
	
AC_PROG_MAKE_SET

AC_CONFIG_FILES([\
	Makefile
	src/Makefile \
	src/xstd/Makefile \
	src/base/Makefile \
	src/xml/Makefile \
	src/runtime/Makefile \
	src/xparser/Makefile \
	src/pgl/Makefile \
	src/csm/Makefile \
	src/dns/Makefile \
	src/icp/Makefile \
	src/kerberos/Makefile \
	src/beep/Makefile \
	src/cache/Makefile \
	src/app/Makefile \
	src/client/Makefile \
	src/server/Makefile \
	src/logextractors/Makefile \
	src/loganalyzers/Makefile \
	src/tools/Makefile \
	src/probe/Makefile \
	src/monitor/Makefile \
	tools/Makefile \
	workloads/Makefile \
	workloads/include/Makefile \
	import/Makefile \
	import/jqplot/Makefile \
	import/jquery-ui/Makefile \
	polygraph.man
])
AC_OUTPUT
