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
#include <stdarg.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include "ephy-actions-extension.h"
#include "ephy-actions-extension-properties-dialog.h"
#include "ephy-actions-extension-util.h"

#define EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_GET_PRIVATE(object) \
	G_TYPE_INSTANCE_GET_PRIVATE ((object), \
				     EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG, \
				     EphyActionsExtensionPropertiesDialogPrivate)

#define NODE_PROPERTY_KEY "EphyActionsExtensionPropertiesDialogNodeProperty"

enum {
	PROP_0,
	PROP_EXTENSION,
	PROP_ACTION
};

struct _EphyActionsExtensionPropertiesDialogPrivate
{
	EphyActionsExtension	*extension;

	gboolean		add;
	EphyNode		*action;

	GtkWidget		*dialog;
	GtkWidget		*name_entry;
};

GType ephy_actions_extension_properties_dialog_type = 0;
static GObjectClass *parent_class = NULL;

enum {
	PROP_ACTION_PROPERTIES,

	PROP_NAME_LABEL,
	PROP_DESCRIPTION_LABEL,
	PROP_COMMAND_LABEL,

	PROP_NAME_ENTRY,
	PROP_DESCRIPTION_ENTRY,
	PROP_COMMAND_ENTRY,
	PROP_APPLIES_TO_PAGES_CHECK,
	PROP_APPLIES_TO_IMAGES_CHECK
};

static const EphyDialogProperty properties[] = {
	{ "action_properties",		NULL, PT_NORMAL, 0 },
  
	{ "name_label",			NULL, PT_NORMAL, 0 },
	{ "description_label",		NULL, PT_NORMAL, 0 },
	{ "command_label",		NULL, PT_NORMAL, 0 },
  
	{ "name_entry",			NULL, PT_NORMAL, 0 },
	{ "description_entry",		NULL, PT_NORMAL, 0 },
	{ "command_entry",		NULL, PT_NORMAL, 0 },
	{ "applies_to_pages_check",	NULL, PT_NORMAL, 0 },
	{ "applies_to_images_check",	NULL, PT_NORMAL, 0 },
  
	{ NULL }
};

static void ephy_actions_extension_properties_dialog_class_init
	(EphyActionsExtensionPropertiesDialogClass *class);
static void ephy_actions_extension_properties_dialog_init
	(EphyActionsExtensionPropertiesDialog *dialog);
static GObject *ephy_actions_extension_properties_dialog_constructor
	(GType type,
	 guint n_construct_properties,
	 GObjectConstructParam *construct_params);
static void ephy_actions_extension_properties_dialog_finalize
	(GObject *object);
static void ephy_actions_extension_properties_dialog_set_property
	(GObject *object,
	 guint prop_id,
	 const GValue *value,
	 GParamSpec *pspec);
static void ephy_actions_extension_properties_dialog_get_property
	(GObject *object,
	 guint prop_id,
	 GValue *value,
	 GParamSpec *pspec);

static void ephy_actions_extension_properties_dialog_update_title
	(EphyActionsExtensionPropertiesDialog *dialog);

static void ephy_actions_extension_properties_dialog_response_cb
	(GtkDialog *dialog,
	 int response,
	 EphyActionsExtensionPropertiesDialog *pdialog);

static void ephy_actions_extension_properties_dialog_name_entry_changed_cb
	(GtkEditable *editable, EphyActionsExtensionPropertiesDialog *dialog);

GType
ephy_actions_extension_properties_dialog_register_type (GTypeModule *module)
{
	const GTypeInfo info = {
		sizeof (EphyActionsExtensionPropertiesDialogClass),
		NULL,
		NULL,
		(GClassInitFunc) ephy_actions_extension_properties_dialog_class_init,
		NULL,
		NULL,
		sizeof (EphyActionsExtensionPropertiesDialog),
		0,
		(GInstanceInitFunc) ephy_actions_extension_properties_dialog_init
	};

	ephy_actions_extension_properties_dialog_type
		= g_type_module_register_type (module,
					       EPHY_TYPE_DIALOG,
					       "EphyActionsExtensionPropertiesDialog",
					       &info,
					       0);

	return ephy_actions_extension_properties_dialog_type;
}

