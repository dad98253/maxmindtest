## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4 --install

EXTRA_DIST = m4/Makefile.in README.md LICENSE build-aux/Makefile.in

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

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
__top_builddir__bin_maxmindtest_LDFLAGS = -Wl,-Map,loadmap.txt -lcsv -lmaxminddb

help:
	-@echo "make                    --> build maxmindtest"
	-@echo "make clean              --> delete what was built"
	-@echo "make distclean          --> delete auto tools products"
	-@echo "make dist               --> build distribution package"

AM_CPPFLAGS = $(DEPS_CFLAGS) $(MY_CPPFLAGS)
AM_CXXFLAGS = -W -Wall -fmessage-length=0

	
