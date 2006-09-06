/*
 *  Copyright (C) 2002 Ricardo Fernádez Pascual
 *  Copyright (C) 2005 Crispin Flowerday
 *  Copyright (C) 2005 Christian Persch
 *  Copyright (C) 2005 Samuel Abels
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

/* this file is based on work of Daniel Erat for galeon 1 */

#include "config.h"

#include "ephy-push-scroller.h"
#include "ephy-debug.h"

#include <gtk/gtkimage.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkmain.h>

#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#define EPHY_PUSH_SCROLLER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_PUSH_SCROLLER, EphyPushScrollerPrivate))

struct _EphyPushScrollerPrivate
{
	EphyWindow *window;
	EphyEmbed *embed;
	GdkCursor *cursor;
	guint start_x;
	guint start_y;
	guint active : 1;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

/* private functions */

static void
ephy_push_scroller_set_window (EphyPushScroller *scroller,
			       EphyWindow *window)
{
	EphyPushScrollerPrivate *priv = scroller->priv;

	priv->window = window;
}

static gboolean
ephy_push_scroller_motion_cb (GtkWidget *widget,
			      GdkEventMotion *event,
			      EphyPushScroller *scroller)
{
	EphyPushScrollerPrivate *priv = scroller->priv;
	int x_dist, x_dist_abs, y_dist, y_dist_abs;

	if (!priv->active)
	{
		return FALSE;
	}

	/* get distance between last known cursor position and cursor */
	x_dist = priv->start_x - event->x_root;
	x_dist_abs = abs (x_dist);
	y_dist = priv->start_y - event->y_root;
	y_dist_abs = abs (y_dist);

	/* scroll */
	ephy_embed_scroll_pixels (priv->embed, x_dist, y_dist);

	priv->start_x = event->x_root;
	priv->start_y = event->y_root;

	return TRUE;
}

static gboolean
ephy_push_scroller_mouse_press_cb (GtkWidget *widget,
				   GdkEventButton *event,
				   EphyPushScroller *scroller)
{
	ephy_push_scroller_stop (scroller, event->time);

	return TRUE;
}

static gboolean
ephy_push_scroller_mouse_release_cb (GtkWidget *widget,
				   GdkEventButton *event,
				   EphyPushScroller *scroller)
{
	ephy_push_scroller_stop (scroller, event->time);

	return TRUE;
}

static gboolean
ephy_push_scroller_key_press_cb (GtkWidget *widget,
				 GdkEventKey *event,
				 EphyPushScroller *scroller)
{
	ephy_push_scroller_stop (scroller, event->time);

	return TRUE;
}

/* FIXME: I think this is a subcase of grab-broken now? */
static gboolean
ephy_push_scroller_unmap_event_cb (GtkWidget *widget,
				   GdkEventAny *event,
				   EphyPushScroller *scroller)
{
	ephy_push_scroller_stop (scroller, gtk_get_current_event_time ());

	return FALSE;
}

static gboolean
ephy_push_scroller_grab_broken_event_cb (GtkWidget *widget,
					 GdkEventGrabBroken *event,
					 EphyPushScroller *scroller)
{
	LOG ("Grab Broken [%p, window %p]", scroller, scroller->priv->window);

	ephy_push_scroller_stop (scroller, GDK_CURRENT_TIME /* FIXME? */);

	return FALSE;
}

static void
ephy_push_scroller_grab_notify_cb (GtkWidget *widget,
				   gboolean was_grabbed,
				   EphyPushScroller *scroller)
{
	LOG ("Grab Notify [%p, window %p]", scroller, scroller->priv->window);

	if (was_grabbed)
	{
		ephy_push_scroller_stop (scroller, GDK_CURRENT_TIME /* FIXME? */);
	}
}

/* public functions */

void
ephy_push_scroller_start (EphyPushScroller *scroller,
			  EphyEmbed *embed,
			  int x,
			  int y)
{
	EphyPushScrollerPrivate *priv = scroller->priv;
	GtkWidget *widget, *child;
	guint32 timestamp;

	g_return_if_fail (embed != NULL);

	LOG ("Start [%p]", scroller);

	if (priv->active) return;

	if (gdk_pointer_is_grabbed ()) return;

	priv->active = TRUE;

	/* FIXME is this good enough? */
	timestamp = gtk_get_current_event_time ();

	g_object_ref (scroller);

	priv->embed = embed;

	g_object_ref (priv->window);

	/* set positions */
	priv->start_x = x;
	priv->start_y = y;

	g_signal_connect (priv->window, "motion-notify-event",
			    G_CALLBACK (ephy_push_scroller_motion_cb), scroller);
	g_signal_connect (priv->window, "button-press-event",
			  G_CALLBACK (ephy_push_scroller_mouse_press_cb), scroller);
	g_signal_connect (priv->window, "button-release-event",
			  G_CALLBACK (ephy_push_scroller_mouse_release_cb), scroller);
	g_signal_connect (priv->window, "key-press-event",
			  G_CALLBACK (ephy_push_scroller_key_press_cb), scroller);
	g_signal_connect (priv->window, "unmap-event",
			  G_CALLBACK (ephy_push_scroller_unmap_event_cb), scroller);
	g_signal_connect (priv->window, "grab-broken-event",
			  G_CALLBACK (ephy_push_scroller_grab_broken_event_cb), scroller);

	/* FIXME: this signal only seems to be emitted on the container children of GtkWindow,
	 * not on GtkWindow itself... is there a better way to get notified of new grabs?
	 */
	child = gtk_bin_get_child (GTK_BIN (priv->window));
	g_signal_connect_object (child, "grab-notify",
				 G_CALLBACK (ephy_push_scroller_grab_notify_cb), scroller, 0);

	/* grab the pointer */
	widget = GTK_WIDGET (priv->window);
	gtk_grab_add (widget);
	if (gdk_pointer_grab (widget->window, FALSE,
			      GDK_POINTER_MOTION_MASK |
			      GDK_BUTTON_PRESS_MASK |
			      GDK_BUTTON_RELEASE_MASK,
			      NULL, priv->cursor, timestamp) != GDK_GRAB_SUCCESS)
	{
		ephy_push_scroller_stop (scroller, timestamp);
		return;
	}

	if (gdk_keyboard_grab (widget->window, FALSE, timestamp) != GDK_GRAB_SUCCESS)
	{
		ephy_push_scroller_stop (scroller, timestamp);
		return;
	}
}

void
ephy_push_scroller_stop (EphyPushScroller *scroller,
			 guint32 timestamp)
{
	EphyPushScrollerPrivate *priv = scroller->priv;
	GtkWidget *widget;

	LOG ("Stop [%p]", scroller);

	if (priv->active == FALSE) return;

	/* disconnect the signals before ungrabbing! */
	g_signal_handlers_disconnect_matched (priv->window,
					      G_SIGNAL_MATCH_DATA, 0, 0, 
					      NULL, NULL, scroller);
	g_signal_handlers_disconnect_matched (gtk_bin_get_child (GTK_BIN (priv->window)),
					      G_SIGNAL_MATCH_DATA, 0, 0, 
					      NULL, NULL, scroller);

	/* ungrab the pointer if it's grabbed */
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (timestamp);
	}

