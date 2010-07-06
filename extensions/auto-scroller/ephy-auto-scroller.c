/*
 *  Copyright © 2002 Ricardo Fernádez Pascual
 *  Copyright © 2005 Crispin Flowerday
 *  Copyright © 2005 Christian Persch
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
 */

/* this file is based on work of Daniel Erat for galeon 1 */

#include "config.h"

#include "ephy-auto-scroller.h"
#include "autoscroll.xpm.h"
#include "ephy-debug.h"

#include <gtk/gtk.h>

#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#define TIMEOUT_DELAY 33 /* ms */

#define EPHY_AUTO_SCROLLER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_AUTO_SCROLLER, EphyAutoScrollerPrivate))

struct _EphyAutoScrollerPrivate
{
	EphyWindow *window;
	EphyEmbed *embed;
	GtkWidget *popup;
	GdkCursor *cursor;
	float step_x;
	float step_y;
	float roundoff_error_x;
	float roundoff_error_y;
	int msecs;
	guint start_x;
	guint start_y;
	guint timeout_id;
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
ephy_auto_scroller_set_window (EphyAutoScroller *scroller,
			       EphyWindow *window)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
	GtkWindowGroup *group;

	priv->window = window;
	group = gtk_window_get_group (GTK_WINDOW (priv->window));

	gtk_window_group_add_window (group, GTK_WINDOW (priv->popup));
}

static gboolean
ephy_auto_scroller_motion_cb (GtkWidget *widget,
			      GdkEventMotion *event,
			      EphyAutoScroller *scroller)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
	int x_dist, x_dist_abs, y_dist, y_dist_abs;

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

/* FIXME: I think this is a subcase of grab-broken now? */
static gboolean
ephy_auto_scroller_unmap_event_cb (GtkWidget *widget,
				   GdkEventAny *event,
				   EphyAutoScroller *scroller)
{
	ephy_auto_scroller_stop (scroller, gtk_get_current_event_time ());

	return FALSE;
}

static gboolean
ephy_auto_scroller_grab_broken_event_cb (GtkWidget *widget,
					 GdkEventGrabBroken *event,
					 EphyAutoScroller *scroller)
{
	LOG ("Grab Broken [%p, window %p]", scroller, scroller->priv->window);

	ephy_auto_scroller_stop (scroller, GDK_CURRENT_TIME /* FIXME? */);

	return FALSE;
}

static void
ephy_auto_scroller_grab_notify_cb (GtkWidget *widget,
				   gboolean was_grabbed,
				   EphyAutoScroller *scroller)
{
	LOG ("Grab Notify [%p, window %p]", scroller, scroller->priv->window);

	if (was_grabbed)
	{
		ephy_auto_scroller_stop (scroller, GDK_CURRENT_TIME /* FIXME? */);
	}
}

static void
ephy_auto_scroller_scroll_pixels (EphyEmbed *embed, int scroll_x, int scroll_y)
{
	GtkAdjustment *adj;
	gdouble value;
	gdouble new_value;
	gdouble page_size;
	gdouble upper;
	gdouble lower;
	GtkWidget *sw;

	sw = gtk_widget_get_parent (GTK_WIDGET (ephy_embed_get_web_view (embed)));
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (sw));

	adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (sw));
	upper = gtk_adjustment_get_upper (adj);
	lower = gtk_adjustment_get_lower (adj);
	value = gtk_adjustment_get_value (adj);
	page_size = gtk_adjustment_get_page_size (adj);

	new_value = CLAMP (value + scroll_x, lower, upper - page_size);
	gtk_adjustment_set_value (adj, new_value);

	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sw));
	upper = gtk_adjustment_get_upper (adj);
	lower = gtk_adjustment_get_lower (adj);
	value = gtk_adjustment_get_value (adj);
	page_size = gtk_adjustment_get_page_size (adj);

	new_value = CLAMP (value + scroll_y, lower, upper - page_size);
	gtk_adjustment_set_value (adj, new_value);
}

static int
ephy_auto_scroller_timeout_cb (EphyAutoScroller *scroller)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
	struct timeval start_time, finish_time;
	long elapsed_msecs;
	float scroll_step_x_adj, scroll_step_y_adj;
	int scroll_step_y_int, scroll_step_x_int;

	g_return_val_if_fail (priv->embed != NULL, FALSE);

	/* return if we're not supposed to scroll */
	if (priv->step_y == 0 && priv->step_x == 0)
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
		priv->roundoff_error_y -= (int) priv->roundoff_error_y;
	}

	scroll_step_x_adj = priv->step_x * priv->msecs / 33;
	scroll_step_x_int = scroll_step_x_adj;
	priv->roundoff_error_x += (scroll_step_x_adj - scroll_step_x_int);

	if (fabs (priv->roundoff_error_x) >= 1.0)
	{
		scroll_step_x_int += priv->roundoff_error_x;
		priv->roundoff_error_x -= (int) priv->roundoff_error_x;
	}

	/* exit if we're not supposed to scroll yet */
	if (!scroll_step_y_int && !scroll_step_x_int) return TRUE;

	/* get the time before we tell the embed to scroll */
	gettimeofday (&start_time, NULL);

	/* do scrolling, moving at a constart speed regardless of the
	 * scrolling delay */

	ephy_auto_scroller_scroll_pixels (priv->embed, scroll_step_x_int, scroll_step_y_int);

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
		priv->timeout_id =
			g_timeout_add (priv->msecs,
				       (GSourceFunc) ephy_auto_scroller_timeout_cb,
				       scroller);

		/* don't run the old timeout again */
		return FALSE;
	}

	/* run again */
	return TRUE;
}

