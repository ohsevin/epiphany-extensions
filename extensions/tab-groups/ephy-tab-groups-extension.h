/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003, 2004 Christian Persch
 *  Copyright © 2004 Justin Wake
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

#ifndef EPHY_TAB_GROUPS_EXTENSION_H
#define EPHY_TAB_GROUPS_EXTENSION_H

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define EPHY_TYPE_TAB_GROUPS_EXTENSION			(ephy_tab_groups_extension_get_type())
#define EPHY_TAB_GROUPS_EXTENSION(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_TAB_GROUPS_EXTENSION, EphyTabGroupsExtension))
#define EPHY_TAB_GROUPS_EXTENSION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_TAB_GROUPS_EXTENSION, EphyTabGroupsExtensionClass))
#define EPHY_IS_TAB_GROUPS_EXTENSION(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_TAB_GROUPS_EXTENSION))
#define EPHY_IS_TAB_GROUPS_EXTENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_TAB_GROUPS_EXTENSION))
#define EPHY_TAB_GROUPS_EXTENSION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_TAB_GROUPS_EXTENSION, EphyTabGroupsExtensionClass))

typedef struct _EphyTabGroupsExtension		EphyTabGroupsExtension;
typedef struct _EphyTabGroupsExtensionClass	EphyTabGroupsExtensionClass;
typedef struct _EphyTabGroupsExtensionPrivate	EphyTabGroupsExtensionPrivate;

struct _EphyTabGroupsExtensionClass
{
	GObjectClass	parent_class;
};

struct _EphyTabGroupsExtension
{
	GObject	parent_object;
};

GType	ephy_tab_groups_extension_get_type	(void);

GType	ephy_tab_groups_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif
