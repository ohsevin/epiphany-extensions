/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#ifndef EPHY_WINDOW_ACTION_H
#define EPHY_WINDOW_ACTION_H

#include <gtk/gtkaction.h>
#include <gmodule.h>

#include <epiphany/ephy-window.h>

G_BEGIN_DECLS

#define EPHY_TYPE_WINDOW_ACTION			(ephy_window_action_get_type ())
#define EPHY_WINDOW_ACTION(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), EPHY_TYPE_WINDOW_ACTION, EphyWindowAction))
#define EPHY_WINDOW_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), EPHY_TYPE_WINDOW_ACTION, EphyWindowActionClass))
#define EPHY_IS_WINDOW_ACTION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EPHY_TYPE_WINDOW_ACTION))
#define EPHY_IS_WINDOW_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), EPHY_TYPE_WINDOW_ACTION))
#define EPHY_WINDOW_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_WINDOW_ACTION, EphyWindowActionClass))

typedef struct _EphyWindowAction	EphyWindowAction;
typedef struct _EphyWindowActionClass	EphyWindowActionClass;
typedef struct _EphyWindowActionPrivate	EphyWindowActionPrivate;

struct _EphyWindowActionClass
{
	GtkActionClass parent_class;
};

struct _EphyWindowAction
{
	GtkAction parent;
	
	EphyWindowActionPrivate *priv;
};

GType		ephy_window_action_get_type		(void);

GType		ephy_window_action_register_type	(GTypeModule *module);

EphyWindow     *ephy_window_action_get_window		(EphyWindowAction *action);

G_END_DECLS

#endif
