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

#include "ephy-tabs-plugin-menu.h"
#include "ephy-window-action.h"
#include "ephy-debug.h"

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-notebook.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>

#include <string.h>
#include <stdlib.h>
#include <libxml/entities.h>
#include <glib/gi18n-lib.h>

#define EPHY_TABS_PLUGIN_MENU_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TABS_PLUGIN_MENU, EphyTabsPluginMenuPrivate))

struct _EphyTabsPluginMenuPrivate
{
	EphyWindow *window;
	GtkActionGroup *action_group;
	guint ui_id;
};

/**
 * Private functions, only availble from this file
 */
static void	ephy_tabs_plugin_menu_class_init	(EphyTabsPluginMenuClass *klass);
static void	ephy_tabs_plugin_menu_init	  	(EphyTabsPluginMenu *menu);
static void	ephy_tabs_plugin_menu_finalize		(GObject *o);
static void	ephy_tabs_plugin_menu_set_window	(EphyTabsPluginMenu *menu,
							 EphyWindow *window);

enum
{
	PROP_0,
	PROP_WINDOW
};

static GObjectClass *parent_class = NULL;

#define VERB_STRING		"ETP-Window-%p"
#define MAX_LABEL_LENGTH	32

/**
 * EphyTabsPluginMenu object
 */

GType
ephy_tabs_plugin_menu_get_type (void)
{
        static GType ephy_tabs_plugin_menu_type = 0;

        if (ephy_tabs_plugin_menu_type == 0)
        {
                static const GTypeInfo our_info =
                {
                        sizeof (EphyTabsPluginMenuClass),
                        NULL, /* base_init */
                        NULL, /* base_finalize */
                        (GClassInitFunc) ephy_tabs_plugin_menu_class_init,
                        NULL,
                        NULL, /* class_data */
                        sizeof (EphyTab),
                        0, /* n_preallocs */
                        (GInstanceInitFunc) ephy_tabs_plugin_menu_init
                };

                ephy_tabs_plugin_menu_type = g_type_register_static (G_TYPE_OBJECT,
								     "EphyTabsPluginMenu",
								     &our_info, 0);
        }

        return ephy_tabs_plugin_menu_type;
}

static void
ephy_tabs_plugin_menu_set_property (GObject *object,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *pspec)
{
        EphyTabsPluginMenu *menu = EPHY_TABS_PLUGIN_MENU (object);

        switch (prop_id)
        {
                case PROP_WINDOW:
                        ephy_tabs_plugin_menu_set_window (menu, g_value_get_object (value));
                        break;
        }
}

static void
ephy_tabs_plugin_menu_get_property (GObject *object,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *pspec)
{
        EphyTabsPluginMenu *menu = EPHY_TABS_PLUGIN_MENU (object);

        switch (prop_id)
        {
                case PROP_WINDOW:
                        g_value_set_object (value, menu->priv->window);
                        break;
        }
}

