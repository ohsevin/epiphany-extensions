/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *  Copyright (C) 2004 Tommi Komulainen
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

#include "mozilla-config.h"

#include "config.h"

#include "mozilla-find.h"

#include <epiphany/ephy-embed.h>
#include "ephy-debug.h"

#define MOZILLA_STRICT_API
#include <nsEmbedString.h>
#undef MOZILLA_STRICT_API

#include <nsCOMPtr.h>
#include <nsIServiceManager.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIDOMWindow.h>
#include <nsIDOMDocument.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMNode.h>
#include <nsIDOMNodeList.h>
#include <nsIDOMElement.h>
#include <nsIDOMHTMLElement.h>
#include <nsIDOMDocumentFragment.h>
#include <nsIDOMDocumentRange.h>
#include <nsIDOMRange.h>
#include <nsIDOMText.h>
#include <nsIFind.h>
#include <nsIWebBrowser.h>
#include <nsIWebBrowserFocus.h>
#include <nsIWebBrowserFind.h>
#include <nsIDOMDocument.h>
#include <nsMemory.h>
#include <nsIDocShell.h>
#include <nsISimpleEnumerator.h>
#include <nsIContentViewer.h>
#include <nsIDocShellTreeItem.h>
#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#include <glib.h>

static const PRUnichar kEmptyLiteral[] = { '\0' };
static const PRUnichar kSpanLiteral[] = { 's', 'p', 'a', 'n', '\0' };
static const PRUnichar kIdLiteral[] = { 'i', 'd', '\0' };
static const PRUnichar kStyleLiteral[] = { 's', 't', 'y', 'l', 'e', '\0' };
static const PRUnichar kBgYellowLiteral[] = { 'b', 'a', 'c', 'k', 'g', 'r', 'o', 'u', 'n', 'd', '-', 'c', 'o', 'l', 'o', 'r', ':', ' ', 'y', 'e', 'l', 'l', 'o', 'w', ';', '\0' };
static const PRUnichar kUniqueIdLiteral[] = { 'V', 'j', 'k', '$', '2', 'v', 'k', 'N', '\0' };
#define HIGHLIGHT_ID "Vjk$2vkN"

typedef void (* DOMSearchForeachFunc) (nsIDOMNode *, nsIDOMRange *, nsIDOMNode **);

class FindHelper
{
public:
	FindHelper (EphyEmbed *aEmbed);
	~FindHelper ();

	nsresult Init ();
	void SetProperties	(const nsAString &aPattern,
				 PRBool aCaseSensitive,
				 PRBool aWrapAround,
				 PRUint32 *aCount);
	void SetShowHighlight	(PRBool aShowHighligh,
				 PRUint32 *aCount);
	nsresult Find		(PRBool aBackwards,
				 PRBool *aDidFind);

private:
	nsresult GetDOMDocument (nsIDOMDocument **aDocument);
	nsresult GetRootElement (nsIDOMDocument *aDocument,
				 nsIDOMElement **aElement);
	nsresult CreateHighlightElement (nsIDOMDocument *aDoc,
					 nsIDOMElement **_retval);
	void DOMSearchForeach (nsIDOMDocument *aDocument,
			       DOMSearchForeachFunc aCallback,
			       PRUint32 *aCount);
	void HighlightDocument (nsIDOMDocument *aDocument,
				PRUint32 *aCount);
	void UnhighlightDocument (nsIDOMDocument *aDocument);
	void Highlight (PRUint32 *aCount);
	void Unhighlight ();

	static void HighlightRange (nsIDOMNode *aSpanTemplate,
				    nsIDOMRange *aRange,
				    nsIDOMNode **_retval);
	static void UnhighlightRange (nsIDOMNode *aSpanTemplate,
				      nsIDOMRange *aRange,
				      nsIDOMNode **_retval);

	nsCOMPtr<nsIWebBrowser> mWebBrowser;
	nsCOMPtr<nsIDOMWindow> mDOMWindow;
	EphyEmbed *mEmbed;
	PRBool mShowHighlight;
	PRBool mCaseSensitive;
	nsEmbedString mPattern;
	nsEmbedString mUniqueId;
};

FindHelper::FindHelper (EphyEmbed *aEmbed)
: mEmbed(aEmbed)
, mShowHighlight(PR_FALSE)
, mUniqueId(kUniqueIdLiteral)
{
	LOG ("FindHelper ctor [%p]", this);
}

FindHelper::~FindHelper ()
{
	LOG ("FindHelper dtor [%p]", this);
}

