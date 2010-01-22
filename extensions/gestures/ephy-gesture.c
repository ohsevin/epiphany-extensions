/*
 *  Copyright © 2002 Ricardo Fernández Pascual
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

#include "config.h"

#include "ephy-gesture.h"
#include "ephy-debug.h"

#include "stroke.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <epiphany/epiphany.h>

#include <string.h>

#define EPHY_GESTURE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GESTURE, EphyGesturePrivate))

struct _EphyGesturePrivate
{
	GtkWidget *window;
	GdkEventButton *event;
	GdkCursor *cursor;
	GtkAction *current_action;
	guint timeout_id;
	guint started : 1;
};

enum
{
	PROP_0,
	PROP_EVENT,
	PROP_WINDOW
};

enum
{
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
	const GTypeInfo our_info =
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

void
ephy_gesture_stop (EphyGesture *gesture,
		   guint32 time)
{
	EphyGesturePrivate *priv = gesture->priv;
	GtkWidget *child;

	if (priv->started == FALSE) return;

	/* disconnect our signals before ungrabbing! */
	g_signal_handlers_disconnect_matched
		(priv->window, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, gesture);

	child = gtk_bin_get_child (GTK_BIN (priv->window));
	g_signal_handlers_disconnect_matched
		(child, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, gesture);

	/* ungrab the pointer if it's grabbed */
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (time);
	}

	gdk_keyboard_ungrab (time);

	gtk_grab_remove (priv->window);

	priv->started = FALSE;

	g_object_unref (priv->window);
	g_object_unref (gesture);
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
button_press_event_cb (GtkWidget *widget,
		       GdkEventButton *event,
		       EphyGesture *gesture)
{
	ephy_gesture_stop (gesture, event->time);

	return TRUE;
}

static gboolean
key_press_event_cb (GtkWidget *widget,
		    GdkEventKey *event,
		    EphyGesture *gesture)
{
	ephy_gesture_stop (gesture, event->time);

	return TRUE;
}

static gboolean
mouse_release_cb (GtkWidget *widget,
		  GdkEventButton *event,
		  EphyGesture *gesture)
{
	char sequence[STROKE_MAX_SEQUENCE + 1];

	g_object_ref (gesture);

	ephy_gesture_stop (gesture, event->time);

	if (stroke_trans (sequence) == FALSE)
	{
		/* fake a 'nothing' move, to bring up the menu */
		sequence[0] = '5';
		sequence[1] = '\0';
	}
 
	g_signal_emit (gesture, signals[PERFORMED], 0, sequence);

	ephy_gesture_set_event (gesture, NULL);

	g_object_unref (gesture);

	return TRUE;
}

static gboolean
unmap_event_cb (GtkWidget *width,
		GdkEventAny *event,
		EphyGesture *gesture)
{
	/* ungrab and disconnect */
	ephy_gesture_stop (gesture, GDK_CURRENT_TIME /* FIXME? */);

	return FALSE;
}

static gboolean
grab_broken_event_cb (GtkWidget *widget,
		      GdkEventGrabBroken *event,
		      EphyGesture *gesture)
{
	ephy_gesture_stop (gesture, GDK_CURRENT_TIME /* FIXME? */);

	return FALSE;
}

static void
grab_notify_cb (GtkWidget *widget,
		gboolean was_grabbed,
		EphyGesture *gesture)
{
	if (was_grabbed)
	{
		ephy_gesture_stop (gesture, GDK_CURRENT_TIME /* FIXME? */);
	}
}

gboolean
ephy_gesture_start (EphyGesture *gesture)
{
	EphyGesturePrivate *priv = gesture->priv;
	GtkWidget *child;
	guint32 time;

	g_object_ref (gesture);
	priv->started = TRUE;

	time = gtk_get_current_event_time ();

	/* attach signals */
	g_signal_connect (priv->window, "button-release-event",
			  G_CALLBACK (mouse_release_cb), gesture);
	g_signal_connect (priv->window, "motion-notify-event",
			  G_CALLBACK (motion_cb), gesture);
	g_signal_connect (priv->window, "button-press-event",
			  G_CALLBACK (button_press_event_cb), gesture);
	g_signal_connect (priv->window, "key-press-event",
			  G_CALLBACK (key_press_event_cb), gesture);
	g_signal_connect (priv->window, "unmap-event",
			  G_CALLBACK (unmap_event_cb), gesture);
	g_signal_connect (priv->window, "grab-broken-event",
			  G_CALLBACK (grab_broken_event_cb), gesture);

	child = gtk_bin_get_child (GTK_BIN (priv->window));
	g_signal_connect (child, "grab-notify",
			  G_CALLBACK (grab_notify_cb), gesture);

	/* get a new cursor, if necessary */
	priv->cursor = gdk_cursor_new (GDK_PENCIL);

	/* init stroke */
	stroke_init ();

	g_object_ref (priv->window);
	gtk_grab_add (priv->window);

	if (gdk_pointer_grab (priv->window->window, FALSE,
			     GDK_POINTER_MOTION_MASK |
			     GDK_BUTTON_RELEASE_MASK |
			     GDK_BUTTON_PRESS_MASK,
			     NULL, priv->cursor, time) != GDK_GRAB_SUCCESS ||
	    gdk_keyboard_grab (priv->window->window, FALSE, time) != GDK_GRAB_SUCCESS)
	{
		ephy_gesture_stop (gesture, time);

		return FALSE;
	}

	return TRUE;
}

