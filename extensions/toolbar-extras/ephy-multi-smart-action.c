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

#include "config.h"

#include "ephy-multi-smart-action.h"

#include "ephy-favicon-cache.h"
#include "ephy-gui.h"
#include "ephy-string.h"
#include "eel-gconf-extensions.h"
#include "ephy-debug.h"

#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-node.h>
#include <epiphany/ephy-bookmarks.h>
#include <epiphany/ephy-history.h>

#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtktoolitem.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkaction.h>
#include <glib/gi18n-lib.h>

#include "galago-gtk-icon-entry.h"

typedef enum
{
	EPHY_MSM_MODE_FIND,
	EPHY_MSM_MODE_SMART
} EphyMultiSmartMode;

static const char *mode_strings [] =
{
	"find",
	"smart"
};

enum
{
	NOTIF_CLEAN_AFTER_USE,
	NOTIF_CASE_SENSITIVE,
	NOTIF_MODE,
	NOTIF_SMART_ADDRESS,
	LAST_NOTIF
};

#define EPHY_MULTI_SMART_ACTION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_MULTI_SMART_ACTION, EphyMultiSmartActionPrivate))

struct _EphyMultiSmartActionPrivate
{
	EphyWindow *window;
	EphyMultiSmartMode mode;
	gulong id;
	gulong removed_signal;
	gulong changed_signal;
	gboolean case_sensitive;
	gboolean clean_after_use;
	guint notifier_id[LAST_NOTIF];
};

enum
{
	PROP_0,
	PROP_MODE,
	PROP_SMARTBOOKMARK_ID,
	PROP_WINDOW,
};

#define CONF_MODE		"/apps/epiphany/extensions/multi-smart/mode"
#define CONF_SMART_ADDRESS	"/apps/epiphany/extensions/multi-smart/smart_address"
#define CONF_CLEAN_AFTER_USE	"/apps/epiphany/extensions/multi-smart/clean_after_use"
#define CONF_CASE_SENSITIVE	"/apps/epiphany/dialogs/find_match_case"

static GObjectClass *parent_class = NULL;

static void ephy_multi_smart_action_init	(EphyMultiSmartAction *action);
static void ephy_multi_smart_action_class_init	(EphyMultiSmartActionClass *class);

static GType type = 0;

GType
ephy_multi_smart_action_get_type (void)
{
	return type;
}

GType
ephy_multi_smart_action_register_type (GTypeModule *module)
{
	static const GTypeInfo type_info =
	{
		sizeof (EphyMultiSmartActionClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) ephy_multi_smart_action_class_init,
		(GClassFinalizeFunc) NULL,
		NULL,
		sizeof (EphyMultiSmartAction),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_multi_smart_action_init,
	};

	type = g_type_module_register_type (module,
					    GTK_TYPE_ACTION,
					    "EphyMultiSmartAction",
					    &type_info, 0);

	return type;
}

static void
menu_activate_find_cb (GtkWidget *item,
		       EphyMultiSmartAction *action)
{
	eel_gconf_set_string (CONF_MODE, mode_strings[EPHY_MSM_MODE_FIND]);
}

static void
menu_case_sensitive_toggled_cb (GtkWidget *item,
				EphyMultiSmartAction *action)
{
	eel_gconf_set_boolean
		(CONF_CASE_SENSITIVE,
		 gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)));
}

static void
menu_clean_after_use_toggled_cb (GtkWidget *item,
				 EphyMultiSmartAction *action)
{
	eel_gconf_set_boolean
		(CONF_CLEAN_AFTER_USE,
		 gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)));
}

