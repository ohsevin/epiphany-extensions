/*
 *  Copyright (C) 2003 David Bordoley
 *  Copyright (C) 2003 Christian Persch
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

#ifndef EPHY_TABS_MENU_EXTENSION_H
#define EPHY_TABS_MENU_EXTENSION_H

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define EPHY_TYPE_TABS_MENU_EXTENSION			(ephy_tabs_menu_extension_get_type())
#define EPHY_TABS_MENU_EXTENSION(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_TABS_MENU_EXTENSION, EphyTabsMenuExtension))
#define EPHY_TABS_MENU_EXTENSION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_TABS_MENU_EXTENSION, EphyTabsMenuExtensionClass))
#define EPHY_IS_TABS_MENU_EXTENSION(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_TABS_MENU_EXTENSION))
#define EPHY_IS_TABS_MENU_EXTENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_TABS_MENU_EXTENSION))
#define EPHY_TABS_MENU_EXTENSION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_TABS_MENU_EXTENSION, EphyTabsMenuExtensionClass))

typedef struct _EphyTabsMenuExtension		EphyTabsMenuExtension;
typedef struct _EphyTabsMenuExtensionClass	EphyTabsMenuExtensionClass;
typedef struct _EphyTabsMenuExtensionPrivate	EphyTabsMenuExtensionPrivate;

struct _EphyTabsMenuExtensionClass
{
	GObjectClass parent_class;
};

struct _EphyTabsMenuExtension
{
	GObject parent_object;

	EphyTabsMenuExtensionPrivate *priv;
};

GType	ephy_tabs_menu_extension_get_type	(void);

GType	ephy_tabs_menu_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif
