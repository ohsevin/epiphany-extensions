/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#ifndef EPHY_PUSH_SCROLLER_EXTENSION_H
#define EPHY_PUSH_SCROLLER_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_PUSH_SCROLLER_EXTENSION		(ephy_push_scroller_extension_get_type ())
#define EPHY_PUSH_SCROLLER_EXTENSION(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_PUSH_SCROLLER_EXTENSION, EphyPushScrollerExtension))
#define EPHY_PUSH_SCROLLER_EXTENSION_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_PUSH_SCROLLER_EXTENSION, EphyPushScrollerExtensionClass))
#define EPHY_IS_PUSH_SCROLLER_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_PUSH_SCROLLER_EXTENSION))
#define EPHY_IS_PUSH_SCROLLER_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_PUSH_SCROLLER_EXTENSION))
#define EPHY_PUSH_SCROLLER_EXTENSION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_PUSH_SCROLLER_EXTENSION, EphyPushScrollerExtensionClass))

typedef struct _EphyPushScrollerExtension	EphyPushScrollerExtension;
typedef struct _EphyPushScrollerExtensionClass	EphyPushScrollerExtensionClass;

struct _EphyPushScrollerExtensionClass
{
	GObjectClass parent_class;
};

struct _EphyPushScrollerExtension
{
	GObject parent_instance;
};

GType	ephy_push_scroller_extension_get_type		(void);

GType	ephy_push_scroller_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif
