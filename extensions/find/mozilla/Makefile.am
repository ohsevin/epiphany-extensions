noinst_LTLIBRARIES = libfindmozilla.la

libfindmozilla_la_SOURCES = \
	mozilla-find.cpp	\
	mozilla-find.h

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	find		\
	xpcom

libfindmozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libfindmozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
