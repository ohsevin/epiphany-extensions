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

#include "config.h"

#include "ephy-adblock-extension.h"
#include "ephy-debug.h"
#include "ad-blocker.h"

#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-extension.h>

#include <gmodule.h>

#define EPHY_ADBLOCK_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_ADBLOCK_EXTENSION, EphyAdblockExtensionPrivate))

struct EphyAdblockExtensionPrivate
{
	AdBlocker *blocker;
};

static void ephy_adblock_extension_class_init	(EphyAdblockExtensionClass *klass);
static void ephy_adblock_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_adblock_extension_init		(EphyAdblockExtension *extension);

enum
{
	PROP_0
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_adblock_extension_get_type (void)
{
	return type;
}

GType
ephy_adblock_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyAdblockExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_adblock_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyAdblockExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_adblock_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_adblock_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAdblockExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_adblock_extension_init (EphyAdblockExtension *extension)
{
	EphyEmbedSingle *single;

	LOG ("EphyAdblockExtension initialising")

	extension->priv = EPHY_ADBLOCK_EXTENSION_GET_PRIVATE (extension);

	single = EPHY_EMBED_SINGLE (ephy_embed_shell_get_embed_single (embed_shell));
	extension->priv->blocker = ad_blocker_new (single);
}

static void
ephy_adblock_extension_finalize (GObject *object)
{
	EphyAdblockExtension *extension = EPHY_ADBLOCK_EXTENSION (object);

	LOG ("EphyAdblockExtension finalising")

	g_object_unref (extension->priv->blocker);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
}

static void
ephy_adblock_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_adblock_extension_class_init (EphyAdblockExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_adblock_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyAdblockExtensionPrivate));
}
