/*
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2005 Jean-François Rameau
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "mozilla-config.h"

#include "config.h"

#include "LiveHTTPHeadersListener.h"
#include "mozilla-helper.h"

#undef MOZILLA_INTERNAL_API
#include <nsEmbedString.h>
#define MOZILLA_INTERNAL_API 1
#include <nsIHttpChannel.h>
#include <nsIHttpHeaderVisitor.h>
#include <nsIHttpProtocolHandler.h>
#include <nsCOMPtr.h>

class HeaderVisitor : public nsIHttpHeaderVisitor
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSIHTTPHEADERVISITOR

	HeaderVisitor();
	~HeaderVisitor();

	nsEmbedCString mHeader;	
};

NS_IMPL_ISUPPORTS1(HeaderVisitor, nsIHttpHeaderVisitor)
HeaderVisitor::HeaderVisitor()
{
}

HeaderVisitor::~HeaderVisitor()
{
}

/* void visitHeader (in ACString aHeader, in ACString aValue); */
NS_IMETHODIMP HeaderVisitor::VisitHeader(const nsACString & aHeader, const nsACString & aValue)
{
	mHeader.Append (aHeader);
	mHeader.Append (':');
	mHeader.Append (aValue);
	mHeader.Append ('\n');

	return NS_OK;
}

NS_IMPL_ISUPPORTS1(LiveHTTPHeadersListener, nsIObserver)

LiveHTTPHeadersListener::LiveHTTPHeadersListener()
{
	mVisitor = new HeaderVisitor ();
	NS_ADDREF (mVisitor);
	mHeaders = NULL;
	mUrls    = g_hash_table_new (g_str_hash, g_str_equal);
}

LiveHTTPHeadersListener::~LiveHTTPHeadersListener()
{
	NS_RELEASE (mVisitor);
	g_hash_table_destroy (mUrls);
}

NS_IMETHODIMP LiveHTTPHeadersListener::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *aData)
{
	nsCOMPtr<nsIHttpChannel> protocol = do_QueryInterface (aSubject);
	NS_ENSURE_TRUE (protocol, NS_ERROR_FAILURE);

	nsEmbedCString name;
	protocol->GetName (name);

	if (strcmp (aTopic, NS_HTTP_ON_MODIFY_REQUEST_TOPIC) == 0)
	{
		protocol->VisitRequestHeaders (mVisitor);

		/* Create a new frame for this header */
		LiveHTTPHeaderFrame *frame = g_new (LiveHTTPHeaderFrame, 1);

		/* Register the request */
		frame->request = g_strdup((mVisitor->mHeader).get());
		frame->response = NULL;
		frame->url = g_strdup (name.get ());
		mHeaders = g_slist_append (mHeaders, (gpointer)frame);
		g_hash_table_insert (mUrls, (gpointer)name.get (), (gpointer)frame);
	}
	else if (strcmp (aTopic, NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC) == 0)
	{
		protocol->VisitResponseHeaders (mVisitor);

		/* Look for the request */
		LiveHTTPHeaderFrame *frame = (LiveHTTPHeaderFrame *) g_hash_table_lookup (mUrls, name.get ());
		if (frame != NULL)
		{
			/* Request/response tuple */
			frame->response = g_strdup((mVisitor->mHeader).get());
		}
	}

	mVisitor->mHeader = "";
	
	return NS_OK;
}

static void
free_frame (LiveHTTPHeaderFrame *frame)
{
	g_free (frame->url);
	g_free (frame->response);
	g_free (frame->request);
	g_free (frame);
}

void LiveHTTPHeadersListener::ClearFrames (void)
{
	/* Be sure the hash table itself is not destroyed */
	g_hash_table_ref (mUrls);
	g_slist_foreach (mHeaders, (GFunc)free_frame, NULL);

	/* Free all the frames */
	g_slist_free (mHeaders);
	mHeaders = NULL;
	g_hash_table_destroy (mUrls);
}