static void
menu_activate_node_cb (GtkWidget *item,
		       EphyMultiSmartAction *action)
{
	EphyBookmarks *bookmarks;
	EphyNode *node;
	gulong id;
	const char *address;

	id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (item), "node-id"));
	g_return_if_fail (id != 0);

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	node = ephy_bookmarks_get_from_id (bookmarks, id);
	if (!EPHY_IS_NODE (node)) return;

	address = ephy_node_get_property_string (node, EPHY_NODE_BMK_PROP_LOCATION);
	g_return_if_fail (address != NULL);

	eel_gconf_set_string (CONF_SMART_ADDRESS, address);
	eel_gconf_set_string (CONF_MODE, mode_strings[EPHY_MSM_MODE_SMART]);
}

#define MAX_LENGTH 32

static int
sort_bookmarks (gconstpointer a,
		gconstpointer b)
{
	EphyNode *node_a = (EphyNode *)a;
	EphyNode *node_b = (EphyNode *)b;
	const char *title1, *title2;
	int retval;

	title1 = ephy_node_get_property_string (node_a, EPHY_NODE_BMK_PROP_TITLE);
	title2 = ephy_node_get_property_string (node_b, EPHY_NODE_BMK_PROP_TITLE);
                                                                                                                      
	if (title1 == NULL)
	{
		retval = -1;
	}
	else if (title2 == NULL)
	{
		retval = 1;
	}
	else
	{
		char *str_a, *str_b;

		str_a = g_utf8_casefold (title1, -1);
		str_b = g_utf8_casefold (title2, -1);
		retval = g_utf8_collate (str_a, str_b);
		g_free (str_a);
		g_free (str_b);
	}

	return retval;
}

static GtkWidget *
build_menu (EphyMultiSmartAction *action)
{
	EphyBookmarks *bookmarks;
	EphyHistory *history;
	EphyFaviconCache *cache;
	EphyNode *smartbookmarks;
	GtkWidget *menu, *item;
	GPtrArray *children;
	GList *node_list = NULL, *l = NULL;
	int i;

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	smartbookmarks = ephy_bookmarks_get_smart_bookmarks (bookmarks);
	history = EPHY_HISTORY (ephy_embed_shell_get_global_history (embed_shell));
	cache = EPHY_FAVICON_CACHE (ephy_embed_shell_get_favicon_cache
					(EPHY_EMBED_SHELL (ephy_shell)));

	menu = gtk_menu_new ();

	/* first the "Find" action */
	item = gtk_image_menu_item_new_from_stock (GTK_STOCK_FIND, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);
	g_signal_connect (item, "activate",
			  G_CALLBACK (menu_activate_find_cb), action);

	item = gtk_check_menu_item_new_with_label (_("Case sensitive"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
					action->priv->case_sensitive);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);
	g_signal_connect (item, "toggled",
			  G_CALLBACK (menu_case_sensitive_toggled_cb), action);

	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	/* now the smart bookmarks */
	children = ephy_node_get_children (smartbookmarks);

	for (i = 0; i < children->len; ++i)
	{
		EphyNode *kid;

		kid = g_ptr_array_index (children, i);
		node_list = g_list_prepend (node_list, kid);
	}

	node_list = g_list_sort (node_list, (GCompareFunc) sort_bookmarks);

	for (l = node_list; l != NULL; l = l->next)
	{
		EphyNode *node = (EphyNode *) l->data;
		const char *title, *location, *icon_location;
		char *short_title, *escaped_title;

		title = ephy_node_get_property_string
			(node, EPHY_NODE_BMK_PROP_TITLE);
		short_title = ephy_string_shorten (title, MAX_LENGTH);
		escaped_title = ephy_string_double_underscores (short_title);

		item = gtk_image_menu_item_new_with_label (escaped_title);
		g_free (short_title);
		g_free (escaped_title);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_widget_show (item);

		location = ephy_node_get_property_string
					(node, EPHY_NODE_BMK_PROP_LOCATION);
		icon_location = ephy_history_get_icon (history, location);

		if (icon_location)
		{
			GdkPixbuf *icon;
			GtkWidget *image;

			icon = ephy_favicon_cache_get (cache, icon_location);
			if (icon != NULL)
			{
				image = gtk_image_new_from_pixbuf (icon);
				gtk_widget_show (image);
				gtk_image_menu_item_set_image
					(GTK_IMAGE_MENU_ITEM (item), image);
				g_object_unref (icon);
			}
		}

		g_object_set_data (G_OBJECT (item), "node-id",
				   GUINT_TO_POINTER (ephy_node_get_id (node)));

		g_signal_connect (item, "activate",
				  G_CALLBACK (menu_activate_node_cb), action);

	}

	g_list_free (node_list);

	/* now append misc settings */
	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	item = gtk_check_menu_item_new_with_label (_("Clean after use"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
					action->priv->clean_after_use);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);
	g_signal_connect (item, "toggled",
			  G_CALLBACK (menu_clean_after_use_toggled_cb), action);

	return menu;
}

