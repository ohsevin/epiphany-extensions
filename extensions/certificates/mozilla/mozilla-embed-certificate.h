/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2004 Christian Persch
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

#ifndef MOZILLA_CERTIFICATES_H
#define MOZILLA_CERTIFICATES_H

#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

void		mozilla_embed_certificate_attach	(EphyEmbed *embed);

void		mozilla_embed_view_certificate		(EphyEmbed *embed);

gboolean	mozilla_embed_has_certificate		(EphyEmbed *embed);

G_END_DECLS

#endif
