INCLUDES = \
	-I..						\
	-I$(top_srcdir)/include				\
	$(MOZILLA_COMPONENT_CFLAGS)			\
	-I$(MOZILLA_INCLUDE_ROOT)/content		\
	-I$(MOZILLA_INCLUDE_ROOT)/docshell		\
	-I$(MOZILLA_INCLUDE_ROOT)/dom			\
	-I$(MOZILLA_INCLUDE_ROOT)/xpcom			\
	-I$(MOZILLA_INCLUDE_ROOT)/sidebar		\
	-I$(MOZILLA_INCLUDE_ROOT)			\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 			\
	-DSHARE_DIR=\"$(pkgdatadir)\"

if MAINTAINER_MODE
INCLUDES += -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED $(WARN_CXXFLAGS)
endif

noinst_LTLIBRARIES = libsidebarmozilla.la

libsidebarmozilla_la_SOURCES = 	\
	register-component.h 	\
	register-component.cpp 	\
	Sidebar.h		\
	Sidebar.cpp
