/*
 *  Copyright (C) 2003 Andrew Ruthven <andrew@etc.gen.nz>
 *
 *  Based on zoom-persistence.c v1.5 by: 
 *    Marco Pesenti Gritti
 *    Christian Persch
 *  And the Dashboard patch for Epiphany by:
 *    James Willcox (anyone else?)
 *
 *  For this plugin to work you need to apply the following patch to your
 *  Epiphany code base:
 *    epiphany-1.0.3.diff
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
 *  $$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dashboard-frontend.c"

#include "ephy-debug.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-embed-persist.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>

#include <gmodule.h>
#include <glib-object.h>
#include <strings.h>

/*
 * Build a clue packet and send it off to Dashboard.
 */
static void
send_cluepacket (EphyTab *tab, gboolean send_content)
{
	LOG ( "Entering send_cluepacket" )

	const char *content = NULL;
	char *cluepacket;

	/* do dashboard stuff */
	EphyEmbedPersist *persist =
		ephy_embed_persist_new (ephy_tab_get_embed (tab));
	
	if (send_content) {
		ephy_embed_persist_get_content (persist, &content);
	}
	
	cluepacket = dashboard_build_cluepacket_then_free_clues (
	             "Web Browser",
	             ephy_tab_get_visibility(tab),
	             ephy_tab_get_location(tab),
	             dashboard_build_clue (ephy_tab_get_location(tab), "url", 10),
	             dashboard_build_clue (ephy_tab_get_title(tab), "title", 10),
	             dashboard_build_clue (content, "content", 10),
	             NULL);

	dashboard_send_raw_cluepacket (cluepacket);

	g_free (cluepacket);

	/*
	 * The following g_free makes Ephy lock up.  But surely we need to
	 * free this memory?
	 */
        /*
	g_free (content);
        */

	g_object_unref (G_OBJECT (persist));
	
	LOG ( "Leaving send_cluepacket" )
}

/*
 * Hey hey, a tab has told us it's loaded something.  If it's at 100% then
 * we let Dashboard know.  I would prefer to receive a signal when the page
 * load is complete.
 */
static void
load_status_cb (EphyTab *tab, GParamSpec *pspec, EphyWindow *window)
{
	if (ephy_tab_get_load_percent (tab) == 100)
	{
		send_cluepacket (tab, TRUE);
	}

	LOG ( "Load status callback" )
}

/*
 * Well, a tab is now visible, let Dashboard know.
 *
 * This callback is currently not being registered.  It causes a number of
 * of CluePackets to be sent out for each page is loaded.  It doesn't seem to
 * work how I expect.
 */
static void
visible_cb (EphyTab *tab, GParamSpec *pspec, EphyWindow *window)
{
	send_cluepacket (tab, FALSE);

	LOG ( "Tab visible callback" )
}

/*
 * If a tab is added, then we'll watch the progress of the page loading, and
 * if the tab is visible.
 */
static void
tab_added_cb (GtkWidget *notebook, GtkWidget *embed)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (embed), "EphyTab"));
	g_return_if_fail (IS_EPHY_TAB (tab));

	g_signal_connect (G_OBJECT (tab), "notify::load-progress",
	                  G_CALLBACK (load_status_cb), embed);
	/*
	g_signal_connect (G_OBJECT (tab), "notify::visible",
	                  G_CALLBACK (visible_cb), embed);
	*/
}

/*
 * If a tab is removed, then we no longer need to listen for signals
 * coming from that tab.
 */
static void
tab_removed_cb (GtkWidget *notebook, GtkWidget *embed)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (embed), "EphyTab"));
	g_return_if_fail (IS_EPHY_TAB (tab));

	g_signal_handlers_disconnect_by_func (G_OBJECT (tab),
	                                      G_CALLBACK (load_status_cb),
	                                      embed);
	/*
	g_signal_handlers_disconnect_by_func (G_OBJECT (tab),
	                                      G_CALLBACK (visible_cb),
	                                      embed);
	*/
}

/*
 * For every new window that is created we need to keep track of the tabs
 * with in it.
 */
static void
new_window_cb (Session *session, EphyWindow *window)
{
	GtkWidget *notebook;

	notebook = ephy_window_get_notebook (window);

	g_signal_connect (notebook, "tab_added",
	                  G_CALLBACK (tab_added_cb), NULL);
	g_signal_connect (notebook, "tab_removed",
	                  G_CALLBACK (tab_removed_cb), NULL);
}

G_MODULE_EXPORT void
plugin_init (GTypeModule *module)
{
	Session *session;

	LOG ("Dashboard plugin initialising")

	session = SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_connect (session, "new_window",
	                  G_CALLBACK (new_window_cb), NULL);
}

G_MODULE_EXPORT void
plugin_exit (void)
{
	LOG ("Dashboard plugin exiting")
}
