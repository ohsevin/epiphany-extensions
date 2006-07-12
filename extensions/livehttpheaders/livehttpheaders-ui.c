/*
*  Copyright (C) 2006 Jean-François Rameau
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

#include "livehttpheaders-ui.h"
#include "mozilla-helper.h"

#include "ephy-gui.h"
#include "ephy-debug.h"

#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-shell.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktextview.h>

#include <string.h>

#define LIVEHTTPHEADERS_UI_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), TYPE_LIVEHTTPHEADERS_UI, LiveHTTPHeadersUIPrivate))

struct _LiveHTTPHeadersUIPrivate
{
	GtkWidget *dialog;
	GtkListStore *store;
	/* TreeView with requests */
	GtkTreeView *treeview;
	/* A request/response couple */
	GtkTextView *a_request;
	GtkTextView *a_response;
	/* The dialog buttons */
	GtkButton *clear_cache;
	GtkButton *clear_headers;
	GtkButton *update;
	GtkButton *close;
};

enum
{
	PROP_DIALOG,
	PROP_URLS,
	PROP_REQUEST,
	PROP_RESPONSE,
	PROP_CLEAR_CACHE,
	PROP_CLEAR_HEADERS,
	PROP_UPDATE,
	PROP_CLOSE,
};

static const
EphyDialogProperty properties [] =
{
	{ "livehttpheaders_ui",	NULL, PT_NORMAL, 0 },
	{ "urls",		NULL, PT_NORMAL, 0 },
	{ "request",		NULL, PT_NORMAL, 0 },
	{ "response",		NULL, PT_NORMAL, 0 },
	{ "clear_cache",	NULL, PT_NORMAL, 0 },
	{ "clear_headers",	NULL, PT_NORMAL, 0 },
	{ "update",		NULL, PT_NORMAL, 0 },
	{ "close",		NULL, PT_NORMAL, 0 },
	{ NULL }
};

enum
{
	COL_URLS,
	N_COLUMNS
};

/* --- GType boilerplate --- */
G_DEFINE_TYPE (LiveHTTPHeadersUI,livehttpheaders_ui, EPHY_TYPE_DIALOG);

static void
livehttpheaders_ui_init (LiveHTTPHeadersUI *dialog)
{		
	dialog->priv = LIVEHTTPHEADERS_UI_GET_PRIVATE (dialog);
}

/* Helpers */

static void
livehttpheaders_ui_fill_model (GtkListStore *store)
{
	GSList       *frames, *framep;
	GtkTreeIter   iter;

	gtk_list_store_clear (store);	

	frames = get_frames ();

	for (framep = frames; framep != NULL; framep = framep->next)
	{ 
		LiveHTTPHeaderFrame *frame =  (LiveHTTPHeaderFrame *)(framep->data);

		gtk_list_store_append (store, &iter);

		gtk_list_store_set (store, &iter, COL_URLS, frame->url, -1);
	}
}

static void
livehttpheaders_ui_clear_fields (LiveHTTPHeadersUI *dialog)
{
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (dialog->priv->a_request);
	gtk_text_buffer_set_text (buffer, "", -1);
	buffer = gtk_text_view_get_buffer (dialog->priv->a_response);
	gtk_text_buffer_set_text (buffer, "", -1);
}

/* Callbacks */

static void
livehttpheaders_ui_response_cb (GtkWidget *widget, 
				int response, 
				LiveHTTPHeadersUI *dialog)
{
	if (response == GTK_RESPONSE_CLOSE)
	{
		g_object_unref (dialog);
	}
}

static void
livehttpheaders_ui_selection_changed_cb (GtkTreeSelection *selection,
				         LiveHTTPHeadersUI *dialog)
{	
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		GSList *frames, *framep;
		gchar *name;
		LiveHTTPHeadersUIPrivate *priv = dialog->priv;	
		GtkTextBuffer *buffer;
		LiveHTTPHeaderFrame *frame;
		
		gtk_tree_model_get (model, &iter, COL_URLS, &name, -1);

		frames = get_frames ();

		for (framep = frames; framep != NULL; framep = framep->next)
		{ 
			frame = (LiveHTTPHeaderFrame *) (framep->data);

			if (strcmp (frame->url, name) != 0) continue;

			buffer = gtk_text_view_get_buffer (priv->a_request);
			gtk_text_buffer_set_text (buffer, frame->request, -1);

			buffer = gtk_text_view_get_buffer (priv->a_response);
			gtk_text_buffer_set_text (buffer, 
						  frame->response == NULL ? "" : frame->response,
						  -1);
		}	

		g_free (name);
	}
}