static GtkWidget *
create_tool_item (GtkAction *action)
{
	GtkWidget *item, *image, *entry;

	item = GTK_WIDGET (gtk_tool_item_new ());

	image = gtk_image_new ();

	entry = galago_gtk_icon_entry_new ();
	galago_gtk_icon_entry_set_icon (GALAGO_GTK_ICON_ENTRY (entry), image);

	gtk_widget_set_size_request (entry, 120, -1);
	gtk_container_add (GTK_CONTAINER (item), entry);

	gtk_widget_show (image);
	gtk_widget_show (entry);

	g_object_set_data (G_OBJECT (item), "icon", image);
	g_object_set_data (G_OBJECT (item), "entry", entry);

	return item;
}

static GtkWidget *
create_menu_item (GtkAction *action)
{
	GtkWidget *menu, *menu_item;
	GValue value = { 0, };
	const char *label_text;

	menu_item = GTK_ACTION_CLASS (parent_class)->create_menu_item (action);
/*	g_value_init (&value, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (action), "label", &value);

	label_text = g_value_get_string (&value);

	LOG ("create_menu_item action %p", action);

	menu_item = gtk_menu_item_new_with_label (label_text);

	g_value_unset (&value);
*/
	menu = build_menu (EPHY_MULTI_SMART_ACTION (action));
	gtk_widget_show (menu);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);

	return menu_item;
}

static void
sync_smartbookmark_id (EphyMultiSmartAction *action,
		       GParamSpec *pspec,
		       GtkWidget *proxy)
{
	EphyFaviconCache *cache;
	EphyBookmarks *bookmarks;
	EphyHistory *history;
	EphyNode *node;
	const char *location, *icon_location;
	GdkPixbuf *pixbuf = NULL;
	GtkWidget *icon;

	if (action->priv->mode != EPHY_MSM_MODE_SMART) return;

	icon = GTK_WIDGET (g_object_get_data (G_OBJECT (proxy), "icon"));
	g_return_if_fail (icon != NULL);

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	history = EPHY_HISTORY (ephy_embed_shell_get_global_history (embed_shell));

	node = ephy_bookmarks_get_from_id (bookmarks, action->priv->id);
	if (!EPHY_IS_NODE (node)) return;

	icon_location = ephy_node_get_property_string
				(node, EPHY_NODE_BMK_PROP_LOCATION);
	icon_location = ephy_history_get_icon (history, location);

	if (icon_location != NULL)
	{
		cache = EPHY_FAVICON_CACHE (ephy_embed_shell_get_favicon_cache (embed_shell));
		pixbuf = ephy_favicon_cache_get (cache, icon_location);
	}

	if (pixbuf != NULL)
	{
		gtk_image_set_from_pixbuf (GTK_IMAGE (icon), pixbuf);
		g_object_unref (pixbuf);
	}
	else
	{
		gtk_image_set_from_stock (GTK_IMAGE (icon),
					  GTK_STOCK_JUMP_TO,
					  GTK_ICON_SIZE_SMALL_TOOLBAR);
	}
}

