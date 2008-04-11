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

#include "config.h"

#include "ephy-sample2-extension.h"
#include "mozilla-sample.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#define EPHY_SAMPLE2_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SAMPLE2_EXTENSION, EphySample2ExtensionPrivate))

struct _EphySample2ExtensionPrivate
{
	gpointer dummy;
};

enum
{
	PROP_0
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

static void
ephy_sample2_extension_init (EphySample2Extension *extension)
{
	extension->priv = EPHY_SAMPLE2_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphySample2Extension initialising");
}

static void
ephy_sample2_extension_finalize (GObject *object)
{
/*
	EphySample2Extension *extension = EPHY_SAMPLE2_EXTENSION (object);
*/
	LOG ("EphySample2Extension finalising");

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphySample2Extension *extension)
{
	gpointer dom_event;

	dom_event = ephy_embed_event_get_dom_event (event);

	LOG ("DOM Event %p", dom_event);

	mozilla_do_something (dom_event);

	return FALSE;
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	LOG ("EphySample2Extension attach_window");

	notebook = ephy_window_get_notebook (window);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	LOG ("EphySample2Extension detach_window");

	notebook = ephy_window_get_notebook (window);
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	LOG ("impl_attach_tab");

	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), ext);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	LOG ("impl_detach_tab");

	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), ext);
}

static void
ephy_sample2_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_sample2_extension_class_init (EphySample2ExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_sample2_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphySample2ExtensionPrivate));
}

GType
ephy_sample2_extension_get_type (void)
{
	return type;
}

GType
ephy_sample2_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphySample2ExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_sample2_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySample2Extension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_sample2_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_sample2_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphySample2Extension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
