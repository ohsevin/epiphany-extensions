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

#ifndef MOZILLA_FIND_H
#define MOZILLA_FIND_H

#include <glib.h>
#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

void	 mozilla_find_detach		(EphyEmbed *embed);

gboolean mozilla_find_next		(EphyEmbed *embed,
					 gboolean backwards);

guint32	 mozilla_find_set_properties	(EphyEmbed *embed,
					 const char *search_string,
					 gboolean case_sensitive,
					 gboolean wrap_around);

guint32	 mozilla_find_set_highlight	(EphyEmbed *embed,
					 gboolean show_highlight);

void	 mozilla_embed_scroll_pages	(EphyEmbed *embed,
					 gint32 pages);

void	 mozilla_embed_scroll_lines	(EphyEmbed *embed,
					 gint32 lines);

G_END_DECLS

#endif
