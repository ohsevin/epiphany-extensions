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

#ifndef _EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_H
#define _EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_H

#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG			(ephy_actions_extension_properties_dialog_type)
#define EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG, EphyActionsExtensionPropertiesDialog))
#define EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG, EphyActionsExtensionPropertiesDialogClass))
#define EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG))
#define EPHY_IS_ACTIONS_EXTENSION_PROPERTIES_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG))
#define EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), EPHY_TYPE_ACTIONS_EXTENSION_PROPERTIES_DIALOG, EphyActionsExtensionPropertiesDialogClass))

typedef struct _EphyActionsExtensionPropertiesDialog		EphyActionsExtensionPropertiesDialog;
typedef struct _EphyActionsExtensionPropertiesDialogClass	EphyActionsExtensionPropertiesDialogClass;
typedef struct _EphyActionsExtensionPropertiesDialogPrivate	EphyActionsExtensionPropertiesDialogPrivate;

struct _EphyActionsExtensionPropertiesDialog
{
  EphyDialog parent;

  /*< private >*/
  EphyActionsExtensionPropertiesDialogPrivate *priv;
};

struct _EphyActionsExtensionPropertiesDialogClass
{
  EphyDialogClass parent_class;
};

extern GType ephy_actions_extension_properties_dialog_type;

GType		ephy_actions_extension_properties_dialog_register_type
			(GTypeModule				*module);

EphyActionsExtensionPropertiesDialog *
		ephy_actions_extension_properties_dialog_new
			(EphyExtension				*extension,
			 EphyNode				*action);

EphyNode *	ephy_actions_extension_properties_dialog_get_action
			(EphyActionsExtensionPropertiesDialog	*dialog);

G_END_DECLS

#endif /* _EPHY_ACTIONS_EXTENSION_PROPERTIES_DIALOG_H */
