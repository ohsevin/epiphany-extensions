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

#include "config.h"

#include "ephy-sidebar.h"

#include <gtk/gtklabel.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkpaned.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkmarshal.h>

#include <string.h>
#include <glib/gi18n-lib.h>

typedef struct
{
	char *id;
	char *title;
	gboolean can_remove;
	GtkWidget *item;
} EphySidebarPage;

#define EPHY_SIDEBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SIDEBAR, EphySidebarPrivate))

struct _EphySidebarPrivate
{
	GtkWidget *content_frame;
	GtkWidget *title_frame;
	GtkWidget *title_menu;
	GtkWidget *title_button;
	GtkWidget *title_label;
	GtkWidget *title_hbox;
	GtkWidget *remove_button;
	GtkWidget *content;
	GList *pages;
	EphySidebarPage *current;
};

enum
{
	PAGE_CHANGED,
	CLOSE_REQUESTED,
	REMOVE_REQUESTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void ephy_sidebar_class_init	(EphySidebarClass *klass);
static void ephy_sidebar_init		(EphySidebar *sidebar);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType 
ephy_sidebar_get_type (void)
{
	return type;
}

GType
ephy_sidebar_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphySidebarClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_sidebar_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySidebar),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_sidebar_init
	};

	type = g_type_module_register_type (module,
					    GTK_TYPE_VBOX,
					    "EphySidebar",
					    &our_info, 0);

	return type;
}

static void
popup_position_func (GtkMenu *menu,
		     int *x, 
		     int *y,
		     gboolean *push_in,
		     gpointer user_data)
{
       GtkWidget *button;

       button = GTK_WIDGET (user_data);
       
       gdk_window_get_origin (button->parent->window, x, y);
       *x += button->allocation.x;
       *y += button->allocation.y + button->allocation.height;

       *push_in = FALSE;
}

static int
title_button_press_cb (GtkWidget *item,
		       GdkEventButton *event,
		       EphySidebar *sidebar)
{
       gtk_menu_popup (GTK_MENU (sidebar->priv->title_menu),
		       NULL, NULL,
		       popup_position_func, sidebar->priv->title_button, 
		       event->button, event->time);

       return TRUE;
}

static void
close_clicked_cb (GtkWidget *button,
		  EphySidebar *sidebar)
{
	g_signal_emit (G_OBJECT (sidebar), signals[CLOSE_REQUESTED], 0);
}

static void
remove_clicked_cb (GtkWidget *button,
		   EphySidebar *sidebar)
{
	g_return_if_fail (sidebar->priv->current != NULL);
	
	g_signal_emit (G_OBJECT (sidebar), signals[REMOVE_REQUESTED], 0,
		       sidebar->priv->current->id);
}


static void
select_page (EphySidebar *sidebar,
	     EphySidebarPage *page)
{
	const char * title;
	gboolean can_remove;
	gboolean title_sensitive = TRUE;

	sidebar->priv->current = page;
	
	if (page != NULL)
	{
		title = page->title;
		can_remove = page->can_remove;
	}
	else
	{
		title = _("No Sidebars installed");
		can_remove = FALSE;
		title_sensitive = FALSE;
	}
	
	gtk_label_set_text (GTK_LABEL (sidebar->priv->title_label), title);
	gtk_widget_set_sensitive (GTK_WIDGET(sidebar->priv->remove_button),
				  can_remove);
	gtk_widget_set_sensitive (GTK_WIDGET(sidebar->priv->title_button),
				  title_sensitive);
	
	g_signal_emit (G_OBJECT (sidebar), signals[PAGE_CHANGED], 0, 
		       page ? page->id : NULL);
}

static void
title_menu_item_activated_cb (GtkWidget *item,
			      EphySidebar *sidebar)
{
	EphySidebarPage *page;

	page = (EphySidebarPage *) g_object_get_data (G_OBJECT(item), "page");

	select_page (sidebar, page);
}

void
ephy_sidebar_add_page (EphySidebar *sidebar,
		       const char *title,
		       const char *page_id,
		       gboolean can_remove)
{
	EphySidebarPage *page;
	GtkWidget *item;

	g_return_if_fail (EPHY_IS_SIDEBAR (sidebar));

	page = g_new (EphySidebarPage, 1);	
	page->id = g_strdup (page_id);
	page->title = g_strdup (title);
	page->can_remove = can_remove;
	
	item = gtk_menu_item_new_with_label (title); 
	g_object_set_data (G_OBJECT(item), "page", (gpointer)page);
	g_signal_connect (G_OBJECT(item), "activate",
			  G_CALLBACK (title_menu_item_activated_cb),
			  sidebar);	
	gtk_menu_shell_append (GTK_MENU_SHELL(sidebar->priv->title_menu),
			       item);
	gtk_widget_show (item);

	page->item = item;
	
	sidebar->priv->pages = g_list_append (sidebar->priv->pages,
					      (gpointer)page);

	if (GTK_WIDGET_VISIBLE (sidebar) && sidebar->priv->current == NULL)
	{
		select_page (sidebar, page);
	}
}

