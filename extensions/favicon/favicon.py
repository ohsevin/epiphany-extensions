#!/usr/bin/env python

#  Copyright (C) 2005 Christian Persch
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#  $Id$

import epiphany;
import gnomevfs;

def net_stop_cb(embed, tab):
	if tab.get_icon_address() == None:
		try:
			uri = gnomevfs.URI(tab.get_address())
			if uri.scheme == "http":
				url = "http://" + uri.host_name
				if uri.host_port != 0:
					url += ':%d' % uri.host_port;
				url += "/favicon.ico"
				tab.set_icon_address(url);
		except Exception:
			pass

def attach_tab(window, tab):
	embed = tab.get_embed()
	handler_id = embed.connect("net_stop", net_stop_cb, tab)
	tab._favicon_details = [ handler_id ]

def detach_tab(window, tab):
	[ handler_id ] = tab._favicon_details
	del tab._favicon_details
	embed = tab.get_embed()
	embed.disconnect(handler_id)
