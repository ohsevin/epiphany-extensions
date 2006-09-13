/*
 *  Copyright © 2005 Raphaël Slinckx <raphael@slinckx.net>
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

#ifndef EPHY_RSS_EXTENSION_H
#define EPHY_RSS_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

/* How to contact feed readers via dbus */
#define RSS_DBUS_SERVICE        "org.gnome.feed.Reader"
#define RSS_DBUS_OBJECT_PATH    "/org/gnome/feed/Reader"
#define RSS_DBUS_INTERFACE      "org.gnome.feed.Reader"
#define RSS_DBUS_SUBSCRIBE      "Subscribe"

G_BEGIN_DECLS

#define EPHY_TYPE_RSS_EXTENSION		(ephy_rss_extension_get_type ())
#define EPHY_RSS_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_CAST((o), EPHY_TYPE_RSS_EXTENSION, EphyRssExtension))
#define EPHY_RSS_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_RSS_EXTENSION, EphyRssExtensionClass))
#define EPHY_IS_RSS_EXTENSION(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), EPHY_TYPE_RSS_EXTENSION))
#define EPHY_IS_RSS_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), EPHY_TYPE_RSS_EXTENSION))
#define EPHY_RSS_EXTENSION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), EPHY_TYPE_RSS_EXTENSION, EphyRssExtensionClass))

typedef struct _EphyRssExtension	EphyRssExtension;
typedef struct _EphyRssExtensionClass	EphyRssExtensionClass;
typedef struct _EphyRssExtensionPrivate	EphyRssExtensionPrivate;

struct _EphyRssExtensionClass
{
	GObjectClass parent_class;
};

struct _EphyRssExtension
{
	GObject parent_instance;

	/*< private >*/
	EphyRssExtensionPrivate *priv;
};

GType	ephy_rss_extension_get_type		(void);

GType	ephy_rss_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif
