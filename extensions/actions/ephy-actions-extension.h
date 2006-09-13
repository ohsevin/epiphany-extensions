/* 
 * Copyright © 2005 Jean-Yves Lefort <jylefort@brutele.be>
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

#ifndef _EPHY_ACTIONS_EXTENSION_H
#define _EPHY_ACTIONS_EXTENSION_H

#include <glib-object.h>
#include <epiphany/ephy-node-db.h>
#include <epiphany/ephy-node.h>
#include "ephy-actions-extension-properties-dialog.h"

G_BEGIN_DECLS

#define EPHY_TYPE_ACTIONS_EXTENSION		(ephy_actions_extension_type)
#define EPHY_ACTIONS_EXTENSION(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), EPHY_TYPE_ACTIONS_EXTENSION, EphyActionsExtension))
#define EPHY_ACTIONS_EXTENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_ACTIONS_EXTENSION, EphyActionsExtensionClass))
#define EPHY_IS_ACTIONS_EXTENSION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), EPHY_TYPE_ACTIONS_EXTENSION))
#define EPHY_IS_ACTIONS_EXTENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_ACTIONS_EXTENSION))
#define EPHY_ACTIONS_EXTENSION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_ACTIONS_EXTENSION, EphyActionsExtensionClass))

typedef struct _EphyActionsExtension		EphyActionsExtension;
typedef struct _EphyActionsExtensionClass	EphyActionsExtensionClass;
typedef struct _EphyActionsExtensionPrivate	EphyActionsExtensionPrivate;

struct _EphyActionsExtension
{
	GObject parent;

	/*< private >*/
	EphyActionsExtensionPrivate *priv;
};

struct _EphyActionsExtensionClass
{
	GObjectClass parent;

	/* signals */
	void (* actions_changed) (EphyActionsExtension *extension);
};

enum
{
	EPHY_ACTIONS_EXTENSION_ACTION_PROP_NAME,
	EPHY_ACTIONS_EXTENSION_ACTION_PROP_DESCRIPTION,
	EPHY_ACTIONS_EXTENSION_ACTION_PROP_COMMAND,
	EPHY_ACTIONS_EXTENSION_ACTION_PROP_APPLIES_TO_PAGES,
	EPHY_ACTIONS_EXTENSION_ACTION_PROP_APPLIES_TO_IMAGES
};

extern GType ephy_actions_extension_type;

GType		ephy_actions_extension_register_type
			(GTypeModule				*module);

void		ephy_actions_extension_add_properties_dialog
			(EphyActionsExtension			*extension,
			 EphyActionsExtensionPropertiesDialog	*dialog);
EphyActionsExtensionPropertiesDialog *
		ephy_actions_extension_get_properties_dialog
			(EphyActionsExtension			*extension,
			 EphyNode				*action);

EphyNodeDb *	ephy_actions_extension_get_db
			(EphyActionsExtension			*extension);
EphyNode *	ephy_actions_extension_get_actions
			(EphyActionsExtension			*extension);

G_END_DECLS

#endif /* _EPHY_ACTIONS_EXTENSION_H */
