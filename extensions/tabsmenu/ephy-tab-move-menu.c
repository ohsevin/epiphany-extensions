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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-tab-move-menu.h"
#include "ephy-string.h"
#include "ephy-debug.h"

#include <epiphany/ephy-session.h>
#include <epiphany/ephy-shell.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkuimanager.h>
#include <glib/gi18n-lib.h>
#include <string.h>

#define EPHY_TAB_MOVE_MENU_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TAB_MOVE_MENU, EphyTabMoveMenuPrivate))

struct _EphyTabMoveMenuPrivate
{
	EphyWindow *window;
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint merge_id;
};

#define MENU_PATH	"/menubar/TabsMenu/TabsOpen"
#define SUBMENU_PATH	"/menubar/TabsMenu/TabsOpen/TabMoveToMenu"
#define VERB_FMT	"MoveTo%p"
#define VERB_FMT_SIZE	(sizeof (VERB_FMT) + 18)
#define MAX_LENGTH	32
#define WINDOW_KEY	"dest-window"

static void ephy_tab_move_menu_class_init (EphyTabMoveMenuClass *klass);
static void ephy_tab_move_menu_init	  (EphyTabMoveMenu *menu);

enum
{
	PROP_0,
	PROP_WINDOW
};

GObjectClass *tab_move_menu_parent_class = NULL;

GType tab_move_menu_type = 0;

GType
ephy_tab_move_menu_get_type (void)
{
	return tab_move_menu_type;
}

GType
ephy_tab_move_menu_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
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

	tab_move_menu_type =
		g_type_module_register_type (module,
					     G_TYPE_OBJECT,
					     "EphyTabMoveMenu",
					     &our_info, 0);

	return tab_move_menu_type;
}

static int
find_name (GtkActionGroup *action_group, const char *name)
{
	return strcmp (gtk_action_group_get_name (action_group), name);
}

