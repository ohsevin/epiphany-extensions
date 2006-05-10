/*
 *  Copyright (C) 2000 Marco Pesenti Gritti
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
 */

#ifndef ERROR_VIEWER_MOZILLA_LINK_CHECKER_H
#define ERROR_VIEWER_MOZILLA_LINK_CHECKER_H

#include <epiphany/ephy-embed.h>

#include "link-checker.h"

G_BEGIN_DECLS

void	mozilla_register_link_checker_component		(void);

void	mozilla_unregister_link_checker_component	(void);

void	mozilla_check_links				(LinkChecker *checker,
							 EphyEmbed *embed);

G_END_DECLS

#endif
