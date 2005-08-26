/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2005  Crispin Flowerday
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

#include "ephy-auto-scroller-extension.h"
#include "ephy-auto-scroller.h"
#include "mozilla-helpers.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#define EPHY_AUTO_SCROLLER_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_AUTO_SCROLLER_EXTENSION, EphyAutoScrollerExtensionPrivate))

struct _EphyAutoScrollerExtensionPrivate
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
ephy_auto_scroller_extension_init (EphyAutoScrollerExtension *extension)
{
	extension->priv = EPHY_AUTO_SCROLLER_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyAutoScrollerExtension initialising");
}

static void
ephy_auto_scroller_extension_finalize (GObject *object)
{
/*
	EphyAutoScrollerExtension *extension = EPHY_AUTO_SCROLLER_EXTENSION (object);
*/
	LOG ("EphyAutoScrollerExtension finalising");

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphyAutoScrollerExtension *extension)
{
	EphyEmbedEventContext context;
	guint button;
	gint handled = FALSE;
	EphyTab *tab;
	GtkWidget *toplevel;
	EphyAutoScroller *as;
	guint x, y;

	button = ephy_embed_event_get_button (event);
	context = ephy_embed_event_get_context (event);
	
	if (button != 2 || (context & EPHY_EMBED_CONTEXT_INPUT))
	{
		return FALSE;
	}

	tab = ephy_tab_for_embed (embed);
	g_return_val_if_fail (EPHY_IS_TAB (tab), handled);

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));
	g_return_val_if_fail (toplevel != NULL, handled);

	as = ephy_auto_scroller_new ();
	ephy_embed_event_get_coords (event, &x, &y);
	ephy_auto_scroller_set_embed (as, embed);
	ephy_auto_scroller_start_scroll (as, GTK_WIDGET (toplevel), x, y);
	g_object_unref (as);

	return TRUE;
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("impl_attach_tab");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), ext);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("impl_detach_tab");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), ext);
}

static void
ephy_auto_scroller_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_auto_scroller_extension_class_init (EphyAutoScrollerExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_auto_scroller_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyAutoScrollerExtensionPrivate));
}

GType
ephy_auto_scroller_extension_get_type (void)
{
	return type;
}

GType
ephy_auto_scroller_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyAutoScrollerExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_auto_scroller_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyAutoScrollerExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_auto_scroller_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_auto_scroller_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAutoScrollerExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
