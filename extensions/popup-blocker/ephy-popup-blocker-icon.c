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

#include "ephy-debug.h"

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-statusbar.h>

#include <gtk/gtkcontainer.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkimage.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkstatusbar.h>

#include <gdk/gdkpixbuf.h>

#include <glib/gi18n-lib.h>

#define EPHY_POPUP_BLOCKER_ICON_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_POPUP_BLOCKER_ICON, EphyPopupBlockerIconPrivate))

struct EphyPopupBlockerIconPrivate
{
	EphyWindow *window;
	GSList *popups;
	GtkTooltips *tooltips;

	GtkWidget *frame;
	GtkWidget *evbox;
	/* FIXME: use the same pixbuf/image for all icons? */
	GtkWidget *image;
};

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
					    G_TYPE_OBJECT,
					    "EphyPopupBlockerIcon",
					    &our_info, 0);

	return type;
}

EphyPopupBlockerIcon *
ephy_popup_blocker_icon_new (EphyWindow *window)
{
	EphyPopupBlockerIcon *ret;
	GtkWidget *statusbar;

	g_return_val_if_fail (EPHY_IS_WINDOW (window), NULL);

	ret = EPHY_POPUP_BLOCKER_ICON
		(g_object_new (EPHY_TYPE_POPUP_BLOCKER_ICON, NULL));

	statusbar = ephy_window_get_statusbar (window);
	g_return_if_fail (EPHY_IS_STATUSBAR (statusbar));

	ephy_statusbar_add_widget (EPHY_STATUSBAR (statusbar),
				   GTK_WIDGET (ret->priv->frame));

	ret->priv->window = window;

	return ret;
}

static void
create_ui (EphyPopupBlockerIcon *icon)
{
	GdkPixbuf *pixbuf;

	icon->priv->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (icon->priv->frame),
				   GTK_SHADOW_IN);

	/* FIXME: Why does 24x24 not work? */
	pixbuf = gdk_pixbuf_new_from_file_at_size
		(SHARE_DIR "/icons/popup-blocker.svg", 20, 20, NULL);
	icon->priv->image = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (G_OBJECT (pixbuf));

	icon->priv->evbox = gtk_event_box_new ();

	gtk_event_box_set_visible_window (GTK_EVENT_BOX (icon->priv->evbox),
					  FALSE);

	gtk_container_add (GTK_CONTAINER (icon->priv->frame),
			   icon->priv->evbox);
	gtk_container_add (GTK_CONTAINER (icon->priv->evbox),
			   icon->priv->image);
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

	create_ui (icon);
}

static void
ephy_popup_blocker_icon_finalize (GObject *object)
{
	EphyPopupBlockerIcon *icon = EPHY_POPUP_BLOCKER_ICON (object);
	GtkWidget *statusbar;

	LOG ("EphyPopupBlockerIcon finalizing")

	statusbar = ephy_window_get_statusbar (icon->priv->window);
	g_return_if_fail (EPHY_IS_STATUSBAR (statusbar));

	gtk_container_remove (GTK_CONTAINER (statusbar),
			      GTK_WIDGET (icon->priv->frame));

	g_object_unref (icon->priv->tooltips);

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

static void
update_ui (EphyPopupBlockerIcon *icon)
{
	guint num_blocked;
	char *tooltip;

	num_blocked = g_slist_length (icon->priv->popups);

	tooltip = g_strdup_printf (ngettext("%d popup blocked",
					    "%d popups blocked",
					    num_blocked),
				   num_blocked);

	gtk_tooltips_set_tip (icon->priv->tooltips, icon->priv->evbox, tooltip,
			      NULL);

	g_free (tooltip);

	if (num_blocked == 0)
	{
		gtk_widget_hide (GTK_WIDGET (icon->priv->frame));
	}
	else
	{
		gtk_widget_show_all (GTK_WIDGET (icon->priv->frame));
	}
}

void
ephy_popup_blocker_icon_set_popups (EphyPopupBlockerIcon *icon,
				    GSList *blocked_list)
{
	icon->priv->popups = blocked_list;

	update_ui (icon);
}
