/*
 *  Copyright © 2010 Gustavo Noronha Silva
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

#ifndef EPHY_HTML5TUBE_EXTENSION_H
#define EPHY_HTML5TUBE_EXTENSION_H

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define EPHY_TYPE_HTML5TUBE_EXTENSION		(ephy_html5tube_extension_get_type ())
#define EPHY_HTML5TUBE_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_HTML5TUBE_EXTENSION, EphyHTML5TubeExtension))
#define EPHY_HTML5TUBE_EXTENSION_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_HTML5TUBE_EXTENSION, EphyHTML5TubeExtensionClass))
#define EPHY_IS_HTML5TUBE_EXTENSION(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_HTML5TUBE_EXTENSION))
#define EPHY_IS_HTML5TUBE_EXTENSION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_HTML5TUBE_EXTENSION))
#define EPHY_HTML5TUBE_EXTENSION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_HTML5TUBE_EXTENSION, EphyHTML5TubeExtensionClass))

typedef struct _EphyHTML5TubeExtension		EphyHTML5TubeExtension;
typedef struct _EphyHTML5TubeExtensionClass	EphyHTML5TubeExtensionClass;
typedef struct _EphyHTML5TubeExtensionPrivate	EphyHTML5TubeExtensionPrivate;

struct _EphyHTML5TubeExtensionClass
{
    PeasExtensionBaseClass parent_class;
};

struct _EphyHTML5TubeExtension
{
    PeasExtensionBase parent_instance;

    /*< private >*/
    EphyHTML5TubeExtensionPrivate *priv;
};

GType	ephy_html5tube_extension_get_type		(void);
GType	ephy_html5tube_extension_register_type		(GTypeModule *module);

G_MODULE_EXPORT void	peas_register_types	(PeasObjectModule *module);

G_END_DECLS

#endif