nsresult
FindHelper::Init ()
{
	nsresult rv = NS_ERROR_FAILURE;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (mEmbed),
					 getter_AddRefs (mWebBrowser));
	NS_ENSURE_TRUE (mWebBrowser, rv);

	rv = mWebBrowser->GetContentDOMWindow (getter_AddRefs (mDOMWindow));
	NS_ENSURE_SUCCESS (rv, rv);

	return rv;
}

nsresult
FindHelper::GetDOMDocument (nsIDOMDocument **aDOMDocument)
{
	nsCOMPtr<nsIWebBrowserFocus> focus (do_GetInterface(mWebBrowser));
	NS_ENSURE_TRUE (focus, NS_ERROR_FAILURE);

	nsresult rv;
	nsCOMPtr<nsIDOMWindow> domWin;
	rv = focus->GetFocusedWindow (getter_AddRefs (domWin));
	if (NS_FAILED (rv) || !domWin)
	{
		domWin = mDOMWindow;
	}
	NS_ENSURE_TRUE (domWin, NS_ERROR_FAILURE);

	return domWin->GetDocument (aDOMDocument);
}

nsresult
FindHelper::GetRootElement (nsIDOMDocument *aDocument,
			    nsIDOMElement **aElement)
{
	nsCOMPtr<nsIDOMHTMLDocument> htmlDoc (do_QueryInterface (aDocument));
	if (htmlDoc)
	{
		nsCOMPtr<nsIDOMHTMLElement> htmlBody;
		htmlDoc->GetBody (getter_AddRefs (htmlBody));
		return CallQueryInterface (htmlBody, aElement);
//		htmlBody->QueryInterface (NS_GET_IID(nsIDOMElement), (void **)aElement);
//		return;
	}

	return aDocument->GetDocumentElement (aElement);
}

//FIXME configure check
//#define HAVE_USABLE_NSIDOMRANGE_SURROUNDCONTENTS

/* static */ void
FindHelper::HighlightRange (nsIDOMNode *aSpanTemplate,
			    nsIDOMRange *aRange,
			    nsIDOMNode **_retval)
{
	LOG ("HighlightRange");

	nsCOMPtr<nsIDOMNode> dummy;
	aSpanTemplate->CloneNode (PR_TRUE, getter_AddRefs (dummy));
	nsCOMPtr<nsIDOMElement> span (do_QueryInterface(dummy));

#ifdef HAVE_USABLE_NSIDOMRANGE_SURROUNDCONTENTS
	// https://bugzilla.mozilla.org/show_bug.cgi?id=58974
	aRange->SurroundContents (span);

	span->GetLastChild (_retval);
#else
	nsCOMPtr<nsIDOMNode> startContainer;
	PRInt32 startOffset;

	aRange->GetStartContainer(getter_AddRefs(startContainer));
	aRange->GetStartOffset(&startOffset);

	nsCOMPtr<nsIDOMText> startText( do_QueryInterface(startContainer) );

	nsCOMPtr<nsIDOMDocumentFragment> contents;
	aRange->ExtractContents(getter_AddRefs(contents));

	nsCOMPtr<nsIDOMText> before;
	startText->SplitText(startOffset, getter_AddRefs(before));
	span->AppendChild(contents, getter_AddRefs(dummy));

	nsCOMPtr<nsIDOMNode> parent;
	before->GetParentNode(getter_AddRefs(parent));
	parent->InsertBefore(span, before, _retval);
#endif
}

/* static */ void
FindHelper::UnhighlightRange (nsIDOMNode *aSpanTemplate,
			      nsIDOMRange *aRange,
			      nsIDOMNode **_retval)
{
	LOG ("UnhighlightRange");

	nsresult rv;
	nsCOMPtr<nsIDOMNode> container;
	rv = aRange->GetStartContainer(getter_AddRefs(container));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && container, );

	nsCOMPtr<nsIDOMNode> span;
	container->GetParentNode (getter_AddRefs (span));

	nsCOMPtr<nsIDOMElement> spanAsElement (do_QueryInterface (span));
	NS_ENSURE_TRUE (spanAsElement, );

	nsEmbedString id;
	rv = spanAsElement->GetAttribute (nsEmbedString (kIdLiteral), id);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && id.Length (), );

	/* FIXME SUCK SUCK SUCK ! */
	nsEmbedCString cId;
	NS_UTF16ToCString (id, NS_CSTRING_ENCODING_UTF8, cId);
