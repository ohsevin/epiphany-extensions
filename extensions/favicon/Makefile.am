favicondir = $(EXTENSIONS_DIR)
favicon_PYTHON = \
	favicon.py

extensionsxmldir = $(EXTENSIONS_DIR)
extensionsxml_in_files = favicon.xml.in.in
extensionsxml_DATA = $(extensionsxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(extensionsxml_in_files)

CLEANFILES = $(extensionsxml_DATA)
DISTCLEANFILES = $(extensionsxml_DATA)
