/*
 *  Copyright (C) 2005 Adam Hooper
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

#include "ephy-greasemonkey-extension.h"
#include "greasemonkey-script.h"
#include "mozilla-helpers.h"
#include "ephy-debug.h"
#include "ephy-file-helpers.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-embed-factory.h>
#include <epiphany/ephy-embed-event.h>
#include <epiphany/ephy-embed-persist.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-window.h>

#include <gtk/gtkuimanager.h>

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define EPHY_GREASEMONKEY_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GREASEMONKEY_EXTENSION, EphyGreasemonkeyExtensionPrivate))

#define WINDOW_DATA_KEY "EphyGreasemonkeyExtensionWindowData"
#define ACTION_NAME "EphyGreasemonkeyInstallScript"
#define ACTION_LINK_KEY "EphyGreasemonkeyScriptUrl"

struct _EphyGreasemonkeyExtensionPrivate
{
	GSList *scripts;
};

enum
{
	PROP_0
};

typedef struct
{
	GtkActionGroup *action_group;
	guint ui_id;
	char *last_clicked_url;
} WindowData;

static void ephy_greasemonkey_extension_install_cb (GtkAction *action,
						    EphyWindow *window);

static const GtkActionEntry action_entries [] =
{
	{ ACTION_NAME,
	  NULL,
	  N_("Install User _Script"),
	  NULL, /* shortcut key */
	  N_("Install Greasemonkey script"),
	  G_CALLBACK (ephy_greasemonkey_extension_install_cb) }
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

static void
load_scripts (EphyGreasemonkeyExtension *extension)
{
	DIR *d;
	struct dirent *e;
	char *path;
	char *file_path;
	GreasemonkeyScript *script;

	path = g_build_filename (ephy_dot_dir (),
				 "extensions", "data", "greasemonkey", NULL);

	d = opendir (path);
	if (d == NULL)
	{
		return;
	}
	while ((e = readdir (d)) != NULL)
	{
		if (g_str_has_suffix (e->d_name, ".user.js"))
		{
			file_path = g_build_filename (path, e->d_name, NULL);
			LOG ("Loading script at %s", file_path);
			script = greasemonkey_script_new (file_path);
			extension->priv->scripts = g_slist_prepend
				(extension->priv->scripts, script);
			g_free (file_path);
		}
	}
	closedir (d);
}

static void
ephy_greasemonkey_extension_init (EphyGreasemonkeyExtension *extension)
{
	extension->priv = EPHY_GREASEMONKEY_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyGreasemonkeyExtension initialising");

	/* FIXME: monitor directory instead */
	load_scripts (extension);
}

static void
ephy_greasemonkey_extension_finalize (GObject *object)
{
	EphyGreasemonkeyExtension *extension = EPHY_GREASEMONKEY_EXTENSION (object);

	LOG ("EphyGreasemonkeyExtension finalising");

	g_slist_foreach (extension->priv->scripts,
			 (GFunc) g_object_unref, NULL);
	g_slist_free (extension->priv->scripts);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static const char *
get_script_dir (void)
{
	static char *script_dir = NULL;

	if (script_dir == NULL)
	{
		char *path;
		char *t;
		int ret;

		path = g_build_filename (ephy_dot_dir (), "extensions", NULL);
		if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
		{
			ret = mkdir (path, 0755);
			g_return_val_if_fail (ret == 0, NULL);
		}

		t = path;
		path = g_build_filename (path, "data", NULL);
		g_free (t);
		if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
		{
			ret = mkdir (path, 0755);
			g_return_val_if_fail (ret == 0, NULL);
		}

		t = path;
		path = g_build_filename (path, "greasemonkey", NULL);
		g_free (t);
		if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
		{
			ret = mkdir (path, 0755);
			g_return_val_if_fail (ret == 0, NULL);
		}

		script_dir = path;
	}

	return script_dir;
}

static char *
script_name_build (const char *url)
{
	char *basename;
	char *path;
	const char *script_dir;

	script_dir = get_script_dir ();
	g_return_val_if_fail (script_dir != NULL, NULL);

	basename = g_filename_from_utf8 (url, -1, NULL, NULL, NULL);
	g_return_val_if_fail (basename != NULL, NULL);

	g_strdelimit (basename, "/", '_');

	path = g_build_filename (script_dir, basename, NULL);

	g_free (basename);

	return path;
}

static void
save_source_completed_cb (EphyEmbedPersist *persist,
			  gpointer dummy)
{
	g_print ("Download to %s complete.\n",
		 ephy_embed_persist_get_dest (persist));
}

static void
ephy_greasemonkey_extension_install_cb (GtkAction *action,
					EphyWindow *window)
{
	EphyEmbed *embed;
	EphyEmbedPersist *persist;
	WindowData *data;
	const char *url;
	const char *filename;

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	url = data->last_clicked_url;
	g_return_if_fail (url != NULL);

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);

	LOG ("Installing script at '%s'", url);

	filename = script_name_build (url);
	g_return_if_fail (filename != NULL);

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object (EPHY_TYPE_EMBED_PERSIST));

	ephy_embed_persist_set_source (persist, url);
	ephy_embed_persist_set_embed (persist, embed);
	ephy_embed_persist_set_dest (persist, filename);

	g_signal_connect (persist, "completed",
			  G_CALLBACK (save_source_completed_cb), NULL);

	ephy_embed_persist_save (persist);

	g_object_unref (G_OBJECT (persist));
}

