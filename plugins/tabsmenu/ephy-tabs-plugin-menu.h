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
 * 
 */

#ifndef EPHY_TABS_PLUGIN_MENU_H
#define EPHY_TABS_PLUGIN_MENU_H

#include "egg-action-group.h"

#include <epiphany/ephy-window.h>

G_BEGIN_DECLS

typedef struct _EphyTabsPluginMenu		EphyTabsPluginMenu;
typedef struct _EphyTabsPluginMenuClass		EphyTabsPluginMenuClass;
typedef struct _EphyTabsPluginMenuPrivate	EphyTabsPluginMenuPrivate;

#define EPHY_TYPE_TABS_PLUGIN_MENU		(ephy_tabs_plugin_menu_get_type())
#define EPHY_TABS_PLUGIN_MENU(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_TABS_PLUGIN_MENU, EphyTabsPluginMenu))
#define EPHY_TABS_PLUGIN_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_TABS_PLUGIN_MENU, EphyTabsPluginMenuClass))
#define EPHY_IS_TABS_PLUGIN_MENU(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_TABS_PLUGIN_MENU))
#define EPHY_IS_TABS_PLUGIN_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_TABS_PLUGIN_MENU))
#define EPHY_TABS_PLUGIN_MENU_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_TABS_PLUGIN_MENU, EphyTabsPluginMenuClass))

struct _EphyTabsPluginMenuClass
{
	GObjectClass parent_class;
};

struct _EphyTabsPluginMenu
{
	GObject parent_object;

	EphyTabsPluginMenuPrivate *priv;
};

GType			ephy_tabs_plugin_menu_get_type	(void);

EphyTabsPluginMenu *	ephy_tabs_plugin_menu_new	(EphyWindow *window);

void			ephy_tabs_plugin_menu_update	(EphyTabsPluginMenu *menu);

G_END_DECLS

#endif