static void
ephy_actions_extension_properties_dialog_class_init
	(EphyActionsExtensionPropertiesDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	parent_class = g_type_class_peek_parent (class);

	g_type_class_add_private
		(class, sizeof (EphyActionsExtensionPropertiesDialogPrivate));

	object_class->constructor = ephy_actions_extension_properties_dialog_constructor;
	object_class->set_property = ephy_actions_extension_properties_dialog_set_property;
	object_class->get_property = ephy_actions_extension_properties_dialog_get_property;
	object_class->finalize = ephy_actions_extension_properties_dialog_finalize;

	g_object_class_install_property (object_class, PROP_EXTENSION,
					 g_param_spec_pointer ("extension",
							       NULL, NULL,
							       G_PARAM_WRITABLE
							       | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class, PROP_ACTION,
					 g_param_spec_pointer ("action",
							       NULL, NULL,
							       G_PARAM_READWRITE
							       | G_PARAM_CONSTRUCT_ONLY));
}

static void
ephy_actions_extension_properties_dialog_init
	(EphyActionsExtensionPropertiesDialog *dialog)
{
	dialog->priv = EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_GET_PRIVATE (dialog);
}

static void
ephy_actions_extension_properties_dialog_link_object_cb
	(GObject *object,
	 GParamSpec *pspec,
	 EphyActionsExtensionPropertiesDialog *dialog)
{
	GValue value = { 0, };
	guint node_property = GPOINTER_TO_UINT
		(g_object_get_data (object, NODE_PROPERTY_KEY));

	g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
	g_object_get_property (object, g_param_spec_get_name (pspec), &value);

	ephy_node_set_property (dialog->priv->action, node_property, &value);
	g_value_unset (&value);
}

static void
ephy_actions_extension_properties_dialog_link_object
	(EphyActionsExtensionPropertiesDialog *dialog,
	 GObject *object,
	 const char *object_property,
	 guint node_property)
{
	GValue value = { 0, };
	char *signal_name;

	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG (dialog));
	g_return_if_fail (dialog->priv->action != NULL);
	g_return_if_fail (G_IS_OBJECT (object));
	g_return_if_fail (object_property != NULL);

	if (ephy_node_get_property (dialog->priv->action, node_property,
				    &value))
	{
		g_object_set_property (object, object_property, &value);
		g_value_unset (&value);
	}

	g_object_set_data (object, NODE_PROPERTY_KEY,
			   GUINT_TO_POINTER (node_property));

	signal_name = g_strconcat ("notify::", object_property, NULL);
	g_signal_connect (object, signal_name,
			  G_CALLBACK (ephy_actions_extension_properties_dialog_link_object_cb),
			  dialog);
	g_free (signal_name);
}

static void
ephy_actions_extension_properties_dialog_link
	(EphyActionsExtensionPropertiesDialog *dialog, ...)
{
	const char *control_id;
	va_list args;

	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG (dialog));

	va_start (args, dialog);
	while ((control_id = va_arg (args, const char *)) != NULL)
	{
		const char *object_property;
		guint node_property;
		GtkWidget *control;

		object_property = va_arg (args, const char *);
		g_return_if_fail (object_property != NULL);

		node_property = va_arg (args, guint);
      
		control = ephy_dialog_get_control (EPHY_DIALOG (dialog),
						   control_id);
		g_return_if_fail (control != NULL);

		ephy_actions_extension_properties_dialog_link_object
			(dialog, G_OBJECT (control),
			 object_property, node_property);
	}
	va_end (args);
}

