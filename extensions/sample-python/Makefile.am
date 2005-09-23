sampledir = $(EXTENSIONS_DIR)
sample_PYTHON = \
	sample-python.py		\
	sample-python-location.py	\
	sample-python-menu.py		\
	sample-python-statusbar.py

extensionsinidir = $(EXTENSIONS_DIR)
extensionsini_in_files = \
	sample-python.ephy-extension.in		\
	sample-python-location.ephy-extension.in	\
	sample-python-menu.ephy-extension.in	\
	sample-python-statusbar.ephy-extension.in
extensionsini_DATA = $(extensionsini_in_files:.ephy-extension.in=.ephy-extension)

@EPIPHANY_EXTENSION_RULE@

EXTRA_DIST = \
	$(extensionsini_in_files)

CLEANFILES = $(extensionsini_DATA)
DISTCLEANFILES = $(extensionsini_DATA)
