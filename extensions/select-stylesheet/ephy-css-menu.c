/*
 *  Copyright (C) 2002  Ricardo Fernández Pascual
 *  Copyright (C) 2004  Crispin Flowerday
 *  Copyright (C) 2004  Adam Hooper
 *  Copyright (C) 2004  Christian Persch
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

#include <glib/gi18n-lib.h>

#include <gtk/gtkuimanager.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkradioaction.h>

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-embed.h>

#include "mozilla/mozilla-helpers.h"
#include "ephy-css-menu.h"

#include "ephy-debug.h"
#include "ephy-string.h"

#define EPHY_CSS_MENU_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_CSS_MENU, EphyCSSMenuPrivate))

struct _EphyCSSMenuPrivate
{
	EphyWindow *window;
	GtkUIManager *manager;
	EphyEmbed *embed;

	GtkActionGroup *item_action_group;
	guint ui_id;

	GtkActionGroup *menu_action_group;
	guint menu_ui_id;

	gboolean updating;
};

#define CSS_MENU_PLACEHOLDER_PATH	"/menubar/ViewMenu"
#define STYLESHEET_KEY			"ECStyleSheet"
#define ACTION_VERB_FORMAT		"ECSSSwitchStyle%x"
#define ACTION_VERB_FORMAT_LENGTH	strlen (ACTION_VERB_FORMAT) + 14 + 1

enum
{
	PROP_0,
	PROP_WINDOW
};

static void ephy_css_menu_class_init	(EphyCSSMenuClass *klass);
static void ephy_css_menu_init		(EphyCSSMenu *m);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_css_menu_get_type (void)
{
	return type;
}

GType
ephy_css_menu_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyCSSMenuClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_css_menu_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyCSSMenu),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_css_menu_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyCSSMenu",
					    &our_info, 0);

	return type;
}

static void ephy_css_menu_rebuild (EphyCSSMenu *menu);

static void
activate_stylesheet_cb (GtkAction *action,
			EphyCSSMenu *menu)
{
	EphyCSSMenuPrivate *p = menu->priv;
	EmbedStyleSheet *style;

	if (menu->priv->updating) return;

	g_return_if_fail (EPHY_IS_EMBED (p->embed));
	g_return_if_fail (ephy_window_get_active_embed (p->window) == p->embed);

	style = g_object_get_data (G_OBJECT (action), STYLESHEET_KEY);
	g_return_if_fail (style != NULL);

	mozilla_set_stylesheet (p->embed, style);
}

static GtkAction *
create_stylesheet_action (EphyCSSMenu *menu,
			  EmbedStyleSheet *style,
			  const char *verb)
{
	GtkAction  *action;
	char *tooltip, *label;

	label = ephy_string_double_underscores (style->name);

	switch (style->type)
	{
		case STYLESHEET_NONE:
			tooltip = g_strdup (_("Render the page without using a style"));
			break;
		case STYLESHEET_BASIC:
			tooltip = g_strdup (_("Render the page using the default style"));
			break;
		default:
			tooltip = g_strdup_printf (_("Render the page using the \"%s\" style"), 
						   style->name);
			break;
	}

	action = g_object_new (GTK_TYPE_RADIO_ACTION,
			       "name", verb,
			       "label", label,
			       "tooltip", tooltip,
			       NULL);

	g_object_set_data_full (G_OBJECT(action), STYLESHEET_KEY, style,
				(GDestroyNotify) mozilla_free_stylesheet);

	g_signal_connect_object (action, "activate",
				 G_CALLBACK (activate_stylesheet_cb),
				 menu, 0);

	gtk_action_group_add_action (menu->priv->item_action_group, action);
	g_object_unref (action);

	g_free (label);
	g_free (tooltip);

	return action;
}

static void
ephy_css_menu_rebuild (EphyCSSMenu *menu)
{
	EphyCSSMenuPrivate *p = menu->priv;
	GList *stylesheets = NULL, *l;
	GtkAction *action;
	EmbedStyleSheet *current = NULL;
	GSList *radio_group = NULL;
	char verb[ACTION_VERB_FORMAT_LENGTH];
	int i;

	p->updating = TRUE;

	if (p->ui_id != 0)
	{
		gtk_ui_manager_remove_ui (p->manager, p->ui_id);
		gtk_ui_manager_ensure_update (p->manager);
	}

	if (p->item_action_group != NULL)
	{
		gtk_ui_manager_remove_action_group (p->manager, p->item_action_group);
		g_object_unref (p->item_action_group);
	}

	LOG ("Rebuilding stylesheet menu");

	stylesheets = mozilla_get_stylesheets (p->embed, &current);

	/* Create the new action group */
	p->item_action_group = 
		gtk_action_group_new ("SelectStylesheetMenuDynamicActions");
	gtk_action_group_set_translation_domain (p->item_action_group, NULL);
	gtk_ui_manager_insert_action_group (p->manager, p->item_action_group, -1);

	p->ui_id = gtk_ui_manager_new_merge_id (p->manager);

	action = gtk_action_group_get_action (p->menu_action_group,
					      "ECSSMenuAction");
	g_object_set (G_OBJECT (action), "sensitive", stylesheets != NULL, NULL);
	
	for (l = stylesheets, i = 0; l != NULL; l = l->next, i++)
	{
		EmbedStyleSheet *style = (EmbedStyleSheet*) l->data;

		g_snprintf (verb, sizeof (verb), ACTION_VERB_FORMAT, i);

		action = create_stylesheet_action (menu, style, verb);

		gtk_ui_manager_add_ui (p->manager, p->ui_id,
				       CSS_MENU_PLACEHOLDER_PATH "/StyleMenu",
				       verb, verb,
				       GTK_UI_MANAGER_MENUITEM, FALSE);

		/* Make sure all widgets are in the same radio_group */
		gtk_radio_action_set_group (GTK_RADIO_ACTION (action),
					    radio_group);
		radio_group = gtk_radio_action_get_group (GTK_RADIO_ACTION (action));

		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), 
					      style == current);
	}

	/* Don't need to free the contents, the refs are held on the actions */
	g_list_free (stylesheets);

	p->updating = FALSE;

	gtk_ui_manager_ensure_update (p->manager);
}

