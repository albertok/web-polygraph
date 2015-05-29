#
# Tools to monitor run-time performance
#

bin_PROGRAMS = \
	polymon \
	polyrrd \
	udp2tcpd

dist_man1_MANS = \
	polymon.man \
	polyrrd.man \
	udp2tcpd.man

EXTRA_DIST = \
	polymon.h2m \
	polyrrd.h2m \
	udp2tcpd.h2m

polymon_SOURCES = \
	polymon.cc

polyrrd_SOURCES = \
	polyrrd.cc

udp2tcpd_SOURCES = \
	udp2tcpd.cc

LDADD = \
	../runtime/libruntime.a \
	../base/libbase.a \
	../xstd/libxstd.a

polymon_LDADD = \
	$(LDADD) \
	@LIB_CURSES@


include $(top_srcdir)/common.am