static GObject *
ephy_actions_extension_properties_dialog_constructor
	(GType type,
	 guint n_construct_properties,
	 GObjectConstructParam *construct_params)
{
	GObject *object;
	EphyActionsExtensionPropertiesDialog *dialog;

	object = parent_class->constructor (type, n_construct_properties,
					    construct_params);
	dialog = EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG (object);

	ephy_dialog_construct (EPHY_DIALOG(dialog), properties,
			       SHARE_DIR "/glade/action-properties.glade",
			       properties[PROP_ACTION_PROPERTIES].id,
			       GETTEXT_PACKAGE);

	if (dialog->priv->action == NULL)
	{
		dialog->priv->add = TRUE;
		dialog->priv->action = ephy_node_new
			(ephy_actions_extension_get_db (dialog->priv->extension));
	}

	ephy_dialog_get_controls (EPHY_DIALOG (dialog),
				  properties[PROP_ACTION_PROPERTIES].id, &(dialog->priv->dialog),
				  properties[PROP_NAME_ENTRY].id, &(dialog->priv->name_entry),
				  NULL);

	ephy_actions_extension_properties_dialog_link
		(dialog,

		 properties[PROP_NAME_ENTRY].id, "text",
		 EPHY_ACTIONS_EXTENSION_ACTION_PROP_NAME,

		 properties[PROP_DESCRIPTION_ENTRY].id, "text",
		 EPHY_ACTIONS_EXTENSION_ACTION_PROP_DESCRIPTION,

		 properties[PROP_COMMAND_ENTRY].id, "text",
		 EPHY_ACTIONS_EXTENSION_ACTION_PROP_COMMAND,

		 properties[PROP_APPLIES_TO_PAGES_CHECK].id, "active",
		 EPHY_ACTIONS_EXTENSION_ACTION_PROP_APPLIES_TO_PAGES,

		 properties[PROP_APPLIES_TO_IMAGES_CHECK].id, "active",
		 EPHY_ACTIONS_EXTENSION_ACTION_PROP_APPLIES_TO_IMAGES,

		 NULL);
				     
	if (dialog->priv->add)
	{
		gtk_window_set_title (GTK_WINDOW (dialog->priv->dialog),
				      _("Add Action"));
		gtk_dialog_add_buttons (GTK_DIALOG (dialog->priv->dialog),
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_ADD, GTK_RESPONSE_OK,
					NULL);
	}
	else
	{
		ephy_actions_extension_properties_dialog_update_title (dialog);
		gtk_dialog_add_button (GTK_DIALOG (dialog->priv->dialog),
				       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	}

	g_signal_connect (dialog->priv->dialog, "response",
  	                  G_CALLBACK (ephy_actions_extension_properties_dialog_response_cb), 
			  dialog);

	g_signal_connect (dialog->priv->name_entry, "changed",
  	                  G_CALLBACK (ephy_actions_extension_properties_dialog_name_entry_changed_cb), 
			  dialog);
	return object;
}

static void
ephy_actions_extension_properties_dialog_finalize (GObject *object)
{
	EphyActionsExtensionPropertiesDialog *dialog
		= EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG (object);

	ephy_node_unref (dialog->priv->action);
  
	parent_class->finalize (object);
}

static void
ephy_actions_extension_properties_dialog_set_property (GObject *object,
						       guint prop_id,
						       const GValue *value,
						       GParamSpec *pspec)
{
	EphyActionsExtensionPropertiesDialog *dialog
		= EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG (object);

	switch (prop_id)
	{
	case PROP_EXTENSION:
		dialog->priv->extension = g_value_get_pointer (value);
		break;
      
	case PROP_ACTION:
		dialog->priv->action = g_value_get_pointer (value);
		if (dialog->priv->action)
		{
			ephy_node_ref (dialog->priv->action);
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
ephy_actions_extension_properties_dialog_get_property (GObject *object,
						       guint prop_id,
						       GValue *value,
						       GParamSpec *pspec)
{
	EphyActionsExtensionPropertiesDialog *dialog
		= EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG (object);

	switch (prop_id)
	{
	case PROP_ACTION:
		ephy_node_ref (dialog->priv->action);
		g_value_set_pointer (value, dialog->priv->action);
		break;
		
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

EphyNode *
ephy_actions_extension_properties_dialog_get_action
	(EphyActionsExtensionPropertiesDialog *dialog)
{
	g_return_val_if_fail
		(EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG (dialog), NULL);

	return dialog->priv->action;
}

static void
ephy_actions_extension_properties_dialog_update_title
	(EphyActionsExtensionPropertiesDialog *dialog)
{
	const char *name;
	char *title;

	g_return_if_fail (EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG (dialog));

	name = gtk_entry_get_text (GTK_ENTRY (dialog->priv->name_entry));
	if (name[0] != '\0')
	{
		char *formatted;

		formatted = ephy_actions_extension_format_name_for_display (name);
		title = g_strdup_printf (_("“%s” Properties"), formatted);
		g_free (formatted);
	}
	else
	{
		title = g_strdup (_("Action Properties"));
	}
    
	gtk_window_set_title (GTK_WINDOW (dialog->priv->dialog), title);
	g_free (title);
}

EphyActionsExtensionPropertiesDialog *
ephy_actions_extension_properties_dialog_new (EphyExtension *extension,
					      EphyNode *action)
{
	g_return_val_if_fail (EPHY_IS_ACTIONS_EXTENSION (extension), NULL);

	return g_object_new (EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG,
			     "extension", extension,
			     "action", action,
			     NULL);
}

static void
ephy_actions_extension_properties_dialog_response_cb
	(GtkDialog *dialog,
	 int response,
	 EphyActionsExtensionPropertiesDialog *pdialog)
{
	if (pdialog->priv->add && response == GTK_RESPONSE_OK)
	{
		EphyNode *actions;
	  
		actions = ephy_actions_extension_get_actions
			(pdialog->priv->extension);

		ephy_node_ref (pdialog->priv->action);
		ephy_node_add_child (actions, pdialog->priv->action);
	}

	g_object_unref (pdialog);
}

static void
ephy_actions_extension_properties_dialog_name_entry_changed_cb
	(GtkEditable *editable, EphyActionsExtensionPropertiesDialog *dialog)
{
	if (dialog->priv->add == FALSE)
	{
		ephy_actions_extension_properties_dialog_update_title (dialog);
	}
}
