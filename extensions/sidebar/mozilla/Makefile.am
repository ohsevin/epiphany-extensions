noinst_LTLIBRARIES = libsidebarmozilla.la

libsidebarmozilla_la_SOURCES = 	\
	register-component.h 	\
	register-component.cpp 	\
	Sidebar.h		\
	Sidebar.cpp

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	sidebar		\
	xpcom

libsidebarmozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libsidebarmozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
