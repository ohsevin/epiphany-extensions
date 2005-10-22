noinst_LTLIBRARIES = libadblockmozilla.la

libadblockmozilla_la_SOURCES = 	\
	MozAdBlocker.cpp	\
	MozAdBlocker.h		\
	mozilla-helpers.cpp	\
	mozilla-helpers.h

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	necko		\
	xpcom

libadblockmozilla_la_CPPFLAGS = \
	-I$(top_srcdir)/extensions/adblock		\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libadblockmozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
