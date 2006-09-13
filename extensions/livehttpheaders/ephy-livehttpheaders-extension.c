/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003 Christian Persch
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

#include "ephy-livehttpheaders-extension.h"
#include "livehttpheaders-ui.h"
#include "mozilla-helper.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-shell.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

typedef struct
{
	GtkActionGroup		*action_group;
	guint			ui_id;
} WindowData;
#define WINDOW_DATA_KEY		"EphyLiveHTTPHeadersWindowData"

#define EPHY_LIVEHTTPHEADERS_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_LIVEHTTPHEADERS_EXTENSION, EphyLivehttpheadersExtensionPrivate))

struct _EphyLivehttpheadersExtensionPrivate
{
	gboolean initialized;
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

static void
ephy_livehttpheaders_extension_init (EphyLivehttpheadersExtension *extension)
{
	EphyEmbedSingle *single;

	LOG ("EphyLivehttpheadersExtension initialising");

	extension->priv = EPHY_LIVEHTTPHEADERS_EXTENSION_GET_PRIVATE (extension);

	/* Initialize Gecko ourselves */
	single = EPHY_EMBED_SINGLE (ephy_embed_shell_get_embed_single (embed_shell));

	livehttpheaders_register_listener ();

	extension->priv->initialized = TRUE;
}

static void
ephy_livehttpheaders_extension_finalize (GObject *object)
{
	LOG ("EphyLivehttpheadersExtension finalising");

	EphyLivehttpheadersExtension *extension = EPHY_LIVEHTTPHEADERS_EXTENSION (object);

	if ( extension->priv->initialized )
	{
		livehttpheaders_unregister_listener ();
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void 
live_http_headers_cb (GtkAction *action,
		      EphyWindow *window)
{
	LiveHTTPHeadersUI *ui = livehttpheaders_ui_new();

	ephy_dialog_show (EPHY_DIALOG (ui));
}

#define HTTP_HEADERS_ACTION "HTTPHeadersExt"
static const GtkActionEntry action_entries [] =
{
	{ HTTP_HEADERS_ACTION,
	  NULL,
	  N_("_HTTP Headers"),
	  NULL,
	  NULL,
	  G_CALLBACK (live_http_headers_cb)
	}
};

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	WindowData *data;
	guint ui_id;

	/* Add extension's menu entry */
	data = g_new (WindowData, 1);
	g_object_set_data_full (G_OBJECT (window), 
			 	WINDOW_DATA_KEY, 
				data,
				g_free);


	LOG ("EphyLivehttpheadersExtension attach_window");

	action_group = gtk_action_group_new ("LiveHTTPHeadersActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager,
			       ui_id,
			       "/menubar/ToolsMenu",
			       HTTP_HEADERS_ACTION "Menu",
			       HTTP_HEADERS_ACTION,
			       GTK_UI_MANAGER_MENUITEM,
			       FALSE);

	data->ui_id = ui_id;
	data->action_group = action_group;	

	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (action_group);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	LOG ("EphyLivehttpheadersExtension detach_window");

	/* Remove editor ui */
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_assert (data != NULL);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
ephy_livehttpheaders_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_livehttpheaders_extension_class_init (EphyLivehttpheadersExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_livehttpheaders_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyLivehttpheadersExtensionPrivate));
}

GType
ephy_livehttpheaders_extension_get_type (void)
{
	return type;
}

GType
ephy_livehttpheaders_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyLivehttpheadersExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_livehttpheaders_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyLivehttpheadersExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_livehttpheaders_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_livehttpheaders_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyLivehttpheadersExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
