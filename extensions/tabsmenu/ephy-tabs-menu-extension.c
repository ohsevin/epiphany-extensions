/*
 *  Copyright (C) 2003  David Bordoley
 *  Copyright (C) 2003  Christian Persch
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

#include "ephy-tabs-menu-extension.h"
#include "ephy-window-action.h"
#include "ephy-debug.h"

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#include <epiphany/ephy-extension.h>

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-notebook.h>
#include <epiphany/ephy-session.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-tab.h>

#include <glib/gi18n-lib.h>

#define EPHY_TABS_MENU_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TABS_MENU_EXTENSION, EphyTabsMenuExtensionPrivate))

struct _EphyTabsMenuExtensionPrivate
{
	GHashTable *infos;
	guint ui_update_timeout;
};

typedef struct
{
	EphyWindow *window;
	GtkActionGroup *action_group;
	guint ui_id;
} WindowInfo;

#define VERB_STRING		"ETME:Window=%p"

static GtkActionEntry action_entries [] =
{
/*
	{ "EphyTabsPluginClone", NULL, N_("_Clone Tab"), "<shift><control>C",
	  N_("Create a copy of the current tab"),
	  G_CALLBACK (clone_cb)
	},
*/
	{ "TabsMoveTo", NULL, N_("_Move Tab To"), NULL,
	  N_("Move the current tab to a different window"),
	  NULL
	}
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static GObjectClass *parent_class = NULL;

static void ephy_tabs_menu_extension_class_init	(EphyTabsMenuExtensionClass *class);
static void ephy_tabs_menu_extension_iface_init (EphyExtensionClass *iface);
static void ephy_tabs_menu_extension_init	(EphyTabsMenuExtension *menu);

static GType type = 0;

GType
ephy_tabs_menu_extension_get_type (void)
{
	return type;
}

GType
ephy_tabs_menu_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyTabsMenuExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tabs_menu_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTab),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_tabs_menu_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tabs_menu_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabsMenuExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
move_to_window_cb (GtkAction *gaction,
		   EphyWindow *src_win)
{
	EphyWindowAction *action = EPHY_WINDOW_ACTION (gaction);
	EphyWindow *dest_win;
	EphyTab *tab;
	EphyEmbed *embed;
	GtkWidget *src_nb, *dest_nb;

	g_return_if_fail (EPHY_IS_WINDOW_ACTION (action));

	dest_win = ephy_window_action_get_window (action);
	g_return_if_fail (dest_win != NULL);

	g_return_if_fail (src_win != dest_win);

	tab = ephy_window_get_active_tab (src_win);
	g_return_if_fail (EPHY_IS_TAB (tab));
	embed = ephy_tab_get_embed (tab);

	src_nb = ephy_window_get_notebook (src_win);
	dest_nb = ephy_window_get_notebook (dest_win);

	ephy_notebook_move_page (EPHY_NOTEBOOK (src_nb), EPHY_NOTEBOOK (dest_nb),
				 GTK_WIDGET (embed), -1);

	ephy_window_jump_to_tab (dest_win, tab);
}

static void
add_action_for_window (EphyWindow *window,
		       WindowInfo *info)
{
	GtkAction *action;
	char verb[64];

	g_snprintf (verb, sizeof (verb), VERB_STRING, window);

	action = GTK_ACTION (g_object_new (EPHY_TYPE_WINDOW_ACTION,
					   "name", verb,
					   "window", window,
					   NULL));

	g_signal_connect (action, "activate",
			  G_CALLBACK (move_to_window_cb), info->window);

	if (window == info->window)
	{
		g_object_set (action, "sensitive", FALSE, NULL);
	}

	gtk_action_group_add_action (info->action_group, action);

	g_object_unref (action);
}

static void
add_action_to_window (EphyWindow *target,
		      WindowInfo *info,
		      EphyWindow *window)
{
	add_action_for_window (window, info);
}

static void
add_ui_for_window (EphyWindow *window,
		   WindowInfo *info)
{
	char name[64];

	g_snprintf (name, sizeof (name), VERB_STRING, window);

	gtk_ui_manager_add_ui (GTK_UI_MANAGER (info->window->ui_merge),
			       info->ui_id,
			       "/menubar/TabsMenu/TabsOpen/TabsMoveToMenu",
			       name,
			       name,
			       GTK_UI_MANAGER_MENUITEM,
			       FALSE);
}

