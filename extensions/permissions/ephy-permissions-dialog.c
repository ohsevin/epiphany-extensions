/*
 *  Copyright © 2002 Jorn Baayen
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003, 2004, 2005 Christian Persch
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

#include "ephy-permissions-dialog.h"

#include <epiphany/epiphany.h>

#include "ephy-debug.h"

#include <glib/gi18n-lib.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <string.h>

#define TAB_DATA_KEY "Tab"

typedef struct
{
	EphyPermissionsDialog *dialog;
	char *type;
	GQuark qtype;
	GtkTreeView *treeview;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkWidget *remove_button;
	guint filled : 1;
} DialogTab;

#define EPHY_PERMISSIONS_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_PERMISSIONS_DIALOG, EphyPermissionsDialogPrivate))

struct EphyPermissionsDialogPrivate
{
	EphyPermissionManager *manager;
	GList *tabs;
	GtkNotebook *notebook;
	GtkSizeGroup *buttons_size_group;
};

enum
{
	COL_HOST,
	COL_PERM
};

static GObjectClass *parent_class = NULL;

/* Helper functions */

#if 0
static void
ephy_permissions_dialog_show_help (EphyPermissionsDialog *pd)
{
	GtkWidget *notebook, *window;
	int id;

	static char * const help_preferences[] = {
		"managing-cookies",
		"managing-passwords"
	};

	ephy_dialog_get_controls
		(EPHY_DIALOG (pd),
		 properties[PROP_WINDOW].id, &window,
		 properties[PROP_NOTEBOOK].id, &notebook,
		 NULL);

	id = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
	g_return_if_fail (id == 0 || id == 1);

	ephy_gui_help (GTK_WINDOW (window), "epiphany", help_preferences[id]);
}
#endif

static void
free_dialog_tab (DialogTab *tab)
{
	g_return_if_fail (tab != NULL);

	g_free (tab->type);
	g_free (tab);
}

static DialogTab *
get_tab_for_type (EphyPermissionsDialog *dialog,
		  GQuark qtype)
{
	EphyPermissionsDialogPrivate *priv = dialog->priv;
	DialogTab *tab = NULL, *data;
	GList *l;

	for (l = priv->tabs; l != NULL; l = l->next)
	{
		data = (DialogTab *) l->data;

		if (data->qtype == qtype)
		{
			tab = data;
			break;
		}
	}

	return tab;
}

static gboolean
host_to_iter (GtkTreeModel *model,
	      const char *host,
	      GtkTreeIter *iter)
{
	gboolean valid;
	gboolean found = FALSE;

	g_return_val_if_fail (host != NULL, FALSE);

	valid = gtk_tree_model_get_iter_first (model, iter);

	while (valid)
	{
		char *data = NULL;

		gtk_tree_model_get (model, iter, COL_HOST, &data, -1);
		g_return_val_if_fail (data != NULL, FALSE);

		found = strcmp (host, data) == 0;

		if (found) break;

		valid = gtk_tree_model_iter_next (model, iter);
	}

	return found;
}

static const char *
permission_to_string (EphyPermission permission)
{
	const char *string = NULL;

	if (permission == EPHY_PERMISSION_ALLOWED)
	{
		string = _("allowed");
	}
	else if (permission == EPHY_PERMISSION_DENIED)
	{
		string = _("denied");
	}
	else /* if (permission == EPHY_PERMISSION_DEFAULT) */
	{
		/* Sometimes we get this... how did this get in the list!? */
		string = _("default");
	}

	return string;
}

static void
add_permission_info (GtkListStore *store,
		     EphyPermissionInfo *info)
{
	GtkTreeIter iter;

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    COL_HOST, info->host,
			    COL_PERM, permission_to_string (info->permission),
			    -1);
}

