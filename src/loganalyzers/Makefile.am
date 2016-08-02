## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	reporter

dist_man1_MANS = \
	reporter.man

EXTRA_PROGRAMS = comparator

if ENABLE_COMPARATOR
bin_PROGRAMS += comparator

comparator_SOURCES = \
	comparator.cc \
	CompOpts.cc

comparator_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/src/Hapy/src/include

comparator_LDADD = \
	libloganalyzers.la \
	../xml/libxml.a \
	../Hapy/src/libHapy.la \
	../base/libbase.a \
	../xstd/libxstd.a \
	@LIBOBJS@
endif


noinst_LTLIBRARIES = \
	libloganalyzers.la

libloganalyzers_la_SOURCES = \
	InfoScope.cc \
	InfoScopeDim.cc \
	InfoScopes.cc \
	Stex.cc \
	LoadStex.cc \
	LoadStexes.cc \
	\
	SomeInfo.cc \
	TestInfo.cc \
	SideInfo.cc \
	ProcInfo.cc \
	PhaseInfo.cc \
	PhaseTrace.cc \
	\
	Plotter.cc \
	ImagePlotter.cc \
	JavascriptPlotter.cc \
	ReportFigure.cc \
	TmSzHistFig.cc \
	RptmHistFig.cc \
	SizeHistFig.cc \
	ReportTraceFigure.cc \
	PointTraceFig.cc \
	RptmTraceFig.cc \
	LevelTraceFig.cc \
	LoadTraceFig.cc \
	HistogramFigure.cc \
	ScatteredFig.cc \
	\
	ReportBlob.cc \
	BlobIdx.cc \
	BlobDb.cc \
	\
	SectionState.cc \
	RepToHtmlFile.cc \
	\
	RepOpts.cc \
	\
	Sample.cc \
	Panorama.cc \
	Formatter.cc \
	\
	StatTable.cc


reporter_SOURCES = \
	reporter.cc

noinst_HEADERS = \
	BlobDb.h \
	BlobIdx.h \
	CompOpts.h \
	Formatter.h \
	ImagePlotter.h \
	InfoScope.h \
	InfoScopeDim.h \
	InfoScopes.h \
	JavascriptPlotter.h \
	StexBase.h \
	PointStex.h \
	HistStex.h \
	LevelStex.h \
	LevelTraceFig.h \
	LoadStex.h \
	LoadStexes.h \
	LoadTraceFig.h \
	Panorama.h \
	PhaseInfo.h \
	PhaseTrace.h \
	Plotter.h \
	ProcInfo.h \
	RepOpts.h \
	RepToHtmlFile.h \
	ReportBlob.h \
	ReportFigure.h \
	ReportTraceFigure.h \
	PointTraceFig.h \
	HistogramFigure.h \
	ScatteredFig.h \
	RptmHistFig.h \
	RptmTraceFig.h \
	Sample.h \
	SectionState.h \
	SideInfo.h \
	SizeHistFig.h \
	SomeInfo.h \
	Stex.h \
	TestInfo.h \
	TmSzHistFig.h \
	StatTable.h


reporter_LDADD = \
	libloganalyzers.la \
	../logextractors/liblogextractors.a \
	../runtime/libruntime.a \
	../xml/libxml.a \
	../base/libbase.a \
	../xstd/libxstd.a \
	@LIBOBJS@

dist_javascripts_DATA = \
	ReportFigures.js

dist_stylesheets_DATA = \
	Print.css \
	ReportFigures.css

include $(top_srcdir)/common.am
