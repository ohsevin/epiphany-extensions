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

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-embed-shell.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkstock.h>

#include <glib/gi18n-lib.h>

#define EPHY_ERROR_VIEWER_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_ERROR_VIEWER_EXTENSION, EphyErrorViewerExtensionPrivate))

struct EphyErrorViewerExtensionPrivate
{
	ErrorViewer *dialog;
	void *listener;
};

static void ephy_error_viewer_extension_class_init (EphyErrorViewerExtensionClass *klass);
static void ephy_error_viewer_extension_iface_init (EphyExtensionClass *iface);
static void ephy_error_viewer_extension_init (EphyErrorViewerExtension *extension);

static void ephy_error_viewer_extension_show_viewer (GtkAction *action, EphyErrorViewerExtension *window);

static GtkActionEntry action_entries [] =
{
	{ "ErrorViewer",
	  GTK_STOCK_DIALOG_ERROR,
	  N_("Errors"),
	  NULL, /* shortcut key */
	  N_("Display web page errors"),
	  G_CALLBACK (ephy_error_viewer_extension_show_viewer) }
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static GObjectClass *error_viewer_extension_parent_class = NULL;

static GType error_viewer_extension_type = 0;

GType
ephy_error_viewer_extension_get_type (void)
{
	return error_viewer_extension_type;
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

	error_viewer_extension_type =
		g_type_module_register_type (module, G_TYPE_OBJECT, "EphyErrorViewerExtension", &our_info, 0);
	
	g_type_module_add_interface (module, error_viewer_extension_type, EPHY_TYPE_EXTENSION, &extension_info);

	return error_viewer_extension_type;
}

static void
ephy_error_viewer_extension_init (EphyErrorViewerExtension *extension)
{
	LOG ("EphyErrorViewerExtension initialising")

	ephy_embed_shell_get_embed_single (embed_shell); /* Fire up Mozilla */

	extension->priv = EPHY_ERROR_VIEWER_EXTENSION_GET_PRIVATE (extension);

	extension->priv->dialog = error_viewer_new ();

	extension->priv->listener = mozilla_register_error_listener (G_OBJECT (extension->priv->dialog));
}

static void
ephy_error_viewer_extension_finalize (GObject *object)
{
	EphyErrorViewerExtension *extension =
		EPHY_ERROR_VIEWER_EXTENSION (object);

	LOG ("EphyErrorViewerExtension finalizing")

	mozilla_unregister_error_listener (extension->priv->listener);

	g_object_unref (G_OBJECT (extension->priv->dialog));

	G_OBJECT_CLASS (error_viewer_extension_parent_class)->finalize (object);
}

static void
ephy_error_viewer_extension_class_init (EphyErrorViewerExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	error_viewer_extension_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_error_viewer_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyErrorViewerExtensionPrivate));
}

static void
ephy_error_viewer_extension_show_viewer (GtkAction *action,
					 EphyErrorViewerExtension *ext)
{
	ephy_dialog_show (EPHY_DIALOG (ext->priv->dialog));
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	guint merge_id;

	LOG ("EphyErrorViewerExtension attach_window")

	manager = GTK_UI_MANAGER (window->ui_merge);

	action_group = gtk_action_group_new ("EphyErrorViewerExtensionActions");

	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      n_action_entries,
				      EPHY_ERROR_VIEWER_EXTENSION (extension));

	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	g_object_unref (action_group);

	merge_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ViewMenu",
			       "ErrorViewerSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ViewMenu",
			       "ErrorViewer", "ErrorViewer",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
}

static void
impl_detach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	LOG ("EphyErrorViewerExtension detach_window")
}

static void
ephy_error_viewer_extension_iface_init (EphyExtensionClass *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}
