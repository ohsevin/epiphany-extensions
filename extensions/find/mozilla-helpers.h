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

#ifndef MOZILLA_HELPERS_H
#define MOZILLA_HELPERS_H

#include <glib.h>
#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

void	mozilla_embed_scroll_pages	(EphyEmbed *embed,
					 gint32 pages);

void	mozilla_embed_scroll_lines	(EphyEmbed *embed,
					 gint32 lines);

void	mozilla_push_prefs		(void);

void	mozilla_pop_prefs		(void);

G_END_DECLS

#endif /* !MOZILLA_HELPERS_H */