static int 
page_compare_func (gconstpointer  a,
		   gconstpointer  b)
{
	EphySidebarPage *page = (EphySidebarPage *) a;

	return strcmp (page->id, (const char*) b);
}

static EphySidebarPage *
ephy_sidebar_find_page_by_id (EphySidebar *sidebar,
			      const char *page_id)
{
	GList *l;
	
	l = g_list_find_custom (sidebar->priv->pages, page_id,
				page_compare_func);
	if (!l) return NULL;
	
	return (EphySidebarPage *) l->data;
}

gboolean 
ephy_sidebar_remove_page (EphySidebar *sidebar,
			  const char *page_id)
{
	EphySidebarPage *page;

	g_return_val_if_fail (EPHY_IS_SIDEBAR (sidebar), FALSE);

	page = ephy_sidebar_find_page_by_id (sidebar, page_id);
	g_return_val_if_fail (page != NULL, FALSE);

	sidebar->priv->pages = g_list_remove (sidebar->priv->pages,
					      (gpointer)page);
	
	if (sidebar->priv->current == page)
	{
		EphySidebarPage *new_page = NULL;

		if (sidebar->priv->pages != NULL)
		{
			new_page = sidebar->priv->pages->data;
		}

		select_page (sidebar, new_page);
	}

	gtk_widget_destroy (page->item);
	
	return TRUE;
}

gboolean 
ephy_sidebar_select_page (EphySidebar *sidebar,
			  const char *page_id)
{
	EphySidebarPage *page;

	g_return_val_if_fail (EPHY_IS_SIDEBAR (sidebar), FALSE);

	page = ephy_sidebar_find_page_by_id (sidebar, page_id);
	g_return_val_if_fail (page != NULL, FALSE);
	
	select_page (sidebar, page);
	
	return FALSE;
}

void 
ephy_sidebar_set_content (EphySidebar *sidebar,
			  GtkWidget *content)
{
	g_return_if_fail (EPHY_IS_SIDEBAR (sidebar));
	g_return_if_fail (GTK_IS_WIDGET (content));

	if (sidebar->priv->content != NULL)
	{
		gtk_container_remove (GTK_CONTAINER(sidebar->priv->content_frame),
				      GTK_WIDGET(sidebar->priv->content));
	}

	sidebar->priv->content = content;

	gtk_container_add (GTK_CONTAINER(sidebar->priv->content_frame),
			   GTK_WIDGET(content));
}

static void
ephy_sidebar_show (GtkWidget *widget)
{
	EphySidebar *sidebar = EPHY_SIDEBAR (widget);

	/* If we aren't showing a page, always select a page, even if
	 * it is the NULL page, our title will always be correct then
	 */
	if (sidebar->priv->current == NULL)
	{
		EphySidebarPage *new_page = NULL;

		if (sidebar->priv->pages != NULL)
		{
			new_page = sidebar->priv->pages->data;
		}

		select_page (sidebar, new_page);		
	}

	GTK_WIDGET_CLASS (parent_class)->show (widget);	
}

static void
ephy_sidebar_size_allocate (GtkWidget *widget,
			    GtkAllocation *allocation)
{
	EphySidebar *sidebar = EPHY_SIDEBAR(widget);
	GtkWidget *frame = sidebar->priv->title_frame;
	GtkWidget *hbox = sidebar->priv->title_hbox;
	GtkAllocation child_allocation;
	GtkRequisition child_requisition;
	int width;
	
	GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
	
	gtk_widget_get_child_requisition (hbox, &child_requisition);
	width = child_requisition.width;
	
	child_allocation = frame->allocation;
	child_allocation.width = MAX (width, frame->allocation.width);
	
	gtk_widget_size_allocate (frame, &child_allocation);
}

