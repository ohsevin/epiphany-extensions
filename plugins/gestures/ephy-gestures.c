/*
 *  Copyright (C) 2002  Ricardo Fernández Pascual
 *  Copright  (C) 2003  Christian Persch
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

#include "ephy-gestures.h"
#include "ephy-debug.h"

#include "stroke.h"

#include <gtk/gtkmain.h>
#include <gtk/gtkdnd.h>
#include <string.h>

/**
 * Private data
 */
 
#define EPHY_GESTURES_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GESTURES, EphyGesturesPrivate))

struct _EphyGesturesPrivate {
	GtkWidget *widget;
	guint button;
	guint start_x, start_y;
	guint autocancel_timeout;
	guint autocancel;
	gboolean started;
};

/**
 * Private functions, only availble from this file
 */
static void		ephy_gestures_class_init	(EphyGesturesClass *klass);
static void		ephy_gestures_init		(EphyGestures *eg);
static void		ephy_gestures_finalize		(GObject *o);
static gboolean		ephy_gestures_motion_cb		(GtkWidget *widget,
							 GdkEventMotion *e,
							 EphyGestures *eg);
static gboolean		ephy_gestures_mouse_press_cb	(GtkWidget *widget,
							 GdkEventButton *e,
							 EphyGestures *eg);
static gboolean		ephy_gestures_mouse_release_cb	(GtkWidget *widget,
							 GdkEventButton *e,
							 EphyGestures *eg);
static gboolean		ephy_gestures_key_press_cb	(GtkWidget *widget,
							 GdkEventKey *e,
							 EphyGestures *eg);
static void		ephy_gestures_stop		(EphyGestures *eg);
static void		ephy_gestures_start_autocancel	(EphyGestures *eg);

