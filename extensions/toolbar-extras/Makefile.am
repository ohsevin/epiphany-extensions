extensiondir = $(libdir)/epiphany/extensions

INCLUDES = \
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libtoolbarextrasextension.la

libtoolbarextrasextension_la_SOURCES = \
	ephy-toolbar-extras-extension.h	\
	ephy-toolbar-extras-extension.c	\
	ephy-multi-smart-action.h	\
	ephy-multi-smart-action.c	\
	toolbar-extras.c

