/*
 *  Copyright (C) 2002 Ricardo Fernádez Pascual
 *  Copyright (C) 2005 Crispin Flowerday
 *  Copyright (C) 2005 Christian Persch
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

#include "ephy-auto-scroller.h"
#include "mozilla-helpers.h"
#include "autoscroll.xpm.h"
#include "ephy-debug.h"

#include <gtk/gtkimage.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkmain.h>

#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#define EPHY_AUTO_SCROLLER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_AUTO_SCROLLER, EphyAutoScrollerPrivate))

struct _EphyAutoScrollerPrivate
{
	GtkWidget *window;
	GtkWidget *popup;
	EphyEmbed *embed;
	guint start_x;
	guint start_y;
	gfloat step_x;
	gfloat step_y;
	gfloat roundoff_error_x;
	gfloat roundoff_error_y;
	int msecs;
	guint timeout_id;
	gboolean active;
	GdkCursor *cursor;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

/* private functions */

void
ephy_auto_scroller_set_window (EphyAutoScroller *scroller,
			       GtkWidget *window)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;

	priv->window = window;

	gtk_window_group_add_window (GTK_WINDOW (priv->window)->group,
				     GTK_WINDOW (priv->popup));
}
 
void
ephy_auto_scroller_set_embed (EphyAutoScroller *scroller,
			      EphyEmbed *embed)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;

	if (priv->embed)
	{
		g_object_unref (priv->embed);
	}

	priv->embed = g_object_ref (embed);
}

static gboolean
ephy_auto_scroller_motion_cb (GtkWidget *widget,
			      GdkEventMotion *event,
			      EphyAutoScroller *scroller)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
	gint x_dist, x_dist_abs, y_dist, y_dist_abs;

	if (!priv->active)
	{
		return FALSE;
	}

	/* get distance between scroll center and cursor */
	x_dist = event->x_root - priv->start_x;
	x_dist_abs = abs (x_dist);
	y_dist = event->y_root - priv->start_y;
	y_dist_abs = abs (y_dist);

	/* calculate scroll step */
	if (y_dist_abs <= 48.0)
	{
		priv->step_y = (float) (y_dist / 4) / 6.0;
	}
	else if (y_dist > 0)
	{
		priv->step_y = (y_dist - 48.0) / 2.0 + 2.0;
	}
	else 
	{
		priv->step_y = (y_dist + 48.0) / 2.0 - 2.0;
	}

	if (x_dist_abs <= 48.0)
	{
		priv->step_x = (float) (x_dist / 4) / 6.0;
	}
	else if (x_dist > 0)
	{
		priv->step_x = (x_dist - 48.0) / 2.0 + 2.0;
	}
	else 
	{
		priv->step_x = (x_dist + 48.0) / 2.0 - 2.0;
	}

	return TRUE;
}

static gboolean
ephy_auto_scroller_mouse_press_cb (GtkWidget *widget,
				   GdkEventButton *event,
				   EphyAutoScroller *scroller)
{
	ephy_auto_scroller_stop (scroller, event->time);

	return TRUE;
}

static gboolean
ephy_auto_scroller_key_press_cb (GtkWidget *widget,
				 GdkEventKey *event,
				 EphyAutoScroller *scroller)
{
	ephy_auto_scroller_stop (scroller, event->time);

	return TRUE;
}

static gboolean
ephy_auto_scroller_unmap_event_cb (GtkWidget *widget,
				   GdkEvent *event,
				   EphyAutoScroller *scroller)
{
	ephy_auto_scroller_stop (scroller, gtk_get_current_event_time () /* FIXME what event type is |event| ? */);

	return FALSE;
}

#if 0
static gboolean
ephy_auto_scroller_grab_broken_event_cb (GtkWidget *widget,
					 GdkEventGrabBroken *event,
					 EphyAutoScroller *scroller)
{
	g_print ("Grab broken!\n");

	return FALSE;
}

static void
ephy_auto_scroller_grab_notify_cb (GtkWidget *widget,
				   gboolean was_grabbed,
				   EphyAutoScroller *scroller)
{
	g_print ("grab notify, was-grabbed:%s\n", was_grabbed ? "t":"f");
}
#endif

