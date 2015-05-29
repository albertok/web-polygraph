#
# server-side library and
# polysrv executable
#

noinst_LIBRARIES = \
	libsrvside.a

libsrvside_a_SOURCES = \
	Server.h \
	Server.cc \
	SrvConnMgr.h \
	SrvConnMgr.cc \
	SrvOpts.h \
	SrvOpts.cc \
	SrvXact.h \
	SrvXact.cc \
	\
	SrvCfg.h \
	SrvCfg.cc \
	\
	HttpSrvXact.h \
	HttpSrvXact.cc \
	FtpSrvXact.h \
	FtpSrvXact.cc


bin_PROGRAMS = \
	server

dist_man1_MANS = \
	server.man

server_SOURCES = \
	PolySrv.h \
	PolySrv.cc

LDADD = \
	libsrvside.a \
	\
	../app/libapp.a \
	../runtime/libagent.a \
	../runtime/libruntime.a \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	../csm/libcsm.a \
	../beep/libbeep.a \
	../xml/libxml.a \
	../base/libbase.a \
	../xstd/libxstd.a



include $(top_srcdir)/common.am
