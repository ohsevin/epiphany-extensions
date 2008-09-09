#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  Copyright © 2005 Christian Persch
#  Copyright © 2008 Stefan Stuhr
#  Copyright © 2008 Diego Escalante Urrelo
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

import epiphany
import urlparse

backend = epiphany.ephy_shell_get_default().get_embed_single().get_backend_name()

def net_stop_cb(embed, status):
    icon_address = embed.get_property('icon_address')

    if icon_address is None or not icon_address and \
       embed.get_property('load-status'):
        try:
            uriparts = urlparse.urlsplit(embed.get_property('address'))

            if uriparts[0] == 'http' or uriparts[0] == 'https':
                url = urlparse.urlunsplit(uriparts[:2] + ('/favicon.ico', '', ''))
                embed.set_property('icon_address', url)
        except Exception:
            pass

def attach_tab(window, embed):
    handler_id = embed.connect('notify::load-status', net_stop_cb)
    embed._favicon_details = [handler_id]

def detach_tab(window, embed):
    [handler_id] = embed._favicon_details
    del embed._favicon_details
    embed.disconnect(handler_id)