static gint
ephy_auto_scroller_timeout_cb (gpointer data)
{
	struct timeval start_time, finish_time;
	long elapsed_msecs;
	EphyAutoScroller *scroller = data;
	EphyAutoScrollerPrivate *priv;
	gfloat scroll_step_y_adj;
	gint scroll_step_y_int;
	gfloat scroll_step_x_adj;
	gint scroll_step_x_int;

	g_return_val_if_fail (EPHY_IS_AUTO_SCROLLER (scroller), FALSE);
	priv = scroller->priv;
	g_return_val_if_fail (EPHY_IS_EMBED (priv->embed), FALSE);

	/* return if we're not supposed to scroll */
	if (!priv->step_y && !priv->step_x)
	{
		return TRUE;
	}

	/* calculate the number of pixels to scroll */
	scroll_step_y_adj = priv->step_y * priv->msecs / 33;
	scroll_step_y_int = scroll_step_y_adj;
	priv->roundoff_error_y += (scroll_step_y_adj - scroll_step_y_int);

	if (fabs (priv->roundoff_error_y) >= 1.0)
	{
		scroll_step_y_int += priv->roundoff_error_y;
		priv->roundoff_error_y -= (gint) priv->roundoff_error_y;
	}

	scroll_step_x_adj = priv->step_x * priv->msecs / 33;
	scroll_step_x_int = scroll_step_x_adj;
	priv->roundoff_error_x += (scroll_step_x_adj - scroll_step_x_int);

	if (fabs (priv->roundoff_error_x) >= 1.0)
	{
		scroll_step_x_int += priv->roundoff_error_x;
		priv->roundoff_error_x -= (gint) priv->roundoff_error_x;
	}

	/* exit if we're not supposed to scroll yet */
	if (!scroll_step_y_int && !scroll_step_x_int) return TRUE;
	
	/* get the time before we tell the embed to scroll */
	gettimeofday (&start_time, NULL);

	/* do scrolling, moving at a constart speed regardless of the
	 * scrolling delay */

	/* FIXME: if mozilla is able to do diagonal scrolling in a
	 * reasonable manner at some point, this should be changed to
	 * calculate x and pass both values instead of just y */
	mozilla_helper_fine_scroll (priv->embed, scroll_step_x_int, scroll_step_y_int);

	/* find out how long the scroll took */
	gettimeofday (&finish_time, NULL);
	elapsed_msecs = (1000000L * finish_time.tv_sec + finish_time.tv_usec -
			 1000000L * start_time.tv_sec - start_time.tv_usec) /
			1000;

	/* check if we should update the scroll delay */
	if ((elapsed_msecs >= priv->msecs + 5) ||
	    ((priv->msecs > 20) && (elapsed_msecs < priv->msecs - 10)))
	{
		/* update the scrolling delay, with a
		 * minimum delay of 20 ms */
		priv->msecs = (elapsed_msecs + 10 >= 20) ? elapsed_msecs + 10 : 20;

		/* create new timeout with adjusted delay */
		priv->timeout_id =	g_timeout_add (priv->msecs, ephy_auto_scroller_timeout_cb, scroller);

		/* kill the old timeout */
		return FALSE;
	}

	/* don't kill timeout */
	return TRUE;
}

/* public functions */

void
ephy_auto_scroller_start_scroll (EphyAutoScroller *scroller,
				 int x,
				 int y)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
	guint32 timestamp;

	g_return_if_fail (priv->embed);

	if (priv->active)
	{
		return;
	}
	priv->active = TRUE;

	/* FIXME is this good enough? */
	timestamp = gtk_get_current_event_time ();

	g_object_ref (scroller);

	g_object_ref (priv->window);

	/* FIXME multihead: move to priv->window's display */
	/* FIXME: use hotspot instead of "12" ? or size/2 ? */
	gtk_window_move (GTK_WINDOW (priv->popup), x - 12, y - 12);
	gtk_widget_show (priv->popup);

	/* set positions */
	priv->start_x = x;
	priv->start_y = y;
	priv->step_x = 0;
	priv->step_y = 0;
	priv->roundoff_error_x = 0;
	priv->roundoff_error_y = 0;

	g_signal_connect (priv->window, "motion_notify_event",
			    G_CALLBACK (ephy_auto_scroller_motion_cb), scroller);
	g_signal_connect (priv->window, "button_press_event",
			  G_CALLBACK (ephy_auto_scroller_mouse_press_cb), scroller);
	g_signal_connect (priv->window, "key_press_event",
			  G_CALLBACK (ephy_auto_scroller_key_press_cb), scroller);
	g_signal_connect (priv->window, "unmap-event",
			  G_CALLBACK (ephy_auto_scroller_unmap_event_cb), scroller);

#if 0
	g_signal_connect (priv->window, "grab-broken-event",
			  G_CALLBACK (ephy_auto_scroller_grab_broken_event_cb), scroller);
	g_signal_connect (priv->window, "grab-notify",
			  G_CALLBACK (ephy_auto_scroller_grab_notify_cb), scroller);