static void
ephy_tabs_plugin_menu_class_init (EphyTabsPluginMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class  = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_tabs_plugin_menu_finalize;
	object_class->set_property = ephy_tabs_plugin_menu_set_property;
	object_class->get_property = ephy_tabs_plugin_menu_get_property;

	g_object_class_install_property (object_class,
                                         PROP_WINDOW,
                                         g_param_spec_object ("window",
                                                              "Window",
                                                              "Parent window",
                                                              EPHY_TYPE_WINDOW,
                                                              G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (EphyTabsPluginMenuPrivate));
}

static void
move_to_window_cb (GtkAction *a, EphyTabsPluginMenu *menu)
{
	EphyWindowAction *action = EPHY_WINDOW_ACTION (a);
	EphyWindow *src_win, *dest_win;
	EphyTab *tab;
	EphyEmbed *embed;
	GtkWidget *src_nb, *dest_nb;

	g_return_if_fail (EPHY_IS_WINDOW_ACTION (action));

	src_win = menu->priv->window;
	g_return_if_fail (src_win != NULL);

	dest_win = ephy_window_action_get_window (action);
	g_return_if_fail (dest_win != NULL);

	if (src_win == dest_win) return;

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
add_action_for_window (EphyWindow *window, EphyTabsPluginMenu *menu)
{
	GtkAction *action;
	char *verb;

	verb = g_strdup_printf (VERB_STRING, window);

	action = GTK_ACTION (g_object_new (EPHY_TYPE_WINDOW_ACTION,
					   "name", verb,
					   "window", window,
					   NULL));

	g_signal_connect (action, "activate",
			  G_CALLBACK (move_to_window_cb), menu);

	gtk_action_group_add_action (menu->priv->action_group, action);

	g_object_unref (action);
	g_free (verb);
}

static void
window_opened_cb (Session *session, EphyWindow *window, EphyTabsPluginMenu *menu)
{
	LOG ("window_opened_cb menu %p window %p", menu, window)

	add_action_for_window (window, menu);

	ephy_tabs_plugin_menu_update (menu);
}

static void
remove_action_for_window (EphyWindow *window, EphyTabsPluginMenu *menu)
{
	GtkAction *action;
	char *verb;

	if (menu->priv->window == NULL) return;

	verb = g_strdup_printf (VERB_STRING, window);

	action = gtk_action_group_get_action (menu->priv->action_group, verb);

	if (action != NULL)
	{
		gtk_action_group_remove_action (menu->priv->action_group, action);
	}
	else
	{
		g_warning ("Action to remove not found (window=%p)", window);
	}

	g_free (verb);
}
	
static void
window_closed_cb (Session *session, EphyWindow *window, EphyTabsPluginMenu *menu)
{
	LOG ("window_closed_cb menu %p", menu)

	/* we're closing, no need to update anything */
	if (window == menu->priv->window) return;

	remove_action_for_window (window, menu);

	ephy_tabs_plugin_menu_update (menu);
}

/*
static void
clone_cb (GtkAction *action, EphyTabsPluginMenu *menu)
{
	EphyWindow *window;
	EphyTab *tab, *new_tab;
	EphyEmbed *embed, *new_embed;

	window = menu->priv->window;
	g_return_if_fail (window != NULL);

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (EPHY_IS_TAB (tab));

	new_tab = ephy_shell_new_tab (ephy_shell, window, tab, NULL,
				      EPHY_NEW_TAB_IN_EXISTING_WINDOW |
				      EPHY_NEW_TAB_APPEND_AFTER |
				      EPHY_NEW_TAB_CLONE_PAGE);
}
*/

static void
ephy_tabs_plugin_menu_init (EphyTabsPluginMenu *menu)
{
	EphyTabsPluginMenuPrivate *p;
	Session *session;
	const GList *wins;

	p = EPHY_TABS_PLUGIN_MENU_GET_PRIVATE (menu);
	menu->priv = p;

	LOG ("EphyTabsPluginMenu initialising %p", menu)	

	p->ui_id = 0;

	p->action_group = gtk_action_group_new ("EphyTabsPluginActions");
	gtk_action_group_set_translation_domain (p->action_group, GETTEXT_PACKAGE);

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));

	wins = session_get_windows (session);
	g_list_foreach ((GList *) wins, (GFunc) add_action_for_window, menu);

	g_signal_connect (session, "new_window",
			  G_CALLBACK (window_opened_cb), menu);
	g_signal_connect (session, "close_window",
			  G_CALLBACK (window_closed_cb), menu);
}

static void
ephy_tabs_plugin_menu_finalize (GObject *o)
{
	EphyTabsPluginMenu *menu;
	EphyTabsPluginMenuPrivate *p;
	EphyWindow *window;
	Session *session;

	menu = EPHY_TABS_PLUGIN_MENU (o);
	g_return_if_fail (menu->priv != NULL);

	p = menu->priv;
	window = p->window;

	if (p->action_group != NULL)
	{
		if (window != NULL)
		{
			gtk_ui_manager_remove_action_group
				(GTK_UI_MANAGER (window->ui_merge),
				 p->action_group);
		}

		g_object_unref (p->action_group);
	}

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_handlers_disconnect_by_func (G_OBJECT (session),
					      G_CALLBACK (window_opened_cb), menu);
	g_signal_handlers_disconnect_by_func (G_OBJECT (session),
					      G_CALLBACK (window_closed_cb), menu);

	if (window != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (window),
					      (gpointer *) &menu->priv->window);
	}

	LOG ("EphyTabsPluginMenu finalised %p", o)

	parent_class->finalize (o);
}

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