/* public functions */

void
ephy_auto_scroller_start (EphyAutoScroller *scroller,
			  EphyEmbed *embed,
			  gdouble x,
			  gdouble y)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
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

	g_signal_connect (priv->window, "motion-notify-event",
			  G_CALLBACK (ephy_auto_scroller_motion_cb), scroller);
	g_signal_connect (priv->window, "button-press-event",
			  G_CALLBACK (ephy_auto_scroller_mouse_press_cb), scroller);
	g_signal_connect (priv->window, "key-press-event",
			  G_CALLBACK (ephy_auto_scroller_key_press_cb), scroller);
	g_signal_connect (priv->window, "unmap-event",
			  G_CALLBACK (ephy_auto_scroller_unmap_event_cb), scroller);

	g_signal_connect (priv->window, "grab-broken-event",
			  G_CALLBACK (ephy_auto_scroller_grab_broken_event_cb), scroller);

	/* FIXME: this signal only seems to be emitted on the container children of GtkWindow,
	 * not on GtkWindow itself... is there a better way to get notified of new grabs?
	 */
	child = gtk_bin_get_child (GTK_BIN (priv->window));
	g_signal_connect_object (child, "grab-notify",
				 G_CALLBACK (ephy_auto_scroller_grab_notify_cb), scroller, 0);

	priv->timeout_id =
		g_timeout_add (priv->msecs,
			       (GSourceFunc) ephy_auto_scroller_timeout_cb,
			       scroller);

	/* grab the pointer */
	widget = GTK_WIDGET (priv->window);
	gtk_grab_add (widget);
	if (gdk_pointer_grab (gtk_widget_get_window (widget), FALSE,
			      GDK_POINTER_MOTION_MASK |
			      GDK_BUTTON_PRESS_MASK,
			      NULL, priv->cursor, timestamp) != GDK_GRAB_SUCCESS)
	{
		ephy_auto_scroller_stop (scroller, timestamp);
		return;
	}

	if (gdk_keyboard_grab (gtk_widget_get_window (widget), FALSE, timestamp) != GDK_GRAB_SUCCESS)
	{
		ephy_auto_scroller_stop (scroller, timestamp);
		return;
	}
}

void
ephy_auto_scroller_stop (EphyAutoScroller *scroller,
			 guint32 timestamp)
{
	EphyAutoScrollerPrivate *priv = scroller->priv;
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

	/* hide the icon */
	gtk_widget_hide (priv->popup);

	widget = GTK_WIDGET (priv->window);
	gtk_grab_remove (widget);

	if (priv->timeout_id != 0)
	{
		g_source_remove (priv->timeout_id);
		priv->timeout_id = 0;
	}

	g_object_unref (priv->window);

	priv->embed = NULL;
	priv->active = FALSE;

	g_object_unref (scroller);
}

EphyAutoScroller *
ephy_auto_scroller_new (EphyWindow *window)
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
	GtkWidget *image;
	GdkPixbuf *pixbuf;
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask = NULL;

	priv = scroller->priv = EPHY_AUTO_SCROLLER_GET_PRIVATE (scroller);

	priv->active = FALSE;
	priv->msecs = TIMEOUT_DELAY;

	priv->cursor = gdk_cursor_new (GDK_FLEUR);

	/* Construct the popup */
	priv->popup = gtk_window_new (GTK_WINDOW_POPUP);

	pixbuf = gdk_pixbuf_new_from_xpm_data (autoscroll_xpm);
	g_return_if_fail (pixbuf != NULL);

	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &mask, 128);
	g_object_unref (pixbuf);
	g_return_if_fail (pixmap != NULL && mask != NULL);

	image = gtk_image_new_from_pixmap (pixmap, mask);
	gtk_container_add (GTK_CONTAINER (priv->popup), image);
	gtk_widget_show_all (image);

	gtk_widget_realize (priv->popup);
	gtk_widget_shape_combine_mask (priv->popup, mask, 0, 0);

	g_object_unref (pixmap);
	g_object_unref (mask);
}

static void
ephy_auto_scroller_finalize (GObject *object)
{
	EphyAutoScroller *scroller = EPHY_AUTO_SCROLLER (object);
	EphyAutoScrollerPrivate *priv = scroller->priv;

	LOG ("Finalize [%p]", object);

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
				      EPHY_TYPE_WINDOW,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

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
	const GTypeInfo our_info =
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
