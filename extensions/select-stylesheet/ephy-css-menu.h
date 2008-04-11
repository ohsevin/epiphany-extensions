/*
 *  Copyright © 2002  Ricardo Fernández Pascual
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

#ifndef EPHY_CSS_MENU_H
#define EPHY_CSS_MENU_H

#include <glib-object.h>
#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define EPHY_TYPE_CSS_MENU			(ephy_css_menu_get_type())
#define EPHY_CSS_MENU(object)			(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_CSS_MENU, EphyCSSMenu))
#define EPHY_CSS_MENU_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_CSS_MENU, EphyCSSMenuClass))
#define EPHY_IS_CSS_MENU(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_CSS_MENU))
#define EPHY_IS_CSS_MENU_ITEM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_CSS_MENU))
#define EPHY_CSS_MENU_GET_CLASS(obj) 		(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_CSS_MENU, EphyCSSMenuClass))

typedef struct _EphyCSSMenu		EphyCSSMenu;
typedef struct _EphyCSSMenuClass	EphyCSSMenuClass;
typedef struct _EphyCSSMenuPrivate	EphyCSSMenuPrivate;

struct _EphyCSSMenuClass
{
	GObjectClass parent_class;
};

struct _EphyCSSMenu
{
	GObject parent_object;

	/*< private >*/
	EphyCSSMenuPrivate *priv;
};

GType		ephy_css_menu_get_type		(void);

GType		ephy_css_menu_register_type	(GTypeModule *module);

EphyCSSMenu *	ephy_css_menu_new		(EphyWindow *window);

G_END_DECLS

#endif
