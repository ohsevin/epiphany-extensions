INCLUDES = \
	-I..						\
	-I$(top_srcdir)/include				\
	$(MOZILLA_COMPONENT_CFLAGS)			\
	-I$(MOZILLA_INCLUDE_ROOT)			\
	-I$(MOZILLA_INCLUDE_ROOT)/content		\
	-I$(MOZILLA_INCLUDE_ROOT)/docshell		\
	-I$(MOZILLA_INCLUDE_ROOT)/dom			\
	-I$(MOZILLA_INCLUDE_ROOT)/xpcom			\
	-I$(MOZILLA_INCLUDE_ROOT)/string		\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 			\
	-DSHARE_DIR=\"$(pkgdatadir)\"   		\
	-include $(MOZILLA_INCLUDE_ROOT)/mozilla-config.h

if MAINTAINER_MODE
INCLUDES += -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED $(WARN_CXXFLAGS)
endif

noinst_LTLIBRARIES = libsamplemozillamozilla.la

libsamplemozillamozilla_la_SOURCES = 	\
	mozilla-sample.cpp		\
	mozilla-sample.h
