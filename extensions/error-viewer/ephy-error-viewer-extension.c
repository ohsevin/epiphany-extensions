/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "register-listener.h"
#include "ephy-error-viewer-extension.h"
#include "error-viewer.h"
#include "ephy-debug.h"

#include "mozilla/mozilla-helpers.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-embed.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkstock.h>

#include <glib/gi18n-lib.h>

#ifdef HAVE_OPENSP
#include "sgml-validator.h"
#endif /* HAVE_OPENSP */

#include "link-checker.h"

#define WINDOW_DATA_KEY	"EphyErrorViewerExtWindowData"

#define EPHY_ERROR_VIEWER_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_ERROR_VIEWER_EXTENSION, EphyErrorViewerExtensionPrivate))

struct EphyErrorViewerExtensionPrivate
{
	ErrorViewer *dialog;
#if HAVE_OPENSP
	SgmlValidator *validator;
#endif /* HAVE_OPENSP */
	LinkChecker *link_checker;
	void *listener;
};

typedef struct
{
	EphyErrorViewerExtension *extension;
	EphyWindow *window;
} ErrorViewerCBData;

typedef struct
{
	GtkActionGroup *action_group;
	guint ui_id;
} WindowData;

static void ephy_error_viewer_extension_class_init	(EphyErrorViewerExtensionClass *klass);
static void ephy_error_viewer_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_error_viewer_extension_init		(EphyErrorViewerExtension *extension);

static void ephy_error_viewer_extension_show_viewer	(GtkAction *action,
							 ErrorViewerCBData *data);
#ifdef HAVE_OPENSP
static void ephy_error_viewer_extension_validate	(GtkAction *action,
							 ErrorViewerCBData *data);
#endif /* HAVE_OPENSP */
static void ephy_error_viewer_extension_check_links	(GtkAction *action,
							 ErrorViewerCBData *data);

static GtkActionEntry action_entries [] =
{
#ifdef HAVE_OPENSP
	{ "SgmlValidate",
	  NULL,
	  N_("Check _HTML"),
	  NULL, /* shortcut key */
	  N_("Display HTML errors in the Error Viewer dialog"),
	  G_CALLBACK (ephy_error_viewer_extension_validate) },
#endif /* HAVE_OPENSP */
	{ "CheckLinks",
	  NULL,
	  N_("Check _Links"),
	  NULL, /* shortcut key */
	  N_("Display invalid hyperlinks in the Error Viewer dialog"),
	  G_CALLBACK (ephy_error_viewer_extension_check_links) },
	{ "ErrorViewer",
	  GTK_STOCK_DIALOG_ERROR,
	  N_("_Error Viewer"),
	  NULL, /* shortcut key */
	  N_("Display web page errors"),
	  G_CALLBACK (ephy_error_viewer_extension_show_viewer) }
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_error_viewer_extension_get_type (void)
{
	return type;
}

GType
ephy_error_viewer_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyErrorViewerExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_error_viewer_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyErrorViewerExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_error_viewer_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_error_viewer_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyErrorViewerExtension",
					    &our_info, 0);
	
	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_error_viewer_extension_init (EphyErrorViewerExtension *extension)
{
	LOG ("EphyErrorViewerExtension initialising")

	ephy_embed_shell_get_embed_single (embed_shell); /* Fire up Mozilla */

	extension->priv = EPHY_ERROR_VIEWER_EXTENSION_GET_PRIVATE (extension);

	extension->priv->dialog = error_viewer_new ();

	extension->priv->link_checker = link_checker_new
		(extension->priv->dialog);

#ifdef HAVE_OPENSP
	extension->priv->validator = sgml_validator_new
		(extension->priv->dialog);
#endif /* HAVE_OPENSP */

	extension->priv->listener = mozilla_register_error_listener
		(G_OBJECT (extension->priv->dialog));
}

static void
ephy_error_viewer_extension_finalize (GObject *object)
{
	EphyErrorViewerExtension *extension =
		EPHY_ERROR_VIEWER_EXTENSION (object);

	LOG ("EphyErrorViewerExtension finalizing")

	mozilla_unregister_error_listener (extension->priv->listener);

#ifdef HAVE_OPENSP
	g_object_unref (G_OBJECT (extension->priv->validator));
#endif /* HAVE_OPENSP */

	g_object_unref (G_OBJECT (extension->priv->link_checker));

	g_object_unref (G_OBJECT (extension->priv->dialog));

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_error_viewer_extension_class_init (EphyErrorViewerExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_error_viewer_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyErrorViewerExtensionPrivate));
}

#ifdef HAVE_OPENSP
static void
ephy_error_viewer_extension_validate (GtkAction *action,
				      ErrorViewerCBData *data)
{
	ErrorViewer *dialog;
	EphyEmbed *embed;

	dialog = data->extension->priv->dialog;

	embed = ephy_window_get_active_embed (data->window);

	ephy_dialog_show (EPHY_DIALOG (dialog));

	sgml_validator_validate (data->extension->priv->validator, embed);
}
#endif /* HAVE_OPENSP */

static void
ephy_error_viewer_extension_check_links	(GtkAction *action,
					 ErrorViewerCBData *data)
{
	ErrorViewer *dialog;
	EphyEmbed *embed;

	dialog = data->extension->priv->dialog;

	embed = ephy_window_get_active_embed (data->window);

	ephy_dialog_show (EPHY_DIALOG (dialog));

	link_checker_check (data->extension->priv->link_checker, embed);
}

