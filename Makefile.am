## Process this file with automake to produce Makefile.in


#SUBDIRS = build-aux
#AC_CONFIG_AUX_DIR = build-aux

ACLOCAL_AMFLAGS = -I m4 --install
#lib_LTLIBRARIES = libpasswin-1.0.a
#libpasswin_1_0_a_SOURCES = xxx
#libpasswin_1_0_a_LDFLAGS = -version-info 0:0:0
#include_HEADERS = xxx
#SUBDIRS = src


EXTRA_DIST = m4/Makefile.in README.md LICENSE build-aux/Makefile.in
	

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

#build passwin library
#lib_LIBRARIES = $(top_builddir)/lib/libpass.a
#libtre_a_LDFLAGS = -no-undefined -version-info 5:0:0 $(LDFLAGS)
#__top_builddir__lib_libpass_a_LIBADD = $(LIBINTL)
#__top_builddir__lib_libpass_a_SOURCES = $(PASSSOURCES)

@GENERATE_CHANGELOG_RULES@
  dist-hook: dist-ChangeLog

distclean-local:
	@rm config.status configure config.log
	@rm Makefile
	@rm -r autom4te.cache/
	@rm aclocal.m4
	@rm Makefile.in

#build maxmindtest
bin_PROGRAMS = $(top_builddir)/bin/maxmindtest 
#add in main()
__top_builddir__bin_maxmindtest_SOURCES = maxmindtest.c
#__top_builddir__bin_maxmindtest_LDADD = $(top_builddir)/lib/libpass.a
__top_builddir__bin_maxmindtest_LDFLAGS = -Wl,-Map,loadmap.txt -lcsv -lmaxminddb

#add header files
#noinst_HEADERS = $(PASSINCLUDES)

help:
	-@echo "make                    --> build maxmindtest"
	-@echo "make clean              --> delete what was built"
	-@echo "make distclean          --> delete auto tools products"
	-@echo "make dist               --> build distribution package"
# __top_builddir__bin_classifier_SOURCES = Classifier.h \
# 	Classifier.cpp \
# 	SequenceInfo.h \
# 	SequenceInfo.cpp \
# 	KmerMatcher.h \
# 	KmerMatcher.cpp \
# 	KmerMatcherOptions.h \
# 	KmerMatcherOptions.cpp \
# 	Timer.h \
# 	Timer.cpp \
# 	types.hpp \
# 	custom_utils.hpp \
# 	data_structures.hpp

#__top_builddir__bin_passwin_LDADD = lib/libpass.a $(lib_LTLIBRARIES)
#AM_CPPFLAGS = -DTEXTMODEONLY -DNOECBM2X -I"/home/dad/repositories/linux/passwin/tutlib2" -I"/home/dad/repositories/linux/treStatic" -I"/home/dad/repositories/linux/passwin/Resourcelib"
AM_CPPFLAGS = $(DEPS_CFLAGS) $(MY_CPPFLAGS)
AM_CXXFLAGS = -W -Wall -fmessage-length=0
#AM_LDFLAGS = -static
#AM_LIBS = $(DEPS_LIBS)

	
