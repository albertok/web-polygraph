#
# polyprobe executable
#

bin_PROGRAMS = \
	polyprobe

dist_man1_MANS = \
	polyprobe.man

polyprobe_SOURCES = \
	PolyProbe.h \
	PolyProbe.cc \
	ProbeOpts.h \
	ProbeOpts.cc \
	\
	ProbeClt.h \
	ProbeClt.cc \
	ProbeSrv.h \
	ProbeSrv.cc \
	ProbeAgent.h \
	ProbeAgent.cc \
	\
	ProbeXact.h \
	ProbeXact.cc \
	ProbeCltXact.h \
	ProbeCltXact.cc \
	ProbeSrvXact.h \
	ProbeSrvXact.cc \
	\
	ProbeLinkStat.h \
	ProbeLinkStat.cc \
	ProbeStatMgr.h \
	ProbeStatMgr.cc

LDADD = \
	../runtime/libruntime.a \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	../base/libbase.a \
	../xstd/libxstd.a


include $(top_srcdir)/common.am
