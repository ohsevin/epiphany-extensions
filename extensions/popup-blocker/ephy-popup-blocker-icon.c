/*
 *  Copyright (C) 2003 Adam Hooper
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

#include "ephy-popup-blocker-icon.h"
#include "ephy-string.h"
#include "ephy-debug.h"

#include <epiphany/ephy-statusbar.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkstatusbar.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkuimanager.h>

#include <gdk/gdkpixbuf.h>

#include <glib/gi18n-lib.h>

#define MAX_MENU_LENGTH 60

#define EPHY_POPUP_BLOCKER_ICON_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_POPUP_BLOCKER_ICON, EphyPopupBlockerIconPrivate))

struct EphyPopupBlockerIconPrivate
{
	EphyPopupBlockerList *popups;
	gulong notify_signal;
	GtkTooltips *tooltips;

	GtkWidget *evbox;
	/* FIXME: use the same pixbuf/image for all icons? */
	GtkWidget *image;

	GtkUIManager *ui;
	guint ui_merge_id;
	GtkActionGroup *actions;
};

typedef struct
{
	EphyPopupBlockerIcon *icon;
	BlockedPopup *popup;
} PopupActivateCBData;

static void ephy_popup_blocker_icon_class_init	(EphyPopupBlockerIconClass *klass);
static void ephy_popup_blocker_icon_init	(EphyPopupBlockerIcon *icon);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_popup_blocker_icon_get_type (void)
{
	return type;
}

GType
ephy_popup_blocker_icon_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyPopupBlockerIconClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_popup_blocker_icon_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPopupBlockerIcon),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_popup_blocker_icon_init
	};

	type = g_type_module_register_type (module,
					    GTK_TYPE_FRAME,
					    "EphyPopupBlockerIcon",
					    &our_info, 0);

	return type;
}

static void
popup_menu_detach (GtkWidget *widget,
		   GtkMenu *menu)
{
	/* There is no point to this function. */
}

static void
icon_button_press_cb (GtkWidget *evbox,
		      GdkEventButton *event,
		      EphyPopupBlockerIcon *icon)
{
	GtkWidget *menu;
	char *path;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_ICON (icon));

	if (event->type != GDK_BUTTON_PRESS || event->button != 1) return;

	path = g_strdup_printf ("/EphyPopupBlockerPopup-%p", icon);

	menu = gtk_ui_manager_get_widget (icon->priv->ui, path);

	g_free (path);

	g_return_if_fail (GTK_IS_MENU (menu) && menu != NULL);

	if (gtk_menu_get_attach_widget (GTK_MENU (menu)) != icon)
	{
		gtk_menu_attach_to_widget (GTK_MENU (menu), GTK_WIDGET (icon),
					   popup_menu_detach);
	}

	gtk_widget_show_all (GTK_WIDGET (menu));
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button,
			event->time);
}

static void
create_ui (EphyPopupBlockerIcon *icon)
{
	GdkPixbuf *pixbuf;
	int width = 0, height = 0;
	char *t;

	gtk_frame_set_shadow_type (GTK_FRAME (icon),
				   GTK_SHADOW_IN);

	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
	pixbuf = gdk_pixbuf_new_from_file_at_size
		(SHARE_DIR "/icons/popup-blocker.svg", width, height, NULL);
	icon->priv->image = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (G_OBJECT (pixbuf));

	icon->priv->evbox = gtk_event_box_new ();

	gtk_event_box_set_visible_window (GTK_EVENT_BOX (icon->priv->evbox),
					  FALSE);

	gtk_widget_add_events (icon->priv->evbox, GDK_BUTTON_PRESS_MASK);
	g_signal_connect (icon->priv->evbox, "button-press-event",
			  G_CALLBACK (icon_button_press_cb), icon);

	gtk_container_add (GTK_CONTAINER (icon),
			   icon->priv->evbox);
	gtk_container_add (GTK_CONTAINER (icon->priv->evbox),
			   icon->priv->image);

	t = g_strdup_printf ("EphyPopupBlockerPopup-%p", icon);
	icon->priv->actions = gtk_action_group_new (t);
	g_free (t);

	gtk_action_group_set_translation_domain (icon->priv->actions,
						 GETTEXT_PACKAGE);

	gtk_ui_manager_insert_action_group (icon->priv->ui,
					    icon->priv->actions,
					    0);
}

