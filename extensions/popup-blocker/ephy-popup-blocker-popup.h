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

#ifndef EPHY_POPUP_BLOCKER_POPUP_H
#define EPHY_POPUP_BLOCKER_POPUP_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_POPUP_BLOCKER_POPUP (ephy_popup_blocker_popup_get_type ())
#define EPHY_POPUP_BLOCKER_POPUP(o)	(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_POPUP_BLOCKER_POPUP, EphyPopupBlockerPopup))
#define EPHY_POPUP_BLOCKER_POPUP_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_POPUP_BLOCKER_POPUP, EphyPopupBlockerPopupClass))
#define EPHY_IS_POPUP_BLOCKER_POPUP(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_POPUP_BLOCKER_POPUP))
#define EPHY_IS_POPUP_BLOCKER_POPUP_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_POPUP_BLOCKER_POPUP))
#define EPHY_POPUP_BLOCKER_POPUP_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_POPUP_BLOCKER_POPUP, EphyPopupBlockerPopupClass))

typedef struct EphyPopupBlockerPopup		EphyPopupBlockerPopup;
typedef struct EphyPopupBlockerPopupClass	EphyPopupBlockerPopupClass;
typedef struct EphyPopupBlockerPopupPrivate	EphyPopupBlockerPopupPrivate;

struct EphyPopupBlockerPopupClass
{
	GObjectClass parent_class;
};

struct EphyPopupBlockerPopup
{
	GObject parent_instance;

	EphyPopupBlockerPopupPrivate *priv;
};

GType	ephy_popup_blocker_popup_get_type	(void);

GType	ephy_popup_blocker_popup_register_type	(GTypeModule *module);

EphyPopupBlockerPopup	*ephy_popup_blocker_popup_new	(const char *url,
							 const char *features);

const char	*ephy_popup_blocker_popup_get_url	(EphyPopupBlockerPopup *popup);

const char	*ephy_popup_blocker_popup_get_features	(EphyPopupBlockerPopup *popup);

G_END_DECLS

#endif /* EPHY_POPUP_BLOCKER_POPUP_H */
