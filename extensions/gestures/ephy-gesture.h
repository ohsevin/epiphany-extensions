/*
 *  Copyright © 2002 Ricardo Fernández Pascual
 *  Copyright © 2003 Christian Persch
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

#ifndef EPHY_GESTURE_H
#define EPHY_GESTURE_H

#include <glib-object.h>
#include <gtk/gtkwidget.h>
#include <gmodule.h>

#include <epiphany/ephy-embed-event.h>

G_BEGIN_DECLS

#define EPHY_TYPE_GESTURE		(ephy_gesture_get_type())
#define EPHY_GESTURE(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_GESTURE, EphyGesture))
#define EPHY_GESTURE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_GESTURE, EphyGestureClass))
#define EPHY_IS_GESTURE(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_GESTURE))
#define EPHY_IS_GESTURE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_GESTURE))
#define EPHY_GESTURE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_GESTURE, EphyGestureClass))

typedef struct _EphyGestureClass	EphyGestureClass;
typedef struct _EphyGesture		EphyGesture;
typedef struct _EphyGesturePrivate	EphyGesturePrivate;

struct _EphyGestureClass
{
	GObjectClass parent_class;

	void	(* performed)	(EphyGesture *gesture,
				 const char *sequence);
};

struct _EphyGesture
{
	GObject parent_object;

	EphyGesturePrivate *priv;
};

GType		ephy_gesture_get_type		(void);

GType		ephy_gesture_register_type	(GTypeModule *module);

EphyGesture    *ephy_gesture_new		(GtkWidget *window);

GtkWidget      *ephy_gesture_get_window		(EphyGesture *gesture);

gboolean	ephy_gesture_start		(EphyGesture *gesture);

void		ephy_gesture_stop		(EphyGesture *gesture,
						 guint32 time);

EphyEmbedEvent *ephy_gesture_get_event		(EphyGesture *gesture);

void		ephy_gesture_set_event		(EphyGesture *gesture,
						 EphyEmbedEvent *event);
void		ephy_gesture_activate		(EphyGesture *gesture,
						 const char *path);

G_END_DECLS

#endif
