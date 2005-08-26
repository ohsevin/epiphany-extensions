/*
 *  Copyright (C) 2002  Ricardo Fernández Pascual
 *  Copyright (C) 2005  Crispin Flowerday
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

#include "ephy-auto-scroller.h"

#include <gtk/gtkimage.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkmain.h>

#include "mozilla-helpers.h"
#include "ephy-debug.h"
#include "autoscroll.xpm.h"

#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Private data
 */
#define EPHY_AUTO_SCROLLER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
				       EPHY_TYPE_AUTO_SCROLLER, EphyAutoScrollerPrivate))


struct _EphyAutoScrollerPrivate {
	EphyEmbed *embed;
	GtkWidget *widget;
	guint start_x;
	guint start_y;
	gfloat step_x;
	gfloat step_y;
	gfloat roundoff_error_x;
	gfloat roundoff_error_y;
	gint msecs;
	guint timeout_id;
	gboolean active;
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

static GtkWidget *autoscroll_icon = NULL;


/**
 * Private functions, only availble from this file
 */
static void		ephy_auto_scroller_finalize_impl	(GObject *o);
static gboolean		ephy_auto_scroller_motion_cb	(GtkWidget *widget, GdkEventMotion *e,
								 EphyAutoScroller *as);
static gboolean		ephy_auto_scroller_mouse_press_cb (GtkWidget *widget, GdkEventButton *e,
								  EphyAutoScroller *as);
static gboolean		ephy_auto_scroller_key_press_cb (GtkWidget *widget, GdkEventKey *e,
								EphyAutoScroller *as);
static gboolean		ephy_auto_scroller_unmap_event_cb  (GtkWidget *widget,
								   GdkEvent *event,
								   EphyAutoScroller *as);
static gint		ephy_auto_scroller_timeout_cb	(gpointer data);
static void		ephy_auto_scroller_stop		(EphyAutoScroller *as);


/**
 * EmbedAutoScroller object
 */

static void
ephy_auto_scroller_class_init (EphyAutoScrollerClass *klass)
{
	GdkPixbuf *icon_pixbuf;
	GdkPixmap *icon_pixmap;
	GdkBitmap *icon_bitmap;
	GtkWidget *icon_img;

	parent_class = g_type_class_peek_parent (klass);

	G_OBJECT_CLASS (klass)->finalize = ephy_auto_scroller_finalize_impl;

	/* initialize the autoscroll icon */

	icon_pixbuf = gdk_pixbuf_new_from_xpm_data (autoscroll_xpm);
	g_return_if_fail (icon_pixbuf);
		
	gdk_pixbuf_render_pixmap_and_mask (icon_pixbuf, &icon_pixmap,
					   &icon_bitmap, 128);
	g_object_unref (icon_pixbuf);

	/*
	  gtk_widget_push_visual (gdk_rgb_get_visual ());
	  gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	*/
	icon_img = gtk_image_new_from_pixmap (icon_pixmap, icon_bitmap);
	
	autoscroll_icon = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_realize (autoscroll_icon);
	gtk_container_add (GTK_CONTAINER (autoscroll_icon), icon_img);
	gtk_widget_shape_combine_mask (autoscroll_icon, icon_bitmap, 0, 0);

	/*
	  gtk_widget_pop_visual ();
	  gtk_widget_pop_colormap ();
	*/
		
	g_object_unref (icon_pixmap);
	g_object_unref (icon_bitmap);

	gtk_widget_show_all (icon_img);

	g_type_class_add_private (klass, sizeof (EphyAutoScrollerPrivate));
}

static void 
ephy_auto_scroller_init (EphyAutoScroller *e)
{
	EphyAutoScrollerPrivate *p = EPHY_AUTO_SCROLLER_GET_PRIVATE (e);
	e->priv = p;

	p->active = FALSE;
	p->msecs = 33;
}

static void
ephy_auto_scroller_finalize_impl (GObject *o)
{
	EphyAutoScroller *as = EPHY_AUTO_SCROLLER (o);
	EphyAutoScrollerPrivate *p = as->priv;

	LOG ("in ephy_auto_scroller_finalize_impl");

	if (p->embed)
	{
		g_object_unref (p->embed);
	}

	if (p->timeout_id)
	{
		g_source_remove (p->timeout_id);
		p->timeout_id = 0;
	}


	G_OBJECT_CLASS (parent_class)->finalize (o);
}

EphyAutoScroller *
ephy_auto_scroller_new (void)
{
	EphyAutoScroller *ret = g_object_new (EPHY_TYPE_AUTO_SCROLLER, NULL);
	return ret;
}

void
ephy_auto_scroller_set_embed (EphyAutoScroller *as,
                             EphyEmbed *embed)
{
	EphyAutoScrollerPrivate *p = as->priv;

	if (p->embed)
	{
		g_object_unref (p->embed);
	}
	
	p->embed = g_object_ref (embed);
}

void
ephy_auto_scroller_start_scroll (EphyAutoScroller *as,
					GtkWidget *widget, gint x, gint y)
{
	static GdkCursor *cursor = NULL;
	EphyAutoScrollerPrivate *p = as->priv;

	g_return_if_fail (p->embed);
	
	if (p->active)
	{
		return;
	}
	p->active = TRUE;
	
	g_object_ref (as);

	p->widget = g_object_ref (widget);

	/* get a new cursor, if necessary */
	if (!cursor) cursor = gdk_cursor_new (GDK_FLEUR);

	/* show icon */
	gtk_window_move (GTK_WINDOW (autoscroll_icon), x - 12, y - 12);
	gtk_widget_show (autoscroll_icon);

	/* set positions */
	p->start_x = x;
	p->start_y = y;
	p->step_x = 0;
	p->step_y = 0;
	p->roundoff_error_x = 0;
	p->roundoff_error_y = 0;

	/* attach signals */
	g_signal_connect (widget, "motion_notify_event",
			    G_CALLBACK (ephy_auto_scroller_motion_cb), as);
	g_signal_connect (widget, "button_press_event",
			  G_CALLBACK (ephy_auto_scroller_mouse_press_cb), as);
	g_signal_connect (widget, "key_press_event",
			  G_CALLBACK (ephy_auto_scroller_key_press_cb), as);
	g_signal_connect (widget, "unmap-event",
			  G_CALLBACK (ephy_auto_scroller_unmap_event_cb), as);

	p->timeout_id =
		g_timeout_add (p->msecs,
			       ephy_auto_scroller_timeout_cb, as);

	/* grab the pointer */
	gtk_grab_add (widget);
	gdk_pointer_grab (widget->window, FALSE,
			  GDK_POINTER_MOTION_MASK |
			  GDK_BUTTON_PRESS_MASK,
			  NULL, cursor, GDK_CURRENT_TIME);
	gdk_keyboard_grab (widget->window, FALSE, GDK_CURRENT_TIME);
}

static gboolean
ephy_auto_scroller_motion_cb (GtkWidget *widget, GdkEventMotion *e,
				     EphyAutoScroller *as)
{
	EphyAutoScrollerPrivate *p = as->priv;
	gint x_dist, x_dist_abs, y_dist, y_dist_abs;

	if (!p->active)
	{
		return FALSE;
	}

	/* get distance between scroll center and cursor */
	x_dist = e->x_root - p->start_x;
	x_dist_abs = abs (x_dist);
	y_dist = e->y_root - p->start_y;
	y_dist_abs = abs (y_dist);

	/* calculate scroll step */
	if (y_dist_abs <= 48.0)
	{
		p->step_y = (float) (y_dist / 4) / 6.0;
	}
	else if (y_dist > 0)
	{
		p->step_y = (y_dist - 48.0) / 2.0 + 2.0;
	}
	else 
	{
		p->step_y = (y_dist + 48.0) / 2.0 - 2.0;
	}

	if (x_dist_abs <= 48.0)
	{
		p->step_x = (float) (x_dist / 4) / 6.0;
	}
	else if (x_dist > 0)
	{
		p->step_x = (x_dist - 48.0) / 2.0 + 2.0;
	}
	else 
	{
		p->step_x = (x_dist + 48.0) / 2.0 - 2.0;
	}

	return TRUE;
}

static gboolean
ephy_auto_scroller_mouse_press_cb (GtkWidget *widget, GdkEventButton *e,
				      EphyAutoScroller *as)
{
	/* ungrab and disconnect */
	ephy_auto_scroller_stop (as);

	return TRUE;
}

static gboolean
ephy_auto_scroller_key_press_cb (GtkWidget *widget, GdkEventKey *e,
					EphyAutoScroller *as)
{
	/* ungrab and disconnect */
	ephy_auto_scroller_stop (as);

	return TRUE;
}

static gboolean
ephy_auto_scroller_unmap_event_cb  (GtkWidget *widget,
					   GdkEvent *event,
					   EphyAutoScroller *as)
{
	/* ungrab and disconnect */
	ephy_auto_scroller_stop (as);

	return FALSE;
}

static gint
ephy_auto_scroller_timeout_cb (gpointer data)
{
	struct timeval start_time, finish_time;
	long elapsed_msecs;
	EphyAutoScroller *as = data;
	EphyAutoScrollerPrivate *p;
	gfloat scroll_step_y_adj;
	gint scroll_step_y_int;
	gfloat scroll_step_x_adj;
	gint scroll_step_x_int;

	g_return_val_if_fail (EPHY_IS_AUTO_SCROLLER (as), FALSE);
	p = as->priv;
	g_return_val_if_fail (EPHY_IS_EMBED (p->embed), FALSE);

	/* return if we're not supposed to scroll */
	if (!p->step_y && !p->step_x)
	{
		return TRUE;
	}

	/* calculate the number of pixels to scroll */
	scroll_step_y_adj = p->step_y * p->msecs / 33;
	scroll_step_y_int = scroll_step_y_adj;
	p->roundoff_error_y += (scroll_step_y_adj - scroll_step_y_int);

	if (fabs (p->roundoff_error_y) >= 1.0)
	{
		scroll_step_y_int += p->roundoff_error_y;
		p->roundoff_error_y -= (gint) p->roundoff_error_y;
	}

	scroll_step_x_adj = p->step_x * p->msecs / 33;
	scroll_step_x_int = scroll_step_x_adj;
	p->roundoff_error_x += (scroll_step_x_adj - scroll_step_x_int);

	if (fabs (p->roundoff_error_x) >= 1.0)
	{
		scroll_step_x_int += p->roundoff_error_x;
		p->roundoff_error_x -= (gint) p->roundoff_error_x;
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
	mozilla_helper_fine_scroll (p->embed, scroll_step_x_int, scroll_step_y_int);

	/* find out how long the scroll took */
	gettimeofday (&finish_time, NULL);
	elapsed_msecs = (1000000L * finish_time.tv_sec + finish_time.tv_usec -
			 1000000L * start_time.tv_sec - start_time.tv_usec) /
			1000;

	/* check if we should update the scroll delay */
	if ((elapsed_msecs >= p->msecs + 5) ||
	    ((p->msecs > 20) && (elapsed_msecs < p->msecs - 10)))
	{
		/* update the scrolling delay, with a
		 * minimum delay of 20 ms */
		p->msecs = (elapsed_msecs + 10 >= 20) ? elapsed_msecs + 10 : 20;

		/* create new timeout with adjusted delay */
		p->timeout_id =	g_timeout_add (p->msecs, ephy_auto_scroller_timeout_cb, as);

		/* kill the old timeout */
		return FALSE;
	}

	/* don't kill timeout */
	return TRUE;
}

static void
ephy_auto_scroller_stop (EphyAutoScroller *as)
{
	EphyAutoScrollerPrivate *p = as->priv;

	/* ungrab the pointer if it's grabbed */
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	gdk_keyboard_ungrab (GDK_CURRENT_TIME);

	/* hide the icon */
	gtk_widget_hide (autoscroll_icon);

	g_return_if_fail (p->widget);
	
	gtk_grab_remove (p->widget);

	/* disconnect all of the signals */
	g_signal_handlers_disconnect_matched (p->widget, G_SIGNAL_MATCH_DATA, 0, 0, 
					      NULL, NULL, as);
	if (p->timeout_id)
	{
		g_source_remove (p->timeout_id);
		p->timeout_id = 0;
	}

	g_object_unref (p->widget);
	p->widget = NULL;

	p->active = FALSE;
	g_object_unref (as);
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
