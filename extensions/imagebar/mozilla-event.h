/*
 *  Copyright (C) 2005 Kang Jeong-Hee
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

#ifndef MOZILLA_EVENT_H
#define MOZILLA_EVENT_H

#include <glib.h>
#include <nsError.h>
#include <nspr.h>

G_BEGIN_DECLS

nsresult	evaluate_dom_event	(gpointer dom_event,
					 PRBool *isImage,
					 PRBool *isCtrlKey,
					 PRBool *isAnchored,
					 PRInt32 *x,
					 PRInt32 *y,
					 char **imgSrc);

G_END_DECLS

#endif
