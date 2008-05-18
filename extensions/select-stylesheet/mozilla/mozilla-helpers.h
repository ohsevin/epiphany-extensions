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

#ifndef MOZILLA_HELPERS_H
#define MOZILLA_HELPERS_H

#include <glib.h>

#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

typedef struct MozillaStyleSheet MozillaStyleSheet;

typedef enum
{
	STYLESHEET_NONE,
	STYLESHEET_BASIC,
	STYLESHEET_NAMED
} StyleSheetType;

StyleSheetType	mozilla_stylesheet_get_type	(MozillaStyleSheet *style);

const char     *mozilla_stylesheet_get_name	(MozillaStyleSheet *style);

void		mozilla_stylesheet_free		(MozillaStyleSheet *style);

GList	       *mozilla_get_stylesheets		(EphyEmbed *embed,
						 MozillaStyleSheet **selected);

void		mozilla_set_stylesheet		(EphyEmbed *embed,
						 MozillaStyleSheet *style);

gboolean       mozilla_glue_startup             (void);

G_END_DECLS

#endif /* MOZILLA_HELPERS_H */
