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

#ifndef __gul_gestures_h
#define __gul_gestures_h

#include <glib-object.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

/* object forward declarations */

typedef struct _GulGestures GulGestures;
typedef struct _GulGesturesClass GulGesturesClass;
typedef struct _GulGesturesPrivate GulGesturesPrivate;

/**
 * Gestures object
 */

#define GUL_TYPE_GESTURES		(gul_gestures_get_type())
#define GUL_GESTURES(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), GUL_TYPE_GESTURES, GulGestures))
#define GUL_GESTURES_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), GUL_TYPE_GESTURES, GulGesturesClass))
#define GUL_IS_GESTURES(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), GUL_TYPE_GESTURES))
#define GUL_IS_GESTURES_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GUL_TYPE_GESTURES))
#define GUL_GESTURES_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GUL_TYPE_GESTURES, GulGesturesClass))

struct _GulGesturesClass 
{
	GObjectClass parent_class;

	/* signals */
	void	(*gesture_performed)	(GulGestures *g,
					 const char *sequence);
	void	(*cancelled)		(GulGestures *g);
	
};

struct _GulGestures
{
	GObject parent_object;
	GulGesturesPrivate *priv;
};

GType		gul_gestures_get_type		(void);

GulGestures *	gul_gestures_new		(void);

void		gul_gestures_start		(GulGestures *g,
						 GtkWidget *widget, 
						 guint button,
						 gint x,
						 gint y);

void 		gul_gestures_set_autocancel	(GulGestures *ges, 
						 gboolean autocancel);

G_END_DECLS

#endif
