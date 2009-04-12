/*
 * Copyright © 2005 Jean-Yves Lefort <jylefort@brutele.be>
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
#include <stdarg.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <epiphany/ephy-embed-container.h>
#include <epiphany/ephy-extension.h>
#include "ephy-file-helpers.h"
#include "ephy-actions-extension.h"
#include "ephy-actions-extension-editor-dialog.h"

#define EPHY_ACTIONS_EXTENSION_GET_PRIVATE(object) G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_ACTIONS_EXTENSION, EphyActionsExtensionPrivate)

#define WINDOW_DATA_KEY		"EphyActionsExtensionWindowData"
#define ACTION_DATA_KEY		"EphyActionsExtensionActionData"

#define ACTIONS_NODE_ID		0
#define ACTIONS_XML_ROOT	"ephy_actions"
#define ACTIONS_XML_VERSION	"1.0"

#define ACTIONS_SAVE_DELAY	(3 * 1000)

struct _EphyActionsExtensionPrivate
{
	EphyNodeDb				*db;
	EphyNode				*actions;
	char					*xml_file;
	guint					save_timeout_id;
	gboolean				dirty;

	EphyActionsExtensionEditorDialog	*editor_dialog;
	GSList					*properties_dialogs;
};

typedef struct
{
	EphyActionsExtension			*extension;
	EphyWindow				*window;

	GtkActionGroup				*action_group;
	guint					ui_id;

	GtkActionGroup				*user_action_group;
	guint					user_ui_id;
} WindowData;

typedef struct
{
	EphyNode				*node;
	gboolean 				apply_to_image;
	gboolean				apply_to_page;
	EphyEmbedEventContext 			context;
} ActionData;

enum
{
	ACTIONS_CHANGED,
	LAST_SIGNAL
};

GType ephy_actions_extension_type = 0;
static GObjectClass *parent_class = NULL;
static unsigned int extension_signals[LAST_SIGNAL] = { 0 };

static void ephy_actions_extension_class_init (EphyActionsExtensionClass *class);
static void ephy_actions_extension_init (EphyActionsExtension *extension);
static void ephy_actions_extension_iface_init (EphyExtensionIface *iface);
static void ephy_actions_extension_finalize (GObject *object);

static void ephy_actions_extension_actions_changed
	(EphyActionsExtension *extension);

static void ephy_actions_extension_attach_window (EphyExtension *extension,
						  EphyWindow *window);
static void ephy_actions_extension_detach_window (EphyExtension *extension,
						  EphyWindow *window);

static void ephy_actions_extension_edit_actions_cb (GtkAction *action,
						    EphyWindow *window);

static void ephy_actions_extension_properties_dialog_weak_notify_cb
	(EphyActionsExtension *extension, GObject *former_object);

static void
ephy_actions_extension_attach_tab (EphyExtension *extension,
				   EphyWindow *window,
				   EphyEmbed *embed);

static void
ephy_actions_extension_detach_tab (EphyExtension *extension,
				   EphyWindow *window,
				   EphyEmbed *embed);

static const GtkActionEntry edit_entries[] = {
	{ "EphyActionsExtensionEditActions", NULL, N_("Actio_ns"), NULL,
	  N_("Customize actions"),
	  G_CALLBACK (ephy_actions_extension_edit_actions_cb) }
};

GType
ephy_actions_extension_register_type (GTypeModule *module)
{
	const GTypeInfo info = {
		sizeof (EphyActionsExtensionClass),
		NULL,
		NULL,
		(GClassInitFunc) ephy_actions_extension_class_init,
		NULL,
		NULL,
		sizeof (EphyActionsExtension),
		0,
		(GInstanceInitFunc) ephy_actions_extension_init
	};
	const GInterfaceInfo extension_info = {
		(GInterfaceInitFunc) ephy_actions_extension_iface_init,
		NULL,
		NULL
	};

	ephy_actions_extension_type
		= g_type_module_register_type (module,
					       G_TYPE_OBJECT,
					       "EphyActionsExtension",
					       &info,
					       0);

	g_type_module_add_interface (module,
				     ephy_actions_extension_type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return ephy_actions_extension_type;
}

static void
ephy_actions_extension_class_init (EphyActionsExtensionClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	parent_class = g_type_class_peek_parent (class);

	g_type_class_add_private (class, sizeof (EphyActionsExtensionPrivate));

	object_class->finalize = ephy_actions_extension_finalize;
	class->actions_changed = ephy_actions_extension_actions_changed;

	extension_signals[ACTIONS_CHANGED] = g_signal_new
		("actions-changed",
		 G_OBJECT_CLASS_TYPE (object_class),
		 G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET (EphyActionsExtensionClass, actions_changed),
		 NULL,
		 NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE,
		 0);
}

static void
ephy_actions_extension_child_added_cb (EphyNode *node,
				       EphyNode *child,
				       EphyActionsExtension *extension)
{
	g_signal_emit (extension, extension_signals[ACTIONS_CHANGED], 0);
}

static void
ephy_actions_extension_child_changed_cb (EphyNode *node,
					 EphyNode *child,
					 guint property_id,
					 EphyActionsExtension *extension)
{
	g_signal_emit (extension, extension_signals[ACTIONS_CHANGED], 0);
}

static void
ephy_actions_extension_child_removed_cb (EphyNode *node,
					 EphyNode *child,
					 guint old_index,
					 EphyActionsExtension *extension)
{
	g_signal_emit (extension, extension_signals[ACTIONS_CHANGED], 0);
}

static void
ephy_actions_extension_init (EphyActionsExtension *extension)
{
	extension->priv = EPHY_ACTIONS_EXTENSION_GET_PRIVATE (extension);

	extension->priv->db = ephy_node_db_new ("EphyActions");
	extension->priv->xml_file = g_build_filename (ephy_dot_dir(),
						      "ephy-actions.xml",
						      NULL);
	extension->priv->actions = ephy_node_new_with_id (extension->priv->db,
							  ACTIONS_NODE_ID);

	ephy_node_db_load_from_file (extension->priv->db,
				     (const char*) extension->priv->xml_file,
				     (const xmlChar*) ACTIONS_XML_ROOT,
				     (const xmlChar*) ACTIONS_XML_VERSION);

	ephy_node_signal_connect_object
		(extension->priv->actions, EPHY_NODE_CHILD_ADDED,
		 (EphyNodeCallback) ephy_actions_extension_child_added_cb,
		 G_OBJECT (extension));
	ephy_node_signal_connect_object
		(extension->priv->actions, EPHY_NODE_CHILD_CHANGED,
		 (EphyNodeCallback) ephy_actions_extension_child_changed_cb,
		 G_OBJECT (extension));
	ephy_node_signal_connect_object
		(extension->priv->actions, EPHY_NODE_CHILD_REMOVED,
		 (EphyNodeCallback) ephy_actions_extension_child_removed_cb,
		 G_OBJECT (extension));
}

static void
ephy_actions_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = ephy_actions_extension_attach_window;
	iface->detach_window = ephy_actions_extension_detach_window;
	iface->attach_tab = ephy_actions_extension_attach_tab;
	iface->detach_tab = ephy_actions_extension_detach_tab;
}

static void
ephy_actions_extension_save_actions (EphyActionsExtension *extension)
{
	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension));
	g_return_if_fail (extension->priv->dirty == TRUE);

	if (ephy_node_db_write_to_xml_safe (extension->priv->db,
					    (const xmlChar*) extension->priv->xml_file,
					    (const xmlChar*) ACTIONS_XML_ROOT, 
					    (const xmlChar*) ACTIONS_XML_VERSION,
					    NULL,
					    extension->priv->actions, NULL, NULL,
					    NULL) == 0)
	{
		extension->priv->dirty = FALSE;
	}
	else
	{
		g_warning ("unable to save actions");
	}
}

static void
ephy_actions_extension_dequeue_save_actions (EphyActionsExtension *extension)
{
	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension));

	if (extension->priv->save_timeout_id != 0)
	{
		g_source_remove (extension->priv->save_timeout_id);
		extension->priv->save_timeout_id = 0;
	}
}

static void
ephy_actions_extension_finalize (GObject *object)
{
	EphyActionsExtension *extension = EPHY_ACTIONS_EXTENSION (object);
	GSList *l;

	if (extension->priv->editor_dialog)
	{
		g_object_unref (extension->priv->editor_dialog);
	}

	for (l = extension->priv->properties_dialogs; l != NULL; l = l->next)
	{
		g_object_weak_unref (l->data,
				     (GWeakNotify) ephy_actions_extension_properties_dialog_weak_notify_cb,
				     extension);
	}

	g_slist_foreach (extension->priv->properties_dialogs,
			 (GFunc) g_object_unref, NULL);
	g_slist_free (extension->priv->properties_dialogs);

	ephy_actions_extension_dequeue_save_actions (extension);
	if (extension->priv->dirty)
	{
		ephy_actions_extension_save_actions (extension);
	}

	ephy_node_unref (extension->priv->actions);
	g_object_unref (extension->priv->db);
	g_free (extension->priv->xml_file);

	parent_class->finalize (object);
}

static gboolean
ephy_actions_extension_save_actions_cb (EphyActionsExtension *extension)
{
	GDK_THREADS_ENTER();

	ephy_actions_extension_save_actions (extension);
	extension->priv->save_timeout_id = 0;

	GDK_THREADS_LEAVE();

	return FALSE;		/* remove callback */
}

