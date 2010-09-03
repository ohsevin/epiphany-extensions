/*
*  Copyright © 2005 Raphaël Slinckx <raphael@slinckx.net>
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

#include "rss-ui.h"
#include <dbus/dbus-glib.h>

#include "ephy-gui.h"
#include "ephy-dnd.h"
#include "ephy-debug.h"
#include "ephy-string.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <string.h>

#define RSS_UI_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), TYPE_RSS_UI, RssUIPrivate))

struct _RssUIPrivate
{
	GtkWidget *dialog;
	GtkListStore *store;
	/* TreeView with feeds */
	GtkTreeView *treeview;
	/* The dialog buttons */
	GtkButton *subscribe;
	GtkButton *close;
	/* The title label */
	GtkLabel *title;
	/* The feed list */
	FeedList *list;
	/* Embed in which we have the feeds */
	EphyEmbed *embed;
	/* The proxy to the rss reader */
	DBusGProxy *proxy;
	/* A boolean flag indicating a dbus error */
	gboolean dbus_error;
	/* Extension reference to unset the dialog when it's unrefed */
	EphyRssExtension *extension;
};

enum
{
	PROP_WINDOW,
	PROP_LIST,
	PROP_EMBED,
	PROP_EXTENSION
};

enum
{
	COL_FEED,	/* NewsFeed* */
	COL_SEARCH,	/* Search column */
	COL_TOGGLE,	/* enabled? */
	COL_DISPLAY,	/* String representation */
	N_COLUMNS
};

typedef enum
{
	FEED_TYPE_RSS,
	FEED_TYPE_ATOM,
} FeedType;

typedef struct
{
	gboolean rss_present;
	gboolean atom_present;
	char 	*hostname;
} FeedSelectionDecision;

/* Drag and drop stuff */
static GtkTargetEntry drag_targets[] =
{
	{ EPHY_DND_URL_TYPE,	  0, 0 },
	{ EPHY_DND_TEXT_TYPE,	  0, 1 }
};

static GObjectClass *parent_class = NULL;
static GType type = 0;

static gboolean
rss_ui_subscribe_selected (GtkTreeModel *model,
			   GtkTreePath *path,
			   GtkTreeIter *iter,
			   RssUI *dialog)
{
	RssUIPrivate *priv = dialog->priv;
	NewsFeed *feed;
	gboolean selected, success;

	/* Get the feed */
	gtk_tree_model_get (model, iter, COL_TOGGLE, &selected, -1);
	gtk_tree_model_get (model, iter, COL_FEED, &feed, -1);

	LOG ("Trying to subscribe");

	if (selected && feed != NULL && feed->title != NULL && feed->type != NULL && feed->address != NULL)
	{
		GError *error = NULL;
		if (!dbus_g_proxy_call (priv->proxy, RSS_DBUS_SUBSCRIBE, &error,
			G_TYPE_STRING, feed->address,
			G_TYPE_INVALID,
			G_TYPE_BOOLEAN, &success,
			G_TYPE_INVALID))
		{
			LOG ("Error while retreiving method answer: %s", error->message);
			g_error_free (error);
			success = FALSE;
		}

		if (success == FALSE)
		{
			GtkWidget *image;
			LOG ("Failed to subscribe, certainly due to dbus..");

			gtk_label_set_markup (priv->title, _("<b><i>Unable to contact the feed reader, is it running ?</i></b>"));

			gtk_button_set_label (GTK_BUTTON (priv->subscribe), _("Retry"));
			image = gtk_image_new_from_stock (GTK_STOCK_REFRESH,  GTK_ICON_SIZE_BUTTON);
			gtk_button_set_image (GTK_BUTTON (priv->subscribe), image);

			priv->dbus_error = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

static void
rss_ui_response_cb (GtkWidget *widget,
		    int response,
		    EphyRssExtension *extension)
{
	RssUI *dialog = ephy_rss_extension_get_dialog (extension);
	RssUIPrivate *priv = dialog->priv;

	if (response == GTK_RESPONSE_OK)
	{
		gtk_tree_model_foreach (GTK_TREE_MODEL (priv->store),
					(GtkTreeModelForeachFunc) rss_ui_subscribe_selected,
					dialog);

		if (dialog->priv->dbus_error)
		{
			dialog->priv->dbus_error = FALSE;
			return;
		}
	}

	g_object_unref (dialog);
	ephy_rss_extension_set_dialog (extension, NULL);
}

static FeedType
rss_ui_get_feed_type (const char *type)
{
	g_return_val_if_fail (type != NULL, FEED_TYPE_RSS);

	if (g_ascii_strcasecmp (type, "application/rss+xml") == 0)
	{
		return FEED_TYPE_RSS;
	}
	else
	{
		return FEED_TYPE_ATOM;
	}
}

/* Called when the user clicks on a checkbox */
static void
rss_feed_toggle_cb (GtkCellRendererToggle *celltoggle,
		    const char *path_string,
		    RssUI *dialog)
{
	RssUIPrivate *priv = dialog->priv;
	GtkListStore *store = priv->store;
	GtkTreeModel *model = GTK_TREE_MODEL (priv->store);
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean active = FALSE;

	path = gtk_tree_path_new_from_string (path_string);

	if (!gtk_tree_model_get_iter (model, &iter, path))
	{
		gtk_tree_path_free (path);
		return;
	}

	gtk_tree_path_free (path);

	gtk_tree_model_get (model, &iter, COL_TOGGLE, &active, -1);
	gtk_list_store_set (store, &iter, COL_TOGGLE, !active, -1);
}

static NewsFeed *
rss_ui_get_selected_feed (RssUI *dialog)
{
	RssUIPrivate *priv = dialog->priv;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	NewsFeed *feed = NULL;

	selection = gtk_tree_view_get_selection (priv->treeview);
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, COL_FEED, &feed, -1);
	}

	return feed;
}

static void
rss_ui_treeview_page_copy_selected (GtkWidget *widget,
				    RssUI *dialog)
{
	NewsFeed *feed;

	LOG ("Copying selected feed");

	feed = rss_ui_get_selected_feed (dialog);
	if (feed != NULL)
	{
		gtk_clipboard_set_text (gtk_clipboard_get (GDK_NONE),
					feed->address, -1);
		gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY),
					feed->address, -1);

		rss_newsfeed_free (feed);
	}
}

