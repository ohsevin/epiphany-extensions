/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-bookmarks-tray-extension.h"

#include "eggtrayicon.h"

#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-session.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-extension.h>
#include "ephy-bookmarks-menu.h"
#include "ephy-gui.h"
#include "eel-gconf-extensions.h"
#include "ephy-debug.h"

#include <gtk/gtkaction.h>
#include <gtk/gtkradioaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkmenushell.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmain.h>

#include <gmodule.h>

#include <glib/gi18n-lib.h>

#include <string.h>

typedef enum
{
	OPEN_IN_CURRENT,
	OPEN_IN_NEW_TAB,
	OPEN_IN_NEW_WINDOW
} OpenInType;

typedef struct
{
	const char *text;
	OpenInType type;
} OpenInDesc;

static const OpenInDesc types [] =
{
	{ "current", OPEN_IN_CURRENT },
	{ "new-tab", OPEN_IN_NEW_TAB },
	{ "new-window", OPEN_IN_NEW_WINDOW },
	{ NULL, OPEN_IN_CURRENT }
};

#define EPHY_BOOKMARKS_TRAY_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_BOOKMARKS_TRAY_EXTENSION, EphyBookmarksTrayExtensionPrivate))

struct _EphyBookmarksTrayExtensionPrivate
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	EphyBookmarksMenu *menu;
	GtkWidget *tray;
	GtkWidget *tray_button;
	GtkTooltips *tray_tips;
	OpenInType open_in;
};

#define CONF_DIR		"/apps/epiphany/extensions/bookmarks-tray"
#define	CONF_OPEN_IN		"/apps/epiphany/extensions/bookmarks-tray/open_in"

/* from ephy-stock-icons.h */
#define EPHY_STOCK_BOOKMARKS	"epiphany-bookmarks"
#define STOCK_NEW_TAB		"stock_new-tab"

static void ephy_bookmarks_tray_extension_class_init	(EphyBookmarksTrayExtensionClass *klass);
static void ephy_bookmarks_tray_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_bookmarks_tray_extension_init		(EphyBookmarksTrayExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_bookmarks_tray_extension_get_type (void)
{
	return type;
}

GType
ephy_bookmarks_tray_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyBookmarksTrayExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_bookmarks_tray_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyBookmarksTrayExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_bookmarks_tray_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_bookmarks_tray_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyBookmarksTrayExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

#if 0
static void
button_size_allocate_cb (GtkWidget *button,
			 GtkAllocation  *allocation,
			 EphyBookmarksTrayExtension *extension)
{

	GtkOrientation orientation;
	gint size;


	orientation = egg_tray_icon_get_orientation (tray);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	size = allocation->height;
	else
	size = allocation->width;
	
	if (status_icon->priv->size != size)
	{
	status_icon->priv->size = size;
	
	g_object_notify (G_OBJECT (status_icon), "size");
	
	if (!emit_size_changed_signal (status_icon, size))
	{
	  egg_status_icon_update_image (status_icon);
	}
	}

}
#endif

static void
open_bookmark_cb (EphyBookmarksMenu *menu,
		  const char *address,
		  gboolean open_in_new,
		  EphyBookmarksTrayExtension *extension)
{
	EphyBookmarksTrayExtensionPrivate *priv = extension->priv;
	EphySession *session;
	EphyWindow *window;
	EphyTab *tab;

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	window = ephy_session_get_active_window (session);

	if (priv->open_in == OPEN_IN_CURRENT && !open_in_new && window != NULL)
	{
		ephy_window_load_url (window, address);
	}
	else if ((priv->open_in == OPEN_IN_NEW_TAB || open_in_new) && window != NULL)
	{
		tab = ephy_window_get_active_tab (window);
		ephy_shell_new_tab (ephy_shell, window, tab, address,
				    EPHY_NEW_TAB_OPEN_PAGE |
				    EPHY_NEW_TAB_IN_EXISTING_WINDOW);
	}
	else
	{
		ephy_shell_new_tab (ephy_shell, window, NULL, address,
				    EPHY_NEW_TAB_OPEN_PAGE |
				    EPHY_NEW_TAB_IN_NEW_WINDOW);
	}
}

/*
 * menu positioning function adapted from gnome-panel/menu-utils.c:
 *
 * GNOME panel menu module.
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 */
static void
menu_position_func (GtkMenu *menu,
		    int *x,
		    int *y,
		    gboolean *push_in,
		    gpointer data)
{
	EphyBookmarksTrayExtension *extension = EPHY_BOOKMARKS_TRAY_EXTENSION (data);
	EphyBookmarksTrayExtensionPrivate *priv = extension->priv;
	GtkRequisition requisition;
	GdkScreen *screen;
	GtkWidget *widget = priv->tray;
	int menu_x = 0, menu_y = 0;

	screen = gtk_widget_get_screen (widget);

	gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

	gdk_window_get_origin (widget->window, &menu_x, &menu_y);

	if (GTK_WIDGET_NO_WINDOW (widget))
	{
		menu_x += widget->allocation.x;
		menu_y += widget->allocation.y;
	}

	/* FIXME:  egg_tray_icon_get_orientation doesn't seem to work */
	if (egg_tray_icon_get_orientation (EGG_TRAY_ICON (priv->tray)) == GTK_ORIENTATION_HORIZONTAL)
	{
		if (menu_y > gdk_screen_get_height (screen) / 2)
			menu_y -= requisition.height;
		else
			menu_y += widget->allocation.height;
	} else {
		if (menu_x > gdk_screen_get_width (screen) / 2)
			menu_x -= requisition.width;
		else
			menu_x += widget->allocation.width;
	}

	*x = menu_x;
	*y = menu_y;
	*push_in = FALSE;
}

static void
show_context_menu (EphyBookmarksTrayExtension *extension,
		   GtkWidget *button,
		   GdkEventButton *event)
{
	EphyBookmarksTrayExtensionPrivate *priv = extension->priv;
	GtkWidget *menu;

	LOG ("show_context_menu")

	menu = gtk_ui_manager_get_widget (priv->manager, "/ContextMenu");
	g_return_if_fail (menu != NULL);

	if (event != NULL && event->type == GDK_BUTTON_PRESS)
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
				menu_position_func,
				extension, event->button, event->time);
		
	}
	else
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
				menu_position_func,
				extension, 0, gtk_get_current_event_time ());
		gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);
	}
}