static void
ephy_actions_extension_actions_changed (EphyActionsExtension *extension)
{
	extension->priv->dirty = TRUE;

	ephy_actions_extension_dequeue_save_actions (extension);
	extension->priv->save_timeout_id = g_timeout_add
		(ACTIONS_SAVE_DELAY,
		 (GSourceFunc) ephy_actions_extension_save_actions_cb,
		 extension);
}

static void
ephy_actions_extension_add_action (EphyWindow *window,
				   EphyNode *action,
				   gboolean apply_to_page,
				   gboolean apply_to_image,
				   int *n,
				   const char *name,
				   const char *description,
				   GCallback callback,
				   ...)
{
	WindowData *data;
	ActionData *action_data;
	char *ui_action_name;
	GtkAction *ui_action;
	va_list args;
	const char *ui_path;
	GtkUIManager *manager;

	g_return_if_fail (EPHY_IS_WINDOW (window));
	g_return_if_fail (EPHY_IS_NODE (action));
	g_return_if_fail (n != NULL);

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	ui_action_name = g_strdup_printf ("EphyActionsExtensionAction%i",
					  (*n)++);
	ui_action = gtk_action_new (ui_action_name,
				    name ? name : "", description, NULL);

	action_data = g_new (ActionData, 1);
	g_object_set_data_full (G_OBJECT (ui_action), ACTION_DATA_KEY, action_data, g_free);
	action_data->node = action;
	action_data->apply_to_page = apply_to_page;
	action_data->apply_to_image = apply_to_image;
	action_data->context = EPHY_EMBED_CONTEXT_NONE;

	if (callback)
	{
		g_signal_connect (ui_action, "activate", callback, window);
	}
	else
	{
		gtk_action_set_sensitive (ui_action, FALSE);
	}

	gtk_action_group_add_action (data->user_action_group, ui_action);
	g_object_unref (ui_action);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager(window));

	va_start (args, callback);
	while ((ui_path = va_arg (args, const char *)))
	{
		gtk_ui_manager_add_ui (manager,
				       data->user_ui_id,
				       ui_path,
				       ui_action_name,
				       ui_action_name,
				       GTK_UI_MANAGER_MENUITEM,
				       FALSE);
	}
	va_end (args);

	g_free (ui_action_name);
}