	gdk_keyboard_ungrab (timestamp);

	widget = GTK_WIDGET (priv->window);
	gtk_grab_remove (widget);

	g_object_unref (priv->window);

	priv->embed = NULL;
	priv->active = FALSE;

	g_object_unref (scroller);
}

EphyPushScroller *
ephy_push_scroller_new (EphyWindow *window)
{
	return EPHY_PUSH_SCROLLER (g_object_new (EPHY_TYPE_PUSH_SCROLLER,
				    		 "window", window,
						 NULL));
}

/* class implementation */

static void 
ephy_push_scroller_init (EphyPushScroller *scroller)
{
	EphyPushScrollerPrivate *priv;
	priv = scroller->priv = EPHY_PUSH_SCROLLER_GET_PRIVATE (scroller);
	priv->active = FALSE;
	priv->cursor = gdk_cursor_new (GDK_FLEUR);
}

static void
ephy_push_scroller_finalize (GObject *object)
{
	EphyPushScroller *scroller = EPHY_PUSH_SCROLLER (object);
	EphyPushScrollerPrivate *priv = scroller->priv;

	LOG ("Finalize [%p]", object);

	gdk_cursor_unref (priv->cursor);

	parent_class->finalize (object);
}

static void
ephy_push_scroller_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	EphyPushScroller *scroller = EPHY_PUSH_SCROLLER (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_push_scroller_set_window (scroller, g_value_get_object (value));
			break;
	}
}

static void
ephy_push_scroller_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
ephy_push_scroller_class_init (EphyPushScrollerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_push_scroller_finalize;
	object_class->set_property = ephy_push_scroller_set_property;
	object_class->get_property = ephy_push_scroller_get_property;

	g_object_class_install_property
		(object_class,
		 PROP_WINDOW,
		 g_param_spec_object ("window",
				      "window",
				      "window",
				      EPHY_TYPE_WINDOW,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	

	g_type_class_add_private (klass, sizeof (EphyPushScrollerPrivate));
}

GType
ephy_push_scroller_get_type (void)
{
	return type;
}

GType
ephy_push_scroller_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyPushScrollerClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_push_scroller_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPushScroller),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_push_scroller_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyPushScroller",
					    &our_info, 0);

	return type;
}
