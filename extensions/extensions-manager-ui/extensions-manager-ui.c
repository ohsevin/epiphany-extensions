/*
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

#include "extensions-manager-ui.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extensions-manager.h>
#include <epiphany/ephy-shell.h>

#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrenderertoggle.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkwindow.h>

#include <string.h>

#include <glib/gi18n-lib.h>

#define IGNORE_SELF /* Don't display *this* extension in the list */

#define EXTENSIONS_MANAGER_UI_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_EXTENSIONS_MANAGER_UI, ExtensionsManagerUIPrivate))

struct ExtensionsManagerUIPrivate
{
	GtkTreeModel *model;
	GtkWidget *window;
	GtkWidget *treeview;
	EphyExtensionsManager *manager;
	gulong added_signal;
	gulong changed_signal;
	gulong removed_signal;
};

enum
{
	PROP_WINDOW,
	PROP_TREEVIEW,
};

static const
EphyDialogProperty properties [] =
{
	{ "extensions_manager_ui",	NULL, PT_NORMAL, 0 },
	{ "extensions_treeview",	NULL, PT_NORMAL, 0 },

	{ NULL }
};

enum
{
	COL_INFO,	/* EphyExtensionInfo* */
	COL_TOGGLE,	/* enabled? */
	COL_DISPLAY,	/* String representation */
	N_COLUMNS
};

/* glade callbacks */
void extensions_manager_ui_response_cb (GtkWidget *button,
					int response,
				  	GObject *dialog);
void extensions_manager_ui_info_dialog_response_cb (GtkWidget *button,
						    int response,
						    GObject *dialog);

static void extensions_manager_ui_class_init	(ExtensionsManagerUIClass *klass);
static void extensions_manager_ui_init		(ExtensionsManagerUI *dialog);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType 
extensions_manager_ui_get_type (void)
{
	return type;
}

GType
extensions_manager_ui_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (ExtensionsManagerUIClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) extensions_manager_ui_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (ExtensionsManagerUI),
		0, /* n_preallocs */
		(GInstanceInitFunc) extensions_manager_ui_init
	};

	type = g_type_module_register_type (module,
					    EPHY_TYPE_DIALOG,
					    "ExtensionsManagerUI",
					    &our_info, 0);

	return type;
}

void
extensions_manager_ui_response_cb (GtkWidget *widget,
				   int response,
				   GObject *dialog)
{
	if (response == GTK_RESPONSE_HELP)
	{
		/* FIXME not implemented yet */
		return;
	}

	g_object_unref (dialog);
}

void
extensions_manager_ui_info_dialog_response_cb (GtkWidget *widget,
					       int response,
					       GObject *dialog)
{
	g_object_unref (dialog);
}

static const char **
string_list_to_array (GList *list)
{
	GList *l;
	int i = 0;
	const char **ret = malloc
		((g_list_length (list) + 1) * sizeof (char *));

	for (l = list; l; l = l->next)
	{
		ret[i++] = (const char *) l->data;
	}

	ret[i] = NULL;

	return ret;
}

static void
show_extension_info (ExtensionsManagerUI *parent_dialog,
		     EphyExtensionInfo *info)
{
	GtkAboutDialog *dialog;
	const char **authors;

	dialog = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());
	gtk_about_dialog_set_name (dialog, info->name);
	gtk_about_dialog_set_comments (dialog, info->description);
	gtk_about_dialog_set_website (dialog, info->url);

	authors = string_list_to_array (info->authors);
	gtk_about_dialog_set_authors (dialog, (const char **) authors);
	free (authors);

	gtk_window_set_transient_for (GTK_WINDOW (dialog),
				      GTK_WINDOW (parent_dialog->priv->window));
	gtk_window_present (GTK_WINDOW (dialog));
}

static char *
display_from_info (EphyExtensionInfo *info)
{
	return g_strdup_printf ("<b>%s</b>\n%s",
				info->name,
				info->description);
}