static void
ephy_gesture_init (EphyGesture *gesture)
{
	EphyGesturePrivate *priv;

	LOG ("Init [%p]", gesture);

	priv = gesture->priv = EPHY_GESTURE_GET_PRIVATE (gesture);

	priv->window = NULL;
	priv->event = NULL;
	priv->cursor = NULL;
	priv->current_action = NULL;
	priv->timeout_id = 0;
	priv->started = FALSE;
}

static void
ephy_gesture_finalize (GObject *object)
{
	EphyGesture *gesture = EPHY_GESTURE (object);
	EphyGesturePrivate *priv = gesture->priv;

	LOG ("Finalise [%p]", object);

	if (priv->started)
	{
		ephy_gesture_stop (gesture, GDK_CURRENT_TIME /* FIXME? */);
	}

	if (priv->cursor != NULL)
	{
		gdk_cursor_unref (gesture->priv->cursor);
	}

	if (priv->timeout_id)
	{
		g_source_remove(priv->timeout_id);
	}

	ephy_gesture_set_event (gesture, NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_gesture_set_property (GObject *object,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	EphyGesture *gesture = EPHY_GESTURE (object);
	EphyGesturePrivate *priv = gesture->priv;

	switch (prop_id)
	{
		case PROP_EVENT:
			ephy_gesture_set_event (gesture, g_value_get_object (value));
			break;
		case PROP_WINDOW:
			priv->window = g_value_get_object (value);
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
	EphyGesturePrivate *priv = gesture->priv;

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, priv->window);
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

GdkEventButton *
ephy_gesture_get_event (EphyGesture *gesture)
{
	EphyGesturePrivate *priv;

	g_return_val_if_fail (EPHY_IS_GESTURE (gesture), NULL);

	priv = gesture->priv;

	return priv->event;
}

void
ephy_gesture_set_event (EphyGesture *gesture,
			GdkEventButton *event)
{
	EphyGesturePrivate *priv;

	g_return_if_fail (EPHY_IS_GESTURE (gesture));

	priv = gesture->priv;
	priv->event = event;
}

static gboolean
ephy_gesture_do_activate_cb (EphyGesture *gesture)
{
	gtk_action_activate (gesture->priv->current_action);

	gesture->priv->current_action = NULL;
	gesture->priv->timeout_id = 0;

	return FALSE;
}

void
ephy_gesture_activate (EphyGesture *gesture,
		       const char *path)
{
	EphyWindow *window = EPHY_WINDOW (ephy_gesture_get_window (gesture));
	g_return_if_fail (EPHY_IS_WINDOW (window));

	if (strcmp (path, "fallback") == 0)
	{
		/* Fall back to normal click */
		EphyEmbed *embed;
		GdkEventButton *event;
		gint handled = FALSE;

		embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
		g_return_if_fail (EPHY_IS_EMBED (embed));

		event = ephy_gesture_get_event (gesture);
		g_return_if_fail (EPHY_IS_EMBED_EVENT (event));

		g_signal_emit_by_name (embed, "button-press-event", event,
				       &handled);
	}
	else
	{
		GtkUIManager *manager;
		GtkAction *action;

		manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

		action = gtk_ui_manager_get_action (manager, path);

		if (action == NULL)
		{
			g_warning ("Action for path '%s' not found!\n", path);
			return;
		}

		gesture->priv->current_action = action;

		/**
		 * We need to activate the action in a timeout callback so
		 * that we do not induce middle-click side-effects in the action.
		 */
		gesture->priv->timeout_id =
			g_timeout_add(0, (GSourceFunc)ephy_gesture_do_activate_cb,
				      gesture);
	}
}

EphyGesture *
ephy_gesture_new (GtkWidget *window)
{
	return EPHY_GESTURE (g_object_new (EPHY_TYPE_GESTURE,
					   "window", window,
					   NULL));
}
