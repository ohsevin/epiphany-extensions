/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
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

#include "ephy-tab-move-menu.h"

#include <epiphany/ephy-session.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-notebook.h>

#include "ephy-debug.h"

#include <gtk/gtkaction.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmenuitem.h>
#include <glib/gi18n-lib.h>
#include <string.h>

#define EPHY_TAB_MOVE_MENU_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TAB_MOVE_MENU, EphyTabMoveMenuPrivate))

struct _EphyTabMoveMenuPrivate
{
	EphyWindow *window;
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	GtkAction *menu_action;
	guint static_merge_id;
	guint dynamic_merge_id;
};

#define MENU_ACTION_NAME	"TabMoveTo"
#define MENU_PATH		"/menubar/TabsMenu/TabsMoveGroup"
#define CONTEXT_MENU_PATH	"/EphyNotebookPopup/TabsMoveGroupENP"
#define SUBMENU_PATH		"/menubar/TabsMenu/TabsMoveGroup/" MENU_ACTION_NAME "Menu"
#define CONTEXT_SUBMENU_PATH	"/EphyNotebookPopup/TabsMoveGroupENP/" MENU_ACTION_NAME "ENP"

#define VERB_FMT	"MoveTo%p"
#define VERB_FMT_SIZE	(sizeof (VERB_FMT) + 18)
#define WINDOW_KEY	"dest-window"
#define LABEL_WIDTH_CHARS	40

static void ephy_tab_move_menu_class_init (EphyTabMoveMenuClass *klass);
static void ephy_tab_move_menu_init	  (EphyTabMoveMenu *menu);

enum
{
	PROP_0,
	PROP_WINDOW
};

GObjectClass *parent_class = NULL;

GType type = 0;

GType
ephy_tab_move_menu_get_type (void)
{
	return type;
}

GType
ephy_tab_move_menu_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyTabMoveMenuClass),
		NULL,
		NULL,
		(GClassInitFunc) ephy_tab_move_menu_class_init,
		NULL,
		NULL,
		sizeof (EphyTabMoveMenu),
		0,
		(GInstanceInitFunc) ephy_tab_move_menu_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabMoveMenu",
					    &our_info, 0);

	return type;
}

static int
find_name (GtkActionGroup *action_group,
	   const char *name)
{
	return strcmp (gtk_action_group_get_name (action_group), name);
}

static GtkActionGroup *
find_action_group (GtkUIManager *manager)
{
	GList *list, *element;

	list = gtk_ui_manager_get_action_groups (manager);
	element = g_list_find_custom (list, "WindowActions", (GCompareFunc) find_name);
	g_return_val_if_fail (element != NULL, NULL);
	g_return_val_if_fail (element->data != NULL, NULL);

	return GTK_ACTION_GROUP (element->data);
}

static EphyWindow *
get_window_from_action (GtkAction *action)
{
	EphyWindow *window;

	window = EPHY_WINDOW (g_object_get_data (G_OBJECT (action), WINDOW_KEY));
	g_return_val_if_fail (EPHY_IS_WINDOW (window), NULL);

	return window;
}

static void
move_cb (GtkAction *action,
	 EphyTabMoveMenu *menu)
{
	EphyTabMoveMenuPrivate *priv = menu->priv;
	EphyWindow *src_win = priv->window;
	EphyWindow *dest_win;
	EphyTab *tab;
	GtkWidget *src_nb, *dest_nb;

	dest_win = get_window_from_action (action);
	g_return_if_fail (dest_win != NULL);

	g_return_if_fail (src_win != dest_win);

	tab = ephy_window_get_active_tab (src_win);
	g_return_if_fail (EPHY_IS_TAB (tab));

	src_nb = ephy_window_get_notebook (src_win);
	dest_nb = ephy_window_get_notebook (dest_win);

	ephy_notebook_move_tab (EPHY_NOTEBOOK (src_nb), EPHY_NOTEBOOK (dest_nb),
				tab, -1);

	ephy_window_jump_to_tab (dest_win, tab);
}

