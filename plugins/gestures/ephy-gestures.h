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

#ifndef EPHY_GESTURES_H
#define EPHY_GESTURES_H

#include <glib-object.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

/* object forward declarations */

typedef struct _EphyGestures		EphyGestures;
typedef struct _EphyGesturesClass	EphyGesturesClass;
typedef struct _EphyGesturesPrivate	EphyGesturesPrivate;

/**
 * Gestures object
 */

#define EPHY_TYPE_GESTURES		(ephy_gestures_get_type())
#define EPHY_GESTURES(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_GESTURES, EphyGestures))
#define EPHY_GESTURES_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_GESTURES, EphyGesturesClass))
#define EPHY_IS_GESTURES(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_GESTURES))
#define EPHY_IS_GESTURES_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_GESTURES))
#define EPHY_GESTURES_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_GESTURES, EphyGesturesClass))

struct _EphyGesturesClass 
{
	GObjectClass parent_class;

	/* signals */
	void	(* performed)	(EphyGestures *eg,
				 const char *sequence);
	void	(* cancelled)	(EphyGestures *eg);
	
};

struct _EphyGestures
{
	GObject parent_object;

	EphyGesturesPrivate *priv;
};

GType		ephy_gestures_get_type		(void);

EphyGestures *	ephy_gestures_new		(void);

void		ephy_gestures_start		(EphyGestures *eg,
						 GtkWidget *widget, 
						 guint button,
						 gint x,
						 gint y);

void 		ephy_gestures_set_autocancel	(EphyGestures *eg, 
						 gboolean autocancel);

G_END_DECLS

#endif