static void
sync_mode (EphyMultiSmartAction *action,
	   GParamSpec *pspec,
	   GtkWidget *proxy)
{
	GtkWidget *hbox;

	if (action->priv->mode == EPHY_MSM_MODE_FIND)
	{
		GtkWidget *icon;

		icon = g_object_get_data (G_OBJECT (proxy), "icon");
		g_return_if_fail (icon != NULL);

		gtk_image_set_from_stock (GTK_IMAGE (icon), 
					  GTK_STOCK_FIND,
					  GTK_ICON_SIZE_SMALL_TOOLBAR);

	}
}

static gboolean
create_menu_proxy (GtkToolItem *item,
		   GtkAction *action)
{
	/* don't show in the overflow menu, it's useless */
	gtk_tool_item_set_proxy_menu_item (item, "ephy-multi-smart-action-menu-id", NULL);

	return TRUE;
}

extern void
ephy_gui_menu_position_under_widget (GtkMenu *menu,
				     gint *x,
				     gint *y,
				     gboolean *push_in,
				     gpointer user_data);

static void
icon_clicked_cb (GtkWidget *entry,
		 int button,
		 EphyMultiSmartAction *action)
{
	GtkWidget *menu;

	menu = build_menu (action);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
			ephy_gui_menu_position_under_widget, (gpointer) entry,
			button, gtk_get_current_event_time ());
}

static void
entry_activate_cb (GtkWidget *widget,
		   EphyMultiSmartAction *action)
{
	char *text = NULL;

	g_return_if_fail (GTK_IS_EDITABLE (widget));

	text = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1);

	LOG ("entry_activate_cb text='%s'", text);

	if (text == NULL || text[0] == '\0')
	{
		g_free (text);
		return;
	}

	if (action->priv->mode == EPHY_MSM_MODE_FIND)
	{
		EphyEmbed *embed;

		embed = ephy_window_get_active_embed (action->priv->window);
		g_return_if_fail (EPHY_IS_EMBED (embed));

		ephy_embed_find_set_properties (embed,
						text,
						action->priv->case_sensitive,
						TRUE /* wrap around */);

		ephy_embed_find_next (embed, FALSE /* forwards */);
	}
	else if (action->priv->mode == EPHY_MSM_MODE_SMART)
	{
		const char *smart_url;
		char *location;
		EphyBookmarks *bookmarks;
		EphyNode *node;
		GdkEvent *event;
		guint state = 0;
		gboolean in_new_tab;
		EphyTab *tab;
		EphyEmbed *embed;

		bookmarks = ephy_shell_get_bookmarks (ephy_shell);
		node = ephy_bookmarks_get_from_id (bookmarks, action->priv->id);
		g_return_if_fail (EPHY_IS_NODE (node));
		
		smart_url = ephy_node_get_property_string
				(node, EPHY_NODE_BMK_PROP_LOCATION);
		
		location = ephy_bookmarks_solve_smart_url (bookmarks,
							   smart_url, text);
		if (location == NULL)
		{
			return;
		}

		event = gtk_get_current_event ();
		if (event != NULL)
		{
			if (event->type == GDK_KEY_RELEASE)
			{
				state = event->button.state;
			}

			gdk_event_free (event);
		}
		in_new_tab = state & GDK_CONTROL_MASK;

		tab = ephy_window_get_active_tab (action->priv->window);

		if (in_new_tab)
		{
			tab = ephy_shell_new_tab (ephy_shell,
						  action->priv->window,
						  tab,
						  location,
						  EPHY_NEW_TAB_OPEN_PAGE |
						  EPHY_NEW_TAB_IN_EXISTING_WINDOW |
						  EPHY_NEW_TAB_JUMP);

			embed = ephy_tab_get_embed (tab);
		}
		else
		{
			embed = ephy_window_get_active_embed (action->priv->window);

			ephy_embed_load_url (embed, location);
		}

		ephy_embed_activate (embed);

		if (action->priv->clean_after_use)
		{
			gtk_editable_delete_text (GTK_EDITABLE (widget), 0, -1);
		}

		g_free (location);
	}

	g_free (text);
}

