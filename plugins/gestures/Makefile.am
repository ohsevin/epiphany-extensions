plugindir = $(libdir)/epiphany/plugins

INCLUDES = \
	-I$(top_srcdir)/include		\
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
	ephy-gestures.c		\
	ephy-gestures.h		\
	stroke.c		\
	stroke.h

gesturesdir = $(pkgdatadir)
gestures_DATA =			\
	ephy-gestures.xml

EXTRA_DIST = $(gestures_DATA)
