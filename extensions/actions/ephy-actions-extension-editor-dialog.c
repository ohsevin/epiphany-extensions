/* 
 * Copyright (C) 2005 Jean-Yves Lefort <jylefort@brutele.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include "ephy-gui.h"
#include "ephy-actions-extension.h"
#include "ephy-actions-extension-editor-dialog.h"
#include "ephy-actions-extension-properties-dialog.h"
#include "ephy-actions-extension-util.h"

#define EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_GET_PRIVATE(object) \
	G_TYPE_INSTANCE_GET_PRIVATE ((object), \
				     EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG, \
				     EphyActionsExtensionEditorDialogPrivate)

enum {
	PROP_0,
	PROP_EXTENSION
};

struct _EphyActionsExtensionEditorDialogPrivate
{
	EphyActionsExtension	*extension;

	GtkWidget		*popup_menu;
	GtkWidget		*popup_remove_item;
	GtkWidget		*popup_properties_item;

	GtkWidget		*dialog;
	GtkWidget		*view;
	GtkWidget		*selection_count_label;
	GtkWidget		*remove_button;
	GtkWidget		*properties_button;
};

enum {
	PROP_ACTIONS_EDITOR,

	PROP_VIEW,
	PROP_SELECTION_COUNT_LABEL,
	PROP_REMOVE_BUTTON,
	PROP_PROPERTIES_BUTTON
};

static const
EphyDialogProperty properties[] = {
	{ "actions_editor",		NULL, PT_NORMAL, 0 },

	{ "view",			NULL, PT_NORMAL, 0 },
	{ "selection_count_label",	NULL, PT_NORMAL, 0 },
	{ "remove_button",		NULL, PT_NORMAL, 0 },
	{ "properties_button",		NULL, PT_NORMAL, 0 },

	{ NULL }
};

enum {
	COLUMN_NODE,
	COLUMN_LABEL,
	N_COLUMNS
};

GType ephy_actions_extension_editor_dialog_type = 0;
static GObjectClass *parent_class = NULL;

static void ephy_actions_extension_editor_dialog_class_init
	(EphyActionsExtensionEditorDialogClass *class);
static void ephy_actions_extension_editor_dialog_init
	(EphyActionsExtensionEditorDialog *dialog);
static GObject *ephy_actions_extension_editor_dialog_constructor
	(GType type,
	 guint n_construct_properties,
	 GObjectConstructParam *construct_params);
static void ephy_actions_extension_editor_dialog_set_property
	(GObject *object,
	 guint prop_id,
	 const GValue *value,
	 GParamSpec *pspec);
static void ephy_actions_extension_editor_dialog_finalize (GObject *object);

static void ephy_actions_extension_editor_dialog_update_controls
	(EphyActionsExtensionEditorDialog *dialog);
static void ephy_actions_extension_editor_dialog_remove_selected
	(EphyActionsExtensionEditorDialog *dialog);
static void ephy_actions_extension_editor_dialog_edit_selected
	(EphyActionsExtensionEditorDialog *dialog);

/* libglade callbacks */

void ephy_actions_extension_editor_dialog_view_row_activated_cb
	(GtkTreeView *view,
	 GtkTreePath *path,
	 GtkTreeViewColumn *column,
	 EphyActionsExtensionEditorDialog *dialog);

gboolean ephy_actions_extension_editor_dialog_view_popup_menu_cb
	(GtkWidget *widget, EphyActionsExtensionEditorDialog *dialog);

gboolean ephy_actions_extension_editor_dialog_view_button_press_event_cb
	(GtkWidget *widget,
	 GdkEventButton *event,
	 EphyActionsExtensionEditorDialog *dialog);

void ephy_actions_extension_editor_dialog_add_clicked_cb
	(GtkButton *button, EphyActionsExtensionEditorDialog *dialog);

void ephy_actions_extension_editor_dialog_remove_clicked_cb
	(GtkButton *button, EphyActionsExtensionEditorDialog *dialog);

void ephy_actions_extension_editor_dialog_properties_clicked_cb
	(GtkButton *button, EphyActionsExtensionEditorDialog *dialog);

void ephy_actions_extension_editor_dialog_response_cb
	(GtkDialog *dialog,
	 int response,
	 EphyActionsExtensionEditorDialog *pdialog);

