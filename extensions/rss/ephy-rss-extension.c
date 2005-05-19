/*
 *  Copyright (C) 2005 Raphaël Slinckx <raphael@slinckx.net>
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

#include "ephy-rss-extension.h"
#include "rss-ui.h"
#include "rss-dbus.h"
#include "rss-feedlist.h"

#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-dialog.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-statusbar.h>
#include <epiphany/ephy-notebook.h>
#include <epiphany/ephy-shell.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#include <gtk/gtkeventbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkimage.h>

#include <glib/gi18n-lib.h>

#define EPHY_RSS_EXTENSION_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE((object), EPHY_TYPE_RSS_EXTENSION, EphyRssExtensionPrivate))

struct _EphyRssExtensionPrivate
{
	RssUI *dialog;
};

static GObjectClass *parent_class = NULL;
static GType type = 0;

/* Menu item stuff */
#define WINDOW_DATA_KEY	"EphyRssExtensionWindowData"
#define MENU_PATH	"/menubar/ToolsMenu"

/* Status Bar stuff */

/* Test these
	"stock_hyperlink-toolbar"	<-- Globe with a link
	"stock_connect-to-url"		<-- Globe with a plug
	"stock_internet"			<-- Simple globe
*/
#define STOCK_ICON	"stock_hyperlink-toolbar"

static void ephy_rss_display_cb 	(GtkAction *action, 
					 EphyWindow *window);
static void ephy_rss_update_statusbar 	(EphyWindow *window,
					 gboolean show);
static void ephy_rss_update_action 	(EphyWindow *window);
static void ephy_rss_feed_subscribe_cb (GtkAction *action, 
					 EphyWindow *window);
typedef struct
{
	EphyRssExtension *extension;
	GtkActionGroup *action_group;
	GtkAction *info_action;
	GtkAction *subscribe_action;
	guint ui_id;

	GtkWidget *frame;
	GtkWidget *evbox;
} WindowData;

static const GtkActionEntry action_entries [] =
{
	{ "RssInfo",
	  NULL,
	  N_("News Feeds Subscription"),
	  NULL,
	  N_("Subscribe to this website's news feeds in your favorite news reader"),
	  G_CALLBACK (ephy_rss_display_cb)
	},
	{ "RssSubscribe",
	  NULL,
	  N_("_Subscribe to this feed"),
	  NULL,
	  N_("Subscribe to this feed in your favorite news reader"),
	  G_CALLBACK (ephy_rss_feed_subscribe_cb)
	},
};

/* We got an rss feed from a tab */
static void
ephy_rss_ge_feed_cb (EphyEmbed *embed,
		     const char *type,
		     const char *title,
		     const char *address,
		     EphyWindow *window)
{
	FeedList *list;

	list = (FeedList *) g_object_steal_data (G_OBJECT (embed), FEEDLIST_DATA_KEY);
	list = rss_feedlist_add (list, type, title, address);
	g_object_set_data_full (G_OBJECT (embed), FEEDLIST_DATA_KEY, list,
				(GDestroyNotify) rss_feedlist_free);

	LOG ("Got a new feed for the site: type=%s, title=%s, address=%s\nWe now have %d feeds", type, title, address, rss_feedlist_length (list));

	ephy_rss_update_action (window);
}

static void
ephy_rss_feed_subscribe_cb (GtkAction *action, 
			 EphyWindow *window)
{
	const GValue *value;

	LOG ("Subscribing to the feed");
	
	EphyEmbedEvent *event = EPHY_EMBED_EVENT (
			g_object_get_data (G_OBJECT (window), "context_event"));
	g_return_if_fail (event != NULL);
	
	ephy_embed_event_get_property (event, "link", &value);

	rss_dbus_subscribe_feed (g_value_get_string (value));
	
	g_object_set(action, "sensitive", FALSE, "visible", FALSE, NULL);	
}

static gboolean
ephy_rss_ge_context_cb	(EphyEmbed *embed,
			EphyEmbedEvent *event,
			EphyWindow *window)
{	
	WindowData *data;
	const GValue *value;
	const char *address;
	FeedList *list;
	gboolean active = FALSE;
	
	list = (FeedList *) g_object_get_data (G_OBJECT (embed), FEEDLIST_DATA_KEY);
	if ((ephy_embed_event_get_context (event) & EPHY_EMBED_CONTEXT_LINK) && (list != NULL))
	{
		LOG ("Context menu on a link");
		data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
		g_return_val_if_fail (data != NULL, FALSE);
		
		ephy_embed_event_get_property (event, "link", &value);
		address = g_value_get_string (value);
		
		active = rss_feedlist_contains (list, address);
		
		LOG ("Showing menu item: %d", active);
		g_object_set(data->subscribe_action, "sensitive", active, "visible", active, NULL);	
	}
	
	return FALSE;
}

