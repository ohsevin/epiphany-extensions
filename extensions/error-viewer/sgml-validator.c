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
#include <epiphany/ephy-embed-factory.h>

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include "mozilla-helpers.h"

#include <unistd.h>
#include <string.h>

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
	int num_errors;
} OpenSPThreadCBData;

typedef struct
{
	SgmlValidator *validator;
	ErrorViewerErrorType error_type;
	char *message;
} SgmlValidatorAppendCBData;

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

	g_setenv ("SP_CHARSET_FIXED", "YES", TRUE);
	g_setenv ("SP_SYSTEM_CHARSET", "utf-8", TRUE);
	g_setenv ("SP_ENCODING", "utf-8", TRUE);
}

static void
sgml_validator_init (SgmlValidator *validator)
{
	LOG ("SgmlValidator initializing %p", validator)

	validator->priv = SGML_VALIDATOR_GET_PRIVATE (validator);
}

static void
sgml_validator_finalize (GObject *object)
{
	SgmlValidatorPrivate *priv = SGML_VALIDATOR_GET_PRIVATE (SGML_VALIDATOR (object));

	LOG ("SgmlValidator finalizing %p", object)

	g_object_unref (priv->error_viewer);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gpointer
opensp_thread (gpointer data)
{
	char *summary, *complete, *errors;
	OpenSPThreadCBData *osp_data;

	osp_data = (OpenSPThreadCBData *) data;

	osp_data->num_errors +=
		validate (osp_data->dest, osp_data->location,
			  osp_data->validator, osp_data->is_xml);

	errors = g_strdup_printf (ngettext ("Found %d error",
					    "Found %d errors",
					    osp_data->num_errors),
				  osp_data->num_errors);
	complete = g_strdup_printf (_("HTML Validation of %s complete"),
				    osp_data->location);

	summary = g_strconcat (complete, "\n", errors, NULL);

	sgml_validator_append (osp_data->validator, ERROR_VIEWER_INFO, summary);

	g_free (summary);
	g_free (complete);
	g_free (errors);

	error_viewer_unuse (osp_data->validator->priv->error_viewer);

	unlink (osp_data->dest);
	g_free (osp_data->dest);
	g_free (osp_data->location);
	g_object_unref (osp_data->validator);
	g_free (osp_data);

	return NULL;
}

static void
check_doctype (SgmlValidator *validator,
	       EphyEmbed *embed,
	       gboolean *is_xml,
	       unsigned int *num_errors)
{
	char *doctype;
	char *location;
	char *content_type;
	char *t;

	*is_xml = FALSE;

	doctype = mozilla_get_doctype (embed);

	/* If it's HTML, ignore content type (it won't be wrong) */
	if (strstr (doctype, "XHTML") == NULL)
	{
		g_free (doctype);
		return;
	}

	content_type = mozilla_get_content_type (embed);

	if (strcmp (content_type, "text/html") != 0)
	{
		*is_xml = TRUE;
	}
	else
	{
		/* Follow specs, people! w3's validator doesn't warn as much */
		location = ephy_embed_get_location (embed, FALSE);

		if (strstr (doctype, "XHTML 1.1"))
		{
			t = g_strdup_printf
				(_("HTML error in %s:\n%s"),
				 location,
				 _("Doctype is \"XHTML\""
				   " but content type is \"text/html\". "
				   " Use \"application/xhtml+xml\" instead."));

			sgml_validator_append (validator,
					       ERROR_VIEWER_ERROR, t);

			g_free (t);

			*num_errors++;
		}
		else
		{
			t = g_strdup_printf
				(_("HTML warning in %s:\n%s"),
				 location,
				 _("Doctype is \"XHTML\""
				   " but content type is \"text/html\". "
				   " Use \"application/xhtml+xml\" instead."));

			sgml_validator_append (validator,
					       ERROR_VIEWER_WARNING, t);

			g_free (t);
		}

		g_free (location);
	}

	g_free (content_type);
	g_free (doctype);
	return;
}

static GError *
convert_to_utf8 (const char *file,
		 char **new_file,
		 EphyEmbed *embed)
{
	const char *static_tmp_dir;
	char *encoding;
	char *base;
	char *buf;
	gsize buf_size = 4096;
	gsize len;
	GIOChannel *in;
	GIOChannel *out;
	GIOStatus status;
	GError *err = NULL;

	static_tmp_dir = ephy_file_tmp_dir ();
	g_return_val_if_fail (static_tmp_dir != NULL, NULL);

	base = g_build_filename (static_tmp_dir, "validateXXXXXX", NULL);
	*new_file = ephy_file_tmp_filename (base, "html");
	g_free (base);
	g_return_val_if_fail (*new_file != NULL, NULL);

	encoding = ephy_embed_get_encoding (embed);

	in = g_io_channel_new_file (file, "r", NULL);
	g_return_val_if_fail (in != NULL, NULL);
	status = g_io_channel_set_encoding (in, encoding, &err);
	g_return_val_if_fail (status == G_IO_STATUS_NORMAL, err);

	g_free (encoding);

	out = g_io_channel_new_file (*new_file, "w", NULL);
	g_return_val_if_fail (out != NULL, NULL);
	status = g_io_channel_set_encoding (out, "UTF-8", &err);
	g_return_val_if_fail (status == G_IO_STATUS_NORMAL, err);

	buf = g_malloc0 (sizeof (char) * buf_size);
	g_return_val_if_fail (buf != NULL, NULL);

	while (TRUE)
	{
		status = g_io_channel_read_chars (in, buf, buf_size, &len,
						  &err);
		if (status == G_IO_STATUS_EOF
		    || status == G_IO_STATUS_ERROR) break;

		status = g_io_channel_write_chars (out, buf, len, NULL, &err);
		if (status == G_IO_STATUS_ERROR) break;
	}

	g_free (buf);

	g_io_channel_unref (in);
	g_io_channel_unref (out);

	return err;
}

static void
save_source_completed_cb (EphyEmbedPersist *persist,
			  SgmlValidator *validator)
{
	const char *dest;
	char *location;
	char *dest_utf8;
	gboolean is_xml;
	unsigned int num_errors = 0;
	OpenSPThreadCBData *data;
	EphyEmbed *embed;
	GError *err = NULL;

	g_return_if_fail (EPHY_IS_EMBED_PERSIST (persist));
	g_return_if_fail (IS_SGML_VALIDATOR (validator));

	dest = ephy_embed_persist_get_dest (persist);
	g_return_if_fail (dest != NULL);

	embed = ephy_embed_persist_get_embed (persist);
	check_doctype (validator, embed, &is_xml, &num_errors);

	location = ephy_embed_get_location (embed, FALSE);

	/*
	 * Convert to UTF-8.
	 * We use a second temp file because I don't trust Mozilla to convert
	 * and not change any of the HTML. Not to mention, it's as good a
	 * chance as any to catch encoding errors.
	 */
	err = convert_to_utf8 (dest, &dest_utf8, embed);
	if (err != NULL)
	{
		char *msg;
		char *err_head;
		char *err_desc;

		err_head = g_strdup_printf (_("HTML error in %s:"),
					    location);

		if (g_error_matches (err,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_ILLEGAL_SEQUENCE))
		{
			err_desc = g_strdup_printf (
				_("Invalid character encoding"));
		}
		else
		{
			err_desc = g_strdup_printf (
				_("Unknown error while converting to UTF-8"));
		}

		msg = g_strconcat (err_head, "\n", err_desc, NULL);

		sgml_validator_append (validator, ERROR_VIEWER_ERROR,
				       msg);

		g_free (msg);
		g_free (err_head);
		g_free (err_desc);

		g_error_free (err);

		error_viewer_unuse (validator->priv->error_viewer);

		unlink (dest);

		if (dest_utf8 != NULL
		    && g_file_test (dest_utf8, G_FILE_TEST_EXISTS))
		{
			unlink (dest_utf8);
		}

		g_free (dest_utf8);

		return;
	}
	g_return_if_fail (dest_utf8 != NULL);

	unlink (dest);

	if (!g_thread_supported ()) g_thread_init (NULL);

	data = g_new0 (OpenSPThreadCBData, 1);
	data->dest = dest_utf8;
	data->location = location;
	g_object_ref (validator);
	data->validator = validator;
	data->is_xml = is_xml;
	data->num_errors = num_errors;

	g_thread_create (opensp_thread, data, FALSE, NULL);
}

void
sgml_validator_validate (SgmlValidator *validator,
			 EphyEmbed *embed)
{
	EphyEmbedPersist *persist;
	char *tmp, *base;
	char *doctype, *location, *t;
	const char *static_tmp_dir;

	/* If there's no doctype, don't validate. */
	doctype = mozilla_get_doctype (embed);

	if (!doctype)
	{
		location = ephy_embed_get_location (embed, FALSE);

		t = g_strdup_printf
			(_("HTML error in %s:\nNo valid doctype specified."),
			 location);

		sgml_validator_append (validator, ERROR_VIEWER_ERROR, t);

		g_free (location);
		g_free (t);
		return;
	}

	g_free (doctype);

	error_viewer_use (validator->priv->error_viewer);

	/* Okay, save to a temp file and validate. */
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

static void
free_sgml_validator_append_cb_data (gpointer data)
{
	SgmlValidatorAppendCBData *cb_data;

	if (data)
	{
		cb_data = (SgmlValidatorAppendCBData *) data;

		g_object_unref (cb_data->validator);
		g_free (cb_data->message);
		g_free (cb_data);
	}
}

static gboolean
sgml_validator_append_internal (gpointer data)
{
	SgmlValidatorAppendCBData *append_data;

	g_return_val_if_fail (data != NULL, FALSE);

	append_data = (SgmlValidatorAppendCBData *) data;

	error_viewer_append (append_data->validator->priv->error_viewer,
			     append_data->error_type,
			     append_data->message);

	return FALSE;
}

void
sgml_validator_append (SgmlValidator *validator,
		       ErrorViewerErrorType error_type,
		       const char *message)
{
	SgmlValidatorAppendCBData *cb_data;

	g_return_if_fail (IS_SGML_VALIDATOR (validator));
	g_return_if_fail (message != NULL);

	/* GTK interaction must be done in the main thread */
	cb_data = g_new0 (SgmlValidatorAppendCBData, 1);
	g_object_ref (validator);
	cb_data->validator = validator;
	cb_data->error_type = error_type;
	cb_data->message = g_strdup (message);

	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			 sgml_validator_append_internal, cb_data,
			 free_sgml_validator_append_cb_data);
}