enum {
	PERFORMED,
	CANCELLED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static GObjectClass *parent_class = NULL;

/**
 * Gestures object
 */

GType
ephy_gestures_get_type (void)
{
	static GType ephy_gestures_type = 0;

	if (ephy_gestures_type == 0)
	{
		static const GTypeInfo our_info =
		{
			sizeof (EphyGesturesClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) ephy_gestures_class_init,
			NULL,
			NULL, /* class_data */
			sizeof (EphyGestures),
			0, /* n_preallocs */
			(GInstanceInitFunc) ephy_gestures_init
		};

		ephy_gestures_type = g_type_register_static (G_TYPE_OBJECT,
							     "EphyGestures",
							     &our_info, 0);
        }

        return ephy_gestures_type;
}

static void
ephy_gestures_class_init (EphyGesturesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_gestures_finalize;

	signals[PERFORMED] =
		g_signal_new ("gesture-performed",
			      G_OBJECT_CLASS_TYPE (klass),  
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_RUN_LAST | G_SIGNAL_RUN_CLEANUP,
			      G_STRUCT_OFFSET (EphyGesturesClass, performed), 
			      NULL, NULL, 
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	signals[CANCELLED] =
		g_signal_new ("cancelled",
			      G_OBJECT_CLASS_TYPE (klass),  
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_RUN_LAST | G_SIGNAL_RUN_CLEANUP,
                	      G_STRUCT_OFFSET (EphyGesturesClass, cancelled), 
			      NULL, NULL, 
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	g_type_class_add_private (object_class, sizeof (EphyGesturesPrivate));
}

static void 
ephy_gestures_init (EphyGestures *eg)
{
	eg->priv = EPHY_GESTURES_GET_PRIVATE (eg);
}

static void
ephy_gestures_finalize (GObject *o)
{
	EphyGestures *eg = EPHY_GESTURES (o);
	
	if (eg->priv->autocancel_timeout)
	{
		g_source_remove (eg->priv->autocancel_timeout);
	}

	LOG ("EphyGestures finalised %p", o)

	parent_class->finalize (o);
}

EphyGestures *
ephy_gestures_new (void)
{
	return g_object_new (EPHY_TYPE_GESTURES, NULL);
}

void
ephy_gestures_start (EphyGestures *eg, GtkWidget *widget, guint button, gint x, gint y)
{
	static GdkCursor *cursor = NULL;
	EphyGesturesPrivate *p = eg->priv;

	g_object_ref (eg);

	p->widget = g_object_ref (widget);
	p->button = button;

	/* get a new cursor, if necessary */
	if (!cursor) cursor = gdk_cursor_new (GDK_PENCIL);

	/* grab the pointer as soon as possible, we might miss button_release
	 * otherwise */
	gdk_pointer_grab (widget->window, FALSE,
			  GDK_POINTER_MOTION_MASK |
			  GDK_BUTTON_RELEASE_MASK |
			  GDK_BUTTON_PRESS_MASK,
			  NULL, cursor, gtk_get_current_event_time ());
	g_signal_connect (widget, "button_release_event",
			  G_CALLBACK (ephy_gestures_mouse_release_cb), eg);

	/* init stroke */
	stroke_init ();

	/* attach signals */
	g_signal_connect (widget, "motion_notify_event",
			  G_CALLBACK (ephy_gestures_motion_cb), eg);
	g_signal_connect (widget, "button_press_event",
			  G_CALLBACK (ephy_gestures_mouse_press_cb), eg);
	g_signal_connect (widget, "key_press_event",
			  G_CALLBACK (ephy_gestures_key_press_cb), eg);

	gtk_grab_add (widget);
	gdk_keyboard_grab (widget->window, FALSE, gtk_get_current_event_time ());

	p->start_x = x;
	p->start_y = y;

	p->started = TRUE;

	ephy_gestures_start_autocancel (eg);
}

static gboolean
ephy_gestures_motion_cb (GtkWidget *widget, GdkEventMotion *e,
			EphyGestures *eg)
{
	EphyGesturesPrivate *p = eg->priv;

	if (p->autocancel_timeout)
	{
		if (gtk_drag_check_threshold (p->widget, 
					      p->start_x, p->start_y,
					      e->x_root, e->y_root))
		{
			g_source_remove (p->autocancel_timeout);
			p->autocancel_timeout = 0;
		}
	}

        stroke_record (e->x_root, e->y_root);
	return TRUE;
}

static gboolean
ephy_gestures_mouse_press_cb (GtkWidget *widget, GdkEventButton *e,
			     EphyGestures *eg)
{
	/* ungrab and disconnect */
	ephy_gestures_stop (eg);

	return TRUE;
}

static gboolean
ephy_gestures_mouse_release_cb (GtkWidget *widget, GdkEventButton *e,
			       EphyGestures *eg)
{
	char sequence[STROKE_MAX_SEQUENCE + 1];

	g_object_ref (eg);

	/* ungrab and disconnect */
	ephy_gestures_stop (eg);

        /* handle gestures */
	if (!stroke_trans (sequence) == TRUE)
	{
		strcpy (sequence, "5"); /* fake a 'nothing' move, to bring up the menu */
	}
 
	g_signal_emit (eg, signals[PERFORMED], 0, sequence);

	g_object_unref (eg);

	return TRUE;
}

static gboolean
ephy_gestures_key_press_cb (GtkWidget *widget, GdkEventKey *e,
			   EphyGestures *eg)
{
	/* ungrab and disconnect */
	ephy_gestures_stop (eg);

	return TRUE;
}

static void
ephy_gestures_stop (EphyGestures *eg)
{
	EphyGesturesPrivate *p = eg->priv;

	/* ungrab the pointer if it's grabbed */
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (gtk_get_current_event_time ());
	}

	gdk_keyboard_ungrab (gtk_get_current_event_time ());

	g_return_if_fail (p->widget);
	
	gtk_grab_remove (p->widget);

	/* disconnect all of the signals */
	g_signal_handlers_disconnect_matched (p->widget, G_SIGNAL_MATCH_DATA, 0, 0, 
					      NULL, NULL, eg);

	g_object_unref (p->widget);
	p->widget = NULL;
	p->started = FALSE;

	g_object_unref (eg);
}

static gboolean
ephy_gestures_autocancel_to (gpointer data)
{
	EphyGestures *eg = data;
	EphyGesturesPrivate *p = eg->priv;
	
	if (p->started)
	{
		g_object_ref (eg);

		ephy_gestures_stop (eg);

		g_signal_emit (eg, signals[CANCELLED], 0);

		g_object_unref (eg);
	}

	return FALSE;
}

#define AUTOCANCEL_TIMEOUT 75

static void
ephy_gestures_start_autocancel (EphyGestures *eg)
{
	EphyGesturesPrivate *p = eg->priv;

	if (p->autocancel_timeout)
	{
		g_source_remove (p->autocancel_timeout);
		p->autocancel_timeout = 0;
	}

	if (p->started && p->autocancel)
	{
		p->autocancel_timeout = g_timeout_add (AUTOCANCEL_TIMEOUT, 
						       ephy_gestures_autocancel_to, eg);
	}
}

void 
ephy_gestures_set_autocancel (EphyGestures *eg, gboolean autocancel)
{
	EphyGesturesPrivate *p = eg->priv;

	p->autocancel = autocancel;
	ephy_gestures_start_autocancel (eg);
}