GType
ephy_actions_extension_editor_dialog_register_type (GTypeModule *module)
{
	static const GTypeInfo info = {
		sizeof (EphyActionsExtensionEditorDialogClass),
		NULL,
		NULL,
		(GClassInitFunc) ephy_actions_extension_editor_dialog_class_init,
		NULL,
		NULL,
		sizeof(EphyActionsExtensionEditorDialog),
		0,
		(GInstanceInitFunc) ephy_actions_extension_editor_dialog_init
	};

	ephy_actions_extension_editor_dialog_type
		= g_type_module_register_type (module,
					       EPHY_TYPE_DIALOG,
					       "EphyActionsExtensionEditorDialog",
					       &info,
					       0);

	return ephy_actions_extension_editor_dialog_type;
}

static void
ephy_actions_extension_editor_dialog_class_init
	(EphyActionsExtensionEditorDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	parent_class = g_type_class_peek_parent(class);

	g_type_class_add_private
		(class, sizeof (EphyActionsExtensionEditorDialogPrivate));

	object_class->constructor = ephy_actions_extension_editor_dialog_constructor;
	object_class->set_property = ephy_actions_extension_editor_dialog_set_property;
	object_class->finalize = ephy_actions_extension_editor_dialog_finalize;

	g_object_class_install_property
		(object_class, PROP_EXTENSION,
		 g_param_spec_pointer ("extension", NULL, NULL,
				       G_PARAM_WRITABLE
				       | G_PARAM_CONSTRUCT_ONLY));
}

static void
ephy_actions_extension_editor_dialog_init
	(EphyActionsExtensionEditorDialog *dialog)
{
	dialog->priv = EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG_GET_PRIVATE (dialog);
}

static GtkWidget *
ephy_actions_extension_editor_dialog_append_popup_item
	(EphyActionsExtensionEditorDialog *dialog,
	 const char *stock_id,
	 GCallback callback)
{
	GtkWidget *item;

	g_return_val_if_fail (EPHY_IS_ACTIONS_EXTENSION_EDITOR_DIALOG (dialog), NULL);
	g_return_val_if_fail (dialog->priv->popup_menu != NULL, NULL);
	g_return_val_if_fail (stock_id != NULL, NULL);
	g_return_val_if_fail (callback != NULL, NULL);

	item = gtk_image_menu_item_new_from_stock (stock_id, NULL);
	gtk_widget_show (item);
	g_signal_connect_swapped (item, "activate", callback, dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (dialog->priv->popup_menu), item);

	return item;
}

static void
ephy_actions_extension_editor_store_set (GtkListStore *store,
					 GtkTreeIter *iter,
					 EphyNode *action)
{
	const char *name;
	char *formatted;
	const char *description;
	char *markup;

	g_return_if_fail (GTK_IS_LIST_STORE (store));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (EPHY_IS_NODE (action));

	name = ephy_node_get_property_string
		(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_NAME);
	description = ephy_node_get_property_string
		(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_DESCRIPTION);

	formatted = name
		? ephy_actions_extension_format_name_for_display (name)
		: NULL;
	markup = g_markup_printf_escaped
		("<span weight=\"bold\">%s</span>\n%s",
		 formatted ? formatted : "",
		 description ? description : "");
	g_free (formatted);

	gtk_list_store_set (store, iter,
			    COLUMN_NODE, action,
			    COLUMN_LABEL, markup,
			    -1);
      
	g_free (markup);
}

static void
ephy_actions_extension_editor_store_append (GtkListStore *store,
					    EphyNode *action)
{
	GtkTreeIter iter;

	g_return_if_fail (GTK_IS_LIST_STORE (store));
	g_return_if_fail (EPHY_IS_NODE (action));

	gtk_list_store_append (store, &iter);
	ephy_actions_extension_editor_store_set (store, &iter, action);
}

static gboolean
ephy_actions_extension_editor_store_get_iter (GtkListStore *store,
					      GtkTreeIter *iter,
					      EphyNode *action)
{
	gboolean valid;
	GtkTreeIter _iter;

	g_return_val_if_fail (GTK_IS_LIST_STORE (store), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (EPHY_IS_NODE (action), FALSE);

	valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &_iter);
	while (valid)
	{
		EphyNode *this_action;

		gtk_tree_model_get (GTK_TREE_MODEL (store), &_iter,
				    COLUMN_NODE, &this_action,
				    -1);

		if (this_action == action)
		{
			*iter = _iter;
			return TRUE;
		}
		
		valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store),
						  &_iter);
	}

	return FALSE;
}

