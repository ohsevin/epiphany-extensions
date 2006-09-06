/*
 *  Copyright (C) 2002 Jorn Baayen
 *  Copyright (C) 2004 Adam Hooper
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

#include "error-viewer.h"
#include "ephy-debug.h"
#include "ephy-error-viewer-extension.h"

#include <gtk/gtk.h>

#define ERROR_VIEWER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_ERROR_VIEWER, ErrorViewerPrivate))

#define MAX_NUM_ROWS 400

struct ErrorViewerPrivate
{
	GtkTreeModel *model;

	GtkWidget *window;
	GtkWidget *treeview;
	gint num_active;
};

enum
{
	PROP_WINDOW,
	PROP_TREEVIEW,
};

static const
EphyDialogProperty properties [] =
{
	{ "error_viewer",	NULL, PT_NORMAL, 0 },
	{ "error_list",		NULL, PT_NORMAL, 0 },

	{ NULL }
};

enum
{
	COL_ICON,
	COL_TEXT,
	N_COLUMNS
};

static void error_viewer_class_init	(ErrorViewerClass *klass);
static void error_viewer_init		(ErrorViewer *dialog);
static void error_viewer_finalize	(GObject *object);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType 
error_viewer_get_type (void)
{
	return type;
}

GType
error_viewer_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (ErrorViewerClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) error_viewer_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (ErrorViewer),
		0, /* n_preallocs */
		(GInstanceInitFunc) error_viewer_init
	};

	type = g_type_module_register_type (module,
					    EPHY_TYPE_DIALOG,
					    "ErrorViewer",
					    &our_info, 0);

	return type;
}

ErrorViewer *
error_viewer_new (void)
{
	return ERROR_VIEWER (g_object_new (TYPE_ERROR_VIEWER,
					   "persist-position", TRUE,
					   NULL));
}

static void
error_viewer_class_init (ErrorViewerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = error_viewer_finalize;

	g_type_class_add_private (object_class, sizeof (ErrorViewerPrivate));
}

void
error_viewer_append (ErrorViewer *dialog,
		     ErrorViewerErrorType type,
		     const char *text)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	const char *stock_id;
	int num_rows;

	switch (type)
	{
		case ERROR_VIEWER_ERROR:
			stock_id = GTK_STOCK_DIALOG_ERROR;
			break;
		case ERROR_VIEWER_WARNING:
			stock_id = GTK_STOCK_DIALOG_WARNING;
			break;
		case ERROR_VIEWER_INFO:
			stock_id = GTK_STOCK_DIALOG_INFO;
			break;
		default:
			g_return_if_reached ();
	}

	model = dialog->priv->model;

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			    COL_ICON, stock_id,
			    COL_TEXT, text,
			    -1);


	num_rows = gtk_tree_model_iter_n_children (model, NULL);

	while (num_rows > MAX_NUM_ROWS)
	{
		gtk_tree_model_get_iter_first (model, &iter);

		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

		num_rows--;
	}

	/* XXX: Only do this sometimes? (i.e., not when validating HTML */
	gtk_tree_model_iter_nth_child (model, &iter, NULL, num_rows - 1);

	path = gtk_tree_model_get_path (model, &iter);

	gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (dialog->priv->treeview),
				      path,
				      NULL, FALSE, 0, 0);

	gtk_tree_path_free (path);
}

static gboolean
delete_window_cb (GtkWidget *widget,
		  GdkEvent *event,
		  gpointer data)
{
	gtk_widget_hide (widget);

	return TRUE;
}

static void
response_cb (GtkWidget *widget,
	     int response,
	     ErrorViewer *dialog)
{
	ErrorViewerPrivate *priv = dialog->priv;

	if (response == 1 /* Clear */)
	{
		gtk_list_store_clear (GTK_LIST_STORE (priv->model));
		gtk_tree_view_columns_autosize (GTK_TREE_VIEW (priv->treeview));
	
		return;
	}

	gtk_widget_hide (widget);
}

static void
build_ui (ErrorViewer *dialog)
{
	GtkListStore *store;
	GtkTreeView *treeview;
	GtkCellRenderer *renderer;
	ErrorViewerPrivate *priv = dialog->priv;

	priv->window = ephy_dialog_get_control (EPHY_DIALOG (dialog),
						properties[PROP_WINDOW].id);
	priv->treeview = ephy_dialog_get_control (EPHY_DIALOG (dialog),
						  properties[PROP_TREEVIEW].id);

	gtk_window_set_icon_name (GTK_WINDOW (priv->window),
				  GTK_STOCK_DIALOG_ERROR);

	g_signal_connect (priv->window, "delete-event",
			  G_CALLBACK (delete_window_cb), NULL);
	g_signal_connect (priv->window, "response",
			  G_CALLBACK (response_cb), dialog);

	treeview = GTK_TREE_VIEW (priv->treeview);

	renderer = gtk_cell_renderer_pixbuf_new ();
	g_object_set (G_OBJECT (renderer),
		      "stock-size", GTK_ICON_SIZE_BUTTON,
		      "xpad", 6,
		      NULL);
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_ICON, "Icon",
						     renderer,
						     "stock-id", COL_ICON,
						     NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),
		      "mode", GTK_CELL_RENDERER_MODE_INERT,
		      "editable", TRUE,
		      NULL);
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_TEXT, "Text",
						     renderer,
						     "text", COL_TEXT,
						     NULL);

	store = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_STRING,
				    G_TYPE_STRING,
				    G_TYPE_STRING);

	gtk_tree_view_set_model (treeview,
				 GTK_TREE_MODEL (store));

	priv->model = GTK_TREE_MODEL (store);
}

static void
error_viewer_init (ErrorViewer *dialog)
{		
	dialog->priv = ERROR_VIEWER_GET_PRIVATE (dialog);

	dialog->priv->num_active = 0;

	ephy_dialog_construct (EPHY_DIALOG (dialog),
			       properties,
			       SHARE_DIR "/glade/error-viewer.glade",
			       "error_viewer",
			       GETTEXT_PACKAGE);

	build_ui (dialog);
}

static void
error_viewer_finalize (GObject *object)
{
//	ErrorViewer *dialog = ERROR_VIEWER (object);

	/* FIXME: Free TreeView stuff? */

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
update_cursor (ErrorViewer *dialog)
{
	if (dialog->priv->num_active > 0)
	{
		GdkCursor *cursor;

		cursor = gdk_cursor_new (GDK_WATCH);
		gdk_window_set_cursor (dialog->priv->window->window, cursor);
		gdk_cursor_unref (cursor);
	}
	else
	{
		gdk_window_set_cursor (dialog->priv->window->window, NULL);
	}
}

void error_viewer_use (ErrorViewer *dialog)
{
	g_return_if_fail (IS_ERROR_VIEWER (dialog));

	dialog->priv->num_active++;
	update_cursor (dialog);
}

void error_viewer_unuse (ErrorViewer *dialog)
{
	g_return_if_fail (IS_ERROR_VIEWER (dialog));

	dialog->priv->num_active--;
	update_cursor (dialog);
}