static void
ephy_actions_extension_run_action (GtkAction *ui_action,
				   EphyWindow *window,
				   const char *url)
{
	ActionData *action_data;
	const char *command;
	char *quoted_url;
	char *full_command;
	GError *err = NULL;

	g_return_if_fail (GTK_IS_ACTION (ui_action));
	g_return_if_fail (EPHY_IS_WINDOW (window));
	g_return_if_fail (url != NULL);

	action_data = g_object_get_data (G_OBJECT (ui_action), ACTION_DATA_KEY);
	g_return_if_fail (action_data != NULL);

	command = ephy_node_get_property_string
		(action_data->node, EPHY_ACTIONS_EXTENSION_ACTION_PROP_COMMAND);

	quoted_url = g_shell_quote (url);
	full_command = g_strdup_printf ("%s %s", command, quoted_url);
	g_free (quoted_url);

	if (g_spawn_command_line_async (full_command, &err) == FALSE)
	{
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new
			(GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT,
			 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			 _("Could not run command"));
		gtk_message_dialog_format_secondary_text
			(GTK_MESSAGE_DIALOG(dialog), err->message, NULL);
		gtk_window_set_icon_name (GTK_WINDOW (dialog), "web-browser");
		gtk_window_set_title (GTK_WINDOW(window), _("Could not Run Command"));
		g_error_free (err);

		g_signal_connect (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show (dialog);
	}
	g_free (full_command);
}

static void
ephy_actions_extension_run_action_on_embed_property (GtkAction *action,
						     EphyWindow *window,
						     const char *property_name)
{
	EphyEmbedEvent *event;
	const GValue *value;

	g_return_if_fail (GTK_IS_ACTION (action));
	g_return_if_fail (EPHY_IS_WINDOW (window));
	g_return_if_fail (property_name != NULL);

	event = ephy_window_get_context_event (window);
	g_return_if_fail (event != NULL);

	value = ephy_embed_event_get_property (event, property_name);
	ephy_actions_extension_run_action (action, window,
					   g_value_get_string (value));
}

static void
ephy_actions_extension_document_popup_cb (GtkAction *action,
					  EphyWindow *window)
{
	EphyEmbedEventContext context;
	EphyEmbed *embed;
	char *url;
	ActionData *action_data;

	action_data = g_object_get_data (G_OBJECT (action), ACTION_DATA_KEY);
	g_return_if_fail (action_data != NULL);

	context = action_data->context;

	if (context & EPHY_EMBED_CONTEXT_IMAGE)
	{
		ephy_actions_extension_run_action_on_embed_property (action,
				window,
				"image");
		return;
	}
	if (context & EPHY_EMBED_CONTEXT_LINK)
	{
		ephy_actions_extension_run_action_on_embed_property (action,
				window,
				"link");
		return;
	}

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	url = ephy_embed_get_location (embed, TRUE);
	ephy_actions_extension_run_action (action, window, url);
	g_free (url);
}

static void
ephy_actions_extension_update_menus (EphyWindow *window)
{
	WindowData *data;
	GList *actions;
	GList *l;
	GtkUIManager *manager;
	int n_children;
	int i;
	int n = 0;
	static const char * const popups[] = {
		"/EphyDocumentPopup",
		"/EphyLinkPopup",
	};

	g_return_if_fail (EPHY_IS_WINDOW (window));

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	actions = gtk_action_group_list_actions (data->user_action_group);
	for (l = actions; l != NULL; l = l->next)
	{
		gtk_action_group_remove_action (data->user_action_group,
						l->data);
	}
	g_list_free (actions);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	gtk_ui_manager_remove_ui (manager, data->user_ui_id);

	for (i = 0; i < G_N_ELEMENTS (popups); i++)
	{
		gtk_ui_manager_add_ui (manager, data->user_ui_id, popups[i],
				       "EphyActionsExtensionSeparator",
				       NULL, GTK_UI_MANAGER_SEPARATOR, FALSE);
	}

	n_children = ephy_node_get_n_children (data->extension->priv->actions);
	for (i = 0; i < n_children; i++)
	{
		EphyNode *action = ephy_node_get_nth_child
			(data->extension->priv->actions, i);
		const char *name;
		const char *description;
		const char *command;
		gboolean has_command;
		gboolean applies_to_pages;
		gboolean applies_to_images;

		name = ephy_node_get_property_string
			(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_NAME);
		description = ephy_node_get_property_string
			(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_DESCRIPTION);
		command = ephy_node_get_property_string
			(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_COMMAND);
		applies_to_pages = ephy_node_get_property_boolean
			(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_APPLIES_TO_PAGES);
		applies_to_images = ephy_node_get_property_boolean
			(action, EPHY_ACTIONS_EXTENSION_ACTION_PROP_APPLIES_TO_IMAGES);

		has_command = command != NULL && command[0] != '\0';

		ephy_actions_extension_add_action
			(window, action, applies_to_pages, applies_to_images, &n, name, description,
			 has_command
			 ? G_CALLBACK (ephy_actions_extension_document_popup_cb)
			 : NULL,
			 "/EphyDocumentPopup",
			 "/EphyLinkPopup",
			 NULL);
	}
}

static void
ephy_actions_extension_window_data_free (WindowData *data)
{
	g_return_if_fail(data != NULL);

	g_signal_handlers_disconnect_by_func
		(data->extension,
		 G_CALLBACK (ephy_actions_extension_update_menus),
		 data->window);

	g_object_unref (data->action_group);
	g_object_unref (data->user_action_group);

	g_free (data);
}

static void
ephy_actions_extension_attach_window (EphyExtension *extension,
				      EphyWindow *window)
{
	WindowData *data;
	GtkUIManager *manager;

	data = g_new (WindowData, 1);
	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) ephy_actions_extension_window_data_free);

	data->extension = EPHY_ACTIONS_EXTENSION (extension);
	data->window = window;

	data->action_group = gtk_action_group_new
		("EphyActionsExtensionActions");
	gtk_action_group_set_translation_domain (data->action_group,
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions (data->action_group, edit_entries,
				      G_N_ELEMENTS(edit_entries), window);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	gtk_ui_manager_insert_action_group (manager, data->action_group, -1);

	data->ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager,
			       data->ui_id,
			       "/menubar/EditMenu",
			       "EphyActionsExtensionEditActions",
			       "EphyActionsExtensionEditActions",
			       GTK_UI_MANAGER_MENUITEM,
			       FALSE);

	data->user_action_group = gtk_action_group_new
		("EphyActionsExtensionUserActions");
	gtk_action_group_set_translation_domain (data->user_action_group,
						 GETTEXT_PACKAGE);

	gtk_ui_manager_insert_action_group (manager,
					    data->user_action_group, -1);

	data->user_ui_id = gtk_ui_manager_new_merge_id (manager);

	ephy_actions_extension_update_menus (window);
	g_signal_connect_swapped (data->extension, "actions-changed",
				  G_CALLBACK (ephy_actions_extension_update_menus),
				  window);
}