static void
add_action_and_menu_item (EphyWindow *window,
			  EphyTabMoveMenu *menu)
{
	EphyTabMoveMenuPrivate *priv = menu->priv;
	EphyTab *tab;
	GtkAction *action;
	GtkWidget *notebook;
	guint num;
	const char *text;
	char *title;
	char verb[VERB_FMT_SIZE], name[VERB_FMT_SIZE + 4];

	/* the list may also contain the bookmarks and history window */
	if (!EPHY_IS_WINDOW (window)) return;

	LOG ("add_action_and_menu_item for window %p", window);

	g_snprintf (verb, sizeof (verb), VERB_FMT, window);
	g_snprintf (name, sizeof (name), "%sItem", verb);

	notebook = ephy_window_get_notebook (window);
	num = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (EPHY_IS_TAB (tab));

	text = dngettext (GETTEXT_PACKAGE, "Window “%s” (%d tab)",
					   "Window “%s” (%d tabs)", num);
	title = g_strdup_printf (text, ephy_tab_get_title (tab), num);

	action = g_object_new (GTK_TYPE_ACTION,
			       "name", verb,
			       "label", title,
			       "sensitive", window != priv->window,
			       NULL);
	g_signal_connect (action, "activate", G_CALLBACK (move_cb), menu);
	g_object_set_data (G_OBJECT (action), WINDOW_KEY, window);
	gtk_action_group_add_action (priv->action_group, action);
	g_object_unref (action);

	gtk_ui_manager_add_ui (priv->manager, priv->dynamic_merge_id,
			       SUBMENU_PATH,
			       name, verb,
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	gtk_ui_manager_add_ui (priv->manager, priv->dynamic_merge_id,
			       CONTEXT_SUBMENU_PATH,
			       name, verb,
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	g_free (title);
}

static void
connect_proxy_cb (GtkActionGroup *action_group,
                  GtkAction *action,
                  GtkWidget *proxy)
{
	if (GTK_IS_MENU_ITEM (proxy))
	{
		GtkLabel *label;
	
		label = (GtkLabel *) ((GtkBin *) proxy)->child;

		gtk_label_set_use_underline (label, FALSE);
		gtk_label_set_ellipsize (label, PANGO_ELLIPSIZE_MIDDLE);
		gtk_label_set_max_width_chars (label, LABEL_WIDTH_CHARS);
	}
}

static void
update_tab_move_menu_cb (GtkAction *dummy,
			 EphyTabMoveMenu *menu)
{
	EphyTabMoveMenuPrivate *priv = menu->priv;
	EphySession *session;
	GList *windows;

	LOG ("update_tab_move_menu_cb");

	START_PROFILER ("Rebuilding tab move menu")

	/* clear the menu */
	if (priv->dynamic_merge_id != 0)
	{
		gtk_ui_manager_remove_ui (priv->manager, priv->dynamic_merge_id);
		gtk_ui_manager_ensure_update (priv->manager);
	}

	if (priv->action_group != NULL)
	{
		gtk_ui_manager_remove_action_group (priv->manager, priv->action_group);
		g_object_unref (priv->action_group);
	}

	/* build the new menu */
	priv->action_group = gtk_action_group_new ("TabMoveToActions");
	g_signal_connect (priv->action_group, "connect-proxy",
			  G_CALLBACK (connect_proxy_cb), NULL);
	gtk_ui_manager_insert_action_group (priv->manager, priv->action_group, 0);

	priv->dynamic_merge_id = gtk_ui_manager_new_merge_id (priv->manager);

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	g_return_if_fail (EPHY_IS_SESSION (session));

	windows = ephy_session_get_windows (session);

	g_list_foreach (windows, (GFunc) add_action_and_menu_item, menu);

	g_object_set (priv->menu_action, "sensitive", g_list_length (windows) > 1, NULL);

	g_list_free (windows);

	STOP_PROFILER ("Rebuilding tab move menu")
}

static void
ephy_tab_move_menu_set_window (EphyTabMoveMenu *menu,
			       EphyWindow *window)
{
	EphyTabMoveMenuPrivate *priv = menu->priv;
	GtkActionGroup *action_group;
	GtkAction *action;

	LOG ("set_window window %p", window);

	g_return_if_fail (EPHY_IS_WINDOW (window));

	priv->window = window;
	priv->manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	action_group = find_action_group (priv->manager);

	priv->menu_action =
		 g_object_new (GTK_TYPE_ACTION,
			       "name", MENU_ACTION_NAME,
			       "label", _("Move Tab To Window"),
			       "tooltip", _("Move the current tab to a different window"),
			       "hide_if_empty", FALSE,
			       NULL);
	gtk_action_group_add_action (action_group, priv->menu_action);
	g_object_unref (priv->menu_action);

	action = gtk_ui_manager_get_action (priv->manager, "/menubar/TabsMenu");
	g_return_if_fail (action != NULL);
	g_signal_connect_object (action, "activate",
				 G_CALLBACK (update_tab_move_menu_cb), menu, 0);

	action = gtk_ui_manager_get_action (priv->manager, "/EphyNotebookPopup");
	g_return_if_fail (action != NULL);
	g_signal_connect_object (action, "activate",
				 G_CALLBACK (update_tab_move_menu_cb), menu, 0);

	priv->static_merge_id = gtk_ui_manager_new_merge_id (priv->manager);

	gtk_ui_manager_add_ui (priv->manager, priv->static_merge_id,
			       MENU_PATH,
			       MENU_ACTION_NAME "Menu",
			       MENU_ACTION_NAME,
			       GTK_UI_MANAGER_MENU, FALSE);

	gtk_ui_manager_add_ui (priv->manager, priv->static_merge_id,
			       CONTEXT_MENU_PATH,
			       MENU_ACTION_NAME "ENP",
			       MENU_ACTION_NAME,
			       GTK_UI_MANAGER_MENU, FALSE);
}

static void
ephy_tab_move_menu_init (EphyTabMoveMenu *menu)
{
	EphyTabMoveMenuPrivate *priv;

	priv = menu->priv = EPHY_TAB_MOVE_MENU_GET_PRIVATE (menu);

	priv->window = NULL;
	priv->manager = NULL;
	priv->action_group = NULL;
	priv->static_merge_id = 0;
	priv->dynamic_merge_id = 0;
}

static void
ephy_tab_move_menu_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	EphyTabMoveMenu *menu = EPHY_TAB_MOVE_MENU (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_tab_move_menu_set_window (menu, g_value_get_object (value));
			break;
	}
}

static void
ephy_tab_move_menu_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
ephy_tab_move_menu_finalize (GObject *object)
{
	EphyTabMoveMenu *menu = EPHY_TAB_MOVE_MENU (object);
	EphyTabMoveMenuPrivate *priv = menu->priv;
	GtkActionGroup *action_group = NULL;
	GtkAction *action;

	if (priv->dynamic_merge_id != 0)
	{
		gtk_ui_manager_remove_ui (priv->manager,
					  priv->dynamic_merge_id);
	}
	if (priv->static_merge_id != 0)
	{
		gtk_ui_manager_remove_ui (priv->manager,
					  priv->static_merge_id);
	}
	if (priv->action_group != NULL)
	{
		g_object_unref (priv->action_group);
	}

	gtk_ui_manager_ensure_update (priv->manager);

	g_object_get (G_OBJECT (priv->menu_action), "action-group", &action_group, NULL);
	g_return_if_fail (action_group != NULL);

	gtk_action_group_remove_action (action_group, priv->menu_action);

	action = gtk_ui_manager_get_action (priv->manager, "/menubar/TabsMenu");
	g_return_if_fail (action != NULL);

	g_signal_handlers_disconnect_by_func (action, G_CALLBACK (update_tab_move_menu_cb), menu);

	action = gtk_ui_manager_get_action (priv->manager, "/EphyNotebookPopup");
	g_return_if_fail (action != NULL);

	g_signal_handlers_disconnect_by_func (action, G_CALLBACK (update_tab_move_menu_cb), menu);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_tab_move_menu_class_init (EphyTabMoveMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_tab_move_menu_finalize;
	object_class->set_property = ephy_tab_move_menu_set_property;
	object_class->get_property = ephy_tab_move_menu_get_property;

	g_object_class_install_property (object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							      "Window",
							      "Parent window",
							      EPHY_TYPE_WINDOW,
							      G_PARAM_WRITABLE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (EphyTabMoveMenuPrivate));
}

EphyTabMoveMenu *
ephy_tab_move_menu_new (EphyWindow *window)
{
	return g_object_new (EPHY_TYPE_TAB_MOVE_MENU,
			     "window", window,
			     NULL);
}