static void
fill_list_store (EphyExtensionsManager *manager,
		 GtkListStore *store)
{
	GList *extensions, *l;
	EphyExtensionInfo *info;
	GtkTreeIter iter;
	char *display;

	gtk_list_store_clear (store);

	extensions = ephy_extensions_manager_get_extensions (manager);

	for (l = extensions; l != NULL; l = l->next)
	{
		info = (EphyExtensionInfo *) l->data;

#ifdef IGNORE_SELF
		if (strcmp (info->identifier, "extensions-manager-ui") == 0)
		{
			continue;
		}
#endif /* IGNORE_SELF */

		gtk_list_store_append (store, &iter);

		display = display_from_info (info);

		gtk_list_store_set (store, &iter,
				    COL_INFO, info,
				    COL_TOGGLE, info->active,
				    COL_DISPLAY, display,
				    -1);

		g_free (display);
	}

	g_list_free (extensions);
}

static void
extension_toggle_cb (GtkCellRendererToggle *celltoggle,
		     const char *path_string,
		     ExtensionsManagerUI *dialog)
{
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean active;
	EphyExtensionInfo *info;

	treeview = GTK_TREE_VIEW (dialog->priv->treeview);
	g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

	model = gtk_tree_view_get_model (treeview);

	path = gtk_tree_path_new_from_string (path_string);

	if (!gtk_tree_model_get_iter (model, &iter, path))
	{
		return;
	}

	gtk_tree_path_free (path);

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COL_INFO, &info,
			    COL_TOGGLE, &active,
			    -1);

	active = !active;

	if (active)
	{
		ephy_extensions_manager_load (dialog->priv->manager,
					      info->identifier);
	}
	else
	{
		ephy_extensions_manager_unload (dialog->priv->manager,
						info->identifier);
	}
}

static void
active_sync (EphyExtensionsManager *manager,
	     EphyExtensionInfo *info,
	     ExtensionsManagerUI *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	EphyExtensionInfo *row_info;

	model = dialog->priv->model;

	if (gtk_tree_model_get_iter_first (model, &iter) == FALSE) return;

	do
	{
		gtk_tree_model_get (model, &iter, COL_INFO, &row_info, -1);

		if (row_info == info)
		{
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
					    COL_TOGGLE, info->active,
					    -1);
			break;
		}
	}
	while (gtk_tree_model_iter_next (model, &iter));
}

static void
row_activated (GtkTreeView *treeview,
	       GtkTreePath *path,
	       GtkTreeViewColumn *col,
	       ExtensionsManagerUI *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	EphyExtensionInfo *info;

	model = gtk_tree_view_get_model (treeview);

	if (!gtk_tree_model_get_iter (model, &iter, path))
	{
		return;
	}

	gtk_tree_model_get (model, &iter,
			    COL_INFO, &info,
			    -1);

	show_extension_info (dialog, info);
}

static void
build_ui (ExtensionsManagerUI *dialog)
{
	GtkListStore *store;
	GtkTreeView *treeview;
	GtkCellRenderer *renderer;
	ExtensionsManagerUIPrivate *priv = dialog->priv;

	priv->window = ephy_dialog_get_control (EPHY_DIALOG (dialog),
						properties[PROP_WINDOW].id);
	priv->treeview = ephy_dialog_get_control (EPHY_DIALOG (dialog),
						  properties[PROP_TREEVIEW].id);

	gtk_window_set_icon_name (GTK_WINDOW (priv->window), GTK_STOCK_PREFERENCES);

	treeview = GTK_TREE_VIEW (priv->treeview);

	g_signal_connect (G_OBJECT (treeview), "row-activated",
			  G_CALLBACK (row_activated), dialog);

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (G_OBJECT (renderer), "toggled",
			  G_CALLBACK (extension_toggle_cb), dialog);
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_TOGGLE, _("Enabled"),
						     renderer,
						     "active", COL_TOGGLE,
						     NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_DISPLAY, _("Description"),
						     renderer,
						     "markup", COL_DISPLAY,
						     NULL);

	store = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_POINTER,
				    G_TYPE_BOOLEAN,
				    G_TYPE_STRING);
	fill_list_store (dialog->priv->manager, store);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store),
					      COL_DISPLAY, GTK_SORT_ASCENDING);

	gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));

	priv->model = GTK_TREE_MODEL (store);
	g_object_unref (store);
}