//	if (!id.Equals(mUniqueId)) return;
	if (strcmp (cId.get (), HIGHLIGHT_ID) != 0)
	  {
	    NS_ADDREF (*_retval = span);
	    return;
	  }

	/// UNTIL HERE 
	nsCOMPtr<nsIDOMNode> parent;
	span->GetParentNode(getter_AddRefs(parent));

	nsCOMPtr<nsIDOMNode> child;
	nsCOMPtr<nsIDOMNode> lastChild (span);
	while (NS_SUCCEEDED (span->GetFirstChild (getter_AddRefs (child))) && child)
	{
		span->RemoveChild(child, getter_AddRefs (container));
		parent->InsertBefore(child, span, getter_AddRefs (container));

		lastChild = child;
		child = nsnull;
	}

	parent->RemoveChild (span, getter_AddRefs (container));
	NS_ADDREF (*_retval = lastChild);
}

nsresult
FindHelper::CreateHighlightElement (nsIDOMDocument *aDoc,
				    nsIDOMElement **_retval)
{
	nsresult rv;
	nsCOMPtr<nsIDOMElement> span;
	rv = aDoc->CreateElement (nsEmbedString (kSpanLiteral), getter_AddRefs (span));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && span, rv);

	span->SetAttribute(nsEmbedString (kIdLiteral), mUniqueId);
	span->SetAttribute(nsEmbedString (kStyleLiteral), nsEmbedString (kBgYellowLiteral));

	NS_ADDREF(*_retval = span);

	return NS_OK;
}

void
FindHelper::DOMSearchForeach (nsIDOMDocument *aDocument,
			      DOMSearchForeachFunc aCallback,
			      PRUint32 *aCount)
{
	NS_ENSURE_TRUE (aDocument, );

	nsresult rv;
	nsCOMPtr<nsIDOMElement> body;
	rv = GetRootElement (aDocument, getter_AddRefs (body));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && body, );

	nsCOMPtr<nsIDOMNodeList> childNodes;
	rv = body->GetChildNodes (getter_AddRefs (childNodes));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && childNodes, );

	PRUint32 childCount = 0;
	rv = childNodes->GetLength (&childCount);
	NS_ENSURE_SUCCESS (rv, );

	nsCOMPtr<nsIDOMDocumentRange> domRange (do_QueryInterface (aDocument));
	NS_ENSURE_TRUE (domRange, );

	nsCOMPtr<nsIDOMRange> searchRange;
	rv = domRange->CreateRange (getter_AddRefs (searchRange));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && searchRange, );

	searchRange->SetStart(body, 0);
	searchRange->SetEnd(body, childCount);

	nsCOMPtr<nsIDOMRange> startPt;
	rv = domRange->CreateRange (getter_AddRefs (startPt));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && startPt, );
	
	startPt->SetStart (body, 0);
	startPt->SetEnd (body, 0);

	nsCOMPtr<nsIDOMRange> endPt;
	domRange->CreateRange (getter_AddRefs (endPt));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && endPt, );

	endPt->SetStart (body, childCount);
	endPt->SetEnd (body, childCount);

	nsCOMPtr<nsIDOMElement> spanTemplate;
	rv = CreateHighlightElement (aDocument, getter_AddRefs (spanTemplate));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && spanTemplate, );

	nsCOMPtr<nsIFind> find (do_CreateInstance("@mozilla.org/embedcomp/rangefind;1", &rv));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && find, );

	find->SetCaseSensitive (mCaseSensitive);

	nsCOMPtr<nsIDOMRange> range;
	while (NS_SUCCEEDED (find->Find (mPattern.get (),
					 searchRange, startPt, endPt,
					 getter_AddRefs (range))) &&
	       range)
	{			     
		++(*aCount);

		nsCOMPtr<nsIDOMNode> node;
		aCallback (spanTemplate, range, getter_AddRefs (node));

		startPt->SetStartAfter (node);
		startPt->SetEndAfter (node);

		range = nsnull;
	}
}

void
FindHelper::HighlightDocument (nsIDOMDocument *aDocument,
			       PRUint32 *aCount)
{
	LOG ("HighlightDocument");

	DOMSearchForeach (aDocument, (DOMSearchForeachFunc) HighlightRange, aCount);
}

void
FindHelper::UnhighlightDocument (nsIDOMDocument *aDocument)
{
	if (!mPattern.Length()) return;

	PRUint32 count;
	DOMSearchForeach (aDocument, (DOMSearchForeachFunc) UnhighlightRange, &count);
}