#endif

	priv->timeout_id =
		g_timeout_add (priv->msecs,
			       ephy_auto_scroller_timeout_cb, scroller);

	if (gdk_pointer_is_grabbed ()) return;

	/* grab the pointer */
	gtk_grab_add (priv->window);
	gdk_pointer_grab (priv->window->window, FALSE,
			  GDK_POINTER_MOTION_MASK |
			  GDK_BUTTON_PRESS_MASK,
			  NULL, priv->cursor, timestamp);
	gdk_keyboard_grab (priv->window->window, FALSE, timestamp);
}

void
ephy_auto_scroller_stop (EphyAutoScroller *scroller,
			 guint32 timestamp)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;

	/* ungrab the pointer if it's grabbed */
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (timestamp);
	}

	gdk_keyboard_ungrab (timestamp);

	/* hide the icon */
	gtk_widget_hide (priv->popup);

	gtk_grab_remove (priv->window);

	/* disconnect all of the signals */
	g_signal_handlers_disconnect_matched (priv->window, G_SIGNAL_MATCH_DATA, 0, 0, 
					      NULL, NULL, scroller);
	if (priv->timeout_id)
	{
		g_source_remove (priv->timeout_id);
		priv->timeout_id = 0;
	}

	g_object_unref (priv->window);
	priv->window = NULL;

	priv->active = FALSE;
	g_object_unref (scroller);
}

EphyAutoScroller *
ephy_auto_scroller_new (GtkWidget *window)
{
	return EPHY_AUTO_SCROLLER (g_object_new (EPHY_TYPE_AUTO_SCROLLER,
				    		 "window", window,
						 NULL));
}

/* class implementation */

static void 
ephy_auto_scroller_init (EphyAutoScroller *scroller)
{
	EphyAutoScrollerPrivate *priv;
	GdkPixbuf *icon_pixbuf;
	GdkPixmap *icon_pixmap;
	GdkBitmap *icon_bitmap;
	GtkWidget *icon_img;

	priv = scroller->priv = EPHY_AUTO_SCROLLER_GET_PRIVATE (scroller);

	priv->active = FALSE;
	priv->msecs = 33;

	/* initialize the autoscroll icon */
	icon_pixbuf = gdk_pixbuf_new_from_xpm_data (autoscroll_xpm);
	g_return_if_fail (icon_pixbuf);
		
	gdk_pixbuf_render_pixmap_and_mask (icon_pixbuf, &icon_pixmap,
					   &icon_bitmap, 128);
	g_object_unref (icon_pixbuf);

	icon_img = gtk_image_new_from_pixmap (icon_pixmap, icon_bitmap);
	
	priv->popup = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_realize (priv->popup);
	gtk_container_add (GTK_CONTAINER (priv->popup), icon_img);
	gtk_widget_shape_combine_mask (priv->popup, icon_bitmap, 0, 0);
	
	g_object_unref (icon_pixmap);
	g_object_unref (icon_bitmap);

	gtk_widget_show_all (icon_img);

	/* get a new cursor, if necessary */
	priv->cursor = gdk_cursor_new (GDK_FLEUR);
}

static void
ephy_auto_scroller_finalize (GObject *object)
{
	EphyAutoScroller *scroller = EPHY_AUTO_SCROLLER (object);
	EphyAutoScrollerPrivate *priv = scroller->priv;

	LOG ("in ephy_auto_scroller_finalize_impl");

	if (priv->embed != NULL)
	{
		g_object_unref (priv->embed);
	}

	if (priv->timeout_id != 0)
	{
		g_source_remove (priv->timeout_id);
		priv->timeout_id = 0;
	}

	gtk_widget_destroy (priv->popup);
	gdk_cursor_unref (priv->cursor);

	parent_class->finalize (object);
}

static void
ephy_auto_scroller_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	EphyAutoScroller *scroller = EPHY_AUTO_SCROLLER (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_auto_scroller_set_window (scroller, g_value_get_object (value));
			break;
	}
}

static void
ephy_auto_scroller_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
ephy_auto_scroller_class_init (EphyAutoScrollerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_auto_scroller_finalize;
	object_class->set_property = ephy_auto_scroller_set_property;
	object_class->get_property = ephy_auto_scroller_get_property;

	g_object_class_install_property
		(object_class,
		 PROP_WINDOW,
		 g_param_spec_object ("window",
				      "window",
				      "window",
				      GTK_TYPE_WINDOW,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	

	g_type_class_add_private (klass, sizeof (EphyAutoScrollerPrivate));
}

GType
ephy_auto_scroller_get_type (void)
{
	return type;
}

GType
ephy_auto_scroller_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyAutoScrollerClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_auto_scroller_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyAutoScroller),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_auto_scroller_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAutoScroller",
					    &our_info, 0);

	return type;
}