static void
connect_proxy (GtkAction *gaction,
	       GtkWidget *proxy)
{
	EphyMultiSmartAction *action = EPHY_MULTI_SMART_ACTION (gaction);

	GTK_ACTION_CLASS (parent_class)->connect_proxy (gaction, proxy);

	sync_mode (action, NULL, proxy);
	g_signal_connect_object (action, "notify::mode",
				 G_CALLBACK (sync_mode), proxy, 0);

	sync_smartbookmark_id (action, NULL, proxy);
	g_signal_connect_object (action, "notify::smartbookmark-id",
				 G_CALLBACK (sync_smartbookmark_id), proxy, 0);

	if (GTK_IS_TOOL_ITEM (proxy))
	{
		GtkWidget *button, *entry;

		g_signal_connect_object (proxy, "create_menu_proxy",
					 G_CALLBACK (create_menu_proxy),
					 action, 0);

		entry = GTK_WIDGET (g_object_get_data (G_OBJECT (proxy), "entry"));
		g_signal_connect (entry, "activate", G_CALLBACK (entry_activate_cb), action);
		g_signal_connect (entry, "icon_clicked", G_CALLBACK (icon_clicked_cb), action);
	}
}

static void
disconnect_proxy (GtkAction *action,
		  GtkWidget *proxy)
{
	GTK_ACTION_CLASS (parent_class)->disconnect_proxy (action, proxy);

	g_signal_handlers_disconnect_by_func (action,
					      G_CALLBACK (sync_mode),
					      proxy);
	g_signal_handlers_disconnect_by_func (action,
					      G_CALLBACK (sync_smartbookmark_id),
					      proxy);

	if (GTK_IS_TOOL_ITEM (proxy))
	{
		GtkWidget *entry;

		g_signal_handlers_disconnect_by_func (proxy,
						      G_CALLBACK (create_menu_proxy),
						      action);

		entry = GTK_WIDGET (g_object_get_data (G_OBJECT (proxy), "entry"));
		g_signal_handlers_disconnect_by_func (entry,
						      G_CALLBACK (entry_activate_cb),
						      action);
		g_signal_handlers_disconnect_by_func (entry,
						      G_CALLBACK (icon_clicked_cb),
						      action);
	}
}

static void
smartbookmarks_child_changed_cb (EphyNode *node,
				 EphyNode *child,
				 EphyMultiSmartAction *action)
{
	if (action->priv->id == ephy_node_get_id (child)
	    && action->priv->mode == EPHY_MSM_MODE_SMART)
	{
		g_object_notify (G_OBJECT (action), "smartbookmark-id");
	}
}

static void
smartbookmarks_child_removed_cb (EphyNode *node,
				 EphyNode *child,
				 guint old_index,
				 EphyMultiSmartAction *action)
{
	if (action->priv->id == ephy_node_get_id (child))
	{
		g_object_set (G_OBJECT (action),
			      "mode", EPHY_MSM_MODE_FIND,
			      "smartbookmark-id", 0,
			      NULL);
	}
}

static void
ephy_multi_smart_action_init (EphyMultiSmartAction *action)
{
	action->priv = EPHY_MULTI_SMART_ACTION_GET_PRIVATE (action);

	LOG ("EphyMultiSmartAction initialising %p", action);

	action->priv->window = NULL;
	action->priv->mode = EPHY_MSM_MODE_FIND;
	action->priv->id = 0;
	action->priv->case_sensitive = FALSE;
	action->priv->clean_after_use = TRUE;
}

