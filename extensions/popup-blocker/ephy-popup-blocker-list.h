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

#ifndef EPHY_POPUP_BLOCKER_LIST_H
#define EPHY_POPUP_BLOCKER_LIST_H

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-window.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_POPUP_BLOCKER_LIST (ephy_popup_blocker_list_get_type ())
#define EPHY_POPUP_BLOCKER_LIST(o)	(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_POPUP_BLOCKER_LIST, EphyPopupBlockerList))
#define EPHY_POPUP_BLOCKER_LIST_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_POPUP_BLOCKER_LIST, EphyPopupBlockerListClass))
#define EPHY_IS_POPUP_BLOCKER_LIST(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_POPUP_BLOCKER_LIST))
#define EPHY_IS_POPUP_BLOCKER_LIST_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_POPUP_BLOCKER_LIST))
#define EPHY_POPUP_BLOCKER_LIST_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_POPUP_BLOCKER_LIST, EphyPopupBlockerListClass))

typedef struct EphyPopupBlockerList		EphyPopupBlockerList;
typedef struct EphyPopupBlockerListClass	EphyPopupBlockerListClass;
typedef struct EphyPopupBlockerListPrivate	EphyPopupBlockerListPrivate;

struct EphyPopupBlockerListClass
{
	GObjectClass parent_class;
};

struct EphyPopupBlockerList
{
	GObject parent_instance;

	EphyPopupBlockerListPrivate *priv;
};

GType			ephy_popup_blocker_list_get_type	(void);

GType			ephy_popup_blocker_list_register_type	(GTypeModule *module);

EphyPopupBlockerList	*ephy_popup_blocker_list_new		(EphyEmbed *embed);

void			ephy_popup_blocker_list_reset		(EphyPopupBlockerList *list);

void			ephy_popup_blocker_list_insert		(EphyPopupBlockerList *list,
								 const char *url,
								 const char *features);

void			ephy_popup_blocker_list_insert_window	(EphyPopupBlockerList *list,
								 EphyWindow *window);

void			ephy_popup_blocker_list_show_all	(EphyPopupBlockerList *list);

void			ephy_popup_blocker_list_hide_all	(EphyPopupBlockerList *list);

G_END_DECLS

#endif /* EPHY_POPUP_BLOCKER_LIST_H */