static void
ephy_actions_extension_detach_window (EphyExtension *extension,
				      EphyWindow *window)
{
	WindowData *data;
	GtkUIManager *manager;

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	gtk_ui_manager_remove_ui (manager, data->user_ui_id);
	gtk_ui_manager_remove_action_group (manager, data->user_action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
ephy_actions_extension_edit_actions_cb (GtkAction *action, EphyWindow *window)
{
	WindowData *data;

	data = g_object_get_data (G_OBJECT(window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	if (data->extension->priv->editor_dialog == NULL)
	{
		data->extension->priv->editor_dialog
			= ephy_actions_extension_editor_dialog_new
			(EPHY_EXTENSION (data->extension));
		g_object_add_weak_pointer
			(G_OBJECT (data->extension->priv->editor_dialog),
			 (gpointer) &data->extension->priv->editor_dialog);
	}

	ephy_dialog_set_parent
		(EPHY_DIALOG (data->extension->priv->editor_dialog),
		 GTK_WIDGET (window));
	ephy_dialog_show (EPHY_DIALOG (data->extension->priv->editor_dialog));
}

void
ephy_actions_extension_add_properties_dialog
	(EphyActionsExtension *extension,
	 EphyActionsExtensionPropertiesDialog *dialog)
{
	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension));
	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG (dialog));

	extension->priv->properties_dialogs = g_slist_append
		(extension->priv->properties_dialogs, dialog);
	g_object_weak_ref (G_OBJECT (dialog),
			   (GWeakNotify) ephy_actions_extension_properties_dialog_weak_notify_cb,
			   extension);
}

static void
ephy_actions_extension_properties_dialog_weak_notify_cb
	(EphyActionsExtension *extension, GObject *former_object)
{
	extension->priv->properties_dialogs = g_slist_remove
		(extension->priv->properties_dialogs, former_object);
}

EphyActionsExtensionPropertiesDialog *
ephy_actions_extension_get_properties_dialog (EphyActionsExtension *extension,
					      EphyNode *action)
{
	GSList *l;

	g_return_val_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension), NULL);
	g_return_val_if_fail (action != NULL, NULL);

	for (l = extension->priv->properties_dialogs; l != NULL; l = l->next)
	{
		if (action == ephy_actions_extension_properties_dialog_get_action(l->data))
		{
			return l->data;
		}
	}

	return NULL;
}

