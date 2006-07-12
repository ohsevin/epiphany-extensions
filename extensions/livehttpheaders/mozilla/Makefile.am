noinst_LTLIBRARIES = liblivehttpheadersmozilla.la

liblivehttpheadersmozilla_la_SOURCES = 	\
	mozilla-helper.cpp		\
	mozilla-helper.h                \
	LiveHTTPHeadersListener.cpp	\
	LiveHTTPHeadersListener.h

mozilla_include_subdirs = \
	.		\
	necko		\
	xpcom

liblivehttpheadersmozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

liblivehttpheadersmozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