static EphyWindow *
ephy_window_for_ephy_embed (EphyEmbed *embed)
{
	EphyTab *tab;
	EphyWindow *window;

	tab = ephy_tab_for_embed (embed);
	g_return_val_if_fail (EPHY_IS_TAB (tab), NULL);

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab)));
	g_return_val_if_fail (EPHY_IS_WINDOW (window), NULL);

	return window;
}

static gboolean
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphyGreasemonkeyExtension *extension)
{
	/*
	 * Set whether or not the action is visible before we display the
	 * context popup menu
	 */
	WindowData *window_data;
	EphyWindow *window;
	EphyEmbedEventContext context;
	GtkAction *action;
	const GValue *value;
	const char *url;
	gboolean show_install;

	context = ephy_embed_event_get_context (event);
	if ((context & EPHY_EMBED_CONTEXT_LINK) == 0)
	{
		return FALSE;
	}

	window = ephy_window_for_ephy_embed (embed);
	g_return_val_if_fail (window != NULL, FALSE);

	ephy_embed_event_get_property (event, "link", &value);
	url = g_value_get_string (value);

	show_install = g_str_has_suffix (url, ".user.js");

	window_data = (WindowData *) g_object_get_data (G_OBJECT (window),
							WINDOW_DATA_KEY);
	g_return_val_if_fail (window_data != NULL, FALSE);

	action = gtk_action_group_get_action (window_data->action_group,
					      ACTION_NAME);
	g_return_val_if_fail (action != NULL, FALSE);

	if (show_install == TRUE)
	{
		g_free (window_data->last_clicked_url);
		window_data->last_clicked_url = g_strdup (url);
	}

	gtk_action_set_visible (action, show_install);

	return FALSE;
}

static void
content_loaded_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphyGreasemonkeyExtension *extension)
{
	GSList *l;
	char *location;
	GreasemonkeyScript *script;
	char *script_str;

	location = ephy_embed_get_location (embed, FALSE);
	if (location == NULL)
	{
		return;
	}

	for (l = extension->priv->scripts; l; l = l->next)
	{
		script = (GreasemonkeyScript *) l->data;

		if (greasemonkey_script_applies_to_url (script, location))
		{
			g_object_get (script, "script", &script_str, NULL);
			mozilla_evaluate_js (event, script_str);
			g_free (script_str);
		}
	}

	g_free (location);
}

static void
free_window_data (WindowData *data)
{
	g_object_unref (data->action_group);
	g_free (data->last_clicked_url);
	g_free (data);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint ui_id;
	WindowData *data;

	LOG ("EphyGreasemonkeyExtension attach_window");

	data = g_new0 (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data->action_group = action_group =
		gtk_action_group_new ("EphyGreasemonkeyExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "GreasemonkeySep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       ACTION_NAME, ACTION_NAME,
			       GTK_UI_MANAGER_MENUITEM, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyLinkPopup",
			       "GreasemonkeySep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       "GreasemonkeySep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       ACTION_NAME, ACTION_NAME,
			       GTK_UI_MANAGER_MENUITEM, TRUE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyImageLinkPopup",
			       "GreasemonkeySep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	LOG ("EphyGreasemonkeyExtension detach_window");

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("impl_attach_tab");

	g_return_if_fail (EPHY_IS_TAB (tab));

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), ext);
	g_signal_connect (embed, "dom_content_loaded",
			  G_CALLBACK (content_loaded_cb), ext);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("impl_detach_tab");

	g_return_if_fail (EPHY_IS_TAB (tab));

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), ext);
	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (content_loaded_cb), ext);
}

static void
ephy_greasemonkey_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_greasemonkey_extension_class_init (EphyGreasemonkeyExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_greasemonkey_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyGreasemonkeyExtensionPrivate));
}

GType
ephy_greasemonkey_extension_get_type (void)
{
	return type;
}

GType
ephy_greasemonkey_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyGreasemonkeyExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_greasemonkey_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyGreasemonkeyExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_greasemonkey_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_greasemonkey_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyGreasemonkeyExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