static void
ephy_sidebar_init (EphySidebar *sidebar)
{
	EphySidebarPrivate *priv;
	GtkWidget *frame, *frame_hbox, *button_hbox, *close_button, *close_image;
	GtkWidget *remove_image, *remove_button, *arrow;
	
	priv = sidebar->priv = EPHY_SIDEBAR_GET_PRIVATE (sidebar);

	priv->content = NULL;
	priv->pages = NULL;
	priv->current = NULL;
		
	frame_hbox = gtk_hbox_new (FALSE, 0);
	
	priv->title_button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (priv->title_button),
			       GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (priv->title_button), FALSE);

	g_signal_connect (priv->title_button, "button_press_event",
			  G_CALLBACK (title_button_press_cb),
			  sidebar);
       
	button_hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button_hbox), 0);
       
	priv->title_label = gtk_label_new ("");
       
	gtk_widget_show (priv->title_label);

	gtk_box_pack_start (GTK_BOX (button_hbox), 
			    priv->title_label,
			    FALSE, FALSE, 0);
       
	arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_ETCHED_IN);
	gtk_widget_show (arrow);
	gtk_box_pack_end (GTK_BOX (button_hbox), arrow, FALSE, FALSE, 0);
       
	gtk_widget_show (button_hbox);
       
	gtk_container_add (GTK_CONTAINER (priv->title_button),
			   button_hbox);

	priv->title_menu = gtk_menu_new ();
	g_object_ref_sink (priv->title_menu);

	gtk_widget_show (priv->title_button);
	gtk_widget_show (priv->title_menu);

	gtk_box_pack_start (GTK_BOX (frame_hbox), 
			    priv->title_button,
			    FALSE, FALSE, 0);

	/* Remove sidebar button */
	remove_image = gtk_image_new_from_stock (GTK_STOCK_DELETE, 
						 GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_widget_show (remove_image);

	remove_button = gtk_button_new ();

	gtk_button_set_relief (GTK_BUTTON (remove_button), GTK_RELIEF_NONE);
	gtk_container_add (GTK_CONTAINER (remove_button), remove_image);

	g_signal_connect (remove_button, "clicked",
			  G_CALLBACK (remove_clicked_cb), 
			  sidebar);

	gtk_widget_show (remove_button);

	priv->remove_button = remove_button;
	
	/* Close button */
	close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, 
						GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_widget_show (close_image);

	close_button = gtk_button_new ();

	gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
	gtk_container_add (GTK_CONTAINER (close_button), close_image);

	g_signal_connect (close_button, "clicked",
			  G_CALLBACK (close_clicked_cb), 
			  sidebar);

	gtk_widget_show (close_button); 
       
	gtk_box_pack_end (GTK_BOX (frame_hbox), close_button, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (frame_hbox), remove_button, FALSE, FALSE, 0);

	priv->title_hbox = frame_hbox;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), 
				   GTK_SHADOW_ETCHED_IN);
	gtk_widget_show (frame);
	priv->title_frame = frame;
 
	gtk_container_add (GTK_CONTAINER (frame), 
			   frame_hbox);

	gtk_widget_show (frame_hbox);

	gtk_box_pack_start (GTK_BOX (sidebar), frame,
			    FALSE, FALSE, 2);

	priv->content_frame = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (priv->content_frame);
		
	gtk_box_pack_start (GTK_BOX (sidebar), priv->content_frame,
			    TRUE, TRUE, 0);
}

static void
ephy_sidebar_dispose (GObject *object)
{
	EphySidebar *sidebar = EPHY_SIDEBAR (object);
	EphySidebarPrivate *priv = sidebar->priv;

	if (priv->title_menu != NULL)
	{
		gtk_menu_shell_deactivate (GTK_MENU_SHELL (priv->title_menu));
		g_object_unref (priv->title_menu);
		priv->title_menu = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
ephy_sidebar_finalize (GObject *object)
{
/*	EphySidebar *sidebar = EPHY_SIDEBAR (object);
	EphySidebarPrivate *priv = sidebar->priv;
*/
	/* FIXME free pages list */

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_sidebar_class_init (EphySidebarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	
	parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

	object_class->dispose = ephy_sidebar_dispose;
	object_class->finalize = ephy_sidebar_finalize;
	
	widget_class->size_allocate = ephy_sidebar_size_allocate;
	widget_class->show = ephy_sidebar_show;
	
	signals[PAGE_CHANGED] =
		g_signal_new ("page_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (EphySidebarClass, page_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	signals[CLOSE_REQUESTED] =
		g_signal_new ("close_requested",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (EphySidebarClass, close_requested),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	signals[REMOVE_REQUESTED] =
		g_signal_new ("remove_requested",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (EphySidebarClass, remove_requested),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	g_type_class_add_private (klass, sizeof (EphySidebarPrivate));
}

GtkWidget *
ephy_sidebar_new (void)
{
	return GTK_WIDGET (g_object_new (EPHY_TYPE_SIDEBAR, NULL));
}
