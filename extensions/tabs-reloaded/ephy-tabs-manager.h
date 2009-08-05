/*
 *  Copyright © 2009 Benjamin Otte
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

#ifndef EPHY_TABS_MANAGER_H
#define EPHY_TABS_MANAGER_H

#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define EPHY_TYPE_TABS_MANAGER		(ephy_tabs_manager_get_type())
#define EPHY_TABS_MANAGER(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EPHY_TYPE_TABS_MANAGER, EphyTabsManager))
#define EPHY_TABS_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_TABS_MANAGER, EphyTabsManagerClass))
#define EPHY_IS_TABS_MANAGER(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), EPHY_TYPE_TABS_MANAGER))
#define EPHY_IS_TABS_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_TABS_MANAGER))
#define EPHY_TABS_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_TABS_MANAGER, EphyTabsManagerClass))

typedef struct _EphyTabsManager		EphyTabsManager;
typedef struct _EphyTabsManagerClass	EphyTabsManagerClass;
typedef struct _EphyTabsManagerPrivate	EphyTabsManagerPrivate;

struct _EphyTabsManager
{
    GtkTreeStore                store;

    GtkNotebook *               notebook;       /* notebook we are attached to */
};

struct _EphyTabsManagerClass
{
    GtkTreeStoreClass           store_class;
};

GType	                ephy_tabs_manager_get_type	(void);
void                    ephy_tabs_manager_register      (GTypeModule *          module);

EphyTabsManager *       ephy_tabs_manager_new           (void);

void                    ephy_tabs_manager_add_tab       (EphyTabsManager *      manager,
                                                         EphyEmbed *            embed,
                                                         int                    position);
void                    ephy_tabs_manager_remove_tab    (EphyTabsManager *      manager,
                                                         EphyEmbed *            embed);

EphyEmbed *             ephy_tabs_manager_get_tab       (EphyTabsManager *      manager,
                                                         GtkTreeIter *          iter);
gboolean                ephy_tabs_manager_find_tab      (EphyTabsManager *      manager,
                                                         GtkTreeIter *          iter,
                                                         EphyEmbed *            embed);

void                    ephy_tabs_manager_attach        (EphyTabsManager *      manager,
                                                         GtkNotebook *          notebook);


G_END_DECLS

#endif
