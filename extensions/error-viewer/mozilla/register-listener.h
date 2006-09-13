/*
 *  Copyright © 2001,2002,2003 Philip Langdale
 *  Copyright © 2003 Marco Pesenti Gritti
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

#ifndef MOZ_REGISTER_LISTENER_H
#define MOZ_REGISTER_LISTENER_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

void *mozilla_register_error_listener (GObject *dialog);
void mozilla_unregister_error_listener (void *listener);

G_END_DECLS

#endif

