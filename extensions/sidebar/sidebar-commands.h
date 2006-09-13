/*
 *  Copyright © 2004 Crispin Flowerday
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

#ifndef SIDEBAR_COMMANDS_H
#define SIDEBAR_COMMANDS_H

#include <gtk/gtkaction.h>

#include "ephy-sidebar-embed.h"

G_BEGIN_DECLS

void sidebar_cmd_file_save_as		(GtkAction *action,
					 EphySidebarEmbed *sidebar);
void sidebar_cmd_save_background_as	(GtkAction *action,
					 EphySidebarEmbed *sidebar);
void sidebar_cmd_open_frame		(GtkAction *action,
					 EphySidebarEmbed *sidebar);
void sidebar_cmd_download_link		(GtkAction *action,
					 EphySidebarEmbed *window);
void sidebar_cmd_download_link_as	(GtkAction *action,
					 EphySidebarEmbed *window);
void sidebar_cmd_save_image_as		(GtkAction *action,
					 EphySidebarEmbed *window);

G_END_DECLS

#endif
