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

#ifndef LINK_CHECKER_H
#define LINK_CHECKER_H

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include "error-viewer.h"
#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

#define TYPE_LINK_CHECKER		(link_checker_get_type ())
#define LINK_CHECKER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_LINK_CHECKER, LinkChecker))
#define LINK_CHECKER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), TYPE_LINK_CHECKER, LinkCheckerClass))
#define IS_LINK_CHECKER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_LINK_CHECKER))
#define IS_LINK_CHECKER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_LINK_CHECKER))
#define LINK_CHECKER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_LINK_CHECKER, LinkCheckerClass))

typedef struct LinkChecker		LinkChecker;
typedef struct LinkCheckerClass		LinkCheckerClass;
typedef struct LinkCheckerPrivate	LinkCheckerPrivate;

struct LinkCheckerClass
{
	GObjectClass parent_class;
};

struct LinkChecker
{
	GObject parent_instance;

	/*< private >*/
	LinkCheckerPrivate *priv;
};

GType		link_checker_get_type		(void);

GType		link_checker_register_type	(GTypeModule *module);

LinkChecker	*link_checker_new		(ErrorViewer *viewer);

void		link_checker_check		(LinkChecker *checker,
						 EphyEmbed *embed);

void		link_checker_append		(LinkChecker *checker,
						 ErrorViewerErrorType error_type,
						 const char *message);

void		link_checker_update_progress	(LinkChecker *checker,
						 int num_checked,
						 int num_total);

G_END_DECLS

#endif
