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

#ifndef RSS_FEEDLIST_H
#define RSS_FEEDLIST_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/* We store the newsfeed associated to an embed in this property */
#define FEEDLIST_DATA_KEY "ephy-rss-extension-feedlist"

#define RSS_TYPE_FEEDLIST	(rss_feedlist_get_type ())
#define RSS_TYPE_NEWSFEED	(rss_newsfeed_get_type ())

typedef struct
{
	char *type;
	char *title;
	char *address;
} NewsFeed;

typedef GSList FeedList;

/* NewsFeed */
GType		rss_newsfeed_get_type	(void);

NewsFeed       *rss_newsfeed_new	(void);

NewsFeed       *rss_newsfeed_copy	(const NewsFeed *feed);

void		rss_newsfeed_free	(NewsFeed *feed);

/* FeedList */
GType		rss_feedlist_get_type	(void);

FeedList       *rss_feedlist_new	(void);

FeedList       *rss_feedlist_copy	(const FeedList *feedlist);

void		rss_feedlist_free	(FeedList *feedlist);

guint		rss_feedlist_length	(FeedList *feedlist);

FeedList	*rss_feedlist_add 	(FeedList *feedlist,
					 const char *type,
					 const char *title,
					 const char *address);

gboolean	rss_feedlist_contains (FeedList *feedlist,
					 const char *url);
G_END_DECLS

#endif
