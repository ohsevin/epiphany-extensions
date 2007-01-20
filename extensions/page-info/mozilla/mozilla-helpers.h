/*
 *  Copyright © 2004 Adam Hooper
 *  Copyright © 2004, 2005 Jean-François Rameau
 *  Copyright © 2004, 2005 Christian Persch
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

#include <glib.h>

#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

typedef enum
{
  EMBED_RENDER_UNKNOWN		= 0,
  EMBED_RENDER_FULL_STANDARDS	= 1,
  EMBED_RENDER_ALMOST_STANDARDS	= 2,
  EMBED_RENDER_QUIRKS		= 3
} EmbedPageRenderMode;

typedef enum
{
  EMBED_SOURCE_NOT_CACHED    = 0,
  EMBED_SOURCE_DISK_CACHE    = 1,
  EMBED_SOURCE_MEMORY_CACHE  = 2,
  EMBED_SOURCE_UNKNOWN_CACHE = 3
} EmbedPageSource;

typedef enum
{
  EMBED_LINK_TYPE_MAIL,
  EMBED_LINK_TYPE_NORMAL
} EmbedPageLinkType;

typedef struct
{
  char *content_type;
  char *encoding;
  char *referring_url;
  int size;
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
} EmbedPageProperties;

/* take care to sort enum by name */
/* so we can sort media as well */
typedef enum 
{
  MEDIUM_APPLET,
  MEDIUM_EMBED,
  MEDIUM_ICON,
  MEDIUM_IMAGE,
  MEDIUM_BG_IMAGE,
  MEDIUM_OBJECT,
  LAST_MEDIUM
} EmbedPageMediumType;

typedef struct
{
  char *url;
  EmbedPageMediumType type;
  char *alt;
  char *title;
  int width;
  int height;
} EmbedPageMedium; 

typedef struct
{
  EmbedPageLinkType type;
  char *url;
  char *title;
  char *rel;
} EmbedPageLink;

typedef struct
{
  char *name;
  char *method;
  char *action;
} EmbedPageForm;

typedef struct
{
  char *name;
  char *content;
} EmbedPageMetaTag;

typedef struct
{
  EmbedPageProperties *props;
  GList *media;
  GList *links;
  GList *forms;
  GList *metatags;
} EmbedPageInfo;

EmbedPageInfo *mozilla_get_page_info        (EphyEmbed *embed);

void           mozilla_free_embed_page_info (EmbedPageInfo *info);

G_END_DECLS

#endif /* PAGE_INFO_MOZILLA_HELPERS_H */