static void
fill_tab (DialogTab *tab)
{
	EphyPermissionsDialog *dialog = tab->dialog;
	EphyPermissionsDialogPrivate *priv = dialog->priv;
	EphyPermissionManager *manager = priv->manager;
	GtkListStore *store = tab->store;
	GList *list, *l;

	if (tab->filled) return;

	list = ephy_permission_manager_list_permissions (manager, tab->type);

	for (l = list; l != NULL; l = l->next)
	{
		add_permission_info (store, (EphyPermissionInfo *) l->data);
	}

	g_list_foreach (list, (GFunc) ephy_permission_info_free, NULL);
	g_list_free (list);

	/* Now turn on sorting */
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (tab->model),
					      COL_HOST, GTK_SORT_ASCENDING);

	tab->filled = TRUE;
}

static void
sync_notebook_tab (GtkNotebook *notebook,
		   GtkNotebookPage *page,
		   int page_num,
		   EphyPermissionsDialog *dialog)
{
	GtkWidget *widget;
	DialogTab *tab;

	widget = gtk_notebook_get_nth_page (notebook, page_num);
	g_return_if_fail (widget != NULL);

	tab = g_object_get_data (G_OBJECT (widget), TAB_DATA_KEY);
	g_return_if_fail (tab != NULL);

	fill_tab (tab);
}

static void
delete_selection (DialogTab *tab)
{
	EphyPermissionsDialog *dialog = tab->dialog;
	EphyPermissionsDialogPrivate *priv = dialog->priv;
	EphyPermissionManager *manager = priv->manager;
	GtkTreeModel *model = tab->model;
	GtkTreeSelection *selection = tab->selection;
	GList *llist, *rlist = NULL, *l, *r;
	GtkTreePath *path;
	GtkTreeIter iter, iter2;
	GtkTreeRowReference *row_ref = NULL;

	llist = gtk_tree_selection_get_selected_rows (selection, &model);

	if (llist == NULL)
	{
		/* nothing to delete, return early */
		return;
	}

	for (l = llist;l != NULL; l = l->next)
	{
		rlist = g_list_prepend (rlist, gtk_tree_row_reference_new
					(model, (GtkTreePath *)l->data));
	}

	/* Intelligent selection logic, no actual selection yet */
	
	path = gtk_tree_row_reference_get_path 
		((GtkTreeRowReference *) g_list_first (rlist)->data);
	
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_path_free (path);
	iter2 = iter;
	
	if (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter))
	{
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
		row_ref = gtk_tree_row_reference_new (model, path);
	}
	else
	{
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter2);
		if (gtk_tree_path_prev (path))
		{
			row_ref = gtk_tree_row_reference_new (model, path);
		}
	}
	gtk_tree_path_free (path);
	
	/* Removal */
	
	for (r = rlist; r != NULL; r = r->next)
	{
		GValue val = { 0, };

		path = gtk_tree_row_reference_get_path
			((GtkTreeRowReference *)r->data);

		gtk_tree_model_get_iter (model, &iter, path);
		gtk_tree_model_get_value (model, &iter, COL_HOST, &val);

		ephy_permission_manager_remove_permission (manager, g_value_get_string (&val),
                                                           tab->type);
		g_value_unset (&val);

		gtk_tree_row_reference_free ((GtkTreeRowReference *)r->data);
		gtk_tree_path_free (path);
	}

	g_list_foreach (llist, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (llist);
	g_list_free (rlist);

	/* Selection */
	
	if (row_ref != NULL)
	{
		path = gtk_tree_row_reference_get_path (row_ref);

		if (path != NULL)
		{
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (tab->treeview), path, NULL, FALSE);
			gtk_tree_path_free (path);
		}

		gtk_tree_row_reference_free (row_ref);
	}
}

static gboolean
treeview_key_press_event_cb (GtkTreeView *treeview,
			     GdkEventKey *event,
			     DialogTab *tab)
{
	if (event->keyval == GDK_Delete || event->keyval == GDK_KP_Delete)
	{
		delete_selection (tab);

		return TRUE;
	}

	return FALSE;
}

