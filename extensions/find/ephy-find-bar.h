/*
*  Copyright (C) 2004 Tommi Komulainen
*  Copyright (C) 2004 Christian Persch
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*  $Id$
*/

#ifndef EPHY_FIND_BAR_H
#define EPHY_FIND_BAR_H

#include "eggfindbar.h"

#include <epiphany/ephy-window.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define EPHY_TYPE_FIND_BAR		(ephy_find_bar_get_type ())
#define EPHY_FIND_BAR(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_FIND_BAR, EphyFindBar))
#define EPHY_FIND_BAR_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_FIND_BAR, EphyFindBarClass))
#define EPHY_IS_FIND_BAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_FIND_BAR))
#define EPHY_IS_FIND_BAR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_FIND_BAR))
#define EPHY_FIND_BAR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_FIND_BAR, EphyFindBarClass))

typedef struct _EphyFindBar		EphyFindBar;
typedef struct _EphyFindBarPrivate	EphyFindBarPrivate;
typedef struct _EphyFindBarClass	EphyFindBarClass;

struct _EphyFindBar
{
	EggFindBar parent;

	/*< private >*/
	EphyFindBarPrivate *priv;
};

struct _EphyFindBarClass
{
	EggFindBarClass parent_class;
};

GType		ephy_find_bar_get_type		(void) G_GNUC_CONST;

GType		ephy_find_bar_register_type	(GTypeModule *module);

GtkWidget      *ephy_find_bar_new		(EphyWindow *window);

G_END_DECLS

#endif /* EPHY_FIND_BAR_H */
