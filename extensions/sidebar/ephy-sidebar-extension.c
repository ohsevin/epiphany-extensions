/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2004,2005 Crispin Flowerday
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

#include "ephy-sidebar-extension.h"
#include "ephy-sidebar-embed.h"
#include "ephy-sidebar.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-session.h>
#include <epiphany/ephy-node.h>
#include <epiphany/ephy-node-db.h>
#include <epiphany/ephy-state.h>

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkaction.h>
#include <gtk/gtktoggleaction.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>

#include <glib/gi18n-lib.h>

#include <gmodule.h>
#include <string.h>

#define EPHY_SIDEBAR_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SIDEBAR_EXTENSION, EphySidebarExtensionPrivate))

struct EphySidebarExtensionPrivate
{
	char *xml_file;
	EphyNodeDb *db;
	EphyNode *sidebars;
	EphyNode *state_parent;
	EphyNode *state;
};

#define SIDEBARS_NODE_ID		16
#define STATE_NODE_ID			17
#define EPHY_SIDEBARS_XML_ROOT		"ephy_sidebars"
#define EPHY_SIDEBARS_XML_VERSION	"1.0"
#define EPHY_NODE_DB_SIDEBARS		"EphySideBars"

enum
{
	SIDEBAR_NODE_PROP_URL	= 1,
	SIDEBAR_NODE_PROP_TITLE,
	
	/* For State */
	SIDEBAR_NODE_PROP_VISIBLE,
	SIDEBAR_NODE_PROP_SELECTED
};

#define WINDOW_DATA_KEY "EphySideBarExtensionWindowData"

typedef struct
{
	EphySidebarExtension *extension;

	GtkActionGroup *action_group;
	guint merge_id;

	GtkWidget *sidebar;
	GtkWidget *hpaned;
	GtkWidget *embed;
} WindowData;

static void cmd_view_sidebar	(GtkAction *action,
				 EphyWindow *window);

static GtkToggleActionEntry toggle_action_entries [] =
{
	{ "ViewSidebar", NULL, N_("_Sidebar"), "F9", 
	  N_("Show or hide the sidebar"), G_CALLBACK(cmd_view_sidebar), FALSE }
};

static void ephy_sidebar_extension_class_init	(EphySidebarExtensionClass *klass);
static void ephy_sidebar_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_sidebar_extension_init		(EphySidebarExtension *extension);
static void extension_weak_notify_cb		(GObject *dialog,
						 GObject *extension);
static void add_dialog_weak_notify_cb		(GObject *extension,
						 GObject *dialog);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_sidebar_extension_get_type (void)
{
	return type;
}

GType
ephy_sidebar_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphySidebarExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_sidebar_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySidebarExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_sidebar_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_sidebar_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphySidebarExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void 
cmd_view_sidebar (GtkAction *action, EphyWindow *window)
{
	GValue value = { 0, };
	gboolean ppview_mode;
	WindowData *data;

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);

	g_object_get (window, "print-preview-mode", &ppview_mode, NULL);
	g_value_init (&value, G_TYPE_BOOLEAN);

	if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)))
	{
		if (!ppview_mode)
		{
			gtk_widget_show (data->sidebar);
		}
		g_value_set_boolean (&value, TRUE);
	}
	else
	{
		gtk_widget_hide (data->sidebar);
		g_value_set_boolean (&value, FALSE);
	}

	ephy_node_set_property (data->extension->priv->state,
				SIDEBAR_NODE_PROP_VISIBLE, &value);
	g_value_unset (&value);
}

static void
window_ppv_mode_notify_cb (EphyWindow *window, GParamSpec   *pspec,
                           WindowData *data)
{
	GtkAction *action;
	gboolean ppview_mode;

	g_object_get (window, "print-preview-mode", &ppview_mode, NULL);
	action = gtk_action_group_get_action (data->action_group, "ViewSidebar");

	if (ppview_mode)
	{
		gtk_widget_hide (data->sidebar);
	}
	else if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)))
	{
		gtk_widget_show (data->sidebar);
	}
}