static void
treeview_selection_changed_cb (GtkTreeSelection *selection,
			       DialogTab *tab)
{
	gboolean has_selection;

	has_selection = gtk_tree_selection_count_selected_rows (selection) > 0;

	gtk_widget_set_sensitive (tab->remove_button, has_selection);
}

static void
remove_button_clicked_cb (GtkWidget *button,
			  DialogTab *tab)
{
	delete_selection (tab);
}

static void
permission_added_cb (EphyPermissionManager *manager,
		     EphyPermissionInfo *info,
		     EphyPermissionsDialog *dialog)
{
	DialogTab *tab;

	g_return_if_fail (info != NULL);

	tab = get_tab_for_type (dialog, info->qtype);
	if (tab == NULL || tab->filled == FALSE) return;

	add_permission_info (tab->store, info);
}

static void
permission_changed_cb (EphyPermissionManager *manager,
		       EphyPermissionInfo *info,
		       EphyPermissionsDialog *dialog)
{
	DialogTab *tab;
	GtkTreeIter iter;

	g_return_if_fail (info != NULL);

	tab = get_tab_for_type (dialog, info->qtype);
	if (tab == NULL || tab->filled == FALSE) return;

	if (host_to_iter (tab->model, info->host, &iter))
	{
		gtk_list_store_set (tab->store, &iter,
				    COL_PERM, permission_to_string (info->permission),
				    -1);
	}
	else
	{
		g_warning ("Unable to find changed permission in list!\n");
	}
}

static void
permission_deleted_cb (EphyPermissionManager *manager,
		       EphyPermissionInfo *info,
		       EphyPermissionsDialog *dialog)
{
	DialogTab *tab;
	GtkTreeIter iter;

	g_return_if_fail (info != NULL);

	tab = get_tab_for_type (dialog, info->qtype);
	if (tab == NULL || tab->filled == FALSE) return;

	if (host_to_iter (tab->model, info->host, &iter))
	{
		gtk_list_store_remove (tab->store, &iter);
	}
	else
	{
		g_warning ("Unable to find changed permission in list!\n");
	}
}

static void
permissions_cleared_cb (EphyPermissionManager *manager,
			EphyPermissionsDialog *dialog)
{
	EphyPermissionsDialogPrivate *priv = dialog->priv;
	DialogTab *tab;
	GList *l;

	for (l = priv->tabs; l != NULL; l = l->next)
	{
		tab = (DialogTab *) l->data;

		gtk_list_store_clear (tab->store);
		tab->filled = FALSE;
	}

	/* Re-fill the current tab */
	sync_notebook_tab (priv->notebook, NULL,
			   gtk_notebook_get_current_page (priv->notebook),
			   dialog);
}

/* Public functions */

