noinst_LTLIBRARIES = liberrorviewermozilla.la

liberrorviewermozilla_la_SOURCES = \
	ErrorViewerConsoleListener.cpp		\
	ErrorViewerConsoleListener.h		\
	ErrorViewerURICheckerObserver.cpp	\
	ErrorViewerURICheckerObserver.h		\
	register-listener.cpp			\
	register-listener.h			\
	mozilla-helpers.cpp			\
	mozilla-helpers.h			\
	mozilla-link-checker.cpp		\
	mozilla-link-checker.h

mozilla_include_subdirs = \
	.		\
	dom		\
	gtkembedmoz	\
	jsconsole	\
	necko		\
	webbrwsr	\
	xpconnect

liberrorviewermozilla_la_CPPFLAGS = \
	-I..						\
	-I$(top_srcdir)/include				\
	-I$(top_srcdir)/extensions/error-viewer/mozilla	\
	-I$(top_srcdir)/extensions/error-viewer		\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

liberrorviewermozilla_la_CXXFLAGS = \
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)

if HAVE_GECKO_1_9
if HAVE_GECKO_XPCOM_GLUE
liberrorviewermozilla_la_CPPFLAGS += \
 	-DXPCOM_GLUE_USE_NSPR
endif
endif