static void
ephy_multi_smart_action_finalize (GObject *object)
{
	EphyMultiSmartAction *action = EPHY_MULTI_SMART_ACTION (object);
	guint i;

	LOG ("EphyMultiSmartAction finalising %p", object);

	for (i = 0; i < LAST_NOTIF; i++)
	{
		eel_gconf_notification_remove (action->priv->notifier_id[i]);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_multi_smart_action_set_mode (EphyMultiSmartAction *action,
				  EphyMultiSmartMode mode)
{
	LOG ("set_mode %d", mode);

	action->priv->mode = mode;

	g_object_notify (G_OBJECT (action), "mode");
}


static void
ephy_multi_smart_action_set_id (EphyMultiSmartAction *action,
				gulong id)
{
	LOG ("set_id %l", id);

	action->priv->id = id;

	g_object_notify (G_OBJECT (action), "smartbookmark-id");
}

static void
smart_address_notifier (GConfClient *client,
			guint cnxn_id,
			GConfEntry *entry,
			EphyMultiSmartAction *action)
{
	EphyBookmarks *bookmarks;
	EphyNode *node;
	char *address;

	address = eel_gconf_get_string (CONF_SMART_ADDRESS);
	if (address == NULL)
	{
		/* fall back to "find" mode */
		ephy_multi_smart_action_set_mode (action, EPHY_MSM_MODE_FIND);
		return;
	}

	LOG ("smart_address_notifier (%s)", address);

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	node = ephy_bookmarks_find_bookmark (bookmarks, address);
	g_free (address);

	if (EPHY_IS_NODE (node))
	{
		ephy_multi_smart_action_set_id
			(action, ephy_node_get_id (node));
	}
	else
	{
		/* not found, fall back to "find" mode */
		ephy_multi_smart_action_set_mode (action, EPHY_MSM_MODE_FIND);
	}
}

static void
mode_notifier (GConfClient *client,
	       guint cnxn_id,
	       GConfEntry *entry,
	       EphyMultiSmartAction *action)
{
	char *mode;

	mode = eel_gconf_get_string (CONF_MODE);

	LOG ("mode_notifier (mode %s)", mode);

	if (mode && strcmp (mode, mode_strings[EPHY_MSM_MODE_FIND]) == 0)
	{
		ephy_multi_smart_action_set_mode (action, EPHY_MSM_MODE_FIND);
	}
	else if (mode && strcmp (mode, mode_strings[EPHY_MSM_MODE_SMART]) == 0)
	{
		ephy_multi_smart_action_set_mode (action, EPHY_MSM_MODE_SMART);
	}
	else
	{
		g_warning ("Invalid mode %s\n", mode);
	}

	g_free (mode);
}

static void
clean_after_use_notifier (GConfClient *client,
		          guint cnxn_id,
		          GConfEntry *entry,
		          EphyMultiSmartAction *action)

{
	LOG ("clean_after_use_notifier");

	action->priv->clean_after_use = eel_gconf_get_boolean (CONF_CLEAN_AFTER_USE);
}

static void
case_sensitive_notifier (GConfClient *client,
			 guint cnxn_id,
			 GConfEntry *entry,
			 EphyMultiSmartAction *action)
{
	LOG ("case_sensitive_notifier");

	action->priv->case_sensitive = eel_gconf_get_boolean (CONF_CASE_SENSITIVE);
}

static void
ephy_multi_smart_action_set_window (EphyMultiSmartAction *action,
				    EphyWindow *window)
{
	EphyBookmarks *bookmarks;
	EphyNode *smartbookmarks;

	action->priv->window = window;

	clean_after_use_notifier (NULL, 0, NULL, action);
	action->priv->notifier_id[NOTIF_CLEAN_AFTER_USE] =
		eel_gconf_notification_add
			(CONF_CLEAN_AFTER_USE,
			 (GConfClientNotifyFunc) clean_after_use_notifier,
			 action);

	case_sensitive_notifier (NULL, 0, NULL, action);
	action->priv->notifier_id[NOTIF_CASE_SENSITIVE] =
		eel_gconf_notification_add
			(CONF_CASE_SENSITIVE,
			 (GConfClientNotifyFunc) case_sensitive_notifier,
			 action);

	mode_notifier (NULL, 0, NULL, action);
	action->priv->notifier_id[NOTIF_MODE] =
		eel_gconf_notification_add
			(CONF_MODE,
			 (GConfClientNotifyFunc) mode_notifier,
			 action);

	smart_address_notifier (NULL, 0, NULL, action);
	action->priv->notifier_id[NOTIF_SMART_ADDRESS] =
		eel_gconf_notification_add
			(CONF_SMART_ADDRESS,
			 (GConfClientNotifyFunc) smart_address_notifier,
			 action);

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	smartbookmarks = ephy_bookmarks_get_smart_bookmarks (bookmarks);

	ephy_node_signal_connect_object (smartbookmarks, EPHY_NODE_CHILD_CHANGED,
				         (EphyNodeCallback) smartbookmarks_child_changed_cb,
					 G_OBJECT (action));
	ephy_node_signal_connect_object (smartbookmarks, EPHY_NODE_CHILD_REMOVED,
					 (EphyNodeCallback) smartbookmarks_child_removed_cb,
					 G_OBJECT (action));
}

static void
ephy_multi_smart_action_set_property (GObject *object,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *pspec)
{
	EphyMultiSmartAction *action = EPHY_MULTI_SMART_ACTION (object);

	switch (prop_id)
	{
		case PROP_MODE:
			ephy_multi_smart_action_set_mode (action, g_value_get_int (value));
			break;
		case PROP_SMARTBOOKMARK_ID:
			ephy_multi_smart_action_set_id (action, g_value_get_ulong (value));
			break;
		case PROP_WINDOW:
			ephy_multi_smart_action_set_window (action, g_value_get_object (value));
			break;
	}
}

static void
ephy_multi_smart_action_get_property (GObject *object,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *pspec)
{
	EphyMultiSmartAction *action = EPHY_MULTI_SMART_ACTION (object);

	switch (prop_id)
	{
		case PROP_MODE:
			g_value_set_int (value, action->priv->mode);
			break;
		case PROP_SMARTBOOKMARK_ID:
			g_value_set_ulong (value, action->priv->id);
			break;
		case PROP_WINDOW:
			g_value_set_object (value, action->priv->window);
			break;
	}
}

static void
ephy_multi_smart_action_class_init (EphyMultiSmartActionClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GtkActionClass *action_class = GTK_ACTION_CLASS (class);

	parent_class = g_type_class_peek_parent (class);

	action_class->toolbar_item_type = GTK_TYPE_TOOL_ITEM;
	action_class->create_tool_item = create_tool_item;
	action_class->create_menu_item = create_menu_item;
	action_class->connect_proxy = connect_proxy;
	action_class->disconnect_proxy = disconnect_proxy;

	object_class->set_property = ephy_multi_smart_action_set_property;
	object_class->get_property = ephy_multi_smart_action_get_property;

	g_object_class_install_property (object_class,
                                         PROP_WINDOW,
                                         g_param_spec_object ("window",
                                                              "Window",
                                                              "Parent window",
                                                              EPHY_TYPE_WINDOW,
                                                              G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
                                         PROP_MODE,
                                         g_param_spec_int ("mode",
                                                           "Mode",
                                                           "Mode",
							   EPHY_MSM_MODE_FIND,
							   EPHY_MSM_MODE_SMART,
                                                           EPHY_MSM_MODE_FIND,
                                                           G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_SMARTBOOKMARK_ID,
					 g_param_spec_ulong ("smartbookmark-id",
							     "Smartbookmark-ID",
							     "Smartbookmark ID",
							     0,
							     G_MAXINT,
							     0,
							     G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (EphyMultiSmartActionPrivate));
}