static void
menu_deactivate_cb (GtkMenuShell *ms,
		    GtkWidget *button)
{
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
        gtk_button_released (GTK_BUTTON (button));
}

static void
show_menu (EphyBookmarksTrayExtension *extension,
	   GtkToggleButton *button)
{
	EphyBookmarksTrayExtensionPrivate *priv = extension->priv;
	GtkAction *action;
	GtkWidget *menu;

	action = gtk_action_group_get_action (priv->action_group, "Bookmarks");
	g_return_if_fail (action != NULL);

	/* this will make the menu update itself */
	gtk_action_activate (action);

	menu = gtk_ui_manager_get_widget (priv->manager, "/BmkMenu");
	g_return_if_fail (menu != NULL);

	g_signal_connect (menu, "deactivate",
			  G_CALLBACK (menu_deactivate_cb), button);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
			menu_position_func,
			extension, 0, gtk_get_current_event_time ());
}

static void
button_toggled_cb (GtkToggleButton *button,
		   EphyBookmarksTrayExtension *extension)
{
	LOG ("button_toggled_cb")

	if (gtk_toggle_button_get_active (button))
	{
		show_menu (extension, button);
	}
}

static void
button_pressed_cb (GtkToggleButton *button,
		   EphyBookmarksTrayExtension *extension)
{
	gtk_toggle_button_set_active (button, TRUE);
}

static gboolean
button_button_press_cb (GtkWidget *button,
			GdkEventButton *event,
			EphyBookmarksTrayExtension *extension)
{
	LOG ("button_button_press_cb")

	if (event->button == 3)
        {
                show_context_menu (extension, button, event);

                return TRUE;
        }

        return FALSE;
}

static gboolean
button_popup_menu_cb (GtkWidget *button,
		      EphyBookmarksTrayExtension *extension)
{
	LOG ("button_popup_menu_cb")

	show_context_menu (extension, button, NULL);

	return TRUE;
}

static void
edit_bookmarks_cb (GtkAction *action,
		   EphyBookmarksTrayExtension *extension)
{
        GtkWidget *editor;

        editor = ephy_shell_get_bookmarks_editor (ephy_shell);
	/* FIXME set parent:
	EphySession *session;
	EphyWindow *window;
	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	window = ephy_session_get_active_window (session);

        ephy_bookmarks_editor_set_parent (EPHY_BOOKMARKS_EDITOR (editor),
                                          window ? GTK_WIDGET (window) : NULL);
	*/
        gtk_window_present (GTK_WINDOW (editor));
}

static OpenInType
type_from_prefs (void)
{
	char *pref;
	int i;

	pref = eel_gconf_get_string (CONF_OPEN_IN);
	if (pref == NULL) return OPEN_IN_CURRENT;

	for (i = 0; types[i].text != NULL; i++)
	{
		if (strcmp (pref, types[i].text) == 0)
		{
			break;
		}
	}

	g_free (pref);

	return types[i].type;
}

static void
open_in_changed_cb (GtkAction *action,
		    GtkRadioAction *current,
		    EphyBookmarksTrayExtension *extension)
{
	extension->priv->open_in =
		(OpenInType) gtk_radio_action_get_current_value (current);

	eel_gconf_set_string (CONF_OPEN_IN,
			      types[extension->priv->open_in].text);
}

static GtkActionEntry action_entries [] =
{
	{ "Bookmarks", NULL, "" },
	{ "Context", NULL, "" },
	{ "EditBookmarks", EPHY_STOCK_BOOKMARKS, N_("_Edit Bookmarks"), NULL,
          N_("Open the bookmarks window"),
          G_CALLBACK (edit_bookmarks_cb) },
};

