/*
 *  Copyright (C) 2002 Ricardo Fernández Pascual
 *  Copright  (C) 2003 Christian Persch
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
 *  This file is based on work of Daniel Erat for galeon 1.
 *
 *  $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-gesture.h"
#include "ephy-debug.h"

#include "stroke.h"

#include <gdk/gdk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkdnd.h>

#define EPHY_GESTURE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GESTURE, EphyGesturePrivate))

struct _EphyGesturePrivate {
	GtkWidget *window;
	EphyEmbedEvent *event;
	GdkCursor *cursor;
	gboolean started;
};

enum
{
	PROP_0,
	PROP_EVENT,
	PROP_WINDOW
};

enum {
	PERFORMED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static GObjectClass *parent_class = NULL;

static void ephy_gesture_class_init (EphyGestureClass *klass);
static void ephy_gesture_init	    (EphyGesture *gesture);

static GType type = 0;

GType
ephy_gesture_get_type (void)
{
	g_return_val_if_fail (type != 0, 0);

	return type;
}

GType
ephy_gesture_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyGestureClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_gesture_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyGesture),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_gesture_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyGesture",
					    &our_info, 0);

	return type;
}

static void
ephy_gesture_stop (EphyGesture *gesture)
{
	/* ungrab the pointer if it's grabbed */
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (gtk_get_current_event_time ());
	}

	gdk_keyboard_ungrab (gtk_get_current_event_time ());

	gtk_grab_remove (gesture->priv->window);

	/* disconnect all our signals */
	g_signal_handlers_disconnect_matched
		(gesture->priv->window, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, gesture);

	gesture->priv->started = FALSE;
}

static gboolean
motion_cb (GtkWidget *widget,
	   GdkEventMotion *event,
	   EphyGesture *gesture)
{
	stroke_record (event->x_root, event->y_root);

	return TRUE;
}

static gboolean
cancel_cb (GtkWidget *widget,
	   GdkEventButton *event,
	   EphyGesture *gesture)
{
	ephy_gesture_stop (gesture);

	g_object_unref (gesture);

	return TRUE;
}

static gboolean
mouse_release_cb (GtkWidget *widget,
		  GdkEventButton *event,
		  EphyGesture *gesture)
{
	char sequence[STROKE_MAX_SEQUENCE + 1];

	ephy_gesture_stop (gesture);

	if (stroke_trans (sequence) == FALSE)
	{
		/* fake a 'nothing' move, to bring up the menu */
		sequence[0] = '5';
		sequence[1] = '\0';
	}
 
	g_signal_emit (gesture, signals[PERFORMED], 0, sequence);

	g_object_unref (gesture);

	return TRUE;
}

void
ephy_gesture_start (EphyGesture *gesture)
{
	EphyGesturePrivate *p = gesture->priv;
	GtkWidget *window = p->window;

	/* get a new cursor, if necessary */
	p->cursor = gdk_cursor_new (GDK_PENCIL);

	/* grab the pointer as soon as possible, we might miss button_release
	 * otherwise */
	gdk_pointer_grab (window->window, FALSE,
			  GDK_POINTER_MOTION_MASK |
			  GDK_BUTTON_RELEASE_MASK |
			  GDK_BUTTON_PRESS_MASK,
			  NULL, p->cursor, gtk_get_current_event_time ());
	g_signal_connect (window, "button_release_event",
			  G_CALLBACK (mouse_release_cb), gesture);

	/* init stroke */
	stroke_init ();

	/* attach signals */
	g_signal_connect (window, "motion_notify_event",
			  G_CALLBACK (motion_cb), gesture);
	g_signal_connect (window, "button_press_event",
			  G_CALLBACK (cancel_cb), gesture);
	g_signal_connect (window, "key_press_event",
			  G_CALLBACK (cancel_cb), gesture);

	gtk_grab_add (window);
	gdk_keyboard_grab (window->window, FALSE, gtk_get_current_event_time ());

	p->started = TRUE;
}

static void 
ephy_gesture_init (EphyGesture *gesture)
{
	gesture->priv = EPHY_GESTURE_GET_PRIVATE (gesture);

	gesture->priv->window = NULL;
	gesture->priv->event = NULL;
	gesture->priv->cursor = NULL;
	gesture->priv->started = FALSE;
}

static void
ephy_gesture_finalize (GObject *object)
{
	EphyGesture *gesture = EPHY_GESTURE (object);

	if (gesture->priv->started)
	{
		ephy_gesture_stop (gesture);
	}

	if (gesture->priv->cursor)
	{
		gdk_cursor_unref (gesture->priv->cursor);
	}

	g_object_unref (gesture->priv->window);
	g_object_unref (gesture->priv->event);

	LOG ("EphyGesture finalised %p", object)

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_gesture_set_property (GObject *object,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	EphyGesture *gesture = EPHY_GESTURE (object);

	switch (prop_id)
	{
		case PROP_EVENT:
			gesture->priv->event = g_object_ref (g_value_get_object (value));
			break;
		case PROP_WINDOW:
			gesture->priv->window = g_object_ref (g_value_get_object (value));
			break;
	}
}

static void
ephy_gesture_get_property (GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
	EphyGesture *gesture = EPHY_GESTURE (object);

	switch (prop_id)
	{
		case PROP_EVENT:
			g_value_set_object (value, gesture->priv->event);
			break;
		case PROP_WINDOW:
			g_value_set_object (value, gesture->priv->window);
			break;
	}
}

static void
ephy_gesture_class_init (EphyGestureClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

	object_class->finalize = ephy_gesture_finalize;
	object_class->get_property = ephy_gesture_get_property;
	object_class->set_property = ephy_gesture_set_property;

	signals[PERFORMED] =
		g_signal_new ("gesture-performed",
			      G_OBJECT_CLASS_TYPE (klass),  
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EphyGestureClass, performed), 
			      NULL, NULL, 
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	g_object_class_install_property (object_class,
					 PROP_EVENT,
					 g_param_spec_object ("event",
							      "Event",
							      "Event",
							      G_TYPE_OBJECT,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							      "Window",
							      "Parent window",
							      GTK_TYPE_WIDGET,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (EphyGesturePrivate));
}

GtkWidget *
ephy_gesture_get_window (EphyGesture *gesture)
{
	return gesture->priv->window;
}

EphyEmbedEvent *
ephy_gesture_get_event (EphyGesture *gesture)
{
	return gesture->priv->event;
}

EphyGesture *
ephy_gesture_new (GtkWidget *window,
		  EphyEmbedEvent *event)
{
	return EPHY_GESTURE (g_object_new (EPHY_TYPE_GESTURE,
					   "event", event,
					   "window", window,
					   NULL));
}