static void
ephy_tabs_plugin_menu_set_window (EphyTabsPluginMenu *menu, EphyWindow *window)
{
	GtkAction *action;
	char *verb;

	g_return_if_fail (EPHY_IS_TABS_PLUGIN_MENU (menu));
	g_return_if_fail (EPHY_IS_WINDOW (window));

	menu->priv->window = window;

	g_object_add_weak_pointer (G_OBJECT (window),
				   (gpointer *) &menu->priv->window);

	gtk_action_group_add_actions (menu->priv->action_group,
				      action_entries, n_action_entries,
				      window);

	/* you can't move a tab to the window it's already in */
	verb = g_strdup_printf (VERB_STRING, window);
	action = gtk_action_group_get_action (menu->priv->action_group, verb);
	g_object_set (action, "sensitive", FALSE, NULL);
	g_free (verb);

	gtk_ui_manager_insert_action_group (GTK_UI_MANAGER (window->ui_merge),
					    menu->priv->action_group, 0);

	ephy_tabs_plugin_menu_update (menu);
}

static void
ephy_tabs_plugin_menu_clean (EphyTabsPluginMenu *menu)
{
	GtkUIManager *manager = GTK_UI_MANAGER (menu->priv->window->ui_merge);

	if (menu->priv->ui_id != 0)
	{
		gtk_ui_manager_remove_ui (manager, menu->priv->ui_id);
		gtk_ui_manager_ensure_update (manager);
		menu->priv->ui_id = 0;
	}
}

EphyTabsPluginMenu *
ephy_tabs_plugin_menu_new (EphyWindow *window)
{
	return EPHY_TABS_PLUGIN_MENU (g_object_new (EPHY_TYPE_TABS_PLUGIN_MENU,
						    "window", window,
						    NULL));
}

void
ephy_tabs_plugin_menu_update (EphyTabsPluginMenu *menu)
{
	EphyTabsPluginMenuPrivate *p;
	GtkUIManager *manager;
	GtkAction *action;
	Session *session;
	const GList *wins = NULL;
	GList *l;
	GString *xml;
	GError *err;

	g_return_if_fail (EPHY_IS_TABS_PLUGIN_MENU (menu));

	p = menu->priv;

	if (p->window == NULL) return;

	manager = GTK_UI_MANAGER (p->window->ui_merge);

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));

	LOG ("Rebuilding tabs plugin menu %p", menu)

	START_PROFILER ("Rebuilding tabs plugin menu")

	ephy_tabs_plugin_menu_clean (menu);

	p->ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, p->ui_id,
			       "/menubar/TabsMenu/TabsPluginMenuPlaceholder",
			       "TabsMoveToMenu",
			       "TabsMoveTo",
			       GTK_UI_MANAGER_MENU, FALSE);

	wins = session_get_windows (session);
	for (l = (GList *) wins; l != NULL; l = l->next)
	{
		EphyWindow *window =(EphyWindow *) l->data;
		char name[32], action[32];

		g_snprintf (action, 32, VERB_STRING, window);
LOG ("ADDING %s", action)
		gtk_ui_manager_add_ui (manager, p->ui_id,
				       "/menubar/TabsMenu/TabsPluginMenuPlaceholder/TabsMoveToMenu",
				       action, action, GTK_UI_MANAGER_MENUITEM, FALSE);
	}

	gtk_ui_manager_add_ui (manager, p->ui_id,
			       "/menubar/TabsMenu/TabsPluginMenuPlaceholder",
			       "EphyTabsPluginSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);

	gtk_ui_manager_add_ui (manager, p->ui_id,
			       "/menubar/TabsMenu/TabsPluginMenuPlaceholder",
			       "EphyTabsPluginCloneItem",
			       "EphyTabsPluginClone",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	STOP_PROFILER ("Rebuilding tabs plugin menu")
}
