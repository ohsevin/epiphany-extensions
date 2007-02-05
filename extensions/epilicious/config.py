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

import libepilicious
import gtk, gtk.glade
import gconf
import os

class ConfigWindow:

    def __init__(self):
        self.gui = gtk.glade.XML(os.path.join(libepilicious.GLADE_DIR, \
                'epilicious-config.glade'), domain='epilicious')
        self.gui.signal_autoconnect(self)

        # I can't remember how to get the whole friggin' lot of 'em...
        self.config = self.gui.get_widget('dlgConfig')
        self.entname = self.gui.get_widget('entName')
        self.entpassword = self.gui.get_widget('entPassword')
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
        self.entname.set_text(self.client.get_string('/apps/epiphany/extensions/epilicious/username'))
        self.entpassword.set_text(self.client.get_string('/apps/epiphany/extensions/epilicious/password'))
        self.enttag.set_text(self.client.get_string('/apps/epiphany/extensions/epilicious/keyword'))
        self.rbexclude.set_active(self.client.get_bool('/apps/epiphany/extensions/epilicious/exclude'))

    def hide(self):
        self.config.hide()
        self.client.set_string('/apps/epiphany/extensions/epilicious/username', self.entname.get_text())
        self.client.set_string('/apps/epiphany/extensions/epilicious/password', self.entpassword.get_text())
        self.client.set_string('/apps/epiphany/extensions/epilicious/keyword', self.enttag.get_text())
        self.client.set_bool('/apps/epiphany/extensions/epilicious/exclude', self.rbexclude.get_active())
        return False

    ### signal handlers
    def on_entName_focus_out_event(self, widget, event):
        self.client.set_string('/apps/epiphany/extensions/epilicious/username', self.entname.get_text())

    def on_entPassword_focus_out_event(self, widget, event):
        self.client.set_string('/apps/epiphany/extensions/epilicious/password', self.entpassword.get_text())

    def on_entTag_focus_out_event(self, widget, event):
        self.client.set_string('/apps/epiphany/extensions/epilicious/keyword', self.enttag.get_text())

    def on_rb_clicked(self, widget):
        self.client.set_bool('/apps/epiphany/extensions/epilicious/exclude', self.rbexclude.get_active())

    def on_btnHelp_clicked(self, widget):
        # Figure out how to open help and open it to the yet-unwritten
        # Epilicious help file. :)
        pass

    def on_btnClose_clicked(self, widget):
        self.hide()
        del(self)
