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

#ifndef AD_URI_TESTER_H
#define AD_URI_TESTER_H

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define TYPE_AD_URI_TESTER		(ad_uri_tester_get_type ())
#define AD_URI_TESTER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AD_URI_TESTER, AdUriTester))
#define AD_URI_TESTER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), TYPE_AD_URI_TESTER, AdUriTesterClass))
#define IS_AD_URI_TESTER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AD_URI_TESTER))
#define IS_AD_URI_TESTER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AD_URI_TESTER))
#define AD_URI_TESTER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AD_URI_TESTER, AdUriTesterClass))

typedef struct _AdUriTester		AdUriTester;
typedef struct _AdUriTesterClass	AdUriTesterClass;
typedef struct _AdUriTesterPrivate	AdUriTesterPrivate;

typedef enum
{
        AD_URI_CHECK_TYPE_OTHER       = 1U,
        AD_URI_CHECK_TYPE_SCRIPT      = 2U, /* Indicates an executable script
					       (such as JavaScript) */
        AD_URI_CHECK_TYPE_IMAGE       = 3U, /* Indicates an image (e.g., IMG
					       elements) */
        AD_URI_CHECK_TYPE_STYLESHEET  = 4U, /* Indicates a stylesheet (e.g.,
					       STYLE elements) */
        AD_URI_CHECK_TYPE_OBJECT      = 5U, /* Indicates a generic object
					       (plugin-handled content
					       typically falls under this
					       category) */
        AD_URI_CHECK_TYPE_DOCUMENT    = 6U, /* Indicates a document at the
					       top-level (i.e., in a
					       browser) */
	AD_URI_CHECK_TYPE_SUBDOCUMENT = 7U, /* Indicates a document contained
					       within another document (e.g.,
					       IFRAMEs, FRAMES, and OBJECTs) */
        AD_URI_CHECK_TYPE_REFRESH     = 8U  /* Indicates a timed refresh */
} AdUriCheckType;

struct _AdUriTester
{
	GObject parent_instance;

	/*< private >*/
	AdUriTesterPrivate *priv;
};

struct _AdUriTesterClass
{
	GObjectClass parent_class;
};

GType		 ad_uri_tester_get_type	(void);

GType		 ad_uri_tester_register_type	(GTypeModule *module);

AdUriTester	*ad_uri_tester_new		(void);

gboolean	 ad_uri_tester_test_uri		(AdUriTester *tester,
						 const char *uri,
						 AdUriCheckType type);

G_END_DECLS

#endif /* AD_URI_TESTER_H */