static void
ephy_popup_blocker_icon_init (EphyPopupBlockerIcon *icon)
{
	LOG ("EphyPopupBlockerIcon initialising")

	icon->priv = EPHY_POPUP_BLOCKER_ICON_GET_PRIVATE (icon);

	icon->priv->tooltips = gtk_tooltips_new ();
	g_object_ref (G_OBJECT (icon->priv->tooltips));
	gtk_object_sink (GTK_OBJECT (icon->priv->tooltips));

	icon->priv->popups = NULL;
	icon->priv->notify_signal = 0;
	icon->priv->ui = NULL;
	icon->priv->ui_merge_id = 0;
	icon->priv->actions = NULL;
}

static void
activate_popup_cb (GtkAction *action,
		   PopupActivateCBData *cb_data)
{
	g_return_if_fail (GTK_IS_ACTION (action));

	ephy_popup_blocker_list_open (cb_data->icon->priv->popups,
				      cb_data->popup);
}

static void
clear_actions (EphyPopupBlockerIcon *icon)
{
	GList *actions;
	GList *t;
	GtkAction *action;

	LOG ("clear_actions on %p\n", icon)

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_ICON (icon));
	g_return_if_fail (icon->priv->actions != NULL);

	if (icon->priv->ui_merge_id == 0) return;

	gtk_ui_manager_remove_ui (icon->priv->ui, icon->priv->ui_merge_id);
	icon->priv->ui_merge_id = 0;

	actions = gtk_action_group_list_actions (icon->priv->actions);

	for (t = actions; t; t = g_list_next (t))
	{
		action = (GtkAction *) t->data;

		g_return_if_fail (GTK_IS_ACTION (action));

		gtk_action_group_remove_action (icon->priv->actions, action);
	}

	g_list_free (actions);
}

static void
build_menu (EphyPopupBlockerIcon *icon)
{
	GSList *blocked_popups;
	GValue value = { 0, };
	PopupActivateCBData *cb_data;
	BlockedPopup *popup;
	GtkAction *action;
	char *name;
	char *label;
	char *short_label;
	char *path;
	char *merge_str;

	if (icon->priv->popups == NULL) return;

	LOG ("build_menu on %p\n", icon)

	g_value_init (&value, G_TYPE_POINTER);
	g_object_get_property (G_OBJECT (icon->priv->popups), "blocked-popups",
			       &value);

	blocked_popups = g_value_get_pointer (&value);

	g_value_unset (&value);

	clear_actions (icon);

	if (blocked_popups == NULL) return;

	path = g_strdup_printf ("/EphyPopupBlockerPopup-%p", icon);

	merge_str = g_strdup_printf
		("<ui><popup name=\"EphyPopupBlockerPopup-%p\"/></ui>", icon);

	icon->priv->ui_merge_id = gtk_ui_manager_add_ui_from_string
		(icon->priv->ui, merge_str, -1, NULL);

	g_return_if_fail (icon->priv->ui_merge_id != 0);

	g_free (merge_str);

	for (; blocked_popups; blocked_popups = g_slist_next (blocked_popups))
	{
		popup = blocked_popups->data;

		if (popup->url == NULL) continue;

		name = g_strdup_printf ("open-popup-%p", popup);
		label = g_strdup_printf (_("Open %s"), popup->url);
		short_label = ephy_string_shorten (label, MAX_MENU_LENGTH);

		action = gtk_action_new (name, short_label, NULL, NULL);

		cb_data = g_new0 (PopupActivateCBData, 1);
		cb_data->icon = icon;
		cb_data->popup = popup;

		g_signal_connect_data
			(G_OBJECT (action), "activate",
			 G_CALLBACK (activate_popup_cb), cb_data,
			 (GClosureNotify) g_free, 0);

		gtk_action_group_add_action (icon->priv->actions, action);

		gtk_ui_manager_add_ui (icon->priv->ui, icon->priv->ui_merge_id,
				       path, name, name,
				       GTK_UI_MANAGER_MENUITEM, FALSE);

		g_free (name);
		g_free (label);
		g_free (short_label);
	}

	g_free (path);
}

