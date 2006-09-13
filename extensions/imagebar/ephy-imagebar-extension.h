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
 *
 *  $Id$
 */

#ifndef EPHY_IMAGEBAR_EXTENSION_H
#define EPHY_IMAGEBAR_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_IMAGEBAR_EXTENSION		(ephy_imagebar_extension_get_type ())
#define EPHY_IMAGEBAR_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_IMAGEBAR_EXTENSION, EphyImagebarExtension))
#define EPHY_IMAGEBAR_EXTENSION_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_IMAGEBAR_EXTENSION, EphyImagebarExtensionClass))
#define EPHY_IS_IMAGEBAR_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_IMAGEBAR_EXTENSION))
#define EPHY_IS_IMAGEBAR_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_IMAGEBAR_EXTENSION))
#define EPHY_IMAGEBAR_EXTENSION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_IMAGEBAR_EXTENSION, EphyImagebarExtensionClass))

typedef struct _EphyImagebarExtension		EphyImagebarExtension;
typedef struct _EphyImagebarExtensionClass	EphyImagebarExtensionClass;
typedef struct _EphyImagebarExtensionPrivate	EphyImagebarExtensionPrivate;

struct _EphyImagebarExtensionClass
{
	GObjectClass parent_class;
};

struct _EphyImagebarExtension
{
	GObject parent_instance;

	/*< private >*/
	EphyImagebarExtensionPrivate *priv;
};

GType	ephy_imagebar_extension_get_type		(void);

GType	ephy_imagebar_extension_register_type	(GTypeModule *module);

G_END_DECLS

#endif