static void
ephy_error_viewer_extension_show_viewer (GtkAction *action,
					 ErrorViewerCBData *data)
{
	ephy_dialog_show (EPHY_DIALOG (data->extension->priv->dialog));
}

static void
free_error_viewer_cb_data (gpointer data)
{
	if (data)
	{
		g_free (data);
	}
}

#ifdef HAVE_OPENSP
static void
update_actions (EphyWindow *window)
{
	EphyTab *tab;
	EphyEmbed *embed;
	GtkAction *action1;
	GtkAction *action2; /* You can tell I didn't want to think... */
	char *content_type;
	GValue sensitive = { 0, };

	g_return_if_fail (EPHY_IS_WINDOW (window));

	g_value_init (&sensitive, G_TYPE_BOOLEAN);
	g_value_set_boolean (&sensitive, FALSE);

	action1 = gtk_ui_manager_get_action (GTK_UI_MANAGER (ephy_window_get_ui_manager (window)),
					     "/menubar/ToolsMenu/SgmlValidate");
	action2 = gtk_ui_manager_get_action (GTK_UI_MANAGER (ephy_window_get_ui_manager (window)),
					     "/menubar/ToolsMenu/CheckLinks");

	tab = ephy_window_get_active_tab (window);

	/* Not finished loading? */
	if (ephy_tab_get_load_status (tab) == TRUE)
	{
		g_object_set_property (G_OBJECT (action1),
				       "sensitive", &sensitive);
		g_object_set_property (G_OBJECT (action2),
				       "sensitive", &sensitive);
		g_value_unset (&sensitive);
		return;
	}

	embed = ephy_tab_get_embed (tab);

	content_type = mozilla_get_content_type (embed);

	if ((strcmp (content_type, "text/html") == 0)
	    || (strcmp (content_type, "application/xhtml+xml") == 0)
	    || (strcmp (content_type, "application/xml") == 0)
	    || (strcmp (content_type, "text/xml") == 0))
	{
		g_value_set_boolean (&sensitive, TRUE);
	}

	g_free (content_type);

	g_object_set_property (G_OBJECT (action1), "sensitive", &sensitive);
	g_object_set_property (G_OBJECT (action2), "sensitive", &sensitive);

	g_value_unset (&sensitive);
}

static void
load_status_cb (EphyTab *tab,
		GParamSpec *pspec,
		EphyWindow *window)
{
	update_actions (window);
}

static void
switch_page_cb (GtkNotebook *notebook,
		GtkNotebookPage *page,
		guint page_num,
		EphyWindow *window)
{
	g_return_if_fail (EPHY_IS_WINDOW (window));
	if (GTK_WIDGET_REALIZED (window) == FALSE) return; /* on startup */

	update_actions (window);
}

static void
tab_added_cb (GtkWidget *notebook,
	      EphyTab *tab,
	      EphyWindow *window)
{
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_connect_after (tab, "notify::load-status",
				G_CALLBACK (load_status_cb), window);
}

static void
tab_removed_cb (GtkWidget *notebook,
		EphyTab *tab,
		EphyWindow *window)
{
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_handlers_disconnect_by_func
		(tab, G_CALLBACK (load_status_cb), window);
}
#endif /* HAVE_OPENSP */

static void
free_window_data (WindowData *data)
{
	if (data) {
		g_object_unref (data->action_group);
		g_free (data);
	}
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	ErrorViewerCBData *cb_data;
	GtkActionGroup *action_group;
	GtkUIManager *manager;
#ifdef HAVE_OPENSP
	GtkWidget *notebook;
#endif /* HAVE_OPENSP */
	guint merge_id;
	WindowData *data;

	LOG ("EphyErrorViewerExtension attach_window")

	cb_data = g_new (ErrorViewerCBData, 1);
	cb_data->extension = EPHY_ERROR_VIEWER_EXTENSION (extension);
	cb_data->window = window;

	data = g_new (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data->action_group = action_group =
		gtk_action_group_new ("EphyErrorViewerExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions_full (action_group, action_entries,
					   n_action_entries, cb_data,
					   free_error_viewer_cb_data);

	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	data->ui_id = merge_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ToolsMenu",
			       "ErrorViewerSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ToolsMenu",
			       "CheckLinks", "CheckLinks",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
#ifdef HAVE_OPENSP
	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ToolsMenu",
			       "SgmlValidate", "SgmlValidate",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
#endif /* HAVE_OPENSP */
	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ToolsMenu",
			       "ErrorViewer", "ErrorViewer",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

#ifdef HAVE_OPENSP
	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "tab_added",
				G_CALLBACK (tab_added_cb), window);
	g_signal_connect_after (notebook, "tab_removed",
				G_CALLBACK (tab_removed_cb), window);
	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), window);
#endif /* HAVE_OPENSP */
}

static void
impl_detach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
#ifdef HAVE_OPENSP
	GtkWidget *notebook;
#endif /* HAVE_OPENSP */

	/* Remove UI */
	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);

#ifdef HAVE_OPENSP
	/* Remove notification signals for HTML/link checking */
	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_added_cb), window);
	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_removed_cb), window);
	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (switch_page_cb), window);
#endif /* HAVE_OPENSP */
}

static void
ephy_error_viewer_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}
