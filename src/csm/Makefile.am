#
# Content Simulation Module
#

noinst_LIBRARIES = \
	libcsm.a

libcsm_a_SOURCES = \
	BodyIter.h \
	BodyIter.cc \
	ContentCfg.h \
	ContentCfg.cc \
	TextDbase.h \
	TextDbase.cc \
	ContentDbase.h \
	ContentDbase.cc \
	ContentMgr.h \
	ContentMgr.cc \
	ContentTypeIdx.h \
	ContentTypeIdx.cc \
	ContentSel.h \
	ContentSel.cc \
	EmbedContMdl.h \
	EmbedContMdl.cc \
	ObjLifeCycle.h \
	ObjLifeCycle.cc \
	RamFile.h \
	RamFiles.h \
	RamFiles.cc \
	\
	InjectIter.h \
	InjectIter.cc \
	\
	RndBodyIter.h \
	RndBodyIter.cc \
	CdbBodyIter.h \
	CdbBodyIter.cc \
	ContainerBodyIter.h \
	ContainerBodyIter.cc \
	RamFileBodyIter.h \
	RamFileBodyIter.cc \
	\
	GzipEncoder.h \
	GzipEncoder.cc \
	cdbBuilders.h \
	cdbBuilders.cc \
	\
	XmlTagIdentifier.h \
	XmlTagIdentifier.cc \
	XmlTagParser.h \
	XmlTagParser.cc \
	XmlParser.h \
	XmlParser.cc \
	\
	cdbEntries.h \
	cdbEntries.cc \
	oid2Url.h \
	oid2Url.cc \
	\
	RangeBodyIter.h \
	RangeBodyIter.cc

bin_PROGRAMS = \
	cdb

dist_man1_MANS = \
	cdb.man

EXTRA_DIST = \
	cdb.h2m

cdb_SOURCES = \
	cdb.cc

LDADD = \
	libcsm.a \
	\
	../runtime/libruntime.a \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	../base/libbase.a \
	../xstd/libxstd.a



include $(top_srcdir)/common.am