static void
ephy_rss_dialog_display (EphyWindow *window)
{
	EphyRssExtensionPrivate *priv;
	EphyEmbed *embed;
	FeedList *list;
	WindowData *data;

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	priv = data->extension->priv;
	
	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);
	
	list = (FeedList *) g_object_get_data (G_OBJECT (embed), FEEDLIST_DATA_KEY);
	if (list == NULL)
		return;
	
	if (priv->dialog == NULL)
	{
		LOG ("Trying to build dialog");
		
		priv->dialog = rss_ui_new (list, embed);
		
		g_object_add_weak_pointer (G_OBJECT (priv->dialog),
					   (gpointer *) &priv->dialog);
	}

	ephy_dialog_set_parent (EPHY_DIALOG (priv->dialog),
				GTK_WIDGET (window));
	ephy_dialog_show (EPHY_DIALOG (priv->dialog));
}

/* Called when the user clicks on our menu item */
static void
ephy_rss_display_cb (GtkAction *action,
		     EphyWindow *window)
{
	ephy_rss_dialog_display (window);
}

/* When the pages changes, invalidate the previous feeds */
static void
ephy_rss_ge_content_cb (EphyEmbed *embed,
			char *uri,
			EphyWindow *window)
{
	LOG ("Page changed, invalidating previous feeds");

	g_object_set_data (G_OBJECT (embed), FEEDLIST_DATA_KEY, NULL);

	ephy_rss_update_action (window);
}

static void
ephy_rss_update_statusbar (EphyWindow *window,
			   gboolean show)
{
	WindowData *data;

	/* Show / Hide statusbar icon */
	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	g_return_if_fail (data->frame != NULL);
	
	g_object_set (data->frame, "visible", show, NULL);
}

/* Update the menu item availability */
static void
ephy_rss_update_action (EphyWindow *window)
{
	WindowData *data;
	FeedList *list;
	gboolean show = TRUE;
	EphyEmbed *embed;

	embed = ephy_window_get_active_embed (window);

	/* The page is loaded, do we have a feed ? */
	list = (FeedList *) g_object_get_data (G_OBJECT (embed), FEEDLIST_DATA_KEY);

	show = rss_feedlist_length (list) > 0;

	/* Disable the menu item when loading the page */
	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	g_object_set (data->info_action, "sensitive", show, NULL);

	ephy_rss_update_statusbar (window, show);
	
	g_object_set(data->subscribe_action, "sensitive", show, "visible", show, NULL);	
}
	
/* Called when the user changes tab */
static void
ephy_rss_sync_active_tab (EphyWindow *window,
			  GParamSpec *pspec,
			  gpointer data)
{
	if (GTK_WIDGET_REALIZED (window) == FALSE) return; /* on startup */

	ephy_rss_update_action (window);
}

/* Attach the callback to this new tab */
static void
impl_attach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("Attach rss listener to tab");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	/* Notify when a new rss feed is parsed */
	g_signal_connect_after (embed, "ge-content-change",
				G_CALLBACK (ephy_rss_ge_content_cb), window);
	g_signal_connect_after (embed, "ge-feed-link",
			    G_CALLBACK (ephy_rss_ge_feed_cb), window);
	g_signal_connect (embed, "ge-context-menu",
			    G_CALLBACK (ephy_rss_ge_context_cb), window);
}

                                             
/* Stop listening for the detached tab rss feeds */
static void
impl_detach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("Detach tab rss listener");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	/* We don't want any new rss notif for this tab */
	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (ephy_rss_ge_feed_cb), window);

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (ephy_rss_ge_content_cb), window);
	
	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (ephy_rss_ge_context_cb), window);
		
	/* destroy data */
	g_object_set_data (G_OBJECT (embed), FEEDLIST_DATA_KEY, NULL);
}

