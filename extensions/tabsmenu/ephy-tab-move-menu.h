/*
 *  Copyright © 2003  Marco Pesenti Gritti
 *  Copyright © 2003  Christian Persch
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

#ifndef EPHY_TAB_MOVE_MENU_H
#define EPHY_TAB_MOVE_MENU_H

#include <epiphany/ephy-window.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define EPHY_TYPE_TAB_MOVE_MENU			(ephy_tab_move_menu_get_type())
#define EPHY_TAB_MOVE_MENU(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_TAB_MOVE_MENU, EphyTabMoveMenu))
#define EPHY_TAB_MOVE_MENU_CLASS(klass) 	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_TAB_MOVE_MENU, EphyTabMoveMenuClass))
#define EPHY_IS_TAB_MOVE_MENU(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_TAB_MOVE_MENU))
#define EPHY_IS_TAB_MOVE_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_TAB_MOVE_MENU))
#define EPHY_TAB_MOVE_MENU_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_TAB_MOVE_MENU, EphyTabMoveMenuClass))

typedef struct _EphyTabMoveMenu		EphyTabMoveMenu;
typedef struct _EphyTabMoveMenuClass	EphyTabMoveMenuClass;
typedef struct _EphyTabMoveMenuPrivate	EphyTabMoveMenuPrivate;

struct _EphyTabMoveMenuClass
{
	GObjectClass parent_class;
};

struct _EphyTabMoveMenu
{
	GObject parent_object;

	/*< private >*/
	EphyTabMoveMenuPrivate *priv;
};

GType		 ephy_tab_move_menu_get_type		(void);

GType		 ephy_tab_move_menu_register_type	(GTypeModule *module);

EphyTabMoveMenu	*ephy_tab_move_menu_new			(EphyWindow *window);

G_END_DECLS

#endif
