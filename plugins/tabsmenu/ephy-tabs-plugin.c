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

#include "ephy-tabs-plugin-menu.h"

#include "ephy-debug.h"

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>

#include <gmodule.h>
#include <glib-object.h>

static void
new_window_cb (Session *session, EphyWindow *window)
{
	EphyTabsPluginMenu *menu;

	LOG ("new window %p", window)

	menu = ephy_tabs_plugin_menu_new (window);

	g_object_set_data_full (G_OBJECT (window), "ephy-tabs-plugin-menu",
				menu, (GDestroyNotify) g_object_unref);
}

G_MODULE_EXPORT void
plugin_init (GTypeModule *module)
{
	Session *session;

	LOG ("Plugin initialising")

#ifdef ENABLE_NLS
       /* Initialize the i18n stuff */
        bindtextdomain (GETTEXT_PACKAGE, EPHY_PLUGINS_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");	
#endif /* ENABLE_NLS */

	session = SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_connect (session, "new_window",
			  G_CALLBACK (new_window_cb), NULL);
}

G_MODULE_EXPORT void
plugin_exit (void)
{
	LOG ("Plugin exiting")
}
