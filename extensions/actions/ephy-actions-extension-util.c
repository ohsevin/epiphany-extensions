/* 
 * Copyright (C) 2005 Jean-Yves Lefort <jylefort@brutele.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <glib.h>
#include "ephy-actions-extension-util.h"

char *
ephy_actions_extension_format_name_for_display (const char *name)
{
	GString *stripped;

	g_return_val_if_fail (name != NULL, NULL);

	stripped = g_string_new (NULL);
	for (; *name; name = g_utf8_next_char (name))
	{
		gunichar c = g_utf8_get_char (name);
	  
		if (c != '_')
		{
			g_string_append_unichar (stripped, c);
		}
	}
  
	if (g_str_has_suffix (stripped->str, "..."))
	{
		g_string_truncate (stripped, stripped->len - 3);
	}

	return g_string_free (stripped, FALSE);
}