static void
update_ui (EphyPopupBlockerIcon *icon)
{
	guint num_blocked;
	GValue value = { 0, };
	char *tooltip;

	if (icon->priv->popups == NULL) return;

	LOG ("update_ui on %p\n", icon)

	g_value_init (&value, G_TYPE_UINT);
	g_object_get_property (G_OBJECT (icon->priv->popups), "count", &value);

	num_blocked = g_value_get_uint (&value);

	LOG ("update_ui: Number of popups on list %p in icon %p: %d\n",
	     icon->priv->popups, icon, num_blocked)

	tooltip = g_strdup_printf (ngettext("%d popup blocked",
					    "%d popups blocked",
					    num_blocked),
				   num_blocked);

	gtk_tooltips_set_tip (icon->priv->tooltips, icon->priv->evbox, tooltip,
			      NULL);

	g_free (tooltip);

	build_menu (icon);

	if (num_blocked == 0)
	{
		gtk_widget_hide (GTK_WIDGET (icon));
	}
	else
	{
		gtk_widget_show_all (GTK_WIDGET (icon));
	}

	g_value_unset (&value);
}

static void
count_changed_cb (EphyPopupBlockerList *list,
		  GParamSpec *pspec,
		  EphyPopupBlockerIcon *icon)
{
	update_ui (icon);
}

EphyPopupBlockerIcon *
ephy_popup_blocker_icon_new (GtkWidget *statusbar,
			     GObject *ui_merge)
{
	EphyPopupBlockerIcon *ret;

	g_return_val_if_fail (EPHY_IS_STATUSBAR (statusbar), NULL);
	g_return_val_if_fail (GTK_IS_UI_MANAGER (ui_merge), NULL);

	ret = EPHY_POPUP_BLOCKER_ICON
		(g_object_new (EPHY_TYPE_POPUP_BLOCKER_ICON, NULL));

	ephy_statusbar_add_widget (EPHY_STATUSBAR (statusbar),
				   GTK_WIDGET (ret));

	ret->priv->ui = GTK_UI_MANAGER (g_object_ref (ui_merge));

	create_ui (ret);

	return ret;
}

static void
ephy_popup_blocker_icon_finalize (GObject *object)
{
	EphyPopupBlockerIcon *icon = EPHY_POPUP_BLOCKER_ICON (object);

	LOG ("EphyPopupBlockerIcon finalizing")

	g_object_unref (icon->priv->tooltips);

	if (icon->priv->popups)
	{
		g_return_if_fail (icon->priv->notify_signal != 0);

		g_signal_handler_disconnect (icon->priv->popups,
					     icon->priv->notify_signal);

		g_object_unref (icon->priv->popups);
	}

	clear_actions (icon);

	g_object_unref (G_OBJECT (icon->priv->actions));
	g_object_unref (G_OBJECT (icon->priv->ui));

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_popup_blocker_icon_class_init (EphyPopupBlockerIconClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_popup_blocker_icon_finalize;

	g_type_class_add_private (object_class, sizeof (EphyPopupBlockerIconPrivate));
}

void
ephy_popup_blocker_icon_set_popups (EphyPopupBlockerIcon *icon,
				    EphyPopupBlockerList *popups)
{
	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_ICON (icon));
	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (popups));

	LOG ("ephy_popup_blocker_icon_set_popups: icon: %p, popups: %p\n",
	     icon, popups);

	if (icon->priv->popups)
	{
		g_return_if_fail (icon->priv->notify_signal != 0);

		g_signal_handler_disconnect (icon->priv->popups,
					     icon->priv->notify_signal);

		g_object_unref (icon->priv->popups);
	}

	if (popups)
	{
		g_object_ref (popups);
		icon->priv->popups = popups;

		icon->priv->notify_signal =
			g_signal_connect_object (popups, "notify::count",
						 G_CALLBACK (count_changed_cb),
						 icon, 0);
	}

	update_ui (icon);
}
