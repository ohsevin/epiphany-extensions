/*
 *  Copyright © 2000, 2001, 2002 Marco Pesenti Gritti
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

#ifndef EPHY_SIDEBAR_H
#define EPHY_SIDEBAR_H

#include <glib-object.h>
#include <gtk/gtk.h>
	
G_BEGIN_DECLS

#define EPHY_TYPE_SIDEBAR		(ephy_sidebar_get_type ())
#define EPHY_SIDEBAR(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_SIDEBAR, EphySidebar))
#define EPHY_SIDEBAR_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_SIDEBAR, EphySidebarClass))
#define EPHY_IS_SIDEBAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_SIDEBAR))
#define EPHY_IS_SIDEBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_SIDEBAR))
#define EPHY_SIDEBAR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_SIDEBAR, EphySidebarClass))

typedef struct _EphySidebar		EphySidebar;
typedef struct _EphySidebarPrivate	EphySidebarPrivate;
typedef struct _EphySidebarClass	EphySidebarClass;
	
struct _EphySidebar 
{
	GtkVBox parent;

	/*< private >*/
	EphySidebarPrivate *priv;
};

struct _EphySidebarClass
{
	GtkVBoxClass parent_class;

	void (* close_requested)	(EphySidebar *sidebar);
	
	void (* page_changed)		(EphySidebar *sidebar,
					 const char *page_id);
	
	void (* remove_requested)	(EphySidebar *sidebar,
					 const char *page_id);
};

GType		 ephy_sidebar_get_type		(void);

GType		 ephy_sidebar_register_type	(GTypeModule *module);

GtkWidget	*ephy_sidebar_new		(void);

void		 ephy_sidebar_add_page		(EphySidebar *sidebar,
						 const char *title,
						 const char *page_id,
						 gboolean can_remove);

gboolean	 ephy_sidebar_remove_page	(EphySidebar *sidebar,
						 const char *page_id);

gboolean 	 ephy_sidebar_select_page	(EphySidebar *sidebar,
						 const char *page_id);

void		 ephy_sidebar_set_content	(EphySidebar *sidebar,
						 GtkWidget *content);

G_END_DECLS

#endif
