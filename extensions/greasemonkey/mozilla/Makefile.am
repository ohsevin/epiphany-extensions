noinst_LTLIBRARIES = libgreasemonkeymozilla.la

libgreasemonkeymozilla_la_SOURCES = 	\
	mozilla-helpers.cpp		\
	mozilla-helpers.h

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	xpcom

libgreasemonkeymozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libgreasemonkeymozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
