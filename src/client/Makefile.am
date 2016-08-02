#
# client-side library and
# polyclt executable
#

noinst_LIBRARIES = \
	libcltside.a

libcltside_a_SOURCES = \
	Client.h \
	Client.cc \
	AsyncClt.h \
	AsyncClt.cc \
	SyncClt.h \
	SyncClt.cc \
	PassClt.h \
	PassClt.cc \
	\
	CltOpts.h \
	CltOpts.cc \
	CltCfg.h \
	CltCfg.cc \
	CltBehaviorCfg.h \
	CltBehaviorCfg.cc \
	SessionWatchRegistry.h \
	SessionWatchRegistry.cc \
	CltDataFilterRegistry.h \
	CltDataFilterRegistry.cc \
	\
	ServerRep.h \
	ServerRep.cc \
	ForeignWorld.h \
	ForeignWorld.cc \
	PrivCache.h \
	PrivCache.cc \
	\
	CltConnMgr.h \
	CltConnMgr.cc \
	ConnHash.h \
	ConnHash.cc \
	WarmupPlan.h \
	WarmupPlan.cc \
	\
	CltXactMgr.h \
	PipelinedCxm.h \
	PipelinedCxm.cc \
	SingleCxm.h \
	SingleCxm.cc \
	FtpCxm.h \
	FtpCxm.cc \
	\
	CltXact.h \
	CltXact.cc \
	HttpCltXact.h \
	HttpCltXact.cc \
	FtpCltXact.h \
	FtpCltXact.cc \
	IcpCltXact.h \
	IcpCltXact.cc \
	\
	SessionMgr.h \
	SessionMgr.cc \
	SessionStats.h \
	SessionStats.cc \
	\
	AclGroup.h \
	AclGroup.cc \
	RegExGroup.h \
	RegExGroup.cc \
	RegExGroups.h \
	RegExGroups.cc \
	MembershipMap.h \
	MembershipMap.cc \
	MembershipGroupIterator.h \
	MembershipGroupIterator.cc \
	\
	BodyParserFarm.h \
	ParseBuffer.h \
	BodyParser.h \
	BodyParser.cc \
	MarkupBodyParser.h \
	MarkupBodyParser.cc \
	UriBodyParser.h \
	UriBodyParser.cc \
	UriScriptBodyParser.h \
	UriScriptBodyParser.cc \
	ChunkedCodingParser.h \
	ChunkedCodingParser.cc \
	AnyBodyParser.h \
	AnyBodyParser.cc \
	\
	SpnegoCodec.h \
	SpnegoCodec.cc \
	NtlmAuth.h \
	NtlmAuth.cc \
	RangeCfg.h \
	RangeCfg.cc \
	SingleRangeCfg.h \
	SingleRangeCfg.cc \
	MultiRangeCfg.h \
	MultiRangeCfg.cc \
	MultiPartParser.h \
	MultiPartParser.cc


bin_PROGRAMS = \
	client \
	pgl2ldif

dist_man1_MANS = \
	client.man \
	pgl2ldif.man

client_SOURCES = \
	PolyClt.h \
	PolyClt.cc \
	WssFreezer.h \
	WssFreezer.cc \
	wssFreezers.h \
	wssFreezers.cc

client_LDADD = \
	libcltside.a \
	\
	../app/libapp.a \
	../dns/libdns.a \
	../icp/libicp.a \
	../beep/libbeep.a \
	../cache/libcache.a \
	../kerberos/libkerberos.a \
	../runtime/libagent.a \
	../runtime/libruntime.a \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	../csm/libcsm.a \
	../xml/libxml.a \
	../base/libbase.a \
	../xstd/libxstd.a \
	../server/libsrvside.a \
	\
	@LIB_DL@

# source code example for loadable modules
# TODO: build these, probly during "make test"
EXTRA_DIST = \
	pgl2ldif.h2m \
	HttpBasicAuthenticator.cc \
	SessionAnnouncer.cc

pgl2ldif_SOURCES = \
	pgl2ldif.cc

pgl2ldif_LDADD = \
	libcltside.a \
	\
	../runtime/libruntime.a \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	../base/libbase.a \
	../xstd/libxstd.a

include $(top_srcdir)/common.am