EphyNodeDb *
ephy_actions_extension_get_db (EphyActionsExtension *extension)
{
	g_return_val_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension), NULL);

	return extension->priv->db;
}

EphyNode *
ephy_actions_extension_get_actions (EphyActionsExtension *extension)
{
	g_return_val_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension), NULL);

	return extension->priv->actions;
}

static gboolean
ephy_actions_extension_context_menu_cb (EphyEmbed *embed,
					EphyEmbedEvent *event,
					EphyWindow *window)
{
	EphyEmbedEventContext context;
	GList *actions, *l;
	WindowData *data;

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_val_if_fail (data != NULL, FALSE);

	context = ephy_embed_event_get_context (event);

	actions = gtk_action_group_list_actions (data->user_action_group);
	for (l = actions; l != NULL; l = l->next)
	{
		GtkAction *action = GTK_ACTION (l->data);
		ActionData *action_data;

		action_data = g_object_get_data (G_OBJECT (action), ACTION_DATA_KEY);
		g_return_val_if_fail (action_data != NULL, FALSE);

		action_data->context = context;

		if (context & EPHY_EMBED_CONTEXT_IMAGE)
		{
			gtk_action_set_visible (action, action_data->apply_to_image);
			continue;
		}
		if (context & EPHY_EMBED_CONTEXT_DOCUMENT)
		{
			gtk_action_set_visible (action, action_data->apply_to_page);
			continue;
		}
		/* Default setting */
		gtk_action_set_visible (action, FALSE);
	}
	g_list_free (actions);

	return FALSE;
}

static void
ephy_actions_extension_attach_tab (EphyExtension *extension,
				   EphyWindow *window,
				   EphyEmbed *embed)
{
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_context_menu",
			  G_CALLBACK (ephy_actions_extension_context_menu_cb), window);
}

static void
ephy_actions_extension_detach_tab (EphyExtension *extension,
				   EphyWindow *window,
				   EphyEmbed *embed)
{
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (ephy_actions_extension_context_menu_cb), window);
}