static void
ephy_actions_extension_editor_store_populate (GtkListStore *store,
					      EphyNode *actions)
{
	int n_children;
	int i;

	g_return_if_fail (GTK_IS_LIST_STORE (store));
	g_return_if_fail (EPHY_IS_NODE (actions));
	
	n_children = ephy_node_get_n_children (actions);

	for (i = 0; i < n_children; i++)
	{
		ephy_actions_extension_editor_store_append
			(store, ephy_node_get_nth_child(actions, i));
	}
}

static void
ephy_actions_extension_editor_dialog_store_child_added_cb (EphyNode *node,
							   EphyNode *child,
							   GtkListStore *store)
{
	ephy_actions_extension_editor_store_append (store, child);
}

static void
ephy_actions_extension_editor_dialog_store_child_changed_cb (EphyNode *node,
							     EphyNode *child,
							     guint property_id,
							     GtkListStore *store)
{
	GtkTreeIter iter;
	gboolean status;

	status = ephy_actions_extension_editor_store_get_iter (store,
							       &iter,
							       child);
	g_return_if_fail (status == TRUE);

	ephy_actions_extension_editor_store_set (store, &iter, child);
}

static void
ephy_actions_extension_editor_dialog_store_child_removed_cb (EphyNode *node,
							     EphyNode *child,
							     guint old_index,
							     GtkListStore *store)
{
	GtkTreeIter iter;
	gboolean status;

	status = ephy_actions_extension_editor_store_get_iter (store,
							       &iter,
							       child);
	g_return_if_fail (status == TRUE);
	
	gtk_list_store_remove (store, &iter);
}

static GObject *
ephy_actions_extension_editor_dialog_constructor
	(GType type,
	 guint n_construct_properties,
	 GObjectConstructParam *construct_params)
{
	GObject *object;
	EphyActionsExtensionEditorDialog *dialog;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	EphyNode *actions;

	object = parent_class->constructor (type, n_construct_properties,
					    construct_params);
	dialog = EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG (object);

	dialog->priv->popup_menu = gtk_menu_new ();
	dialog->priv->popup_remove_item
		= ephy_actions_extension_editor_dialog_append_popup_item
		(dialog, GTK_STOCK_REMOVE,
		 G_CALLBACK (ephy_actions_extension_editor_dialog_remove_selected));
	dialog->priv->popup_properties_item
		= ephy_actions_extension_editor_dialog_append_popup_item
		(dialog, GTK_STOCK_PROPERTIES,
		 G_CALLBACK(ephy_actions_extension_editor_dialog_edit_selected));
  
	ephy_dialog_construct (EPHY_DIALOG (dialog), properties,
			       SHARE_DIR "/glade/actions-editor.glade",
			       properties[PROP_ACTIONS_EDITOR].id,
			       GETTEXT_PACKAGE);

	dialog->priv->dialog = ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_ACTIONS_EDITOR].id);
	dialog->priv->view = ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_VIEW].id);
	dialog->priv->selection_count_label = ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_SELECTION_COUNT_LABEL].id);
	dialog->priv->remove_button = ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_REMOVE_BUTTON].id);
	dialog->priv->properties_button = ephy_dialog_get_control
		(EPHY_DIALOG (dialog), properties[PROP_PROPERTIES_BUTTON].id);

	store = gtk_list_store_new (N_COLUMNS, G_TYPE_POINTER, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->priv->view),
				 GTK_TREE_MODEL (store));
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store),
					      COLUMN_LABEL,
					      GTK_SORT_ASCENDING);

	column = gtk_tree_view_column_new_with_attributes
		(NULL, gtk_cell_renderer_text_new (),
		 "markup", COLUMN_LABEL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->priv->view),
				     column);
  
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (dialog->priv->view),
					 COLUMN_LABEL);
  
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	actions = ephy_actions_extension_get_actions (dialog->priv->extension);

	ephy_actions_extension_editor_store_populate (store, actions);

	ephy_node_signal_connect_object (actions, EPHY_NODE_CHILD_ADDED,
					 (EphyNodeCallback) ephy_actions_extension_editor_dialog_store_child_added_cb,
					 G_OBJECT (store));
	ephy_node_signal_connect_object (actions, EPHY_NODE_CHILD_CHANGED,
					 (EphyNodeCallback) ephy_actions_extension_editor_dialog_store_child_changed_cb,
					 G_OBJECT (store));
	ephy_node_signal_connect_object (actions, EPHY_NODE_CHILD_REMOVED,
					 (EphyNodeCallback) ephy_actions_extension_editor_dialog_store_child_removed_cb,
					 G_OBJECT (store));

	g_object_unref (store);

	ephy_actions_extension_editor_dialog_update_controls (dialog);
	g_signal_connect_swapped
		(selection, "changed",
		 G_CALLBACK (ephy_actions_extension_editor_dialog_update_controls),
		 dialog);

	return object;
}

