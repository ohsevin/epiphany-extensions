extensiondir = $(libdir)/epiphany/extensions

INCLUDES =	\
   	-I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"pkgdatadir\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libdashboardextension.la

libdashboardextension_la_SOURCES =	\
	ephy-dashboard.c		\
     	ephy-dashboard-extension.c	\
	ephy-dashboard-extension.h	\
	dashboard-frontend-xmlwriter.h	\
	dashboard-frontend-xmlwriter.c