static gboolean
ephy_rss_statusbar_icon_clicked_cb (GtkWidget *widget,
				    GdkEventButton *event,
				    EphyWindow *window)
{
	if (event->button == 1)
	{
		ephy_rss_dialog_display (window);

		return TRUE;
	}

	return FALSE;
}

static void
ephy_rss_create_statusbar_icon (EphyWindow *window,
				WindowData *data)
{
	EphyStatusbar *statusbar;
	GtkWidget *icon;
	char *tooltip;

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	data->frame = gtk_frame_new (NULL);
	/* don't show the frame */

	data->evbox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (data->evbox), FALSE);
	gtk_container_add (GTK_CONTAINER (data->frame), data->evbox);
	gtk_widget_show (data->evbox);

	icon = gtk_image_new_from_icon_name (STOCK_ICON, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (data->evbox), icon);
	gtk_widget_show (icon);

	tooltip = g_strdup_printf (_("Subscribe to site's news feed"));
	gtk_tooltips_set_tip (statusbar->tooltips, data->evbox, tooltip, NULL);
	g_free (tooltip);

	ephy_statusbar_add_widget (statusbar, data->frame);

	g_signal_connect_after (data->evbox, "button-press-event",
				G_CALLBACK (ephy_rss_statusbar_icon_clicked_cb),
				window);
}

static void
ephy_rss_destroy_statusbar_icon (EphyWindow *window,
				 WindowData *data)
{
	EphyStatusbar *statusbar;
	
	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	g_return_if_fail (data->frame != NULL);
	g_return_if_fail (data->evbox != NULL);	

	gtk_tooltips_set_tip (statusbar->tooltips, GTK_WIDGET (data->evbox), NULL, NULL);

	ephy_statusbar_remove_widget (statusbar, GTK_WIDGET (data->frame));
}

/* Create the menu item to subscribe to an rss feed */
static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyRssExtension *extension = EPHY_RSS_EXTENSION (ext);
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	guint ui_id;
	WindowData *data;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	action_group = gtk_action_group_new ("EphyRssExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (action_group);

	ui_id = gtk_ui_manager_new_merge_id (manager);

	/* Add feed info to tools menu */
	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "RssInfoSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "RssInfo", "RssInfo",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	/* Add subscribe to popup context (normal document) */
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "RssInfoSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "RssSubscribe", "RssSubscribe",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
			       
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       "RssInfoSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       "RssSubscribe", "RssSubscribe",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
			       
	/* Register for tab switch events */
	ephy_rss_sync_active_tab (window, NULL, NULL);
	g_signal_connect_after (window, "notify::active-tab",
				G_CALLBACK (ephy_rss_sync_active_tab), NULL);
				
	/* store data */
	data = g_new (WindowData, 1);

	data->extension = extension;
	data->action_group = action_group;
	data->info_action = gtk_action_group_get_action (action_group, "RssInfo");
	data->subscribe_action = gtk_action_group_get_action (action_group, "RssSubscribe");
	data->ui_id = ui_id;
		
	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) g_free);

	/* Create the status bar icon */
	ephy_rss_create_statusbar_icon (window, data);
}

/* Delete the menu item to subscribe to a feed */
static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	/* Remove the menu item */
	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	/* Remove the tab switch notification */
	g_signal_handlers_disconnect_by_func
		(window, G_CALLBACK (ephy_rss_sync_active_tab), NULL);

	/* Remove the status bar icon */
	ephy_rss_destroy_statusbar_icon (window, data);

	/* Destroy data */
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

/* Epiphany init/dispose stuff ------------------------- */
static void
ephy_rss_extension_init (EphyRssExtension *extension)
{
	extension->priv = EPHY_RSS_EXTENSION_GET_PRIVATE (extension);
}

static void
ephy_rss_extension_finalize (GObject *object)
{
	EphyRssExtension *extension = EPHY_RSS_EXTENSION (object);

	/* Dispose the dialog */
	if (extension->priv->dialog != NULL)
	{
		g_object_unref (extension->priv->dialog);
		g_object_remove_weak_pointer (G_OBJECT (extension->priv->dialog),
					      (gpointer *) &extension->priv->dialog);
	}

	parent_class->finalize (object);
}

static void
ephy_rss_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_rss_extension_class_init (EphyRssExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_rss_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyRssExtensionPrivate));
}

GType
ephy_rss_extension_get_type (void)
{
	return type;
}

GType
ephy_rss_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyRssExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_rss_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyRssExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_rss_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_rss_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyRssExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
