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
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-tabs-plugin-menu.h"
#include "ephy-window-action.h"

#include "egg-menu-merge.h"
#include "egg-action.h"
#include "egg-action-group.h"
#include "ephy-debug.h"

#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-notebook.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>

#include <string.h>
#include <stdlib.h>
#include <libxml/entities.h>
#include <bonobo/bonobo-i18n.h>

/**
 * Private data
 */
struct _EphyTabsPluginMenuPrivate
{
	EphyWindow *window;
	EggActionGroup *action_group;
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
                                                              EPHY_WINDOW_TYPE,
                                                              G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));
}

static void
move_to_window_cb (EggAction *a, EphyTabsPluginMenu *menu)
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
	g_return_if_fail (IS_EPHY_TAB (tab));
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
	EggAction *action;
	char *verb;

	verb = g_strdup_printf (VERB_STRING, window);

	action = EGG_ACTION (g_object_new (EPHY_TYPE_WINDOW_ACTION,
					   "name", verb,
					   "window", window,
					   NULL));

	g_signal_connect (action, "activate",
			  G_CALLBACK (move_to_window_cb), menu);

	egg_action_group_add_action (menu->priv->action_group, action);

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
	EggAction *action;
	char *verb;

	if (menu->priv->window == NULL) return;

	verb = g_strdup_printf (VERB_STRING, window);

	action = egg_action_group_get_action (menu->priv->action_group, verb);

	if (action != NULL)
	{
		egg_action_group_remove_action (menu->priv->action_group, action);
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

static void
clone_cb (EggAction *action, EphyTabsPluginMenu *menu)
{
	EphyWindow *window;
	EphyTab *tab;

	window = menu->priv->window;
	g_return_if_fail (window != NULL);

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (IS_EPHY_TAB (tab));

	ephy_shell_new_tab (ephy_shell, window, tab, NULL,
			    EPHY_NEW_TAB_IN_EXISTING_WINDOW |
			    EPHY_NEW_TAB_APPEND_AFTER |
			    EPHY_NEW_TAB_CLONE_PAGE);
}

static void
set_action_accelerator (EggActionGroup *action_group,
			EggAction *action,
			const char *accel)
{
	char *accel_path = NULL;
	guint accel_key = 0;
	GdkModifierType accel_mods;

	/* set the accel path for the menu item */
	accel_path = g_strconcat ("<Actions>/", action_group->name, "/",
				  action->name, NULL);

	gtk_accelerator_parse (accel, &accel_key, &accel_mods);

	if (accel_key != 0)
	{
		gtk_accel_map_change_entry (accel_path, accel_key,
					    accel_mods, TRUE);
	}

	action->accel_quark = g_quark_from_string (accel_path);
	g_free (accel_path);
}

static void
ephy_tabs_plugin_menu_init (EphyTabsPluginMenu *menu)
{
	EphyTabsPluginMenuPrivate *p;
	EggAction *action;
	Session *session;
	const GList *wins;

	LOG ("EphyTabsPluginMenu initialising %p", menu)
	
	p = g_new0 (EphyTabsPluginMenuPrivate, 1);
	menu->priv = p;

	p->ui_id = 0;

	/* construct action group */
	p->action_group = egg_action_group_new ("EphyTabsPluginActions");

	action = g_object_new (EGG_TYPE_ACTION,
				"name", "EphyTabsPluginClone",
				"label", _("_Clone Tab"),
				"tooltip", _("Create a copy of the current tab"),
				NULL);
	egg_action_group_add_action (p->action_group, action);
	g_signal_connect (action, "activate",
			  G_CALLBACK (clone_cb), menu);
	set_action_accelerator (p->action_group, action, "<shift><control>C");
	g_object_unref (action);

	action = g_object_new (EGG_TYPE_ACTION,
				"name", "EphyTabsPluginMove",
				"label", _("_Move Tab To"),
				"tooltip", _("Move the current tab to a different window"),
				NULL);
	egg_action_group_add_action (p->action_group, action);
	g_object_unref (action);

	session = SESSION (ephy_shell_get_session (ephy_shell));

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
			egg_menu_merge_remove_action_group
				(EGG_MENU_MERGE (window->ui_merge),
				 p->action_group);
		}

		g_object_unref (p->action_group);
	}

	session = SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_handlers_disconnect_by_func (G_OBJECT (session),
					      G_CALLBACK (window_opened_cb), menu);
	g_signal_handlers_disconnect_by_func (G_OBJECT (session),
					      G_CALLBACK (window_closed_cb), menu);

	if (window != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (window),
					      (gpointer *) &menu->priv->window);
	}

	g_free (p);

	LOG ("EphyTabsPluginMenu finalised %p", o)

	parent_class->finalize (o);
}

