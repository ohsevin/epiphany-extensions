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
 */

#ifndef JAVASCRIPT_CONSOLE_H
#define JAVASCRIPT_CONSOLE_H

#include <glib.h>
#include <glib-object.h>

#include "error-viewer.h"
#include <epiphany/ephy-embed.h>

G_BEGIN_DECLS

#define TYPE_SGML_VALIDATOR (sgml_validator_get_type ())
#define SGML_VALIDATOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SGML_VALIDATOR, SgmlValidator))
#define SGML_VALIDATOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), TYPE_SGML_VALIDATOR, SgmlValidatorClass))
#define IS_SGML_VALIDATOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SGML_VALIDATOR))
#define IS_SGML_VALIDATOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SGML_VALIDATOR))
#define SGML_VALIDATOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SGML_VALIDATOR, SgmlValidatorClass))

typedef struct SgmlValidator		SgmlValidator;
typedef struct SgmlValidatorClass	SgmlValidatorClass;
typedef struct SgmlValidatorPrivate	SgmlValidatorPrivate;

struct SgmlValidatorClass
{
	GObjectClass parent_class;
};

struct SgmlValidator
{
	GObject parent_instance;
	SgmlValidatorPrivate *priv;
};

GType		sgml_validator_get_type		(void);

GType		sgml_validator_register_type	(GTypeModule *module);

SgmlValidator	*sgml_validator_new		(ErrorViewer *viewer);

void		sgml_validator_validate		(SgmlValidator *validator,
						 EphyEmbed *embed);

G_END_DECLS

#endif

