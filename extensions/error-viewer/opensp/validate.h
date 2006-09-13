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

#ifndef ERROR_VIEWER_VALIDATE_H
#define ERROR_VIEWER_VALIDATE_H

#include <glib.h>

#include "sgml-validator.h"

G_BEGIN_DECLS

unsigned int validate (const char *filename,
		       const char *location,
		       SgmlValidator *validator,
		       gboolean is_xml);

G_END_DECLS

#endif

