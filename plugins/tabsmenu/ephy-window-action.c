/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#include "ephy-window-action.h"
#include "ephy-plugins-i18n.h"

#include "ephy-string.h"
#include "ephy-debug.h"

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-tab.h>

#include <glib-object.h>
#include <gtk/gtknotebook.h>

struct _EphyWindowActionPrivate {
	EphyWindow *window;
	int num_tabs;
	EphyTab *old_tab;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static void ephy_window_action_init       (EphyWindowAction *action);
static void ephy_window_action_class_init (EphyWindowActionClass *class);
static void ephy_window_action_finalize	  (GObject *object);
static void ephy_window_action_set_window (EphyWindowAction *action,
					   EphyWindow *window);

static GObjectClass *parent_class = NULL;

GType
ephy_window_action_get_type (void)
{
        static GType ephy_window_action_type = 0;

        if (ephy_window_action_type == 0)
        {
                static const GTypeInfo our_info =
			{
				sizeof (EphyWindowActionClass),
				NULL, /* base_init */
				NULL, /* base_finalize */
				(GClassInitFunc) ephy_window_action_class_init,
				NULL,
				NULL, /* class_data */
				sizeof (EphyWindowAction),
				0, /* n_preallocs */
				(GInstanceInitFunc) ephy_window_action_init,
			};

                ephy_window_action_type = g_type_register_static (EGG_TYPE_ACTION,
								  "EphyWindowAction",
								  &our_info, 0);
        }

        return ephy_window_action_type;
}

static void
ephy_window_action_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	EphyWindowAction *action;

	action = EPHY_WINDOW_ACTION (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_window_action_set_window (action, g_value_get_object (value));
			break;
	}
}

static void
ephy_window_action_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	EphyWindowAction *action;

	action = EPHY_WINDOW_ACTION (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, action->priv->window);
			break;
	}
}

static void
ephy_window_action_class_init (EphyWindowActionClass *class)
{
	EggActionClass *action_class;
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->set_property = ephy_window_action_set_property;
	object_class->get_property = ephy_window_action_get_property;
	object_class->finalize = ephy_window_action_finalize;

	parent_class = g_type_class_peek_parent (class);
	action_class = EGG_ACTION_CLASS (class);

	action_class->menu_item_type = GTK_TYPE_MENU_ITEM;

	g_object_class_install_property (object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							      "Window",
							      "The window this action reflects",
							      EPHY_WINDOW_TYPE,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));
}

static void
ephy_window_action_init (EphyWindowAction *action)
{
	action->priv = g_new0 (EphyWindowActionPrivate, 1);

	action->priv->window = NULL;
	action->priv->num_tabs = 0;
	action->priv->old_tab = NULL;
}

#define MAX_LENGTH	32

static void
sync_title (EphyTab *tab, GParamSpec *pspec, EphyWindowAction *action)
{
	guint num;
	const char *text;
	char *title, *win_title = NULL;

	if (action->priv->window == NULL) return;

	num = (guint) action->priv->num_tabs;

	if (tab)
	{
		win_title = ephy_string_shorten (ephy_tab_get_title (tab), MAX_LENGTH);
	}

	text = dngettext (GETTEXT_PACKAGE, "Window '%s' (%d tab)",
					   "Window '%s' (%d tabs)", num);
	
	title = g_strdup_printf (text, win_title, num);

	g_object_set (G_OBJECT (action), "label", title, NULL);

	g_free (win_title);
	g_free (title);
}

static void
tab_num_changed_cb (GtkWidget *notebook, GtkWidget *child, EphyWindowAction *action)
{
	action->priv->num_tabs = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

	sync_title (action->priv->old_tab, NULL, action);
}

static void
set_active_tab (EphyWindowAction *action)
{
	EphyTab *tab;

	if (action->priv->window == NULL) return;

	tab = ephy_window_get_active_tab (action->priv->window);
	g_return_if_fail (IS_EPHY_TAB (tab));

	/* shouldn't happen but who knows */
 	if (tab == action->priv->old_tab) return;

	if (action->priv->old_tab != NULL)
	{
		g_signal_handlers_disconnect_by_func (G_OBJECT (action->priv->old_tab),
						      G_CALLBACK (sync_title),
						      action);
	}

	action->priv->old_tab = tab;

	g_signal_connect_object (G_OBJECT (tab), "notify::title",
				 G_CALLBACK (sync_title), action, 0);

	sync_title (tab, NULL, action);
}

static void
switch_page_cb (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, EphyWindowAction *action)
{
	set_active_tab (action);
}

static void
ephy_window_action_set_window (EphyWindowAction *action, EphyWindow *window)
{
	GtkWidget *notebook;

	g_return_if_fail (EPHY_IS_WINDOW_ACTION (action));
	g_return_if_fail (IS_EPHY_WINDOW (window));

	action->priv->window = window;

	g_object_add_weak_pointer (G_OBJECT (window),
				   (gpointer *) &action->priv->window);

	notebook = ephy_window_get_notebook (window);

	action->priv->num_tabs = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

	if (action->priv->num_tabs > 0)
	{
		set_active_tab (action);
	}

	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), action);
	g_signal_connect_after (notebook, "tab_added",
				G_CALLBACK (tab_num_changed_cb), action);	
	g_signal_connect_after (notebook, "tab_removed",
				G_CALLBACK (tab_num_changed_cb), action);

	sync_title (action->priv->old_tab, NULL, action);
}

EphyWindow *
ephy_window_action_get_window (EphyWindowAction *action)
{
	g_return_val_if_fail (EPHY_IS_WINDOW_ACTION (action), NULL);

	return action->priv->window;
}

static void
ephy_window_action_finalize (GObject *object)
{
	EphyWindowAction *action = EPHY_WINDOW_ACTION (object);

	if (action->priv->old_tab != NULL)
	{
		g_signal_handlers_disconnect_by_func (G_OBJECT (action->priv->old_tab),
						      G_CALLBACK (sync_title),
						      action);
	}

	if (action->priv->window != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (action->priv->window),
					      (gpointer *) &action->priv->window);
	}

	g_free (action->priv);

	LOG ("EphyWindowAction finalised %p", object)

	G_OBJECT_CLASS (parent_class)->finalize (object);
}
