INCLUDES = \
	-I..						\
	-I$(top_srcdir)/include				\
	$(MOZILLA_COMPONENT_CFLAGS)			\
	-I$(MOZILLA_INCLUDE_ROOT)			\
	-I$(MOZILLA_INCLUDE_ROOT)/content		\
	-I$(MOZILLA_INCLUDE_ROOT)/docshell		\
	-I$(MOZILLA_INCLUDE_ROOT)/necko			\
        -I$(MOZILLA_INCLUDE_ROOT)/pipnss		\
	-I$(MOZILLA_INCLUDE_ROOT)/pipboot		\
	-I$(MOZILLA_INCLUDE_ROOT)/uriloader		\
	-I$(MOZILLA_INCLUDE_ROOT)/webbrwsr		\
	-I$(MOZILLA_INCLUDE_ROOT)/webshell		\
	-I$(MOZILLA_INCLUDE_ROOT)/xpcom			\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 			\
	-DSHARE_DIR=\"$(pkgdatadir)\"   		\
	-include $(MOZILLA_INCLUDE_ROOT)/mozilla-config.h

if MAINTAINER_MODE
INCLUDES += -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED $(WARN_CXXFLAGS)
endif

noinst_LTLIBRARIES = libmozillacertificates.la

libmozillacertificates_la_SOURCES = \
	mozilla-embed-certificate.cpp		\
	mozilla-embed-certificate.h		\
	MozillaEmbedCertificate.cpp		\
	MozillaEmbedCertificate.h
