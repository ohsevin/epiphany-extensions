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
 *  $Id$
 */

#ifndef EPHY_PUSH_SCROLLER_H
#define EPHY_PUSH_SCROLLER_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define EPHY_TYPE_PUSH_SCROLLER	        (ephy_push_scroller_get_type())
#define EPHY_PUSH_SCROLLER(object)	(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_PUSH_SCROLLER, EphyPushScroller))
#define EPHY_PUSH_SCROLLER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_PUSH_SCROLLER, EphyPushScrollerClass))
#define EPHY_IS_PUSH_SCROLLER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_PUSH_SCROLLER))
#define EPHY_IS_PUSH_SCROLLER_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_PUSH_SCROLLER))
#define EPHY_PUSH_SCROLLER_GET_CLASS(obj)(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_PUSH_SCROLLER, EphyPushScrollerClass))

typedef struct _EphyPushScrollerClass	EphyPushScrollerClass;
typedef struct _EphyPushScroller	EphyPushScroller;
typedef struct _EphyPushScrollerPrivate	EphyPushScrollerPrivate;

struct _EphyPushScrollerClass 
{
	GObjectClass parent_class;
};

struct _EphyPushScroller
{
	GObject parent_object;

	/*< private >*/
	EphyPushScrollerPrivate *priv;
};

GType			ephy_push_scroller_get_type	 (void);

GType                   ephy_push_scroller_register_type (GTypeModule *module);

EphyPushScroller       *ephy_push_scroller_new		 (EphyWindow *window);

void			ephy_push_scroller_start	 (EphyPushScroller *scroller,
							  EphyEmbed *embed,
							  gdouble x,
							  gdouble y);

void			ephy_push_scroller_stop		 (EphyPushScroller *scroller,
							  guint32 timestamp);

G_END_DECLS

#endif
