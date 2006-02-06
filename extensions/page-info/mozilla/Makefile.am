noinst_LTLIBRARIES = libpageinfomozilla.la

libpageinfomozilla_la_SOURCES = \
	mozilla-helpers.cpp	\
	mozilla-helpers.h	\
	PageInfoPrivate.cpp	\
	PageInfoPrivate.h

mozilla_include_subdirs = \
	.		\
	chardet		\
	content		\
	dom		\
	gtkembedmoz	\
	layout		\
	nkcache		\
	necko		\
	pref		\
	uconv		\
	webbrwsr

libpageinfomozilla_la_CPPFLAGS = \
	-I..						\
	-I$(top_srcdir)/include				\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libpageinfomozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