static void
ephy_tabs_plugin_menu_set_window (EphyTabsPluginMenu *menu, EphyWindow *window)
{
	EggAction *action;
	char *verb;

	g_return_if_fail (EPHY_IS_TABS_PLUGIN_MENU (menu));
	g_return_if_fail (IS_EPHY_WINDOW (window));

	menu->priv->window = window;

	g_object_add_weak_pointer (G_OBJECT (window),
				   (gpointer *) &menu->priv->window);

	/* you can't move a tab to the window it's already in */
	verb = g_strdup_printf (VERB_STRING, window);
	action = egg_action_group_get_action (menu->priv->action_group, verb);
	g_object_set (action, "sensitive", FALSE, NULL);
	g_free (verb);

	egg_menu_merge_insert_action_group (EGG_MENU_MERGE (window->ui_merge),
					    menu->priv->action_group, 0);

	ephy_tabs_plugin_menu_update (menu);
}

static void
ephy_tabs_plugin_menu_clean (EphyTabsPluginMenu *menu)
{
	EphyTabsPluginMenuPrivate *p;
	EggMenuMerge *merge;

	p = menu->priv;

	merge = EGG_MENU_MERGE (p->window->ui_merge);

	if (p->ui_id > 0)
	{
		egg_menu_merge_remove_ui (merge, p->ui_id);
		egg_menu_merge_ensure_update (merge);
		p->ui_id = 0;
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
	EggMenuMerge *merge;
	EggAction *action;
	Session *session;
	GString *xml;
	const GList *wins = NULL;
	GList *l;
	GError *error = NULL;

	g_return_if_fail (EPHY_IS_TABS_PLUGIN_MENU (menu));

	p = menu->priv;

	if (p->window == NULL) return;

	merge = EGG_MENU_MERGE (p->window->ui_merge);

	session = SESSION (ephy_shell_get_session (ephy_shell));

	LOG ("Rebuilding tabs plugin menu %p", menu)

	START_PROFILER ("Rebuilding tabs plugin menu")

	ephy_tabs_plugin_menu_clean (menu);

	wins = session_get_windows (session);

	xml = g_string_sized_new (1024);

	g_string_append (xml, "<Root><menu><submenu name=\"TabsMenu\">"
			      "<placeholder name=\"TabsPluginPlaceholder\">"
			      "<menuitem name=\"EphyTabsPluginCloneItem\" "
			      "verb=\"EphyTabsPluginClone\"/>\n"
			      "<submenu name=\"EphyTabsPluginMoveMenu\" "
			      "verb=\"EphyTabsPluginMove\">\n");		

	for (l = (GList *) wins; l != NULL; l = l->next)
	{
		EphyWindow *window =(EphyWindow *) l->data;

		g_string_append_printf (xml, "<menuitem name=\"" VERB_STRING
					"Item\" verb=\"" VERB_STRING "\"/>\n",
					window, window);
	}

	g_string_append (xml, "</submenu></placeholder></submenu></menu></Root>");

	p->ui_id = egg_menu_merge_add_ui_from_string
				(merge, xml->str, -1, &error);	

	g_string_free (xml, TRUE);

	STOP_PROFILER ("Rebuilding tabs plugin menu")
}
