/*
 *  Copyright © 2006 Jean-François Rameau
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

#ifndef MOZ_HELPER_H
#define MOZ_HELPER_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct 
{
	char *url;
	char *request;
	char *response;
} LiveHTTPHeaderFrame;

void 		livehttpheaders_register_listener (void);
void 		livehttpheaders_unregister_listener (void);

GSList *	get_frames (void);
void 		clear_frames (void);

gboolean       mozilla_glue_startup             (void);

G_END_DECLS

#endif

