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

#ifndef POPUP_BLOCKER_MOZILLA_HELPERS_H
#define POPUP_BLOCKER_MOZILLA_HELPERS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <epiphany/ephy-embed.h>

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
	void *listener;
	void *eventTarget;
} PopupListenerFreeData;

PopupListenerFreeData	*mozilla_register_popup_listener	(EphyEmbed *embed);

void			mozilla_unregister_popup_listener	(PopupListenerFreeData *data);

void			mozilla_enable_javascript		(EphyEmbed *embed,
								 gboolean enable);

void			mozilla_open_popup			(EphyEmbed *embed,
								 const char *url,
								 const char *features);

char *			mozilla_get_location			(EphyEmbed *embed);

G_END_DECLS

#endif /* POPUP_BLOCKER_MOZILLA_HELPERS_H */