void
ephy_permissions_dialog_add_tab (EphyPermissionsDialog *dialog,
				 const char *type,
				 const char *title)
{
	EphyPermissionsDialogPrivate *priv = dialog->priv;
	GtkWidget *hbox, *view, *vbuttonbox, *button;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeView *treeview;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	DialogTab *tab;

	tab = g_new0 (DialogTab, 1);
	tab->dialog = dialog;
	tab->type = g_strdup (type);
	tab->qtype = g_quark_from_string (type);
	tab->filled = FALSE;
	
	priv->tabs = g_list_prepend (priv->tabs, tab);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
	g_object_set_data (G_OBJECT (hbox), TAB_DATA_KEY, tab);

	view = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (view),
					GTK_POLICY_NEVER /* FIXME? */,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (view),
					     GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX (hbox), view, TRUE, TRUE, 0);

	tab->store = store = gtk_list_store_new (2,
						 G_TYPE_STRING,
						 G_TYPE_STRING);
	tab->model = GTK_TREE_MODEL (store);

	tab->treeview = treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (tab->model));
	g_object_unref (store);

	g_signal_connect (treeview, "key-press-event",
			  G_CALLBACK (treeview_key_press_event_cb), tab);

	gtk_tree_view_set_headers_visible (treeview, TRUE);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_HOST,
						     _("Domain"),
						     renderer,
						     "text", COL_HOST,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_HOST);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_HOST);

	gtk_tree_view_insert_column_with_attributes (treeview,
						     COL_PERM,
						     _("State"),
						     renderer,
						     "text", COL_PERM,
						     NULL);
	column = gtk_tree_view_get_column (treeview, COL_PERM);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_sort_column_id (column, COL_PERM);

	gtk_container_add (GTK_CONTAINER (view), GTK_WIDGET (treeview));

	tab->selection = selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
	g_signal_connect (selection, "changed",
			  G_CALLBACK (treeview_selection_changed_cb), tab);

	/* Button box */
	vbuttonbox = gtk_vbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox),
				   GTK_BUTTONBOX_START);
	gtk_box_pack_start (GTK_BOX (hbox), vbuttonbox, FALSE, FALSE, 0);

	/* Remove button */
	tab->remove_button = button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	g_signal_connect (button, "clicked",
			  G_CALLBACK (remove_button_clicked_cb), tab);
	gtk_box_pack_start (GTK_BOX (vbuttonbox), button, FALSE, FALSE, 0);

	gtk_size_group_add_widget (priv->buttons_size_group, button);

	gtk_widget_show_all (hbox);

	/* And finally insert it in the notebook */
	gtk_notebook_append_page (priv->notebook, hbox,
				  gtk_label_new (title));
}

/* Class implementation */

static void
ephy_permissions_dialog_response (GtkDialog *widget,
				  int response)
{
//	EphyPermissionsDialog *dialog = EPHY_PERMISSIONS_DIALOG (widget);

	if (GTK_DIALOG_CLASS (parent_class)->response)
	{
		GTK_DIALOG_CLASS (parent_class)->response (widget, response);
	}

	switch (response)
	{
		case GTK_RESPONSE_CLOSE:
			gtk_widget_destroy (GTK_WIDGET (widget));
			break;
		case GTK_RESPONSE_HELP:
//			ephy_permissions_dialog_show_help (dialog);
			break;
		default:
			break;
	}
}

static void
ephy_permissions_dialog_init (EphyPermissionsDialog *dialog)
{
	dialog->priv = EPHY_PERMISSIONS_DIALOG_GET_PRIVATE (dialog);
}

static GObject *
ephy_permissions_dialog_constructor (GType type,
				     guint n_construct_properties,
				     GObjectConstructParam *construct_params)
{
	EphyPermissionsDialog *dialog;
	EphyPermissionsDialogPrivate *priv;
	EphyEmbedShell *shell;
	EphyPermissionManager *manager;
	GObject *object;
	GtkWidget *notebook;
		
	object = parent_class->constructor (type, n_construct_properties,
					    construct_params);

	dialog = EPHY_PERMISSIONS_DIALOG (object);
	priv = dialog->priv;

        ephy_state_add_window (GTK_WIDGET (dialog),
                               "permissions-manager",
                               540, 400, FALSE,
                               EPHY_STATE_WINDOW_SAVE_SIZE);

        gtk_window_set_title (GTK_WINDOW (dialog), _("Site Permissions"));
	gtk_window_set_role (GTK_WINDOW (dialog), "epiphany-permissions-manager");
	gtk_window_set_icon_name (GTK_WINDOW (dialog), GTK_STOCK_PROPERTIES /* FIXME? */);

	/* Don't stay on top of all windows! */
	gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_NORMAL);

        gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

        gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), 2);

        gtk_dialog_add_button (GTK_DIALOG (dialog),
                               GTK_STOCK_HELP, GTK_RESPONSE_HELP);
        gtk_dialog_add_button (GTK_DIALOG (dialog),
			       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

	notebook = gtk_notebook_new ();
	priv->notebook = GTK_NOTEBOOK (notebook);
	gtk_container_set_border_width (GTK_CONTAINER (priv->notebook), 5);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), notebook,
			    TRUE, TRUE, 0);
	gtk_widget_show (notebook);

	priv->buttons_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	ephy_permissions_dialog_add_tab (dialog, EPT_COOKIE, _("Cookies"));
	ephy_permissions_dialog_add_tab (dialog, EPT_IMAGE, _("Images"));
	ephy_permissions_dialog_add_tab (dialog, EPT_POPUP, _("Popup Windows"));
	/* ephy_permissions_dialog_add_tab (dialog, "password", _("Passwords")); */

	shell = ephy_embed_shell_get_default ();
	priv->manager = manager =
		EPHY_PERMISSION_MANAGER (ephy_embed_shell_get_embed_single (shell));

	sync_notebook_tab (priv->notebook, NULL, 0, dialog);
	g_signal_connect (G_OBJECT (notebook), "switch-page",
			  G_CALLBACK (sync_notebook_tab), dialog);

	g_signal_connect (priv->manager, "permission-added",
			  G_CALLBACK (permission_added_cb), dialog);
	g_signal_connect (priv->manager, "permission-changed",
			  G_CALLBACK (permission_changed_cb), dialog);
	g_signal_connect (priv->manager, "permission-deleted",
			  G_CALLBACK (permission_deleted_cb), dialog);
	g_signal_connect (priv->manager, "permissions-cleared",
			  G_CALLBACK (permissions_cleared_cb), dialog);

	return object;
}

