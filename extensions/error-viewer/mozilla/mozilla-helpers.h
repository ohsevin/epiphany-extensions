/*
 *  Copyright © 2004 Adam Hooper
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

#ifndef ERROR_VIEWER_MOZILLA_HELPERS_H
#define ERROR_VIEWER_MOZILLA_HELPERS_H

#include <glib.h>

#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

char		*mozilla_get_doctype		(EphyEmbed *embed);

char		*mozilla_get_content_type	(EphyEmbed *embed);

GSList		*mozilla_get_links		(EphyEmbed *embed);

gboolean	mozilla_check_url		(EphyEmbed *embed, char *url);

gboolean       mozilla_glue_startup             (void);

G_END_DECLS

#endif
