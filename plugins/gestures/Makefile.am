plugindir = $(libdir)/epiphany/plugins

INCLUDES = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

plugin_LTLIBRARIES = libgestures.la

libgestures_la_SOURCES = 	\
	gestures.c		\
	gul-gestures.c		\
	gul-gestures.h		\
	gul-gobject-misc.h	\
	stroke.c		\
	stroke.h