static void
extension_added_cb (EphyExtensionsManager *manager,
		    EphyExtensionInfo *info,
		    ExtensionsManagerUI *dialog)
{
	GtkTreeView *treeview;
	GtkListStore *store;
	GtkTreeIter iter;
	char *display;

	treeview = GTK_TREE_VIEW (dialog->priv->treeview);
	g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

	store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));
	g_return_if_fail (GTK_IS_LIST_STORE (store));

	gtk_list_store_append (store, &iter);

	display = display_from_info (info);

	gtk_list_store_set (store, &iter,
			    COL_INFO, info,
			    COL_TOGGLE, info->active,
			    COL_DISPLAY, display,
			    -1);

	g_free (display);
}

static void
extension_removed_cb (EphyExtensionsManager *manager,
		      EphyExtensionInfo *info,
		      ExtensionsManagerUI *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	EphyExtensionInfo *row_info;

	model = dialog->priv->model;

	if (gtk_tree_model_get_iter_first (model, &iter) == FALSE) return;

	do
	{
		gtk_tree_model_get (model, &iter, COL_INFO, &row_info, -1);

		if (row_info == info)
		{
			gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
			break;
		}
	}
	while (gtk_tree_model_iter_next (model, &iter));
}

static void
extensions_manager_ui_init (ExtensionsManagerUI *dialog)
{		
	LOG ("ExtensionsManagerUI initializing")

	dialog->priv = EXTENSIONS_MANAGER_UI_GET_PRIVATE (dialog);

	dialog->priv->manager = EPHY_EXTENSIONS_MANAGER
		(ephy_shell_get_extensions_manager (ephy_shell));

	ephy_dialog_construct (EPHY_DIALOG (dialog),
			       properties,
			       SHARE_DIR "/glade/extensions-manager-ui.glade",
			       "extensions_manager_ui",
			       GETTEXT_PACKAGE);

	build_ui (dialog);

	dialog->priv->added_signal =
		g_signal_connect (G_OBJECT (dialog->priv->manager), "added",
				  G_CALLBACK (extension_added_cb), dialog);
	dialog->priv->changed_signal =
		g_signal_connect (G_OBJECT (dialog->priv->manager), "changed",
				  G_CALLBACK (active_sync), dialog);
	dialog->priv->removed_signal =
		g_signal_connect (G_OBJECT (dialog->priv->manager), "removed",
				  G_CALLBACK (extension_removed_cb), dialog);
}

static void
extensions_manager_ui_finalize (GObject *object)
{
	ExtensionsManagerUI *dialog = EXTENSIONS_MANAGER_UI (object);

	LOG ("ExtensionsManagerUI finalising")

	g_signal_handler_disconnect (G_OBJECT (dialog->priv->manager),
				     dialog->priv->changed_signal);
	g_signal_handler_disconnect (G_OBJECT (dialog->priv->manager),
				     dialog->priv->added_signal);
	g_signal_handler_disconnect (G_OBJECT (dialog->priv->manager),
				     dialog->priv->removed_signal);

	parent_class->finalize (object);
}

static void
extensions_manager_ui_class_init (ExtensionsManagerUIClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = extensions_manager_ui_finalize;

	g_type_class_add_private (object_class, sizeof (ExtensionsManagerUIPrivate));
}

ExtensionsManagerUI *
extensions_manager_ui_new (void)
{
	return g_object_new (TYPE_EXTENSIONS_MANAGER_UI,
			     "persist-position", TRUE,
			     "default-width", 300,
			     "default-height", 400,
			     NULL);
}