static void
ephy_permissions_dialog_finalize (GObject *object)
{
	EphyPermissionsDialog *dialog = EPHY_PERMISSIONS_DIALOG (object);
	EphyPermissionsDialogPrivate *priv = dialog->priv;
	EphyEmbedShell *shell;
	GObject *single;

	shell = ephy_embed_shell_get_default ();
	single = ephy_embed_shell_get_embed_single (embed_shell);

	g_signal_handlers_disconnect_matched
		(single, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, dialog);

#if 0
	dialog->priv->cookies->destruct (dialog->priv->cookies);
	dialog->priv->passwords->destruct (dialog->priv->passwords);
	
	g_free (dialog->priv->passwords);
	g_free (dialog->priv->cookies);
#endif

	g_object_unref (priv->buttons_size_group);

	g_list_foreach (priv->tabs, (GFunc) free_dialog_tab, NULL);
	g_list_free (priv->tabs);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_permissions_dialog_class_init (EphyPermissionsDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
//	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->constructor = ephy_permissions_dialog_constructor;
	object_class->finalize = ephy_permissions_dialog_finalize;

	dialog_class->response = ephy_permissions_dialog_response;

	g_type_class_add_private (object_class, sizeof (EphyPermissionsDialogPrivate));
}

#if 0
GType 
ephy_permissions_dialog_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0))
	{
		const GTypeInfo our_info =
		{
			sizeof (EphyPermissionsDialogClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) ephy_permissions_dialog_class_init,
			NULL,
			NULL, /* class_data */
			sizeof (EphyPermissionsDialog),
			0, /* n_preallocs */
			(GInstanceInitFunc) ephy_permissions_dialog_init
		};

		type = g_type_register_static (GTK_TYPE_DIALOG,
					       "EphyPermissionsDialog",
					       &our_info, 0);
	}

	return type;
}
#endif

static GType type = 0;

GType
ephy_permissions_dialog_get_type (void)
{
	return type;
}

GType
ephy_permissions_dialog_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyPermissionsDialogClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_permissions_dialog_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPermissionsDialog),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_permissions_dialog_init
	};

	type = g_type_module_register_type (module,
					    GTK_TYPE_DIALOG,
					    "EphyPermissionsDialog",
					    &our_info, 0);

	return type;
}

GtkWidget *
ephy_permissions_dialog_new (void)
{
	return g_object_new (EPHY_TYPE_PERMISSIONS_DIALOG, NULL);
}
