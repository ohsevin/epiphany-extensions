/*
 *  Copyright (C) 2000-2004 Marco Pesenti Gritti
 *  Copyright (C) 2004 Crispin Flowerday
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

#include "sidebar-commands.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-embed-factory.h>
#include <epiphany/ephy-embed-persist.h>

#include "ephy-prefs.h"

#include <gtk/gtkwindow.h>

/* NOTE: we include gi18n.h instead of gi18n-lib.h since all string here
 * are the same as in epiphany. If that ever changes, fix this include!
 * #include <glib/gi18n-lib.h>
 */
#include <glib/gi18n.h>

static EphyEmbedEvent *
get_event_info (EphyWindow *window)
{
	EphyEmbedEvent *info;

	info = EPHY_EMBED_EVENT (g_object_get_data
		(G_OBJECT (window), "context_event"));
	g_return_val_if_fail (info != NULL, NULL);

	return info;
}

void
sidebar_cmd_file_save_as (GtkAction *action,
			  EphySidebarEmbed *sidebar)
{
	EphyEmbed *embed;
	EphyEmbedPersist *persist;
	EphyWindow *window;

	embed = ephy_sidebar_embed_get_embed (sidebar);
	g_return_if_fail (embed != NULL);

	window = ephy_sidebar_embed_get_window (sidebar);

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object (EPHY_TYPE_EMBED_PERSIST));

	ephy_embed_persist_set_embed (persist, embed);
	ephy_embed_persist_set_fc_title (persist, _("Save As"));
	ephy_embed_persist_set_fc_parent (persist, GTK_WINDOW (window));
	ephy_embed_persist_set_flags
		(persist, EMBED_PERSIST_MAINDOC | EMBED_PERSIST_ASK_DESTINATION);
	ephy_embed_persist_set_persist_key
		(persist, CONF_STATE_SAVE_DIR);

	ephy_embed_persist_save (persist);

	g_object_unref (G_OBJECT(persist));
}

static void
save_property_url (GtkAction *action,
		   const char *title,
		   EphySidebarEmbed *sidebar,
		   gboolean ask_dest,
		   const char *property)
{
	EphyEmbedEvent *info;
	const char *location;
	const GValue *value;
	EphyEmbedPersist *persist;
	EphyEmbed *embed;
	EphyWindow *window;

	embed = ephy_sidebar_embed_get_embed (sidebar);
	g_return_if_fail (embed != NULL);

	window = ephy_sidebar_embed_get_window (sidebar);

	info = get_event_info (window);
	ephy_embed_event_get_property (info, property, &value);
	location = g_value_get_string (value);

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object (EPHY_TYPE_EMBED_PERSIST));

	ephy_embed_persist_set_embed (persist, embed);
	ephy_embed_persist_set_fc_title (persist, title);
	ephy_embed_persist_set_fc_parent (persist, GTK_WINDOW (window));
	ephy_embed_persist_set_flags
		(persist, ask_dest ? EMBED_PERSIST_ASK_DESTINATION : 0);
	ephy_embed_persist_set_persist_key
		(persist, CONF_STATE_SAVE_DIR);
	ephy_embed_persist_set_source (persist, location);

	ephy_embed_persist_save (persist);

	g_object_unref (G_OBJECT(persist));
}

void
sidebar_cmd_save_background_as (GtkAction *action,
				EphySidebarEmbed *sidebar)
{
	save_property_url (action, _("Save Background As"),
			   sidebar, TRUE, "background_image");
}

void
sidebar_cmd_open_frame (GtkAction *action,
			EphySidebarEmbed *sidebar)
{
	char *location;
	EphyEmbed *embed;

	embed = ephy_sidebar_embed_get_embed (sidebar);
	g_return_if_fail (embed != NULL);

	location = ephy_embed_get_location (embed, FALSE);

	ephy_embed_load_url (embed, location);

	g_free (location);
}

void
sidebar_cmd_download_link (GtkAction *action,
			   EphySidebarEmbed *sidebar)
{
	save_property_url (action, _("Download Link"), sidebar,
			   FALSE, "link");
}

void
sidebar_cmd_download_link_as (GtkAction *action,
			      EphySidebarEmbed *sidebar)
{
	save_property_url (action, _("Save Link As"), sidebar,
			   TRUE, "link");
}
void
sidebar_cmd_save_image_as (GtkAction *action,
			   EphySidebarEmbed *sidebar)
{
	save_property_url (action, _("Save Image As"),
			   sidebar, TRUE, "image");
}
