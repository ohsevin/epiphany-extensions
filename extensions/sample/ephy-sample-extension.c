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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-sample-extension.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>

#include <gmodule.h>

#define EPHY_SAMPLE_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SAMPLE_EXTENSION, EphySampleExtensionPrivate))

struct EphySampleExtensionPrivate
{
};

static void ephy_sample_extension_class_init	(EphySampleExtensionClass *klass);
static void ephy_sample_extension_iface_init	(EphyExtensionClass *iface);
static void ephy_sample_extension_init		(EphySampleExtension *extension);

enum
{
	PROP_0
};

static GObjectClass *sample_extension_parent_class = NULL;

static GType sample_extension_type = 0;

GType
ephy_sample_extension_get_type (void)
{
	return sample_extension_type;
}

GType
ephy_sample_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphySampleExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_sample_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySampleExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_sample_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_sample_extension_iface_init,
		NULL,
		NULL
	};

	sample_extension_type =
		g_type_module_register_type (module,
					     G_TYPE_OBJECT,
					     "EphySampleExtension",
					     &our_info, 0);

	g_type_module_add_interface (module,
				     sample_extension_type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	return sample_extension_type;
}

static void
ephy_sample_extension_init (EphySampleExtension *extension)
{
	extension->priv = EPHY_SAMPLE_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphySampleExtension initialising")
}

static void
ephy_sample_extension_finalize (GObject *object)
{
/*
	EphySampleExtension *extension = EPHY_SAMPLE_EXTENSION (object);
*/
	LOG ("EphySampleExtension finalising")

	G_OBJECT_CLASS (sample_extension_parent_class)->finalize (object);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
//	EphySampleExtension *extension = EPHY_SAMPLE_EXTENSION (ext);
	LOG ("EphySampleExtension attach_window")
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
//	EphySampleExtension *extension = EPHY_SAMPLE_EXTENSION (ext);
	LOG ("EphySampleExtension detach_window")
}

static void
ephy_sample_extension_set_property (GObject *object,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *pspec)
{
/*
	EphySampleExtension *extension = EPHY_SAMPLE_EXTENSION (object);

	switch (prop_id)
	{
	}
*/
}

static void
ephy_sample_extension_get_property (GObject *object,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *pspec)
{
/*
	EphySampleExtension *extension = EPHY_SAMPLE_EXTENSION (object);

	switch (prop_id)
	{
	}
*/
}

static void
ephy_sample_extension_iface_init (EphyExtensionClass *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_sample_extension_class_init (EphySampleExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	sample_extension_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_sample_extension_finalize;
	object_class->get_property = ephy_sample_extension_get_property;
	object_class->set_property = ephy_sample_extension_set_property;

	g_type_class_add_private (object_class, sizeof (EphySampleExtensionPrivate));
}
