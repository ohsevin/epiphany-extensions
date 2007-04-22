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
import os.path

class ProgressBar:

    def __init__(self):
        self.gui = gtk.glade.XML(os.path.join(libepilicious.GLADE_DIR, \
                'epilicious-progress.glade'), domain=libepilicious.L10N_DOMAIN)
        self.gui.signal_autoconnect(self)

        self.dlg = self.gui.get_widget('dlgProgress')

    def show(self):
        self.dlg.show()

    def hide(self):
        self.dlg.hide()

    def step(self):
        images = ['imgStep1', 'imgStep2', 'imgStep3', 'imgStep4', 'imgStep5', 'imgStep6',]
        for i in images:
            img = self.gui.get_widget(i)
            img.show()
            while gtk.events_pending():
                gtk.main_iteration()
            yield True
            img.set_from_icon_name('gtk-apply', gtk.ICON_SIZE_MENU)
        self.gui.get_widget('btnClose').set_sensitive(True)
        while gtk.events_pending():
            gtk.main_iteration()
        yield True

    def failed(self):
        self.dlg.set_title(self.dlg.get_title() + ' FAILED!')
        self.gui.get_widget('btnClose').set_sensitive(True)

    ### signal handlers
    def on_btnClose_clicked(self, widget):
        self.hide()
        del(self)
