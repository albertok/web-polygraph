#
# Tools to extract information from binary logs
#

noinst_LIBRARIES = \
	liblogextractors.a

liblogextractors_a_SOURCES = \
	LogCatFilter.h \
	LogCatFilter.cc \
	LogIter.h \
	LogIter.cc \
	matchAndPrint.h \
	matchAndPrint.cc \
	\
	AggrInfoLoader.h \
	AggrInfoLoader.cc


bin_PROGRAMS = \
	lx \
	lr \
	ltrace

dist_man1_MANS = \
	lx.man \
	lr.man \
	ltrace.man

EXTRA_DIST = \
	lx.h2m \
	lr.h2m \
	ltrace.h2m

lx_SOURCES = \
	lx.cc

LDADD = \
	liblogextractors.a \
	\
	../base/libbase.a \
	../xstd/libxstd.a

lr_SOURCES = \
	lr.cc

ltrace_SOURCES = \
	ltrace.cc


include $(top_srcdir)/common.am