static GtkRadioActionEntry radio_entries [] =
{
	{ "OpenInCurrent", NULL, N_("Open in _Current Tab"), NULL,
	  N_("Open bookmarks in the current tab of the active window"),
	  OPEN_IN_CURRENT },
	{ "OpenInNewTab", NULL, N_("Open in New _Tab"), NULL,
	  N_("Open bookmarks in a new tab of the active window"),
	  OPEN_IN_NEW_TAB },
	{ "OpenInNewWindow", NULL, N_("Open in New _Window"), NULL,
	  N_("Open bookmarks in new a window"),
	  OPEN_IN_NEW_WINDOW },
};

static void
ephy_bookmarks_tray_extension_init (EphyBookmarksTrayExtension *extension)
{
	EphyBookmarksTrayExtensionPrivate *priv;
	GError *error = NULL;
	GtkWidget *hbox, *image, *label;

	LOG ("EphyBookmarksTrayExtension initialising")

	priv = extension->priv = EPHY_BOOKMARKS_TRAY_EXTENSION_GET_PRIVATE (extension);

	priv->open_in = type_from_prefs ();

	priv->manager = gtk_ui_manager_new ();

	priv->action_group = gtk_action_group_new ("BmkTrayActions");
	gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group, action_entries,
				      G_N_ELEMENTS (action_entries), extension);
	gtk_action_group_add_radio_actions (priv->action_group, radio_entries,
					    G_N_ELEMENTS (radio_entries),
					    priv->open_in,
					    G_CALLBACK (open_in_changed_cb),
					    extension);
	gtk_ui_manager_insert_action_group (priv->manager, priv->action_group, -1);
	g_object_unref (priv->action_group);

        gtk_ui_manager_add_ui_from_file (priv->manager,
					 SHARE_DIR "/xml/ephy-bookmarks-tray-ui.xml",
					 &error);
        if (error != NULL)
        {
                g_warning ("Could not merge ephy-bookmarks-tray-ui.xml: %s", error->message);
                g_error_free (error);
        }

	priv->menu = ephy_bookmarks_menu_new (priv->manager, "/BmkMenu");
	g_signal_connect (priv->menu, "open",
			  G_CALLBACK (open_bookmark_cb), extension);

	priv->tray_button = gtk_toggle_button_new ();

	gtk_button_set_relief (GTK_BUTTON (priv->tray_button), GTK_RELIEF_NONE);
 
	g_signal_connect (priv->tray_button, "toggled",
			  G_CALLBACK (button_toggled_cb), extension);
	g_signal_connect (priv->tray_button, "pressed",
			  G_CALLBACK (button_pressed_cb), extension);
	g_signal_connect (priv->tray_button, "button-press-event",
			  G_CALLBACK (button_button_press_cb), extension);
	g_signal_connect (priv->tray_button, "popup_menu",
			  G_CALLBACK (button_popup_menu_cb), extension);

#if 0
	g_signal_connect (priv->tray_button, "size-allocate",
			  G_CALLBACK (button_size_allocate_cb), extension);
#endif

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_container_add (GTK_CONTAINER (priv->tray_button), hbox);

	image = gtk_image_new_from_stock (EPHY_STOCK_BOOKMARKS, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	label = gtk_label_new (_("Bookmarks"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	gtk_widget_show (label);
	gtk_widget_show (image);
	gtk_widget_show (hbox);
	gtk_widget_show (priv->tray_button);
	
	priv->tray_tips = gtk_tooltips_new ();
	g_object_ref (priv->tray_tips);
	gtk_object_sink (GTK_OBJECT (priv->tray_tips));

	priv->tray = GTK_WIDGET (egg_tray_icon_new (NULL));
	gtk_container_add (GTK_CONTAINER (priv->tray), priv->tray_button);

	gtk_widget_show (priv->tray);

	eel_gconf_monitor_add (CONF_DIR);
}

static void
ephy_bookmarks_tray_extension_finalize (GObject *object)
{
	EphyBookmarksTrayExtension *extension = EPHY_BOOKMARKS_TRAY_EXTENSION (object);
	EphyBookmarksTrayExtensionPrivate *priv = extension->priv;

	LOG ("EphyBookmarksTrayExtension finalising")

	eel_gconf_monitor_remove (CONF_DIR);

	g_object_unref (priv->tray_tips);
	gtk_widget_destroy (GTK_WIDGET (priv->tray));

	g_object_unref (priv->menu);
	g_object_unref (priv->manager);

	parent_class->finalize (object);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
//	EphyBookmarksTrayExtension *extension = EPHY_BOOKMARKS_TRAY_EXTENSION (ext);
	LOG ("EphyBookmarksTrayExtension attach_window")
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
//	EphyBookmarksTrayExtension *extension = EPHY_BOOKMARKS_TRAY_EXTENSION (ext);
	LOG ("EphyBookmarksTrayExtension detach_window")
}

static void
ephy_bookmarks_tray_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_bookmarks_tray_extension_class_init (EphyBookmarksTrayExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_bookmarks_tray_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyBookmarksTrayExtensionPrivate));
}
