consoledir = $(EXTENSIONS_DIR)
console_PYTHON = \
	console.py

extensionsxmldir = $(EXTENSIONS_DIR)
extensionsxml_in_files = python-console.xml.in
extensionsxml_DATA = $(extensionsxml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(extensionsxml_in_files)

CLEANFILES = $(extensionsxml_DATA)
DISTCLEANFILES = $(extensionsxml_DATA)