static GtkMenu *
rss_ui_build_context_menu (RssUI *dialog)
{
	GtkWidget *menu;
	GtkWidget *item, *image;

	LOG ("Building context menu");

	menu = gtk_menu_new ();

	image = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image);

	item = gtk_image_menu_item_new_with_mnemonic (_("_Copy Feed Address"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect (item, "activate",
			  G_CALLBACK (rss_ui_treeview_page_copy_selected),
			  dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	return GTK_MENU (menu);
}

static gboolean
rss_ui_treeview_show_popup (GtkTreeView *treeview,
			    RssUI *dialog)
{
	GtkMenu *menu;

	LOG ("Got a keyboard popup");

	menu = rss_ui_build_context_menu (dialog);
	gtk_menu_popup (menu, NULL, NULL,
			ephy_gui_menu_position_tree_selection, treeview, 0,
			gtk_get_current_event_time ());
	gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);

	return TRUE;
}

static gboolean
rss_ui_treeview_button_pressed_cb (GtkTreeView *treeview,
				   GdkEventButton *event,
				   RssUI *dialog)
{
	GtkTreePath *path = NULL;
	GtkMenu *menu;

	LOG ("Got a button press");

	/* right-click? */
	if (event->button != 3)
		return FALSE;

	/* Get tree path for row that was clicked */
	if (!gtk_tree_view_get_path_at_pos (treeview,
					    event->x, event->y,
					    &path, NULL, NULL, NULL))
	{
		return FALSE;
	}

	/* now popup the menu */
	menu = rss_ui_build_context_menu (dialog);
	gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
			event->button, event->time);

	return FALSE;
}

/* Set dnd cursor to the default dnd Gtk one and not the GtktreeView one */
static void
rss_ui_drag_begin_cb (GtkWidget *widget,
		      GdkDragContext *dc,
		      RssUI *dialog)
{
	gtk_drag_set_icon_default (dc);
}

static void
rss_ui_drag_data_get_cb (GtkWidget *widget,
			 GdkDragContext *context,
			 GtkSelectionData *selection_data,
			 guint info,
			 guint32 time,
			 RssUI *dialog)
{
	NewsFeed *feed;

	LOG ("Drag trying to get data");

	feed = rss_ui_get_selected_feed (dialog);
	if (feed == NULL || feed->address == NULL) return;

	gtk_selection_data_set (selection_data,
			        gtk_selection_data_get_target (selection_data),
			        8,
			        (const guchar*) feed->address,
			        strlen (feed->address));

	rss_newsfeed_free (feed);
}

