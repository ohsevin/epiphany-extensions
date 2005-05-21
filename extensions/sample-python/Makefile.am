sampledir = $(EXTENSIONS_DIR)
sample_PYTHON = \
	sample-python.py		\
	sample-python-location.py	\
	sample-python-menu.py		\
	sample-python-statusbar.py

extensionsxmldir = $(EXTENSIONS_DIR)
extensionsxml_in_files = \
	sample-python.xml.in		\
	sample-python-location.xml.in	\
	sample-python-menu.xml.in	\
	sample-python-statusbar.xml.in
extensionsxml_DATA = $(extensionsxml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(extensionsxml_in_files)

CLEANFILES = $(extensionsxml_DATA)
DISTCLEANFILES = $(extensionsxml_DATA)