void
FindHelper::Highlight (PRUint32 *aCount)
{
        NS_ENSURE_TRUE (mWebBrowser, );

	LOG ("Highlight");

        nsCOMPtr<nsIDocShell> rootDocShell (do_GetInterface (mWebBrowser));
        NS_ENSURE_TRUE (rootDocShell, );

        nsCOMPtr<nsISimpleEnumerator> enumerator;
        rootDocShell->GetDocShellEnumerator(nsIDocShellTreeItem::typeContent,
                                            nsIDocShell::ENUMERATE_FORWARDS,
                                            getter_AddRefs(enumerator));
        NS_ENSURE_TRUE (enumerator, );

        PRBool hasMore;
        while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMore)) && hasMore)
        {
                nsCOMPtr<nsISupports> element;
                enumerator->GetNext (getter_AddRefs(element));
                if (!element) continue;

                nsCOMPtr<nsIDocShell> docShell = do_QueryInterface (element);
                if (!docShell) continue;

                nsCOMPtr<nsIContentViewer> contentViewer;
                docShell->GetContentViewer (getter_AddRefs(contentViewer));
                if (!contentViewer) continue;

                nsCOMPtr<nsIDOMDocument> domDoc;
                contentViewer->GetDOMDocument (getter_AddRefs (domDoc));

		HighlightDocument (domDoc, aCount);
	}
}

void
FindHelper::Unhighlight ()
{
        NS_ENSURE_TRUE (mWebBrowser, );

        nsCOMPtr<nsIDocShell> rootDocShell (do_GetInterface (mWebBrowser));
        NS_ENSURE_TRUE (rootDocShell, );

        nsCOMPtr<nsISimpleEnumerator> enumerator;
        rootDocShell->GetDocShellEnumerator(nsIDocShellTreeItem::typeContent,
                                            nsIDocShell::ENUMERATE_FORWARDS,
                                            getter_AddRefs(enumerator));
        NS_ENSURE_TRUE (enumerator, );

        PRBool hasMore;
        while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMore)) && hasMore)
        {
                nsCOMPtr<nsISupports> element;
                enumerator->GetNext (getter_AddRefs(element));
                if (!element) continue;

                nsCOMPtr<nsIDocShell> docShell = do_QueryInterface (element);
                if (!docShell) continue;

                nsCOMPtr<nsIContentViewer> contentViewer;
                docShell->GetContentViewer (getter_AddRefs(contentViewer));
                if (!contentViewer) continue;

                nsCOMPtr<nsIDOMDocument> domDoc;
                contentViewer->GetDOMDocument (getter_AddRefs (domDoc));

		UnhighlightDocument (domDoc);
	}
}

void
FindHelper::SetShowHighlight (PRBool aShowHighlight,
			      PRUint32 *aCount)
{
	NS_ENSURE_TRUE (mWebBrowser, );

	if (mShowHighlight == aShowHighlight) return;

	mShowHighlight = aShowHighlight;

	if (mShowHighlight)
	{
		nsCOMPtr<nsIWebBrowserFind> find (do_GetInterface(mWebBrowser));
		NS_ENSURE_TRUE (find, );

		nsresult rv;
		PRUnichar *pattern = nsnull;
		rv = find->GetSearchString( &pattern);
		if (NS_SUCCEEDED (rv) && pattern)
		{
			nsEmbedString uPattern (pattern); /* FIXME use dependent string */
			nsMemory::Free (pattern);

			if (uPattern.Length () >= 3)
			{
				mPattern = uPattern;
				Highlight (aCount);
			}

		}
	}
	else
	{
		Unhighlight ();
		mPattern.Assign (kEmptyLiteral);
	}
}

void
FindHelper::SetProperties (const nsAString &aPattern,
			   PRBool aCaseSensitive,
			   PRBool aWrapAround,
			   PRUint32 *aCount)
{
	NS_ENSURE_TRUE (mWebBrowser, );

	nsresult rv;
	nsCOMPtr<nsIDOMDocument> domDoc;
	rv = GetDOMDocument (getter_AddRefs (domDoc));
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && domDoc, );

	Unhighlight ();
	mPattern.Assign (kEmptyLiteral);

	mCaseSensitive = aCaseSensitive;

	nsCOMPtr<nsIWebBrowserFind> find (do_GetInterface(mWebBrowser));
	NS_ENSURE_TRUE (find, );

	nsEmbedString uPattern (aPattern);
	find->SetSearchString (uPattern.get ());
	find->SetMatchCase (aCaseSensitive);
	find->SetWrapFind (aWrapAround);

	if (mShowHighlight && uPattern.Length() >= 3)
	{
		mPattern = uPattern;
		Highlight (aCount);
	}
}

nsresult
FindHelper::Find (PRBool aBackwards,
		  PRBool *aDidFind)
{
	NS_ENSURE_TRUE (mWebBrowser ,NS_ERROR_FAILURE);

	nsCOMPtr<nsIWebBrowserFind> find (do_GetInterface(mWebBrowser));
	NS_ENSURE_TRUE (find, NS_ERROR_FAILURE);

	find->SetFindBackwards (aBackwards);

	return find->FindNext (aDidFind);
}