static gboolean
rss_ui_select_feeds (GtkTreeModel *model,
		     GtkTreePath *path,
		     GtkTreeIter *iter,
		     gpointer data)
{
	FeedSelectionDecision *decision = (FeedSelectionDecision *) data;
	GtkListStore *store = GTK_LIST_STORE (model);
	NewsFeed *feed;
	gboolean selected = TRUE;

	LOG ("Selecting appropriate feeds");

	/* Get the feed */
	gtk_tree_model_get (model, iter, COL_FEED, &feed, -1);

	/* If we have rss feeds, check if atom is on the same host */
	if (decision->hostname != NULL &&
	    decision->rss_present &&
	    rss_ui_get_feed_type (feed->type) == FEED_TYPE_ATOM)
	{
		/* Atom is on the same host, probably a duplicate */
		const char *host = ephy_string_get_host_name (feed->address);

		selected = g_ascii_strcasecmp (decision->hostname, host) != 0;
	}

	gtk_list_store_set (store, iter, COL_TOGGLE, selected, -1);

	return FALSE;
}

static void
rss_ui_fill_list_store (RssUI *dialog,
			FeedSelectionDecision *decision)
{
	RssUIPrivate *priv = dialog->priv;
	GSList *l, *list = (GSList *) priv->list;

	gtk_list_store_clear (priv->store);

	for (l = list; l != NULL; l = l->next)
	{
		const NewsFeed *feed = (const NewsFeed *) l->data;
		char *display;
		FeedType type;
		GtkTreeIter iter;

		LOG ("Feed: %s|%s|%s", feed->title, feed->address, feed->type);

		/* Determine feed type */
		type = rss_ui_get_feed_type (feed->type);
		decision->rss_present |= (type == FEED_TYPE_RSS);
		decision->atom_present |= (type == FEED_TYPE_ATOM);

		display = g_markup_printf_escaped ("<b>%s</b>\n%s",
						   feed->title, feed->address);

		gtk_list_store_append (priv->store, &iter);
		gtk_list_store_set (priv->store, &iter,
				    COL_FEED, feed,
				    COL_SEARCH, feed->title,
				    COL_TOGGLE, FALSE,
				    COL_DISPLAY, display,
				    -1);

		g_free (display);
	}
}

static void
rss_ui_populate_store (RssUI *dialog)
{
	RssUIPrivate *priv = dialog->priv;
	FeedSelectionDecision decision = { FALSE, FALSE, NULL };
	char *location;

	if (priv->embed == NULL) return;

	/* We start populating the list, and try to select as much as possible
	 * avoiding duplicate feeds
	 */
	location = ephy_web_view_get_location
			(ephy_embed_get_web_view (priv->embed), TRUE);
	decision.hostname = g_strdup (ephy_string_get_host_name (location));

	/* Fill the store, and select the appropriate feeds */
	rss_ui_fill_list_store (dialog, &decision);

	gtk_tree_model_foreach (GTK_TREE_MODEL (priv->store),
				(GtkTreeModelForeachFunc) rss_ui_select_feeds,
				&decision);

	g_free (location);
	g_free (decision.hostname);
}

static void
rss_ui_init (RssUI *dialog)
{
	DBusGConnection *connection;
	GError *error = NULL;

	dialog->priv = RSS_UI_GET_PRIVATE (dialog);

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL)
	{
		LOG ("No connection to dbus:%s", error->message);
		g_error_free (error);
		return;
	}

	dialog->priv->proxy = dbus_g_proxy_new_for_name (connection,
                                     RSS_DBUS_SERVICE,
                                     RSS_DBUS_OBJECT_PATH,
                                     RSS_DBUS_INTERFACE);
}