static void
sidebar_page_changed_cb (GtkWidget* sidebar,
			 const char * page_id, 
			 WindowData *data)
{
	GValue value = { 0, };

	/* If you want to show custom stuff in the sidebar, here
	 * is where you set the content for the sidebar:
	 * if (!strcmp (page_id, "MyPage"))
	 * {
	 *      ephy_sidebar_embed_set_url (data->embed, NULL);
	 *      ephy_sidebar_set_content (data->sidebar, mycontentwidget);
	 * } else { .... }
	 */
	
	ephy_sidebar_embed_set_url (EPHY_SIDEBAR_EMBED (data->embed), page_id);
	ephy_sidebar_set_content (EPHY_SIDEBAR (data->sidebar),
				  GTK_WIDGET (data->embed));
	gtk_widget_show (GTK_WIDGET (data->embed));

	/* Remeber the current page */
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, page_id ? page_id : "");
	ephy_node_set_property (data->extension->priv->state, SIDEBAR_NODE_PROP_SELECTED, &value);
	g_value_unset (&value);
}

static void
sidebar_close_requested_cb (GtkWidget *sidebar,
			    GtkAction *action)
{
	g_return_if_fail (GTK_IS_TOGGLE_ACTION (action));

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
}

static void
sidebar_page_remove_requested_cb (GtkWidget *sidebar,
				  const char * page_id,
				  EphySidebarExtension *extension)
{
	EphyNode *removeNode = NULL, *node;
	const char *url;
	int i;

	g_return_if_fail (EPHY_IS_SIDEBAR (sidebar));
	g_return_if_fail (page_id != NULL);

	for (i = 0 ; i < ephy_node_get_n_children (extension->priv->sidebars); i++)
	{
		node = ephy_node_get_nth_child (extension->priv->sidebars, i);
		url = ephy_node_get_property_string (node, SIDEBAR_NODE_PROP_URL);
		
		if (strcmp (page_id, url) == 0)
		{
			removeNode = node;
			break;
		}
	}

	if (removeNode == NULL)
	{
		g_warning ("Remove requested for Sidebar not in EphyNodeDB");
		return;
	}

	ephy_node_remove_child (extension->priv->sidebars, removeNode);
}

static void
node_child_added_cb (EphyNode *node,
		     EphyNode *child, 
		     EphySidebar *sidebar)
{
	const char * url, *title;

	g_return_if_fail (EPHY_IS_SIDEBAR (sidebar));

	url   = ephy_node_get_property_string (child, SIDEBAR_NODE_PROP_URL);
	title = ephy_node_get_property_string (child, SIDEBAR_NODE_PROP_TITLE);

	ephy_sidebar_add_page (sidebar, title, url, TRUE);
}

static void
node_child_removed_cb (EphyNode *node,
		       EphyNode *child, 
		       guint old_index,
		       EphySidebar *sidebar)
{
	const char * url;

	g_return_if_fail (EPHY_IS_SIDEBAR (sidebar));

	url = ephy_node_get_property_string (child, SIDEBAR_NODE_PROP_URL);
	
	ephy_sidebar_remove_page (sidebar, url);
}

struct ResponseCallbackData
{
	char *url;
	char *title;
	EphySidebarExtension *extension;
};

static void
free_response_data (struct ResponseCallbackData *data)
{
	g_free (data->url);
	g_free (data->title);
	g_free (data);
}

