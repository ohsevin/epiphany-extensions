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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sgml-validator.h"

#include "opensp/validate.h"

#include <epiphany/ephy-embed-persist.h>
#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include "get-doctype.h"

#include <unistd.h>

#include <glib/gi18n-lib.h>

#define SGML_VALIDATOR_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_SGML_VALIDATOR, SgmlValidatorPrivate))

#define MAX_NUM_ROWS 50

static void sgml_validator_class_init (SgmlValidatorClass *klass);
static void sgml_validator_init (SgmlValidator *dialog);
static void sgml_validator_finalize (GObject *object);

static GObjectClass *parent_class = NULL;

static GType type = 0;

typedef struct
{
	char *dest;
	char *location;
	SgmlValidator *validator;
	gboolean is_xml;
} OpenSPThreadCBData;

struct SgmlValidatorPrivate
{
	ErrorViewer *error_viewer;
};

GType
sgml_validator_get_type (void)
{
	return type;
}

GType
sgml_validator_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (SgmlValidatorClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) sgml_validator_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (SgmlValidator),
		0, /* n_preallocs */
		(GInstanceInitFunc) sgml_validator_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "SgmlValidator",
					    &our_info, 0);

	return type;
}

SgmlValidator *
sgml_validator_new (ErrorViewer *viewer)
{
	SgmlValidator *retval;

	retval = g_object_new (TYPE_SGML_VALIDATOR, NULL);

	g_object_ref (viewer);

	retval->priv->error_viewer = viewer;

	return retval;
}

static void
sgml_validator_class_init (SgmlValidatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = sgml_validator_finalize;

	g_type_class_add_private (object_class, sizeof (SgmlValidatorPrivate));
}

static void
sgml_validator_init (SgmlValidator *validator)
{
	validator->priv = SGML_VALIDATOR_GET_PRIVATE (validator);
}

static void
sgml_validator_finalize (GObject *object)
{
	SgmlValidatorPrivate *priv = SGML_VALIDATOR_GET_PRIVATE (SGML_VALIDATOR (object));

	g_object_unref (priv->error_viewer);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gpointer
opensp_thread (gpointer data)
{
	guint num_errors;
	char *summary;
	OpenSPThreadCBData *osp_data;

	osp_data = (OpenSPThreadCBData *) data;

	num_errors = validate (osp_data->dest,
			       osp_data->location,
			       osp_data->validator->priv->error_viewer,
			       osp_data->is_xml);

	summary = g_strdup_printf
		(ngettext ("HTML Validation of %s complete\nFound %d error",
			   "HTML Validation of %s complete\nFound %d errors",
			   num_errors),
		 osp_data->location, num_errors);

	error_viewer_append (osp_data->validator->priv->error_viewer,
			     ERROR_VIEWER_INFO, summary);

	g_free (summary);

	unlink (osp_data->dest);
	g_free (osp_data->dest);
	g_free (osp_data->location);
	g_object_unref (osp_data->validator);
	g_free (osp_data);

	return NULL;
}

static void
save_source_completed_cb (EphyEmbedPersist *persist,
			  SgmlValidator *validator)
{
	const char *dest;
	gboolean is_xml = FALSE;
	char *doctype;
	char *location;
	char *content_type;
	char *t;
	OpenSPThreadCBData *data;
	EphyEmbed *embed;

	g_return_if_fail (EPHY_IS_EMBED_PERSIST (persist));
	g_return_if_fail (IS_SGML_VALIDATOR (validator));

	dest = ephy_embed_persist_get_dest (persist);
	g_return_if_fail (dest != NULL);

	embed = ephy_embed_persist_get_embed (persist);
	doctype = mozilla_get_doctype (embed);
	if (!doctype)
	{
		location = ephy_embed_get_location (embed, FALSE);

		t = g_strdup_printf
			(_("HTML error in %s:\nNo valid doctype specified."),
			 location);

		g_free (location);

		error_viewer_append (validator->priv->error_viewer,
				     ERROR_VIEWER_ERROR, t);

		g_free (t);
		return;
	}
	if (strstr (doctype, "XHTML"))
	{
		content_type = mozilla_get_content_type (embed);

		if (strcmp (content_type, "text/html") == 0)
		{
			location = ephy_embed_get_location (embed, FALSE);

			t = g_strdup_printf
				(_("HTML error in %s:\nDoctype is XHTML "
				   "but content type is text/html"),
				 location);

			g_free (location);

			error_viewer_append (validator->priv->error_viewer,
					     ERROR_VIEWER_ERROR, t);
		}

		g_free (content_type);

		is_xml = TRUE;
	}
	g_free (doctype);

	if (!g_thread_supported ()) g_thread_init (NULL);

	data = g_new0 (OpenSPThreadCBData, 1);
	data->dest = g_strdup (dest);
	data->location = ephy_embed_get_location (embed, FALSE);
	g_object_ref (validator);
	data->validator = validator;
	data->is_xml = is_xml;

	g_thread_create (opensp_thread, data, FALSE, NULL);
}

void
sgml_validator_validate (SgmlValidator *validator,
			 EphyEmbed *embed)
{
	EphyEmbedPersist *persist;
	char *tmp, *base;
	const char *static_tmp_dir;

	static_tmp_dir = ephy_file_tmp_dir ();
	g_return_if_fail (static_tmp_dir != NULL);

	base = g_build_filename (static_tmp_dir, "validateXXXXXX", NULL);
	tmp = ephy_file_tmp_filename (base, "html");
	g_free (base);
	g_return_if_fail (tmp != NULL);

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object ("EphyEmbedPersist"));

	ephy_embed_persist_set_embed (persist, embed);
	ephy_embed_persist_set_flags (persist, EMBED_PERSIST_NO_VIEW |
				      EMBED_PERSIST_COPY_PAGE);
	ephy_embed_persist_set_dest (persist, tmp);

	g_signal_connect (persist, "completed",
			  G_CALLBACK (save_source_completed_cb), validator);

	ephy_embed_persist_save (persist);

	g_object_unref (persist);
	g_free (tmp);
}
