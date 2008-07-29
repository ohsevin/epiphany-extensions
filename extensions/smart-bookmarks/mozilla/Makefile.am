noinst_LTLIBRARIES = libsbmozilla.la

libsbmozilla_la_SOURCES = 	\
	mozilla-selection.cpp	\
	mozilla-selection.h

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	gtkembedmoz	\
	webbrwsr	\
	widget		\
	xpcom

libsbmozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS) \
	-UGTK_DISABLE_DEPRECATED

libsbmozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)

if HAVE_GECKO_1_9
if HAVE_GECKO_XPCOM_GLUE
libsbmozilla_la_CPPFLAGS += \
 	-DXPCOM_GLUE_USE_NSPR
endif
endif
