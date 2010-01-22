/*
 *  Copyright © 2005 Rapha� Slinckx <raphael@slinckx.net>
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

#include "ephy-debug.h"

#include "rss-feedlist.h"

#include <strings.h>

/* NewsFeed */
GType
rss_newsfeed_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0))
	{
		type = g_boxed_type_register_static ("NewsFeed",
						     (GBoxedCopyFunc) rss_newsfeed_copy,
						     (GBoxedFreeFunc) rss_newsfeed_free);
	}

	return type;
}

NewsFeed *
rss_newsfeed_new (void)
{
	return g_new0 (NewsFeed, 1);
}

NewsFeed *
rss_newsfeed_copy (const NewsFeed *feed)
{
	NewsFeed *copy = rss_newsfeed_new ();

	copy->type = g_strdup (feed->type);
	copy->title = g_strdup (feed->title);
	copy->address = g_strdup (feed->address);

	return copy;
}

void
rss_newsfeed_free (NewsFeed *feed)
{
	if (feed != NULL)
	{
		g_free (feed->type);
		g_free (feed->title);
		g_free (feed->address);

		g_free (feed);
	}
}

/* FeedList methods */
GType
rss_feedlist_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0))
	{
		type = g_boxed_type_register_static ("FeedList",
						     (GBoxedCopyFunc) rss_feedlist_copy,
						     (GBoxedFreeFunc) rss_feedlist_free);
	}

	return type;
}

FeedList *
rss_feedlist_new (void)
{
	return NULL;
}

static void
rss_feedlist_copy_foreach (NewsFeed *feed,
			   GSList **list)
{
	*list = g_slist_prepend (*list, rss_newsfeed_copy (feed));
}

FeedList *
rss_feedlist_copy (const FeedList *feedlist)
{
	FeedList *copy = NULL;

	g_slist_foreach ((GSList *) feedlist, (GFunc) rss_feedlist_copy_foreach, &copy);

	return copy;
}

/* Completely delete the list, including newsfeeds */
void
rss_feedlist_free (FeedList *feedlist)
{
	GSList *list = (GSList*) feedlist;

	g_slist_foreach (list, (GFunc) rss_newsfeed_free, NULL);
	g_slist_free (list);
}

guint
rss_feedlist_length (FeedList *feedlist)
{
	return g_slist_length ((GSList*) feedlist);
}

FeedList *
rss_feedlist_add (FeedList *feedlist,
		  const char *type,
		  const char *title,
		  const char *address)
{
	GSList *list = (GSList *) feedlist;
	NewsFeed *feed;

	/* Alloc the new feed structure */
	feed = rss_newsfeed_new ();
	feed->type = g_strdup (type);
	feed->title = g_strdup (title);
	feed->address = g_strdup (address);

	return (FeedList *) g_slist_prepend (list, feed);
}

static gint
rss_feedlist_compare_feeds (gconstpointer a, gconstpointer b)
{
	NewsFeed *feed = (NewsFeed *) a;
	const char *address = (char *) b;

	return g_ascii_strcasecmp (feed->address, address);
}

gboolean
rss_feedlist_contains (FeedList *feedlist,
		       const char *address)
{
	return g_slist_find_custom ((GSList *) feedlist, address, (GCompareFunc) rss_feedlist_compare_feeds) != NULL;
}
