extensiondir = $(libdir)/epiphany/extensions

INCLUDES = 				\
	-I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libgesturesextension.la

libgesturesextension_la_SOURCES = 	\
	ephy-gesture.c			\
	ephy-gesture.h			\
	ephy-gestures-extension.c	\
	ephy-gestures-extension.h	\
	gestures.c			\
	stroke.c			\
	stroke.h

gesturesdir = $(pkgdatadir)
gestures_DATA =				\
	ephy-gestures.xml

EXTRA_DIST = $(gestures_DATA)
