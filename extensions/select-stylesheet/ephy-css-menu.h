/*
 *  Copyright (C) 2002  Ricardo Fernández Pascual
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
 */

#ifndef __ephy_altenate_css_menu_
#define __ephy_altenate_css_menu_

#include <glib-object.h>
#include <epiphany/ephy-window.h>

/* object forward declarations */

typedef struct _EphyCssMenu EphyCssMenu;
typedef struct _EphyCssMenuClass EphyCssMenuClass;
typedef struct _EphyCssMenuPrivate EphyCssMenuPrivate;

/**
 * EphyCssMenu object
 */

#define EPHY_TYPE_CSS_MENU		(ephy_css_menu_get_type())
#define EPHY_CSS_MENU(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_CSS_MENU,\
					 EphyCssMenu))
#define EPHY_CSS_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_CSS_MENU,\
					 EphyCssMenuClass))
#define EPHY_IS_CSS_MENU(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_CSS_MENU))
#define EPHY_IS_CSS_MENU_ITEM_CLASS(klass) \
 					(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_CSS_MENU))
#define EPHY_CSS_MENU_GET_CLASS(obj) 	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_CSS_MENU,\
					 EphyCssMenuClass))

struct _EphyCssMenuClass
{
	GObjectClass parent_class;
};

/* Remember: fields are public read-only */
struct _EphyCssMenu
{
	GObject parent_object;
	
	EphyCssMenuPrivate *priv;
};

GType		ephy_css_menu_get_type			(void);
EphyCssMenu *	ephy_css_menu_new			(EphyWindow *window);

#endif
