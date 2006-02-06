noinst_LTLIBRARIES = libselectstylesheetmozilla.la

libselectstylesheetmozilla_la_SOURCES = \
	mozilla-helpers.cpp	\
	mozilla-helpers.h

mozilla_include_subdirs = \
	.		\
	dom		\
	gtkembedmoz	\
	necko		\
	nkcache		\
	webbrwsr

libselectstylesheetmozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libselectstylesheetmozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