static GObject *
rss_ui_constructor (GType type,
		    guint n_construct_properties,
		    GObjectConstructParam *construct_params)
{
	GObject *object;
	RssUI *dialog;
	RssUIPrivate *priv;
	EphyDialog *edialog;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;
	EphyRssExtension *extension;

	object = parent_class->constructor (type, n_construct_properties,
					    construct_params);

	dialog = RSS_UI (object);
	edialog = EPHY_DIALOG (object);
	priv = dialog->priv;

	ephy_dialog_construct (EPHY_DIALOG (edialog),
			       SHARE_DIR "/ui/rss-ui.ui",
			       "rss_ui",
			       GETTEXT_PACKAGE);

	ephy_dialog_get_controls (edialog,
				  "rss_ui", &priv->dialog,
				  "title", &priv->title,
				  "feeds", &priv->treeview,
				  "subscribe", &priv->subscribe,
				  "close", &priv->close,
				  NULL);

	g_object_get (object,
		      "extension", &extension,
		      NULL);
	g_signal_connect (priv->dialog, "response",
			  G_CALLBACK (rss_ui_response_cb), extension);

	priv->store = gtk_list_store_new (N_COLUMNS,
					  RSS_TYPE_NEWSFEED,
					  G_TYPE_STRING,
					  G_TYPE_BOOLEAN,
					  G_TYPE_STRING);

	/* Put the feeds in our store */
	rss_ui_populate_store (dialog);

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer, "toggled",
			  G_CALLBACK (rss_feed_toggle_cb), dialog);

	gtk_tree_view_insert_column_with_attributes (priv->treeview,
						     COL_TOGGLE, _("Subscribe"),
						     renderer,
						     "active", COL_TOGGLE,
						     NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_insert_column_with_attributes (priv->treeview,
						     COL_DISPLAY, _("Description"),
						     renderer,
						     "markup", COL_DISPLAY,
						     NULL);

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (priv->store),
					      COL_DISPLAY, GTK_SORT_ASCENDING);

	gtk_tree_view_set_model (priv->treeview, GTK_TREE_MODEL (priv->store));
	gtk_tree_view_set_search_column (priv->treeview, COL_SEARCH);

	g_object_unref (priv->store);

	selection = gtk_tree_view_get_selection (priv->treeview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	/* Build the context menu stuff */
	g_signal_connect (priv->treeview, "popup-menu",
			  G_CALLBACK (rss_ui_treeview_show_popup), dialog);
	g_signal_connect (priv->treeview, "button-press-event",
			  G_CALLBACK (rss_ui_treeview_button_pressed_cb), dialog);

	/* Set up drag and drop */
	gtk_tree_view_enable_model_drag_source (priv->treeview, GDK_BUTTON1_MASK,
						drag_targets, G_N_ELEMENTS (drag_targets),
						GDK_ACTION_COPY);
	g_signal_connect (priv->treeview, "drag_data_get",
			  G_CALLBACK(rss_ui_drag_data_get_cb), dialog);

	/* Be careful, default handler forces dnd icon to be an image of the row ! */
	g_signal_connect_after (priv->treeview, "drag_begin",
				G_CALLBACK (rss_ui_drag_begin_cb), dialog);

	return object;
}

static void
rss_ui_finalize (GObject *object)
{
	RssUI *dialog = RSS_UI (object);
	RssUIPrivate *priv = dialog->priv;

	g_object_unref (priv->proxy);
	rss_feedlist_free (priv->list);

	parent_class->finalize (object);
}

static void
rss_ui_get_property (GObject *object,
				guint prop_id,
				GValue *value,
				GParamSpec *pspec)
{
	RssUI *dialog = RSS_UI (object);

	switch (prop_id)
	{
		case PROP_EXTENSION:
			g_value_set_object (value, dialog->priv->extension);
			break;
	}
}

static void
rss_ui_set_property (GObject *object,
				guint prop_id,
				const GValue *value,
				GParamSpec *pspec)
{
	RssUI *dialog = RSS_UI (object);

	switch (prop_id)
	{
		case PROP_LIST:
			dialog->priv->list = g_value_dup_boxed (value);
			break;
		case PROP_EMBED:
			dialog->priv->embed = g_value_get_object (value);
			break;
		case PROP_EXTENSION:
			dialog->priv->extension = g_value_get_object (value);
			break;
	}
}

static void
rss_ui_class_init (RssUIClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->constructor = rss_ui_constructor;
	object_class->finalize = rss_ui_finalize;
	object_class->get_property = rss_ui_get_property;
	object_class->set_property = rss_ui_set_property;

	g_object_class_install_property
		(object_class,
		 PROP_LIST,
		 g_param_spec_boxed ("list",
				     "Feed List",
				     "Feed List",
				     RSS_TYPE_FEEDLIST,
				     G_PARAM_WRITABLE |
				     G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property
		(object_class,
		 PROP_EMBED,
		 g_param_spec_object ("embed",
				      "Embed",
				      "Embed",
				      GTK_TYPE_WIDGET,
				      G_PARAM_WRITABLE |
				      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property
		(object_class,
		 PROP_EXTENSION,
		 g_param_spec_object ("extension",
				      "Extension",
				      "Extension",
				      EPHY_TYPE_RSS_EXTENSION,
				      G_PARAM_READABLE |
				      G_PARAM_WRITABLE |
				      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (RssUIPrivate));
}

GType
rss_ui_get_type (void)
{
	return type;
}

GType
rss_ui_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (RssUIClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) rss_ui_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (RssUI),
		0, /* n_preallocs */
		(GInstanceInitFunc) rss_ui_init
	};

	type = g_type_module_register_type (module,
					    EPHY_TYPE_DIALOG,
					    "RssUI",
					    &our_info, 0);

	return type;
}

RssUI *
rss_ui_new (FeedList *list,
	    EphyEmbed *embed,
	    EphyRssExtension *extension)
{
	return g_object_new (TYPE_RSS_UI,
			     "list",  list,
			     "embed", embed,
			     "extension", extension,
			     NULL);
}
