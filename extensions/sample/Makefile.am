extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libsampleextension.la

libsampleextension_la_SOURCES = \
	ephy-sample-extension.c	\
	ephy-sample-extension.h	\
	sample.c

EXTRA_DIST = spinner-ex-extension.diff