static void
livehttpheaders_ui_clear_cache_cb (GtkButton *button, 
				   LiveHTTPHeadersUI *dialog)
{
	EphyEmbedShell *shell;
	EphyEmbedSingle *single;

	shell = ephy_embed_shell_get_default ();

	single = EPHY_EMBED_SINGLE (ephy_embed_shell_get_embed_single (shell));
	ephy_embed_single_clear_cache (single);
}

static void
livehttpheaders_ui_clear_headers_cb (GtkButton *button, 
				     LiveHTTPHeadersUI *dialog)
{
	livehttpheaders_ui_clear_fields (dialog);

	clear_frames ();

	livehttpheaders_ui_fill_model (dialog->priv->store);
}

static void
livehttpheaders_ui_update_cb (GtkButton *button, 
			      LiveHTTPHeadersUI *dialog)
{
	livehttpheaders_ui_clear_fields (dialog);

	livehttpheaders_ui_fill_model (dialog->priv->store);
}

static GtkTreeModel *
livehttpheaders_ui_create_and_fill_model (LiveHTTPHeadersUI *dialog)
{
	GtkListStore *store;

	store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);

	livehttpheaders_ui_fill_model (store);

	dialog->priv->store = store;

	return GTK_TREE_MODEL (store);
}

/* GUI */

static void
livehttpheaders_ui_create_view_and_model (LiveHTTPHeadersUI *dialog)
{
	GtkCellRenderer *renderer;
	GtkTreeModel    *model;
	GtkTreeView     *view;
	GtkTreeSelection *selection;

	view = dialog->priv->treeview;

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
						     -1,
						     "URLs",
						     renderer,
						     "text", COL_URLS,
						     NULL);
	selection = gtk_tree_view_get_selection (dialog->priv->treeview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	g_signal_connect (selection, "changed",
			  G_CALLBACK (livehttpheaders_ui_selection_changed_cb),
			  dialog);

	model = livehttpheaders_ui_create_and_fill_model (dialog);

	gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

	g_object_unref (model);
}

static GObject *
livehttpheaders_ui_constructor (GType type,
		    guint n_construct_properties,
		    GObjectConstructParam *construct_params)
{
	GObject *object;
	LiveHTTPHeadersUI *dialog;
	LiveHTTPHeadersUIPrivate *priv;
	EphyDialog *edialog;

	object =  G_OBJECT_CLASS (livehttpheaders_ui_parent_class)
			->constructor (type, n_construct_properties, construct_params);

	dialog = LIVEHTTPHEADERS_UI (object);
	edialog = EPHY_DIALOG (object);
	priv = dialog->priv;
	
	ephy_dialog_construct (EPHY_DIALOG (edialog),
			       properties,
			       SHARE_DIR "/glade/livehttpheaders-ui.glade",
			       "livehttpheaders_ui",
			       GETTEXT_PACKAGE);

	ephy_dialog_get_controls (edialog,
				  properties[PROP_DIALOG].id, &priv->dialog,
				  properties[PROP_URLS].id, &priv->treeview,			
				  properties[PROP_REQUEST].id, &priv->a_request,
				  properties[PROP_RESPONSE].id, &priv->a_response,
				  properties[PROP_CLEAR_HEADERS].id, &priv->clear_headers,
				  properties[PROP_CLEAR_CACHE].id, &priv->clear_cache,
				  properties[PROP_UPDATE].id, &priv->update,
				  properties[PROP_CLOSE].id, &priv->close,
				  NULL);

	g_signal_connect (priv->dialog, "response",
			  G_CALLBACK (livehttpheaders_ui_response_cb), dialog);
	g_signal_connect (priv->clear_headers, "clicked",
			  G_CALLBACK (livehttpheaders_ui_clear_headers_cb), dialog);
	g_signal_connect (priv->clear_cache, "clicked",
			  G_CALLBACK (livehttpheaders_ui_clear_cache_cb), dialog);
	g_signal_connect (priv->update, "clicked",
			  G_CALLBACK (livehttpheaders_ui_update_cb), dialog);

	/* Put the feeds in our store */
	livehttpheaders_ui_create_view_and_model (dialog);

	return object;
}

static void
livehttpheaders_ui_finalize (GObject *object)
{
	((GObjectClass *)livehttpheaders_ui_parent_class)->finalize (object);
}

static void
livehttpheaders_ui_class_init (LiveHTTPHeadersUIClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructor = livehttpheaders_ui_constructor;
	object_class->finalize = livehttpheaders_ui_finalize;

	g_type_class_add_private (object_class, sizeof (LiveHTTPHeadersUIPrivate));
}

LiveHTTPHeadersUI *
livehttpheaders_ui_new (void)
{
	return g_object_new (TYPE_LIVEHTTPHEADERS_UI, NULL);
}