/* ------------------------------------------------------------------------- */

#define FIND_HELPER_KEY	"EphyFindExtension::FindHelper"

static void
delete_helper (FindHelper *helper)
{
	g_return_if_fail (helper != NULL);

	delete helper;
}

static FindHelper *
ensure_find_helper (EphyEmbed *embed)
{
	FindHelper *helper;

	helper = (FindHelper *) g_object_get_data (G_OBJECT (embed), FIND_HELPER_KEY);
	if (!helper)
	{
		helper = new FindHelper (embed);

		nsresult rv;
		rv = helper->Init ();
		if (NS_FAILED (rv))
		{
			g_warning ("Failed to initialise find helper for embed %p\n", embed);
		}

		g_object_set_data_full (G_OBJECT (embed), FIND_HELPER_KEY,
					helper, (GDestroyNotify) delete_helper);
	}

	return helper;
}

extern "C" void
mozilla_find_detach (EphyEmbed *embed)
{
	g_object_set_data (G_OBJECT (embed), FIND_HELPER_KEY, NULL);
}

extern "C" gboolean
mozilla_find_next (EphyEmbed *embed,
		   gboolean backwards)
{
	FindHelper *helper;

	LOG ("mozilla_find_next");

	helper = ensure_find_helper (embed);
	g_return_val_if_fail (helper != NULL, FALSE);

	nsresult rv;
	PRBool didFind = PR_FALSE;
	rv = helper->Find (backwards, &didFind);

	return NS_SUCCEEDED (rv) && didFind != PR_FALSE;
}

extern "C" guint32
mozilla_find_set_properties (EphyEmbed *embed,
			      const char *search_string,
			      gboolean case_sensitive,
			      gboolean wrap_around)
{
	FindHelper *helper;

	g_return_val_if_fail (search_string != NULL, 0);

	LOG ("mozilla_find_set_properties search_string:%s", search_string);

	helper = ensure_find_helper (embed);
	g_return_val_if_fail (helper != NULL, 0);

	PRUint32 count = 0;
	nsEmbedString uPattern;
	NS_CStringToUTF16 (nsEmbedCString (search_string), NS_CSTRING_ENCODING_UTF8, uPattern);
	helper->SetProperties (uPattern, case_sensitive, wrap_around, &count);

	return (guint32) count;
}

extern "C" guint32
mozilla_find_set_highlight (EphyEmbed *embed,
			    gboolean show_highlight)
{
	FindHelper *helper;

	LOG ("mozilla_find_set_highlight");

	helper = ensure_find_helper (embed);
	g_return_val_if_fail (helper != NULL, 0);

	PRUint32 count = 0;
	helper->SetShowHighlight (show_highlight, &count);

	return (guint32) count;
}

void
mozilla_embed_scroll_pages (EphyEmbed *embed,
			    gint32 pages)
{
	g_return_if_fail (EPHY_IS_EMBED (embed));

	nsCOMPtr<nsIWebBrowser> webBrowser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (webBrowser));
	nsCOMPtr<nsIWebBrowserFocus> focus (do_GetInterface(webBrowser));
	NS_ENSURE_TRUE (webBrowser && focus, );

	nsresult rv;
	nsCOMPtr<nsIDOMWindow> domWin;
	rv = focus->GetFocusedWindow (getter_AddRefs (domWin));
	if (NS_FAILED (rv) || !domWin)
	{
		rv = webBrowser->GetContentDOMWindow (getter_AddRefs (domWin));
	}
	NS_ENSURE_TRUE (domWin, );

	domWin->ScrollByPages ((PRInt32) pages);
}

void
mozilla_embed_scroll_lines (EphyEmbed *embed,
			    gint32 lines)
{
	g_return_if_fail (EPHY_IS_EMBED (embed));

	nsCOMPtr<nsIWebBrowser> webBrowser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (webBrowser));
	nsCOMPtr<nsIWebBrowserFocus> focus (do_GetInterface(webBrowser));
	NS_ENSURE_TRUE (webBrowser && focus, );

	nsresult rv;
	nsCOMPtr<nsIDOMWindow> domWin;
	rv = focus->GetFocusedWindow (getter_AddRefs (domWin));
	if (NS_FAILED (rv) || !domWin)
	{
		rv = webBrowser->GetContentDOMWindow (getter_AddRefs (domWin));
	}
	NS_ENSURE_TRUE (domWin, );

	domWin->ScrollByLines ((PRInt32) lines);
}
