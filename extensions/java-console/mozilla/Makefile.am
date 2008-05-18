noinst_LTLIBRARIES = libjavaconsolemozilla.la

libjavaconsolemozilla_la_SOURCES = 	\
	java-console.cpp		\
	java-console.h

mozilla_include_subdirs = \
	.		\
	java		\
	oji		\
	xpcom

libjavaconsolemozilla_la_CPPFLAGS = \
	-I..			\
	-I$(top_srcdir)/include	\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

libjavaconsolemozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)

if HAVE_GECKO_1_9
if HAVE_GECKO_XPCOM_GLUE
libjavaconsolemozilla_la_CPPFLAGS += \
 	-DXPCOM_GLUE_USE_NSPR
endif
endif
