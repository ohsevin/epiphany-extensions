# -*- coding: utf-8 -*-

# Copyright (C) 2006 by Magnus Therning

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import gtk, gtk.glade
import gconf
import os

import libepilicious
import libepilicious.backend

class ConfigWindow:

    def __init__(self):
        self.gui = gtk.glade.XML(os.path.join(libepilicious.GLADE_DIR, \
                'epilicious-config.glade'), domain=libepilicious.L10N_DOMAIN)
        self.gui.signal_autoconnect(self)

        # I can't remember how to get the whole friggin' lot of 'em...
        self.config = self.gui.get_widget('dlgConfig')
        self.entname = self.gui.get_widget('entName')
        self.entpassword = self.gui.get_widget('entPassword')
        self.cbbackend = self.gui.get_widget('cbBackend')
        self.enttag = self.gui.get_widget('entTag')
        self.rbinclude = self.gui.get_widget('rbInclude')
        self.rbexclude = self.gui.get_widget('rbExclude')
        #self.btnhelp = self.gui.get_widget('btnHelp')
        self.btnclose = self.gui.get_widget('btnClose')
        self.client = gconf.client_get_default()

        # For some reason this gets messed up if set in Glade
        self.entpassword.set_visibility(False)
        self.entpassword.set_invisible_char('●')

    def show(self):
        self.entname.set_text(self.client.get_string(libepilicious.GCONF_UN))
        self.entpassword.set_text(self.client.get_string(libepilicious.GCONF_PWD))
        self.enttag.set_text(self.client.get_string(libepilicious.GCONF_KW))
        self.rbexclude.set_active(self.client.get_bool(libepilicious.GCONF_EXCL))

        # setting up of the combo box is a bit ugly, it's all about DRY
        for k in libepilicious.backend.stores.keys():
            self.cbbackend.append_text(k)
        self.cbbackend.remove_text(0)
        be = self.client.get_string(libepilicious.GCONF_BACK)
        try:
            i = libepilicious.backend.backends.keys().index(be)
        except ValueError:
            i = 0
        self.cbbackend.set_active(i)

    def hide(self):
        self.config.hide()
        self.client.set_string(libepilicious.GCONF_UN, self.entname.get_text())
        self.client.set_string(libepilicious.GCONF_PWD, self.entpassword.get_text())
        self.client.set_string(libepilicious.GCONF_KW, self.enttag.get_text())
        self.client.set_bool(libepilicious.GCONF_EXCL, self.rbexclude.get_active())
        self.client.set_string(libepilicious.GCONF_BACK, self.cbbackend.get_active_text())
        return False

    ### signal handlers
    def on_entName_focus_out_event(self, widget, event):
        self.client.set_string(libepilicious.GCONF_UN, self.entname.get_text())

    def on_entPassword_focus_out_event(self, widget, event):
        self.client.set_string(libepilicious.GCONF_PWD, self.entpassword.get_text())

    def on_entTag_focus_out_event(self, widget, event):
        self.client.set_string(libepilicious.GCONF_KW, self.enttag.get_text())

    def on_rb_clicked(self, widget):
        self.client.set_bool(libepilicious.GCONF_EXCL, self.rbexclude.get_active())

    def on_cbBackend_changed(self, widget):
        self.client.set_string(libepilicious.GCONF_BACK, self.cbbackend.get_active_text())

    def on_btnHelp_clicked(self, widget):
        # Figure out how to open help and open it to the yet-unwritten
        # Epilicious help file. :)
        pass

    def on_btnClose_clicked(self, widget):
        self.hide()
        del(self)
