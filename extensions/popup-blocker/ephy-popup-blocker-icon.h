/*
 *  Copyright (C) 2003 Adam Hooper
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

#ifndef EPHY_POPUP_BLOCKER_ICON_H
#define EPHY_POPUP_BLOCKER_ICON_H

#include <epiphany/ephy-window.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_POPUP_BLOCKER_ICON (ephy_popup_blocker_icon_get_type ())
#define EPHY_POPUP_BLOCKER_ICON(o)	(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_POPUP_BLOCKER_ICON, EphyPopupBlockerIcon))
#define EPHY_POPUP_BLOCKER_ICON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_POPUP_BLOCKER_ICON, EphyPopupBlockerIconClass))
#define EPHY_IS_POPUP_BLOCKER_ICON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_POPUP_BLOCKER_ICON))
#define EPHY_IS_POPUP_BLOCKER_ICON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_POPUP_BLOCKER_ICON))
#define EPHY_POPUP_BLOCKER_ICON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_POPUP_BLOCKER_ICON, EphyPopupBlockerIconClass))

typedef struct EphyPopupBlockerIcon		EphyPopupBlockerIcon;
typedef struct EphyPopupBlockerIconClass	EphyPopupBlockerIconClass;
typedef struct EphyPopupBlockerIconPrivate	EphyPopupBlockerIconPrivate;

struct EphyPopupBlockerIconClass
{
	GObjectClass parent_class;
};

struct EphyPopupBlockerIcon
{
	GObject parent_instance;

	EphyPopupBlockerIconPrivate *priv;
};

GType	ephy_popup_blocker_icon_get_type	(void);

GType	ephy_popup_blocker_icon_register_type	(GTypeModule *module);

EphyPopupBlockerIcon	*ephy_popup_blocker_icon_new	(EphyWindow *window);

void	ephy_popup_blocker_icon_set_popups	(EphyPopupBlockerIcon *icon,
						 GSList *blocked_list);

G_END_DECLS

#endif /* EPHY_POPUP_BLOCKER_H */