static GtkActionGroup *
find_action_group (GtkUIManager *manager)
{
	GList *list, *element;

	list = gtk_ui_manager_get_action_groups (manager);
	element = g_list_find_custom (list, "WindowActions", (GCompareFunc) find_name);
	g_return_if_fail (element != NULL);
	g_return_if_fail (element->data != NULL);

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
	EphyWindow *src_win = menu->priv->window;
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
add_action_and_menu_item (EphyWindow *window, EphyTabMoveMenu *menu)
{
	EphyTab *tab;
	GtkAction *action;
	GtkWidget *notebook;
	guint num;
	const char *text;
	char *title, *win_title_doubled, *win_title;
	char verb[VERB_FMT_SIZE], name[VERB_FMT_SIZE + 4];

	/* the list may also contain the bookmarks and history window */
	if (!EPHY_IS_WINDOW (window)) return;

	LOG ("add_action_and_menu_item for window %p", window)

	g_snprintf (verb, sizeof (verb), VERB_FMT, window);
	g_snprintf (name, sizeof (name), "%sItem", verb);

	notebook = ephy_window_get_notebook (window);
	num = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (EPHY_IS_TAB (tab));

	win_title = ephy_string_shorten (ephy_tab_get_title (tab), MAX_LENGTH);
	win_title_doubled = ephy_string_double_underscores (win_title);

	text = dngettext (GETTEXT_PACKAGE, "Window '%s' (%d tab)",
					   "Window '%s' (%d tabs)", num);
	
	title = g_strdup_printf (text, win_title_doubled, num);

	action = g_object_new (GTK_TYPE_ACTION,
			       "name", verb,
			       "label", title,
			       "sensitive", window != menu->priv->window,
			       NULL);
	g_signal_connect (action, "activate", G_CALLBACK (move_cb), menu);
	g_object_set_data (G_OBJECT (action), WINDOW_KEY, window);
	gtk_action_group_add_action (menu->priv->action_group, action);
	g_object_unref (action);

	gtk_ui_manager_add_ui (menu->priv->manager, menu->priv->merge_id,
			       SUBMENU_PATH,
			       name, verb,
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	g_free (win_title);
	g_free (win_title_doubled);
	g_free (title);
}

static void
update_tab_move_menu_cb (GtkAction *dummy,
			 EphyTabMoveMenu *menu)
{
	EphyTabMoveMenuPrivate *p = menu->priv;
	EphySession *session;
	GtkAction *action;
	GList *windows, *l;

	LOG ("update_tab_move_menu_cb")

	START_PROFILER ("Rebuilding tab move menu")

	/* clear the menu */
	if (p->merge_id != 0)
	{
		gtk_ui_manager_remove_ui (p->manager, p->merge_id);
		gtk_ui_manager_ensure_update (p->manager);
	}

	if (p->action_group != NULL)
	{
		gtk_ui_manager_remove_action_group (p->manager, p->action_group);
		g_object_unref (p->action_group);
	}

	/* build the new menu */
	p->action_group = gtk_action_group_new ("TabMoveToActions");
	gtk_ui_manager_insert_action_group (p->manager, p->action_group, 0);

	p->merge_id = gtk_ui_manager_new_merge_id (p->manager);

	gtk_ui_manager_add_ui (p->manager, p->merge_id,
			       MENU_PATH,
			       "TabMoveToMenu",
			       "TabMoveTo",
			       GTK_UI_MANAGER_MENU, TRUE);

	gtk_ui_manager_add_ui (p->manager, p->merge_id,
			       MENU_PATH,
			       "TabMoveToSep1Item", "TabMoveToSep1",
			       GTK_UI_MANAGER_SEPARATOR, TRUE);

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	g_return_if_fail (EPHY_IS_SESSION (session));

	windows = ephy_session_get_windows (session);

	g_list_foreach (windows, (GFunc) add_action_and_menu_item, menu);

	action = gtk_ui_manager_get_action (p->manager, SUBMENU_PATH);
	g_object_set (G_OBJECT (action), "sensitive", g_list_length (windows) > 1, NULL);

	g_list_free (windows);

	STOP_PROFILER ("Rebuilding tab move menu")
}

static void
ephy_tab_move_menu_set_window (EphyTabMoveMenu *menu,
			       EphyWindow *window)
{
	GtkAction *action;
	GtkActionGroup *action_group;

	g_return_if_fail (EPHY_IS_WINDOW (window));

	menu->priv->window = window;
	menu->priv->manager = GTK_UI_MANAGER (window->ui_merge);

	action_group = find_action_group (menu->priv->manager);

	action = g_object_new (GTK_TYPE_ACTION,
			       "name", "TabMoveTo",
			       "label", _("Move Tab To Window"),
			       "tooltip", _("Move the current tab to a different window"),
			       "hide_if_empty", FALSE,
			       NULL);
	gtk_action_group_add_action (action_group, action);
	g_object_unref (action);

	action = gtk_ui_manager_get_action (menu->priv->manager, "/menubar/TabsMenu");
	g_return_if_fail (action != NULL);

	g_signal_connect_object (action, "activate",
				 G_CALLBACK (update_tab_move_menu_cb), menu, 0);
}

static void
ephy_tab_move_menu_init (EphyTabMoveMenu *menu)
{
	menu->priv = EPHY_TAB_MOVE_MENU_GET_PRIVATE (menu);

	menu->priv->window = NULL;
	menu->priv->manager = NULL;
	menu->priv->action_group = NULL;
	menu->priv->merge_id = 0;
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
	EphyTabMoveMenu *menu = EPHY_TAB_MOVE_MENU (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, menu->priv->window);
			break;
	}
}

static void
ephy_tab_move_menu_finalize (GObject *object)
{
	EphyTabMoveMenu *menu = EPHY_TAB_MOVE_MENU (object); 

	GtkActionGroup *action_group;
	GtkAction *action;

/*
	action_group = find_action_group (menu->priv->manager);
	g_return_if_fail (action_group != NULL);

	action = gtk_action_group_get_action (action_group, "TabMoveTo");
	g_return_if_fail (action != NULL);

	gtk_action_group_remove_action (action_group, action);

	action = gtk_ui_manager_get_action (menu->priv->manager, "/menubar/TabsMenu");
	g_return_if_fail (action != NULL);

	g_signal_handlers_disconnect_by_func (action, G_CALLBACK (update_tab_move_menu_cb), menu);
*/
	if (menu->priv->merge_id != 0)
	{
		gtk_ui_manager_remove_ui (menu->priv->manager,
					  menu->priv->merge_id);
	}
	if (menu->priv->action_group != NULL)
	{
		g_object_unref (menu->priv->action_group);
	}

	G_OBJECT_CLASS (tab_move_menu_parent_class)->finalize (object);
}

static void
ephy_tab_move_menu_class_init (EphyTabMoveMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	tab_move_menu_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_tab_move_menu_finalize;
	object_class->set_property = ephy_tab_move_menu_set_property;
	object_class->get_property = ephy_tab_move_menu_get_property;

	g_object_class_install_property (object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							      "Window",
							      "Parent window",
							      EPHY_TYPE_WINDOW,
							      G_PARAM_READWRITE |
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
