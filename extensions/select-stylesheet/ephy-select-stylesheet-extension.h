/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *  Copyright (C) 2004 Adam Hooper
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

#ifndef EPHY_SELECT_STYLESHEET_EXTENSION_H
#define EPHY_SELECT_STYLESHEET_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_SELECT_STYLESHEET_EXTENSION		(ephy_select_stylesheet_extension_get_type ())
#define EPHY_SELECT_STYLESHEET_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_SELECT_STYLESHEET_EXTENSION, EphySelectStylesheetExtension))
#define EPHY_SELECT_STYLESHEET_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_SELECT_STYLESHEET_EXTENSION, EphySelectStylesheetExtensionClass))
#define EPHY_IS_SELECT_STYLESHEET_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_SELECT_STYLESHEET_EXTENSION))
#define EPHY_IS_SELECT_STYLESHEET_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_SELECT_STYLESHEET_EXTENSION))
#define EPHY_SELECT_STYLESHEET_EXTENSION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_SELECT_STYLESHEET_EXTENSION, EphySelectStylesheetExtensionClass))

typedef struct _EphySelectStylesheetExtension		EphySelectStylesheetExtension;
typedef struct _EphySelectStylesheetExtensionClass	EphySelectStylesheetExtensionClass;
typedef struct _EphySelectStylesheetExtensionPrivate	EphySelectStylesheetExtensionPrivate;

struct _EphySelectStylesheetExtensionClass
{
	GObjectClass parent_class;
};

struct _EphySelectStylesheetExtension
{
	GObject parent_instance;

	/*< private >*/
	EphySelectStylesheetExtensionPrivate *priv;
};

GType	ephy_select_stylesheet_extension_get_type	(void);

GType	ephy_select_stylesheet_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif /* EPHY_SELECT_STYLESHEET_H */
