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
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "error-viewer.h"
#include "ephy-debug.h"
#include "ephy-error-viewer-extension.h"

#include <gtk/gtk.h>

#define ERROR_VIEWER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_ERROR_VIEWER, ErrorViewerPrivate))

static void error_viewer_class_init (ErrorViewerClass *klass);
static void error_viewer_init (ErrorViewer *dialog);
static void error_viewer_finalize (GObject *object);

static GObjectClass *parent_class = NULL;

struct ErrorViewerPrivate
{
	GtkTreeModel *model;

	GtkWidget *window;
	GtkWidget *treeview;
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

GType 
error_viewer_get_type (void)
{
	static GType error_viewer_type = 0;
	
	if (error_viewer_type == 0)
	{
		static const GTypeInfo our_info =
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

		error_viewer_type = g_type_register_static (EPHY_TYPE_DIALOG,
							    "ErrorViewer",
							    &our_info, 0);
	}
	
	return error_viewer_type;
}

ErrorViewer *
error_viewer_new (void)
{
	return ERROR_VIEWER (g_object_new (TYPE_ERROR_VIEWER, NULL));
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
	const char *stock_id;

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

	gtk_list_store_append (GTK_LIST_STORE (dialog->priv->model), &iter);

	gtk_list_store_set (GTK_LIST_STORE (dialog->priv->model), &iter,
			    COL_ICON, stock_id,
			    COL_TEXT, text,
			    -1);
}

static void
build_ui (ErrorViewer *dialog)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;

	dialog->priv->window = ephy_dialog_get_control (EPHY_DIALOG (dialog),
							properties[PROP_WINDOW].id);
	dialog->priv->treeview = ephy_dialog_get_control (EPHY_DIALOG (dialog),
							  properties[PROP_TREEVIEW].id);
	/*
	g_object_unref (store);
	*/

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (dialog->priv->treeview),
						     COL_ICON, "Icon",
						     renderer,
						     "stock-id", COL_ICON,
						     NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (dialog->priv->treeview),
						     COL_TEXT, "Text",
						     renderer,
						     "text", COL_TEXT,
						     NULL);

	store = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_STRING,
				    G_TYPE_STRING,
				    G_TYPE_STRING);

	gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->priv->treeview),
				 GTK_TREE_MODEL (store));

	dialog->priv->model = GTK_TREE_MODEL (store);
}

static void
error_viewer_init (ErrorViewer *dialog)
{		
	dialog->priv = ERROR_VIEWER_GET_PRIVATE (dialog);

	ephy_dialog_construct (EPHY_DIALOG (dialog),
			       properties,
			       SHARE_DIR "/glade/error-viewer.glade",
			       "error_viewer");

	build_ui (dialog);
}

static void
error_viewer_finalize (GObject *object)
{
	ErrorViewer *dialog = ERROR_VIEWER (object);

	/* FIXME: Free TreeView stuff? */

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

void
error_viewer_clear_cb (GtkWidget *button,
		       EphyDialog *dialog)
{
	ErrorViewerPrivate *priv = ERROR_VIEWER_GET_PRIVATE (dialog);

	gtk_list_store_clear (GTK_LIST_STORE (priv->model));
}

void
error_viewer_close_cb (GtkWidget *button,
		       ErrorViewer *dialog)
{
	gtk_widget_hide (dialog->priv->window);
}