static void
ephy_css_menu_set_embed (EphyCSSMenu *menu,
			 EphyEmbed *embed)
{
	EphyCSSMenuPrivate *p = menu->priv;

	if (p->embed != NULL)
	{
		g_signal_handlers_disconnect_matched
			(p->embed, G_SIGNAL_MATCH_DATA, 
			 0, 0, NULL, NULL, menu);
		g_object_unref (p->embed);
	}

	p->embed = embed;

	if (p->embed != NULL)
	{
		g_object_ref (p->embed);
		g_signal_connect_object (p->embed, "net_stop",
					 G_CALLBACK (ephy_css_menu_rebuild),
					 menu, G_CONNECT_SWAPPED);
	}

	ephy_css_menu_rebuild (menu);
}

static void
sync_active_tab_cb (EphyWindow *window,
		    GParamSpec *pspec,
		    EphyCSSMenu *menu)
{
	EphyEmbed *embed;

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);

	ephy_css_menu_set_embed (menu, embed);
}

static GtkActionEntry entries[] =
{
	{ "ECSSMenuAction", NULL, N_("St_yle"), NULL,
	  N_("Select a different style for this page"), NULL }
};

static void
ephy_css_menu_set_window (EphyCSSMenu *menu, EphyWindow *window)
{
	EphyCSSMenuPrivate *p = menu->priv;
	EphyEmbed *embed;
	GtkAction *action;

	p->window = window;
	
	/* Create the Action Group */
	p->manager = g_object_ref (GTK_UI_MANAGER (window->ui_merge));
	p->menu_action_group = gtk_action_group_new ("EphyCSSMenuActions");
	gtk_action_group_set_translation_domain (p->menu_action_group, GETTEXT_PACKAGE);
	gtk_ui_manager_insert_action_group(p->manager, p->menu_action_group, -1);
	gtk_action_group_add_actions (p->menu_action_group, entries,
				      G_N_ELEMENTS (entries), window);

	action = gtk_action_group_get_action (p->menu_action_group,
					      "ECSSMenuAction");
	g_object_set (G_OBJECT (action), "hide_if_empty", FALSE, NULL);

	p->menu_ui_id = gtk_ui_manager_new_merge_id (p->manager);

	gtk_ui_manager_add_ui (p->manager, p->menu_ui_id,
			       CSS_MENU_PLACEHOLDER_PATH,
			       "StyleSep0", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (p->manager, p->menu_ui_id,
			       CSS_MENU_PLACEHOLDER_PATH,
			       "StyleMenu", "ECSSMenuAction",
			       GTK_UI_MANAGER_MENU, FALSE);
	gtk_ui_manager_add_ui (p->manager, p->menu_ui_id,
			       CSS_MENU_PLACEHOLDER_PATH,
			       "StyleSep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);

	/* Attach the signals to the window */
	g_signal_connect (window, "notify::active-tab",
			  G_CALLBACK (sync_active_tab_cb), menu);

	if (GTK_WIDGET_REALIZED (window))
	{
		embed = ephy_window_get_active_embed (window);
		ephy_css_menu_set_embed (menu, embed);
	}
}

static void
ephy_css_menu_finalize (GObject *object)
{
	EphyCSSMenu *menu = EPHY_CSS_MENU(object);
	EphyCSSMenuPrivate *p = menu->priv;

	if (p->ui_id != 0)
	{
		gtk_ui_manager_remove_ui (p->manager, p->ui_id);
	}

	if (p->menu_ui_id != 0)
	{
		gtk_ui_manager_remove_ui (p->manager, p->menu_ui_id);
	}

	if (p->menu_action_group)
	{
		gtk_ui_manager_remove_action_group
			(p->manager, p->menu_action_group);
		g_object_unref (p->menu_action_group);
	}

	if (p->item_action_group)
	{
		gtk_ui_manager_remove_action_group (p->manager, p->item_action_group);
		g_object_unref (p->item_action_group);
	}

	if (p->embed)
	{
		g_object_unref (p->embed);
	}

	G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void 
ephy_css_menu_init(EphyCSSMenu *menu)
{
	menu->priv = EPHY_CSS_MENU_GET_PRIVATE (menu);
}

static void
ephy_css_menu_get_property (GObject *object,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
ephy_css_menu_set_property (GObject *object,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *pspec)
{
	EphyCSSMenu *menu = EPHY_CSS_MENU (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_css_menu_set_window (menu, g_value_get_object (value));
		break;
	}
}

static void
ephy_css_menu_class_init(EphyCSSMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->set_property = ephy_css_menu_set_property;
	object_class->get_property = ephy_css_menu_get_property;
	object_class->finalize = ephy_css_menu_finalize;

	g_object_class_install_property (object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							      "Window",
							      "Parent window",
							      EPHY_TYPE_WINDOW,
							      G_PARAM_WRITABLE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (klass, sizeof (EphyCSSMenuPrivate));
}

EphyCSSMenu *
ephy_css_menu_new (EphyWindow *window)
{
	return g_object_new (EPHY_TYPE_CSS_MENU, 
			     "window", window,
			     NULL);
}
