#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  Copyright © 2005 Christian Persch
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

def net_stop_cb(embed):
	if embed.get_property('icon-address') == None or len(embed.get_property('icon-address')) < 1:
		try:
			uri = gnomevfs.URI(embed.get_property('address'))
			single = epiphany.ephy_shell_get_default().get_embed_single()
			backend = single.get_backend_name();
			if uri.scheme == "http" or (uri.scheme == "https" and backend != "gecko-1.7") :
				url = uri.scheme + "://" + uri.host_name
				if uri.host_port != 0:
					url += ':%d' % uri.host_port
				url += "/favicon.ico"
				embed.set_property('icon_address', url)
		except Exception:
			pass

def attach_tab(window, embed):
	handler_id = embed.connect("net_stop", net_stop_cb)
	embed._favicon_details = [ handler_id ]

def detach_tab(window, embed):
	[ handler_id ] = embed._favicon_details
	del embed._favicon_details
	embed.disconnect(handler_id)