static void
add_dialog_response_cb (GtkDialog *dialog,
			int response, 
			struct ResponseCallbackData *data)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		EphyNode *node;
		GValue value = { 0, };

		node = ephy_node_new (data->extension->priv->db);

		g_value_init (&value, G_TYPE_STRING);
		g_value_set_string (&value, data->url);
		ephy_node_set_property (node, SIDEBAR_NODE_PROP_URL, &value);
		g_value_unset (&value);

		g_value_init (&value, G_TYPE_STRING);
		g_value_set_string (&value, data->title);
		ephy_node_set_property (node, SIDEBAR_NODE_PROP_TITLE, &value);
		g_value_unset (&value);

		ephy_node_add_child (data->extension->priv->sidebars, node);
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
extension_weak_notify_cb (GObject *dialog,
			  GObject *extension)
{
	g_object_weak_unref (dialog,
			     (GWeakNotify) add_dialog_weak_notify_cb, extension);

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
add_dialog_weak_notify_cb (GObject *extension,
			   GObject *dialog)
{
	g_object_weak_unref (extension,
			     (GWeakNotify) extension_weak_notify_cb, dialog);
}

static gboolean
ephy_sidebar_extension_add_sidebar_cb (EphyEmbedSingle *single,
                                       const char *url,
                                       const char *title,
                                       EphySidebarExtension *extension)
{
	EphySession *session;
	EphyWindow *window;
	GtkWidget *dialog;
	struct ResponseCallbackData *cb_data;
	int i;
	
	/* See if the Sidebar is already added */
	for (i = 0 ; i < ephy_node_get_n_children (extension->priv->sidebars); i++)
	{
		EphyNode *node = ephy_node_get_nth_child (extension->priv->sidebars, i);
		const char * node_url = ephy_node_get_property_string (node, SIDEBAR_NODE_PROP_URL);
		
		if (strcmp (node_url, url) == 0)
		{
			/* TODO Should we raise a dialog or perform an action here */
			return TRUE;
		}
	}

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	window = ephy_session_get_active_window (session);

	dialog = gtk_message_dialog_new
		(GTK_WINDOW (window),
		 GTK_DIALOG_DESTROY_WITH_PARENT,
		 GTK_MESSAGE_QUESTION,
		 GTK_BUTTONS_CANCEL,
		 _("Add \"%s\" to the Sidebar?"), title);

	gtk_message_dialog_format_secondary_text
		(GTK_MESSAGE_DIALOG (dialog),
		 _("The source to the new sidebar page is %s."), url);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Add Sidebar"));
	gtk_window_set_icon_name (GTK_WINDOW (dialog), "web-browser");

	gtk_dialog_add_button (GTK_DIALOG (dialog),
			       _("_Add Sidebar"), GTK_RESPONSE_ACCEPT);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	cb_data = g_new (struct ResponseCallbackData, 1);
	cb_data->url = g_strdup (url);
	cb_data->title = g_strdup (title);
	cb_data->extension = extension;

	g_signal_connect_data (dialog, "response", 
			       G_CALLBACK (add_dialog_response_cb),
			       cb_data, (GClosureNotify) free_response_data,
			       0);

	g_object_weak_ref (G_OBJECT (extension),
			   (GWeakNotify) extension_weak_notify_cb, dialog);
	g_object_weak_ref (G_OBJECT (dialog),
			   (GWeakNotify) add_dialog_weak_notify_cb, extension);

	gtk_widget_show (GTK_WIDGET (dialog));

	return TRUE;
}

/* work-around for http://bugzilla.gnome.org/show_bug.cgi?id=169116 */
static void
fixup (GtkWidget *widget)
{
	gtk_widget_hide (widget);
	gtk_widget_show (widget);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphySidebarExtension *extension = EPHY_SIDEBAR_EXTENSION (ext);
	WindowData *data;
	GtkActionGroup *action_group;
	GtkAction *action;
	guint merge_id;
	GtkUIManager *manager;
	GtkWidget *sidebar, *notebook, *parent, *hpaned;
	GValue position = { 0, };
	int i;
	const char * current_page;
	gboolean show_sidebar;

	LOG ("EphySidebarExtension attach_window");
	
	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = g_new (WindowData, 1);
	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) g_free);

	data->extension = extension;
	data->sidebar = sidebar = ephy_sidebar_new();

	data->action_group = action_group =
		gtk_action_group_new ("EphySidebarExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_toggle_actions (action_group, toggle_action_entries,
					     G_N_ELEMENTS (toggle_action_entries),
					     window);
	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (action_group);

	data->merge_id = merge_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, merge_id,
			       "/menubar/ViewMenu/ViewTogglesGroup",
			       "ViewSidebar", "ViewSidebar",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	g_signal_connect (window, "notify::print-preview-mode",
			  G_CALLBACK (window_ppv_mode_notify_cb), data);

	/* Add the sidebar and the current notebook to a
	 * GtkHPaned, and add the hpaned to where the notebook
	 * currently is */
	notebook = ephy_window_get_notebook (window);
	parent = gtk_widget_get_parent (notebook);

	g_value_init (&position, G_TYPE_INT);
	gtk_container_child_get_property (GTK_CONTAINER (parent),
					  notebook, "position", &position);

	hpaned = data->hpaned = gtk_hpaned_new ();
	gtk_widget_show (hpaned);
	gtk_paned_add1 (GTK_PANED(hpaned), sidebar);

	g_object_ref (notebook);
	gtk_container_remove (GTK_CONTAINER (parent), notebook);
	gtk_paned_add2 (GTK_PANED(hpaned), notebook);
	g_object_unref (notebook);

	/* work-around for http://bugzilla.gnome.org/show_bug.cgi?id=169116 */
	fixup (ephy_window_get_notebook (window));

	gtk_container_add (GTK_CONTAINER (parent), hpaned);
	gtk_container_child_set_property (GTK_CONTAINER (parent),
					  hpaned, "position", &position);
	g_value_unset (&position);

	ephy_state_add_paned (hpaned, "EphySidebarExtension::HPaned", 220);

	data->embed = ephy_sidebar_embed_new (window);
	g_object_ref (data->embed);
	gtk_object_sink (GTK_OBJECT (data->embed));

	g_signal_connect (sidebar, "page_changed", 
			  G_CALLBACK(sidebar_page_changed_cb), data);

	/* Add the current sidebar pages */
	for (i = 0 ; i < ephy_node_get_n_children (extension->priv->sidebars); i++)
	{
		EphyNode *node;

		node = ephy_node_get_nth_child (extension->priv->sidebars, i);
		node_child_added_cb (extension->priv->sidebars, node,
				     EPHY_SIDEBAR (data->sidebar));
	}

	/* And keep it all in-sync */
	g_signal_connect (sidebar, "remove_requested",
			  G_CALLBACK (sidebar_page_remove_requested_cb), ext);
	ephy_node_signal_connect_object (extension->priv->sidebars,
					 EPHY_NODE_CHILD_ADDED,
					 (EphyNodeCallback)node_child_added_cb, 
					 G_OBJECT(sidebar));
	ephy_node_signal_connect_object (extension->priv->sidebars,
					 EPHY_NODE_CHILD_REMOVED,
					 (EphyNodeCallback)node_child_removed_cb, 
					 G_OBJECT(sidebar));

	action = gtk_action_group_get_action (action_group, "ViewSidebar");
	g_signal_connect (sidebar, "close_requested",
			  G_CALLBACK (sidebar_close_requested_cb),
			  action);

	/* Persist page */
	current_page = ephy_node_get_property_string (extension->priv->state, 
						      SIDEBAR_NODE_PROP_SELECTED);
	if (current_page && current_page[0])
	{
		ephy_sidebar_select_page (EPHY_SIDEBAR(sidebar), current_page);
	}

	/* Persist visibility */
	show_sidebar = ephy_node_get_property_boolean (extension->priv->state, 
						       SIDEBAR_NODE_PROP_VISIBLE);
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), show_sidebar);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	GtkWidget *notebook, *parent;
	GValue position = {0,};

	LOG ("EphySidebarExtension detach_window");

	/* Remove UI */
	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->merge_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_signal_handlers_disconnect_by_func
		(window, G_CALLBACK (window_ppv_mode_notify_cb), data);

	/* Remove the Sidebar, replacing our hpaned with the
	 * notebook itself */
	notebook = ephy_window_get_notebook (window);
	parent = gtk_widget_get_parent (data->hpaned);

	g_value_init (&position, G_TYPE_INT);
	gtk_container_child_get_property (GTK_CONTAINER (parent),
					  data->hpaned, "position", &position);

	g_object_ref (notebook);
	gtk_container_remove (GTK_CONTAINER (data->hpaned), notebook);
	
	gtk_container_remove (GTK_CONTAINER (parent), data->hpaned);
	
	gtk_container_add (GTK_CONTAINER (parent), notebook);
	g_object_unref (notebook);

	/* work-around for http://bugzilla.gnome.org/show_bug.cgi?id=169116 */
	fixup (ephy_window_get_notebook (window));

	gtk_container_child_set_property (GTK_CONTAINER (parent),
					  notebook, "position", &position);

	g_object_unref (data->embed);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_fixup_tab (EphyExtension *extension,
		EphyWindow *window,
		EphyTab *tab)
{
	fixup (GTK_WIDGET (tab));
}
		
