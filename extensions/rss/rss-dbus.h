/*
 *  Copyright (C) 2005 Raphaël Slinckx <raphael@slinckx.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifndef RSS_DBUS_H
#define RSS_DBUS_H

#include <glib.h>

G_BEGIN_DECLS

/* How to contact feed readers via dbus */
#define RSS_DBUS_SERVICE	"org.gnome.feed.Reader"
#define	RSS_DBUS_OBJECT_PATH	"/org/gnome/feed/Reader"
#define	RSS_DBUS_INTERFACE	"org.gnome.feed.Reader"
#define	RSS_DBUS_SUBSCRIBE	"Subscribe"

gboolean rss_dbus_subscribe_feed (const char *address);

G_END_DECLS

#endif
