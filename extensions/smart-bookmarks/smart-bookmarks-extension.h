/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2004 Jean-François Rameau
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

#ifndef SMART_BOOKMARKS_EXTENSION_H
#define SMART_BOOKMARKS_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_SMART_BOOKMARKS_EXTENSION		(smart_bookmarks_extension_get_type ())
#define SMART_BOOKMARKS_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SMART_BOOKMARKS_EXTENSION, SmartBookmarksExtension))
#define SMART_BOOKMARKS_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), TYPE_SMART_BOOKMARKS_EXTENSION, SmartBookmarksExtensionClass))
#define IS_SMART_BOOKMARKS_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SMART_BOOKMARKS_EXTENSION))
#define IS_SMART_BOOKMARKS_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SMART_BOOKMARKS_EXTENSION))
#define SMART_BOOKMARKS_EXTENSION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SMART_BOOKMARKS_EXTENSION, SmartBookmarksExtensionClass))

typedef struct SmartBookmarksExtension		SmartBookmarksExtension;
typedef struct SmartBookmarksExtensionClass	SmartBookmarksExtensionClass;
typedef struct SmartBookmarksExtensionPrivate	SmartBookmarksExtensionPrivate;

struct SmartBookmarksExtensionClass
{
	GObjectClass parent_class;
};

struct SmartBookmarksExtension
{
	GObject parent_instance;

	/*< private >*/
	SmartBookmarksExtensionPrivate *priv;
};

GType	smart_bookmarks_extension_get_type	(void);

GType	smart_bookmarks_extension_register_type (GTypeModule *module);

G_END_DECLS

#endif
