/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *  Copyright (C) 2003, 2004 Lee Willis
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
#include "dashboard-frontend-xmlwriter.h"

#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed-persist.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-node.h>
#include <epiphany/ephy-bookmarks.h>
#include <epiphany/ephy-embed-factory.h>

#include <gmodule.h>

#define EPHY_DASHBOARD_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_DASHBOARD_EXTENSION, EphyDashboardExtensionPrivate))

struct EphyDashboardExtensionPrivate
{
};

#define EPIPHANY_FRONTEND_IDENTIFIER	"Web Browser"

static void ephy_dashboard_extension_class_init	(EphyDashboardExtensionClass *klass);
static void ephy_dashboard_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_dashboard_extension_init	(EphyDashboardExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_dashboard_extension_get_type (void)
{
	return type;
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

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyDashboardExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
load_status_cb (EphyTab *tab,
		GParamSpec *pspec,
		EphyDashboardExtension *extension)
{
	gboolean load_status;
	
	load_status = ephy_tab_get_load_status(tab);

	/* FALSE means load is finished */
	if (load_status == FALSE)
	{
		EphyEmbed *embed;
		EphyEmbedPersist *persist;
		EphyBookmarks *bookmarks;
		char *location;
		const char *page_title;
		gboolean is_bookmarked;

		embed = ephy_tab_get_embed (tab);
		g_return_if_fail (EPHY_IS_EMBED (embed));

		/* Get the URL from the embed, since tab may contain modified url */
		location = ephy_embed_get_location (embed, TRUE);

		/* Get page title */
		page_title = ephy_tab_get_title(tab);

		/* See if the page is bookmarked */
		bookmarks = ephy_shell_get_bookmarks (ephy_shell);
		is_bookmarked = ephy_bookmarks_find_bookmark (bookmarks, location) != NULL;

		/* Get the page content if the page is bookmarked */
		if (is_bookmarked)
		{
			char *content;

			LOG ("Page is bookmarked, sending full content")

			persist = EPHY_EMBED_PERSIST (ephy_embed_factory_new_object ("EphyEmbedPersist"));
			ephy_embed_persist_set_embed (persist, embed);
			ephy_embed_persist_set_flags (persist, EMBED_PERSIST_NO_VIEW);
			content = ephy_embed_persist_to_string (persist);
			g_object_unref (persist);

			DashboardSendCluePacket (
				EPIPHANY_FRONTEND_IDENTIFIER,
				ephy_tab_get_visibility(tab), /* focused */
				location, /* context */
				location, "url", 10,
				page_title, "title", 10,
				content, "htmlblock", 10,
				NULL);

			g_free (content);
		}
		else
		{
			LOG ("Page not bookmarked, sending only location and title")

			DashboardSendCluePacket (
				EPIPHANY_FRONTEND_IDENTIFIER,
				ephy_tab_get_visibility(tab), /* focused */
				location, /* context */
				location, "url", 10,
				page_title, "title", 10,
				NULL);
		}

		g_free (location);
	} 
}

static void
tab_added_cb (GtkWidget *notebook,
	      EphyTab *tab,
	      EphyDashboardExtension *extension)
{
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_connect_after (tab, "notify::load-status",
				G_CALLBACK (load_status_cb), extension);
}

static void
tab_removed_cb (GtkWidget *notebook,
		EphyTab *tab,
		EphyDashboardExtension *extension)
{
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_handlers_disconnect_by_func
		(tab, G_CALLBACK (load_status_cb), extension);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "tab_added",
				G_CALLBACK (tab_added_cb), ext);
	g_signal_connect_after (notebook, "tab_removed",
				G_CALLBACK (tab_removed_cb), ext);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_added_cb), ext);
	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_removed_cb), ext);
}

static void
ephy_dashboard_extension_init (EphyDashboardExtension *extension)
{
/*	extension->priv = EPHY_DASHBOARD_EXTENSION_GET_PRIVATE (extension);*/

	LOG ("EphyDashboardExtension initialising")
}

static void
ephy_dashboard_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_dashboard_extension_class_init (EphyDashboardExtensionClass *klass)
{
/*	GObjectClass *object_class = G_OBJECT_CLASS (klass);*/

	parent_class = g_type_class_peek_parent (klass);

/*	g_type_class_add_private (object_class, sizeof (EphyDashboardExtensionPrivate));*/
}
