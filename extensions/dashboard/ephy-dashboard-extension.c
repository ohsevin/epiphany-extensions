/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2003 Lee Willis
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

#include "ephy-dashboard-extension.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed-persist.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-node.h>
#include <epiphany/ephy-bookmarks.h>

#include <gmodule.h>
#include "dashboard-frontend.c"

#define EPHY_DASHBOARD_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_DASHBOARD_EXTENSION, EphyDashboardExtensionPrivate))

struct EphyDashboardExtensionPrivate
{
};

static void ephy_dashboard_extension_class_init (EphyDashboardExtensionClass *klass);
static void ephy_dashboard_extension_iface_init (EphyExtensionClass *iface);
static void ephy_dashboard_extension_init (EphyDashboardExtension *extension);

enum
{
	PROP_0
	/* Extension-specific properties go here, to be added using
	 * g_object_class_install_property in the
	 * class_init function.
	 */
};

static GObjectClass *dashboard_extension_parent_class = NULL;

static GType dashboard_extension_type = 0;

GType
ephy_dashboard_extension_get_type (void)
{
	return dashboard_extension_type;
}

GType
ephy_dashboard_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyDashboardExtensionClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) ephy_dashboard_extension_class_init,
			NULL,
			NULL, /* class_data */
			sizeof (EphyDashboardExtension),
			0, /* n_preallocs */
			(GInstanceInitFunc) ephy_dashboard_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_dashboard_extension_iface_init,
		NULL,
		NULL
	};

	dashboard_extension_type =
		g_type_module_register_type (module, G_TYPE_OBJECT, "EphyDashboardExtension", &our_info, 0);

	g_type_module_add_interface (module, dashboard_extension_type, EPHY_TYPE_EXTENSION, &extension_info);

	return dashboard_extension_type;
}

static void
ephy_dashboard_extension_init (EphyDashboardExtension *extension)
{
	extension->priv = EPHY_DASHBOARD_EXTENSION_GET_PRIVATE (extension);
	LOG ("EphyDashboardExtension initialising\n");
}

static void
ephy_dashboard_extension_finalize (GObject *object)
{
	/* If needed,
	 * EphyDashboardExtension *extension = EPHY_DASHBOARD_EXTENSION (object);
	 * ...followed by, say, some g_free calls.
	 */
		
	LOG ("EphyDashboardExtension finalizing\n");

	G_OBJECT_CLASS (dashboard_extension_parent_class)->finalize (object);
}

static void
ephy_dashboard_extension_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	/* Code this function if anything was placed after PROP_0 in the enum above. */
	/*
	 *	EphyDashboardExtension *extension = EPHY_DASHBOARD_EXTENSION (object);
	 *
	 * 	switch (prop_id)
	 * 	{
	 * 	}
	 */
}

static void
ephy_dashboard_extension_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	/* Code this function if anything was placed after PROP_0 in the enum above. */
	/*
	 * 	EphyDashboardExtension *extension = EPHY_DASHBOARD_EXTENSION (object);
	 *
	 * 	switch (prop_id)
	 * 	{
	 * 	}
	 */
}

static void
ephy_dashboard_extension_class_init (EphyDashboardExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	dashboard_extension_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_dashboard_extension_finalize;
	object_class->get_property = ephy_dashboard_extension_get_property;
	object_class->set_property = ephy_dashboard_extension_set_property;

	g_type_class_add_private (object_class, sizeof (EphyDashboardExtensionPrivate));
}

static void
load_status_cb (EphyTab *tab,
		GParamSpec *pspec,
		EphyDashboardExtension *extension)
{
	gboolean load_status;
	
	load_status = ephy_tab_get_load_status(tab);

	if (!load_status)
	{
		char *cluepacket;
		const char *location;
		const char *page_title;
		EphyEmbed *embed;
		EphyEmbedPersist *persist;
		EphyBookmarks *bookmarks;
		char *content;

		// Get URL & page title
		location = ephy_tab_get_location(tab);
		page_title = ephy_tab_get_title(tab);
		LOG ("Got page location and title\n");

		// See if the page is bookmarked
		embed = ephy_tab_get_embed (tab);
		bookmarks = ephy_shell_get_bookmarks (ephy_shell);

		// Get the page content if the page is bookmarked
		LOG ("Title ::%s::", page_title);
		LOG ("URL ::%s::", location);
		if (embed && ephy_bookmarks_find_bookmark (bookmarks,location))
		{
			LOG ("Page exists in bookmark - sending full content");
			persist = EPHY_EMBED_PERSIST (ephy_embed_factory_new_object ("EphyEmbedPersist"));

			ephy_embed_persist_set_embed (persist, embed);
			ephy_embed_persist_set_flags (persist, EMBED_PERSIST_NO_VIEW);
			
			content = ephy_embed_persist_to_string(persist);

			g_object_unref (persist);
		
			LOG ("Content ::%s::", content);
			cluepacket = dashboard_build_cluepacket_then_free_clues (
	             		"Web Browser",
	             		ephy_tab_get_visibility(tab),
	             		dashboard_xml_quote(location),
	             		dashboard_build_clue (location, "url", 10),
	             		dashboard_build_clue (page_title, "title", 10),
	             		dashboard_build_clue (content, "htmlblock", 10),
	             		NULL);
		}
		else
		{
			LOG ("Page not in bookmark, sending only location and title");

			cluepacket = dashboard_build_cluepacket_then_free_clues (
	             		"Web Browser",
	             		ephy_tab_get_visibility(tab),
	             		dashboard_xml_quote(location),
	             		dashboard_build_clue (location, "url", 10),
	             		dashboard_build_clue (page_title, "title", 10),
	             		NULL);
		}

		// Send dashboard packet
		dashboard_send_raw_cluepacket (cluepacket);

		g_free (cluepacket);

	} 
}

static void
tab_added_cb (GtkWidget *notebook,
              EphyEmbed *embed,
	      EphyDashboardExtension *extension)
{
	EphyTab *tab;

	LOG ("In tab_added_cb \n");
	tab = ephy_tab_for_embed (embed);
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_connect_after (G_OBJECT (tab), "notify::load-status", G_CALLBACK (load_status_cb), extension);
}

static void
tab_removed_cb (EphyEmbed *notebook, GtkWidget *child, EphyDashboardExtension *extension)
{
	EphyTab *tab;

	LOG ("In tab_removed_cb \n");
	tab = EPHY_TAB (g_object_get_data (G_OBJECT (child), "EphyTab"));
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_handlers_disconnect_by_func (G_OBJECT (child), G_CALLBACK (load_status_cb), tab);
}

static void
impl_attach_window (EphyExtension *ext, EphyWindow *window)
{
	GtkWidget *notebook;
	LOG ("EphyDashboardExtension attach_window\n");

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "tab_added", G_CALLBACK (tab_added_cb), ext);
	g_signal_connect_after (notebook, "tab_removed", G_CALLBACK (tab_removed_cb), ext);
}

static void
impl_detach_window (EphyExtension *ext, EphyWindow *window)
{
	GtkWidget *notebook;
	LOG ("EphyDashboardExtension detach_window\n");

	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func (notebook, G_CALLBACK (tab_added_cb), ext);
	g_signal_handlers_disconnect_by_func (notebook, G_CALLBACK (tab_removed_cb), ext);
	g_signal_handlers_disconnect_by_func (notebook, G_CALLBACK (load_status_cb), ext);
}

static void
ephy_dashboard_extension_iface_init (EphyExtensionClass *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

