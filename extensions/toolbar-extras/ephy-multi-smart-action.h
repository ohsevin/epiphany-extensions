/*
 *  Copyright © 2003 Marco Pesenti Gritti
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
 */

#ifndef EPHY_MULTI_SMART_ACTION_H
#define EPHY_MULTI_SMART_ACTION_H

#include <gmodule.h>
#include <glib-object.h>
#include <gtk/gtkaction.h>

G_BEGIN_DECLS

#define EPHY_TYPE_MULTI_SMART_ACTION		(ephy_multi_smart_action_get_type ())
#define EPHY_MULTI_SMART_ACTION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EPHY_TYPE_MULTI_SMART_ACTION, EphyMultiSmartAction))
#define EPHY_MULTI_SMART_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EPHY_TYPE_MULTI_SMART_ACTION, EphyMultiSmartActionClass))
#define EPHY_IS_MULTI_SMART_ACTION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EPHY_TYPE_MULTI_SMART_ACTION))
#define EPHY_IS_MULTI_SMART_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), EPHY_TYPE_MULTI_SMART_ACTION))
#define EPHY_MULTI_SMART_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_MULTI_SMART_ACTION, EphyMultiSmartActionClass))

typedef struct _EphyMultiSmartAction		EphyMultiSmartAction;
typedef struct _EphyMultiSmartActionClass	EphyMultiSmartActionClass;
typedef struct _EphyMultiSmartActionPrivate	EphyMultiSmartActionPrivate;

struct _EphyMultiSmartAction
{
	GtkAction parent;

	/*< private >*/
	EphyMultiSmartActionPrivate *priv;
};

struct _EphyMultiSmartActionClass
{
	GtkActionClass parent_class;
};

GType		ephy_multi_smart_action_get_type	(void);

GType		ephy_multi_smart_action_register_type	(GTypeModule *module);

G_END_DECLS

#endif
