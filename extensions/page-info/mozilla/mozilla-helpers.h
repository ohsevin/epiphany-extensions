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
	EMBED_RENDER_QUIRKS    = 1,
	EMBED_RENDER_STANDARDS = 2
} EmbedPageRenderMode;

typedef enum
{
	EMBED_SOURCE_NOT_CACHED    = 0,
	EMBED_SOURCE_DISK_CACHE    = 1,
	EMBED_SOURCE_MEMORY_CACHE  = 2,
	EMBED_SOURCE_UNKNOWN_CACHE = 3
} EmbedPageSource;

typedef struct
{
	char *content_type;
	char *encoding;
	char *referring_url;
	gint size;
	GTime expiration_time;
	GTime modification_time;
	EmbedPageRenderMode rendering_mode;
	EmbedPageSource page_source;

	/*
	char *cipher_name;
	char *cert_issuer_name;
	gint key_length;
	gint secret_key_length;
	*/

//	/* lists of hashtables with gvalues */
//	GList *metatags;    /* name, value */
//	GList *images;      /* url, alt, title, width, height */
//	GList *forms;       /* name, method, action */
//	GList *links;       /* url, title, relation */
//	GList *stylesheets; /* url, title */
} EmbedPageProperties;

typedef struct
{
	char *url;
	char *alt;
	char *title;
	gint width;
	gint height;
} EmbedPageImage; 

void			 mozilla_free_page_properties	(EmbedPageProperties *props);

EmbedPageProperties	*mozilla_get_page_properties	(EphyEmbed *embed);

void			 mozilla_free_embed_page_image	(EmbedPageImage *image);

GList			*mozilla_get_images		(EphyEmbed *embed);

G_END_DECLS

#endif /* PAGE_INFO_MOZILLA_HELPERS_H */
