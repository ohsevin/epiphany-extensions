/*
 *  Copyright (C) 2002  Ricardo Fernández Pascual
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
 */

/* this file is based on work of Daniel Erat for galeon 1 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gul-gestures.h"
#include "stroke.h"
#include <gtk/gtkmain.h>
#include <gtk/gtkdnd.h>
#include <string.h>

#define NOT_IMPLEMENTED g_warning ("not implemented: " G_STRLOC);

//#define DEBUG_MSG(x) g_print x
#define DEBUG_MSG(x)

/**
 * Private data
 */
struct _GulGesturesPrivate {
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
static void		gul_gestures_class_init		(GulGesturesClass *klass);
static void		gul_gestures_init		(GulGestures *as);
static void		gul_gestures_finalize_impl	(GObject *o);
static gboolean		gul_gestures_motion_cb		(GtkWidget *widget,
							 GdkEventMotion *e,
							 GulGestures *as);
static gboolean		gul_gestures_mouse_press_cb	(GtkWidget *widget,
							 GdkEventButton *e,
							 GulGestures *as);
static gboolean		gul_gestures_mouse_release_cb	(GtkWidget *widget,
							 GdkEventButton *e,
							 GulGestures *as);
static gboolean		gul_gestures_key_press_cb	(GtkWidget *widget,
							 GdkEventKey *e,
							 GulGestures *as);
static void		gul_gestures_stop		(GulGestures *as);
static void		gul_gestures_start_autocancel	(GulGestures *ges);

static GObjectClass *parent_class = NULL;

/* signals enums and ids */
enum GulGesturesSignalsEnum {
	GUL_GESTURES_GESTURE_PERFORMED,
	GUL_GESTURES_CANCELLED,
	GUL_GESTURES_LAST_SIGNAL
};
static gint GulGesturesSignals[GUL_GESTURES_LAST_SIGNAL];

/**
 * Gestures object
 */

GType
gul_gestures_get_type (void)
{
        static GType gul_gestures_type = 0;

        if (gul_gestures_type == 0)
        {
                static const GTypeInfo our_info =
                {
                        sizeof (GulGesturesClass),
                        NULL, /* base_init */
                        NULL, /* base_finalize */
                        (GClassInitFunc) gul_gestures_class_init,
                        NULL,
                        NULL, /* class_data */
                        sizeof (GulGestures),
                        0, /* n_preallocs */
                        (GInstanceInitFunc) gul_gestures_init
                };

		gul_gestures_type = g_type_register_static (G_TYPE_OBJECT,
							    "GulGestures",
							    &our_info, 0);
        }

        return gul_gestures_type;
}

static void
gul_gestures_class_init (GulGesturesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gul_gestures_finalize_impl;

	GulGesturesSignals[GUL_GESTURES_GESTURE_PERFORMED] = g_signal_new (
		"gesture-performed", G_OBJECT_CLASS_TYPE (klass),  
		G_SIGNAL_RUN_FIRST | G_SIGNAL_RUN_LAST | G_SIGNAL_RUN_CLEANUP,
                G_STRUCT_OFFSET (GulGesturesClass, gesture_performed), 
		NULL, NULL, 
		g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);

	GulGesturesSignals[GUL_GESTURES_CANCELLED] = g_signal_new (
		"cancelled", G_OBJECT_CLASS_TYPE (klass),  
		G_SIGNAL_RUN_FIRST | G_SIGNAL_RUN_LAST | G_SIGNAL_RUN_CLEANUP,
                G_STRUCT_OFFSET (GulGesturesClass, gesture_performed), 
		NULL, NULL, 
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
}

static void 
gul_gestures_init (GulGestures *e)
{
	GulGesturesPrivate *p = g_new0 (GulGesturesPrivate, 1);
	e->priv = p;
}

static void
gul_gestures_finalize_impl (GObject *o)
{
	GulGestures *as = GUL_GESTURES (o);
	GulGesturesPrivate *p = as->priv;

	DEBUG_MSG (("in gul_gestures_finalize_impl\n"));
	
	if (p->autocancel_timeout)
	{
		g_source_remove (p->autocancel_timeout);
	}

	g_free (p);

	parent_class->finalize (o);
}

GulGestures *
gul_gestures_new (void)
{
	GulGestures *ret = g_object_new (GUL_TYPE_GESTURES, NULL);
	return ret;
}

void
gul_gestures_start (GulGestures *as, GtkWidget *widget, guint button, gint x, gint y)
{
	static GdkCursor *cursor = NULL;
	GulGesturesPrivate *p = as->priv;

	g_object_ref (as);

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
			  G_CALLBACK (gul_gestures_mouse_release_cb), as);

	/* init stroke */
	stroke_init ();

	/* attach signals */
	g_signal_connect (widget, "motion_notify_event",
			  G_CALLBACK (gul_gestures_motion_cb), as);
	g_signal_connect (widget, "button_press_event",
			  G_CALLBACK (gul_gestures_mouse_press_cb), as);
	g_signal_connect (widget, "key_press_event",
			  G_CALLBACK (gul_gestures_key_press_cb), as);

	gtk_grab_add (widget);
	gdk_keyboard_grab (widget->window, FALSE, gtk_get_current_event_time ());

	p->start_x = x;
	p->start_y = y;

	p->started = TRUE;

	gul_gestures_start_autocancel (as);
}

static gboolean
gul_gestures_motion_cb (GtkWidget *widget, GdkEventMotion *e,
			GulGestures *as)
{
	GulGesturesPrivate *p = as->priv;

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
gul_gestures_mouse_press_cb (GtkWidget *widget, GdkEventButton *e,
			     GulGestures *as)
{
	/* ungrab and disconnect */
	gul_gestures_stop (as);

	return TRUE;
}

static gboolean
gul_gestures_mouse_release_cb (GtkWidget *widget, GdkEventButton *e,
			       GulGestures *as)
{
	char sequence[STROKE_MAX_SEQUENCE + 1];

	g_object_ref (as);

	/* ungrab and disconnect */
	gul_gestures_stop (as);

        /* handle gestures */
	if (!stroke_trans (sequence) == TRUE)
	{
		strcpy (sequence, "5"); /* fake a 'nothing' move, to bring up the menu */
	}
 
	g_signal_emit (as, GulGesturesSignals[GUL_GESTURES_GESTURE_PERFORMED], 0, sequence);

	g_object_unref (as);

	return TRUE;
}

static gboolean
gul_gestures_key_press_cb (GtkWidget *widget, GdkEventKey *e,
			   GulGestures *as)
{
	/* ungrab and disconnect */
	gul_gestures_stop (as);

	return TRUE;
}

static void
gul_gestures_stop (GulGestures *as)
{
	GulGesturesPrivate *p = as->priv;

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
					      NULL, NULL, as);

	g_object_unref (p->widget);
	p->widget = NULL;
	p->started = FALSE;

	g_object_unref (as);
}

static gboolean
gul_gestures_autocancel_to (gpointer data)
{
	GulGestures *ges = data;
	GulGesturesPrivate *p = ges->priv;
	
	if (p->started)
	{
		g_object_ref (ges);

		gul_gestures_stop (ges);

		g_signal_emit (ges, GulGesturesSignals[GUL_GESTURES_CANCELLED], 0);

		g_object_unref (ges);
	}

	return FALSE;
}

#define AUTOCANCEL_TIMEOUT 75

static void
gul_gestures_start_autocancel (GulGestures *ges)
{
	GulGesturesPrivate *p = ges->priv;

	if (p->autocancel_timeout)
	{
		g_source_remove (p->autocancel_timeout);
		p->autocancel_timeout = 0;
	}

	if (p->started && p->autocancel)
	{
		p->autocancel_timeout = g_timeout_add (AUTOCANCEL_TIMEOUT, 
						       gul_gestures_autocancel_to, ges);
	}
}

void 
gul_gestures_set_autocancel (GulGestures *ges, gboolean autocancel)
{
	GulGesturesPrivate *p = ges->priv;
	p->autocancel = autocancel;
	gul_gestures_start_autocancel (ges);
}