static void
ephy_actions_extension_editor_dialog_set_property (GObject *object,
						   guint prop_id,
						   const GValue *value,
						   GParamSpec *pspec)
{
	EphyActionsExtensionEditorDialog *dialog
		= EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG (object);

	switch (prop_id)
	{
	case PROP_EXTENSION:
		dialog->priv->extension = g_value_get_pointer (value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
ephy_actions_extension_editor_dialog_finalize (GObject *object)
{
	EphyActionsExtensionEditorDialog *dialog
		= EPHY_ACTIONS_EXTENSION_EDITOR_DIALOG (object);

	gtk_widget_destroy (dialog->priv->popup_menu);
  
	parent_class->finalize (object);
}

static void
ephy_actions_extension_editor_dialog_update_controls
	(EphyActionsExtensionEditorDialog *dialog)
{
	GtkTreeSelection *selection;
	int n_rows;

	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_EDITOR_DIALOG (dialog));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->view));
	n_rows = gtk_tree_selection_count_selected_rows (selection);

	if (n_rows == 0)
	{
		gtk_label_set_text
			(GTK_LABEL (dialog->priv->selection_count_label),
			 _("No action selected."));
		gtk_widget_set_sensitive (dialog->priv->remove_button, FALSE);
		gtk_widget_set_sensitive (dialog->priv->popup_remove_item, FALSE);
		gtk_widget_set_sensitive (dialog->priv->properties_button, FALSE);
		gtk_widget_set_sensitive (dialog->priv->popup_properties_item, FALSE);
	}
	else
	{
		char *str;

		str = g_strdup_printf (ngettext ("%i action selected.",
						 "%i actions selected.",
						 n_rows), n_rows);
		gtk_label_set_text
			(GTK_LABEL (dialog->priv->selection_count_label), str);
		g_free (str);

		gtk_widget_set_sensitive (dialog->priv->remove_button, TRUE);
		gtk_widget_set_sensitive (dialog->priv->popup_remove_item, TRUE);
		gtk_widget_set_sensitive (dialog->priv->properties_button, TRUE);
		gtk_widget_set_sensitive (dialog->priv->popup_properties_item, TRUE);
	}
}

static void
ephy_actions_extension_editor_dialog_remove_selected
	(EphyActionsExtensionEditorDialog *dialog)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *rows;
	GList *l;
	GSList *actions_to_remove = NULL;
	GSList *sl;
	EphyNode *actions;

	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_EDITOR_DIALOG (dialog));

	selection = gtk_tree_view_get_selection
		(GTK_TREE_VIEW (dialog->priv->view));
	rows = gtk_tree_selection_get_selected_rows (selection, &model);

	for (l = rows; l != NULL; l = l->next)
	{
		GtkTreePath *path = l->data;
		GtkTreeIter iter;
		gboolean status;
		EphyNode *action;

		status = gtk_tree_model_get_iter (model, &iter, path);
		g_return_if_fail (status == TRUE);

		gtk_tree_model_get (model, &iter, COLUMN_NODE, &action, -1);
		actions_to_remove = g_slist_append (actions_to_remove, action);
	}

	g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (rows);

	actions = ephy_actions_extension_get_actions (dialog->priv->extension);

	for (sl = actions_to_remove; sl != NULL; sl = sl->next)
	{
		EphyNode *action = sl->data;

		ephy_node_remove_child (actions, action);
		ephy_node_unref (action);
	}

	g_slist_free (actions_to_remove);
}

