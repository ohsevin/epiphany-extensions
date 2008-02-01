NULL =

private_icons = \
	hicolor_status_16x16_ad-blocked.png \
	hicolor_status_16x16_feed-presence.png \
	hicolor_status_22x22_feed-presence.png \
	hicolor_status_22x22_ad-blocked.png \
	hicolor_status_24x24_feed-presence.png \
	hicolor_status_24x24_ad-blocked.png \
	hicolor_status_32x32_feed-presence.png \
	hicolor_status_scalable_feed-presence.svg \
	$(NULL)

noinst_DATA = \
	hicolor_status_16x16_ad-blocked.svg \
	hicolor_status_22x22_ad-blocked.png \
	$(NULL)

EXTRA_DIST = \
	$(public_icons)		\
	$(private_icons)	\
	$(noinst_DATA)		\
	$(NULL)

###############################################################################

install-icons:
	for icon in $(private_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(EPIPHANY_ICONDIR)/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(EPIPHANY_ICONDIR)/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done

uninstall-icons:
	-for icon in $(private_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		rm -f $(DESTDIR)$(EPIPHANY_ICONDIR)/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done

install-data-local: install-icons

uninstall-local: uninstall-icons