static void
ephy_sidebar_extension_init (EphySidebarExtension *extension)
{
	EphyNodeDb *db;
	GObject *single;

	LOG ("EphySidebarExtension initialising");

	extension->priv = EPHY_SIDEBAR_EXTENSION_GET_PRIVATE (extension);

	db = ephy_node_db_new (EPHY_NODE_DB_SIDEBARS);
	extension->priv->db = db;

	extension->priv->xml_file = g_build_filename (ephy_dot_dir (),
						      "ephy-sidebars.xml",
						      NULL);

	extension->priv->sidebars = ephy_node_new_with_id (db, SIDEBARS_NODE_ID);
	extension->priv->state_parent = ephy_node_new_with_id (db, STATE_NODE_ID);

	ephy_node_db_load_from_file (extension->priv->db,
				     extension->priv->xml_file,
				     EPHY_SIDEBARS_XML_ROOT,
				     EPHY_SIDEBARS_XML_VERSION);
	
	if (ephy_node_get_n_children (extension->priv->state_parent))
	{
		extension->priv->state = 
			ephy_node_get_nth_child (extension->priv->state_parent, 0);
	}
	else
	{
		extension->priv->state = ephy_node_new (db);
		ephy_node_add_child (extension->priv->state_parent,
				     extension->priv->state);
	}

	single = ephy_embed_shell_get_embed_single (embed_shell);
	g_signal_connect (single, "add-sidebar",
			  G_CALLBACK (ephy_sidebar_extension_add_sidebar_cb),
			  extension);
}

static void
ephy_sidebar_extension_finalize (GObject *object)
{
	EphySidebarExtension *extension = EPHY_SIDEBAR_EXTENSION (object);
	GObject *single;

	LOG ("EphySidebarExtension finalising");

	ephy_node_db_write_to_xml_safe
		(extension->priv->db, 
		 extension->priv->xml_file,
		 EPHY_SIDEBARS_XML_ROOT,
		 EPHY_SIDEBARS_XML_VERSION,
		 NULL,
		 extension->priv->sidebars, 0, 
		 extension->priv->state_parent, 0, 
		 NULL);

	g_free (extension->priv->xml_file);
	ephy_node_unref (extension->priv->sidebars);
	ephy_node_unref (extension->priv->state_parent);
	ephy_node_unref (extension->priv->state);

	g_object_unref (extension->priv->db);

	/* Disconnect our signal */
	single = ephy_embed_shell_get_embed_single (embed_shell);
	g_signal_handlers_disconnect_matched
		(single, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_sidebar_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_fixup_tab;
	iface->detach_tab = impl_fixup_tab;
}

static void
ephy_sidebar_extension_class_init (EphySidebarExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_sidebar_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphySidebarExtensionPrivate));
}
