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

#include "config.h"

#include "link-checker.h"
#include "mozilla-link-checker.h"

#include "ephy-debug.h"

#include <glib/gi18n-lib.h>

#define NUM_THREADS 10

#define LINK_CHECKER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_LINK_CHECKER, LinkCheckerPrivate))

#define MAX_NUM_ROWS 50

static void link_checker_class_init (LinkCheckerClass *klass);
static void link_checker_class_finalize (LinkCheckerClass *klass);
static void link_checker_init (LinkChecker *dialog);
static void link_checker_finalize (GObject *object);

static GObjectClass *parent_class = NULL;

static GType type = 0;

typedef struct
{
	LinkChecker *checker;
	ErrorViewerErrorType error_type;
	char *message;
} LinkCheckerAppendCBData;

struct LinkCheckerPrivate
{
	ErrorViewer *error_viewer;
};

GType
link_checker_get_type (void)
{
	return type;
}

GType
link_checker_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (LinkCheckerClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) link_checker_class_init,
		(GClassFinalizeFunc) link_checker_class_finalize,
		NULL, /* class_data */
		sizeof (LinkChecker),
		0, /* n_preallocs */
		(GInstanceInitFunc) link_checker_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "LinkChecker",
					    &our_info, 0);

	return type;
}

static void
free_link_checker_append_cb_data (gpointer data)
{
	LinkCheckerAppendCBData *cb_data;

	if (data)
	{
		cb_data = (LinkCheckerAppendCBData *) data;

		if (IS_LINK_CHECKER (cb_data->checker))
		{
			g_object_unref (cb_data->checker);
		}

		if (cb_data->message)
		{
			g_free (cb_data->message);
		}

		g_free (cb_data);
	}
}

static gboolean
link_checker_append_internal (gpointer data)
{
	LinkCheckerAppendCBData *append_data;

	g_return_val_if_fail (data != NULL, FALSE);

	append_data = (LinkCheckerAppendCBData *) data;

	g_return_val_if_fail (IS_LINK_CHECKER (append_data->checker), FALSE);
	g_return_val_if_fail (IS_ERROR_VIEWER
			   (append_data->checker->priv->error_viewer), FALSE);
	g_return_val_if_fail (append_data->message != NULL, FALSE);

	error_viewer_append (append_data->checker->priv->error_viewer,
			     append_data->error_type,
			     append_data->message);

	return FALSE;
}

void
link_checker_append (LinkChecker *checker,
		     ErrorViewerErrorType error_type,
		     const char *message)
{
	LinkCheckerAppendCBData *cb_data;

	g_return_if_fail (IS_LINK_CHECKER (checker));
	g_return_if_fail (message != NULL);

	/* GTK interaction must be done in the main thread */
	cb_data = g_new0 (LinkCheckerAppendCBData, 1);
	g_object_ref (checker);
	cb_data->checker = checker;
	cb_data->error_type = error_type;
	cb_data->message = g_strdup (message);

	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			 link_checker_append_internal, cb_data,
			 free_link_checker_append_cb_data);
}

LinkChecker *
link_checker_new (ErrorViewer *viewer)
{
	LinkChecker *retval;

	retval = g_object_new (TYPE_LINK_CHECKER, NULL);

	g_object_ref (viewer);

	retval->priv->error_viewer = viewer;

	return retval;
}

static void
link_checker_class_init (LinkCheckerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = link_checker_finalize;

	g_type_class_add_private (object_class, sizeof (LinkCheckerPrivate));

	mozilla_register_link_checker_component ();
}

static void
link_checker_class_finalize (LinkCheckerClass *klass)
{
	mozilla_unregister_link_checker_component ();
}

static void
link_checker_init (LinkChecker *checker)
{
	LOG ("LinkChecker initializing %p", checker);

	checker->priv = LINK_CHECKER_GET_PRIVATE (checker);
}

static void
link_checker_finalize (GObject *object)
{
	LinkCheckerPrivate *priv = LINK_CHECKER_GET_PRIVATE (LINK_CHECKER (object));

	LOG ("LinkChecker finalizing %p", object);

	g_object_unref (priv->error_viewer);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

void
link_checker_check (LinkChecker *checker,
		    EphyEmbed *embed)
{
	g_return_if_fail (IS_LINK_CHECKER (checker));
	g_return_if_fail (EPHY_IS_EMBED (embed));

	mozilla_check_links (checker, embed);
}

void
link_checker_use (LinkChecker *checker)
{
	g_return_if_fail (IS_LINK_CHECKER (checker));

	error_viewer_use (checker->priv->error_viewer);
}

void
link_checker_unuse (LinkChecker *checker)
{
	g_return_if_fail (IS_LINK_CHECKER (checker));

	error_viewer_unuse (checker->priv->error_viewer);
}