static void
update_ui (EphyWindow *window,
	   WindowInfo *info,
	   GList *windows)
{
	GtkUIManager *manager = GTK_UI_MANAGER (window->ui_merge);

	LOG ("EphyTabsMenuExtension updating UI")

	if (info->ui_id != 0)
	{
		gtk_ui_manager_remove_ui (manager, info->ui_id);
		// FIXME do we need this?
		// gtk_ui_manager_ensure_update (manager);
		info->ui_id = 0;
	}

	info->ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager,
			       info->ui_id,
			       "/menubar/TabsMenu/TabsOpen",
			       "TabsMoveToMenu",
			       "TabsMoveTo",
			       GTK_UI_MANAGER_MENU,
			       TRUE);

	gtk_ui_manager_add_ui (manager,
			       info->ui_id,
			       "/menubar/TabsMenu/TabsOpen",
			       "EphyTabsMenuExtensionSep",
			       NULL,
			       GTK_UI_MANAGER_SEPARATOR,
			       TRUE);

	g_list_foreach (windows, (GFunc) add_ui_for_window, info);

}

static gboolean
ui_update_cb (EphyTabsMenuExtension *extension)
{
	EphySession *session;
	GList *windows;

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	windows = ephy_session_get_windows (session);

	START_PROFILER ("Rebuilding all tabs menu extension menus")

	g_hash_table_foreach (extension->priv->infos, (GHFunc) update_ui, windows);

	STOP_PROFILER ("Rebuilding all tabs menu extension menus")

	g_list_free (windows);

	extension->priv->ui_update_timeout = 0;

	/* don't run again */
	return FALSE;
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyTabsMenuExtension *extension = EPHY_TABS_MENU_EXTENSION (ext);
	EphySession *session;
	WindowInfo *info;
	GList *list;

	info = g_hash_table_lookup (extension->priv->infos, window);
	g_return_if_fail (info == NULL);

	info = g_new0 (WindowInfo, 1);
	info->window = window;
	info->ui_id = 0;
	info->action_group = gtk_action_group_new ("EphyTabsMenuExtensionActions");

	gtk_action_group_set_translation_domain (info->action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (info->action_group,
				      action_entries, n_action_entries,
				      window);

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	list = ephy_session_get_windows (session);
	/* remove it [it may or may not be present already] */
	list = g_list_remove (list, window);
	g_list_foreach (list, (GFunc) add_action_for_window, info);
	g_list_free (list);

	/* and finally add the info to the hash table */
	g_hash_table_insert (extension->priv->infos, window, info);

	/* Now inform all windows of the new window */
	g_hash_table_foreach (extension->priv->infos, (GHFunc) add_action_to_window, window);

	gtk_ui_manager_insert_action_group (GTK_UI_MANAGER (window->ui_merge),
					    info->action_group, 0);

	g_object_unref (info->action_group);

	/* schedule UI updates */
	if (extension->priv->ui_update_timeout == 0)
	{
		extension->priv->ui_update_timeout =
			g_idle_add ((GSourceFunc) ui_update_cb, extension);
	}
}

static void
remove_from_window (EphyWindow *window, 
		    WindowInfo *info,
		    gpointer dummy)
{
	gtk_ui_manager_remove_action_group (GTK_UI_MANAGER (window->ui_merge),
					    info->action_group);
}

static void
remove_action_from_window (EphyWindow *window,
			   WindowInfo *info,
			   EphyWindow *target)
{
	GtkAction *action;
	char name[64];

	g_snprintf (name, sizeof (name), VERB_STRING, target);

	action = gtk_action_group_get_action (info->action_group, name);

	if (action != NULL)
	{
		gtk_action_group_remove_action (info->action_group, action);
	}
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyTabsMenuExtension *extension = EPHY_TABS_MENU_EXTENSION (ext);
	WindowInfo *info;

	info = g_hash_table_lookup (extension->priv->infos, window);
	g_return_if_fail (info != NULL);

	remove_from_window (window, info, NULL);

	g_hash_table_remove (extension->priv->infos, window);

	g_hash_table_foreach (extension->priv->infos,
			      (GHFunc) remove_action_from_window,
			      window);
}

static void
ephy_tabs_menu_extension_init (EphyTabsMenuExtension *extension)
{
	extension->priv = EPHY_TABS_MENU_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyTabsMenuExtension initialising")

	extension->priv->ui_update_timeout = 0;
	extension->priv->infos = g_hash_table_new_full (g_direct_hash,
							g_direct_equal,
							NULL,
							(GDestroyNotify) g_free);
}

static void
ephy_tabs_menu_extension_finalize (GObject *object)
{
	EphyTabsMenuExtension *extension = EPHY_TABS_MENU_EXTENSION (object);

	LOG ("EphyTabsMenuExtension finalising")

	g_hash_table_foreach (extension->priv->infos, (GHFunc) remove_from_window, NULL);
	g_hash_table_destroy (extension->priv->infos);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_tabs_menu_extension_iface_init (EphyExtensionClass *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_tabs_menu_extension_class_init (EphyTabsMenuExtensionClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	parent_class  = (GObjectClass *) g_type_class_peek_parent (class);

	object_class->finalize = ephy_tabs_menu_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyTabsMenuExtensionPrivate));
}
