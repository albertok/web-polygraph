#
# Various utilities
#

bin_PROGRAMS = \
	aka \
	pop-test \
	distr-test \
	rng-test \
	dns-cfg \
	pgl2ips

dist_man1_MANS = \
	aka.man \
	pop-test.man \
	distr-test.man \
	rng-test.man \
	dns-cfg.man \
	pgl2ips.man

EXTRA_DIST = \
	aka.h2m \
	rng-test.h2m \
	dns-cfg.h2m \
	pgl2ips.h2m

pop_test_SOURCES = \
	pop_test.cc \
	IntIntHash.h \
	IntIntHash.cc


distr_test_SOURCES = \
	distr_test.cc 

rng_test_SOURCES = \
	rng_test.cc

LDADD = \
	../base/libbase.a \
	../xstd/libxstd.a


aka_SOURCES = \
	aka.cc

aka_LDADD = \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	\
	$(LDADD)
	
dns_cfg_SOURCES = \
	dns_cfg.cc

dns_cfg_LDADD = \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	\
	$(LDADD)

pgl2ips_SOURCES = \
	pgl2ips.cc

pgl2ips_LDADD = \
	../pgl/libpgl.a \
	../xparser/libxparser.a \
	../runtime/libruntime.a \
	\
	$(LDADD)



include $(top_srcdir)/common.am