static void
ephy_actions_extension_editor_dialog_edit_selected
	(EphyActionsExtensionEditorDialog *dialog)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *rows;
	GList *l;
	EphyNode *actions;

	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_EDITOR_DIALOG (dialog));

	selection = gtk_tree_view_get_selection
		(GTK_TREE_VIEW (dialog->priv->view));
	rows = gtk_tree_selection_get_selected_rows (selection, &model);

	actions = ephy_actions_extension_get_actions (dialog->priv->extension);

	for (l = rows; l != NULL; l = l->next)
	{
		GtkTreePath *path = l->data;
		GtkTreeIter iter;
		gboolean status;
		EphyNode *action;
		EphyActionsExtensionPropertiesDialog *properties_dialog;
      
		status = gtk_tree_model_get_iter (model, &iter, path);
		g_return_if_fail (status == TRUE);

		gtk_tree_model_get (model, &iter, COLUMN_NODE, &action, -1);

		properties_dialog
			= ephy_actions_extension_get_properties_dialog
			(dialog->priv->extension, action);

		if (! properties_dialog)
		{
			properties_dialog
				= ephy_actions_extension_properties_dialog_new
				(EPHY_EXTENSION (dialog->priv->extension),
				 action);
			ephy_actions_extension_add_properties_dialog
				(dialog->priv->extension, properties_dialog);
		}

		ephy_dialog_set_parent (EPHY_DIALOG (properties_dialog),
					GTK_WIDGET (dialog->priv->dialog));
		ephy_dialog_show (EPHY_DIALOG (properties_dialog));
	}

	g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (rows);
}

EphyActionsExtensionEditorDialog *
ephy_actions_extension_editor_dialog_new (EphyExtension *extension)
{
	g_return_val_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension), NULL);

	return g_object_new (EPHY_TYPE_ACTIONS_EXTENSION_EDITOR_DIALOG,
			     "extension", extension,
			     NULL);
}

/* libglade callbacks */

void
ephy_actions_extension_editor_dialog_view_row_activated_cb
	(GtkTreeView *view,
	 GtkTreePath *path,
	 GtkTreeViewColumn *column,
	 EphyActionsExtensionEditorDialog *dialog)
{
	ephy_actions_extension_editor_dialog_edit_selected (dialog);
}

gboolean
ephy_actions_extension_editor_dialog_view_popup_menu_cb
	(GtkWidget *widget, EphyActionsExtensionEditorDialog *dialog)
{
	gtk_menu_popup (GTK_MENU (dialog->priv->popup_menu), NULL, NULL,
			ephy_gui_menu_position_tree_selection,
			dialog->priv->view, 0, gtk_get_current_event_time ());
	gtk_menu_shell_select_first (GTK_MENU_SHELL (dialog->priv->popup_menu),
				     FALSE);

	return TRUE;		/* menu activated */
}

gboolean
ephy_actions_extension_editor_dialog_view_button_press_event_cb
	(GtkWidget *widget,
	 GdkEventButton *event,
	 EphyActionsExtensionEditorDialog *dialog)
{
	if (event->button == 3)
	{
		gtk_menu_popup (GTK_MENU (dialog->priv->popup_menu), NULL,
				NULL, NULL, NULL, event->button, event->time);
	}

	return FALSE;		/* propagate event */
}

void
ephy_actions_extension_editor_dialog_add_clicked_cb
	(GtkButton *button, EphyActionsExtensionEditorDialog *dialog)
{
	EphyActionsExtensionPropertiesDialog *properties_dialog;

	properties_dialog = ephy_actions_extension_properties_dialog_new
		(EPHY_EXTENSION (dialog->priv->extension), NULL);
	ephy_dialog_set_parent (EPHY_DIALOG (properties_dialog),
				GTK_WIDGET (dialog->priv->dialog));
	ephy_actions_extension_add_properties_dialog (dialog->priv->extension,
						      properties_dialog);
	ephy_dialog_show (EPHY_DIALOG (properties_dialog));
}

void
ephy_actions_extension_editor_dialog_remove_clicked_cb
	(GtkButton *button, EphyActionsExtensionEditorDialog *dialog)
{
	ephy_actions_extension_editor_dialog_remove_selected (dialog);
}

void
ephy_actions_extension_editor_dialog_properties_clicked_cb
	(GtkButton *button, EphyActionsExtensionEditorDialog *dialog)
{
	ephy_actions_extension_editor_dialog_edit_selected (dialog);
}

void
ephy_actions_extension_editor_dialog_response_cb
	(GtkDialog *dialog,
	 int response,
	 EphyActionsExtensionEditorDialog *pdialog)
{
	g_object_unref (pdialog);
}
