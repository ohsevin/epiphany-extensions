INCLUDES = \
	-I..					\
	-I$(top_srcdir)/include			\
	$(EPIPHANY_DEPENDENCY_CFLAGS)		\
	-DSHARE_DIR=\"$(pkgdatadir)\"		\
	-include $(MOZILLA_INCLUDE_ROOT)/mozilla-config.h

if MAINTAINER_MODE
INCLUDES += -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED $(WARN_CXXFLAGS)
endif

noinst_LTLIBRARIES = liberrorvieweropensp.la

liberrorvieweropensp_la_SOURCES = \
	validate.cpp	\
	validate.h
