/*
 *  Copyright (C) 2004 Adam Hooper
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

#ifndef PAGE_INFO_MOZILLA_HELPERS_H
#define PAGE_INFO_MOZILLA_HELPERS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

typedef enum
{
	STYLESHEET_NONE,
	STYLESHEET_BASIC,
	STYLESHEET_NAMED
} EmbedStyleSheetType;

typedef struct
{
	char *name;
	gpointer sheet; /* opaque pointer to underlying sheet object */
	EmbedStyleSheetType type;
} EmbedStyleSheet;

void		 mozilla_free_stylesheet		(EmbedStyleSheet *style);

GList		*mozilla_get_stylesheets		(EphyEmbed *embed,
							 EmbedStyleSheet **selected);

void		 mozilla_set_stylesheet			(EphyEmbed *embed,
							 EmbedStyleSheet *style);

G_END_DECLS

#endif /* PAGE_INFO_MOZILLA_HELPERS_H */
