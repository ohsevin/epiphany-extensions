SUBDIRS = mozilla

extensiondir = $(libdir)/epiphany/extensions

INCLUDES = \
	-Imozilla			\
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libcertificatesextension.la

libcertificatesextension_la_SOURCES = \
	ephy-certificates-extension.c	\
	ephy-certificates-extension.h	\
	extension.c

libcertificatesextension_la_LIBADD = \
	mozilla/libmozillacertificates.la	\
	$(MOZILLA_COMPONENT_LIBS)
