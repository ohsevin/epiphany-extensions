/*
 *  Copyright (C) 2005 Kang Jeong-Hee
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

#undef MOZILLA_INTERNAL_API
#include <nsEmbedString.h>
#define MOZILLA_INTERNAL_API 1

#include <nsCOMPtr.h>
#include <nsIDOMNode.h>
#include <nsIDOMEvent.h>
#include <nsIDOMMouseEvent.h>
#include <nsIDOMEventTarget.h>
#include <nsIDOMHTMLAnchorElement.h>
#include <nsIDOMHTMLImageElement.h>
#include <dom/nsIDOMCSS2Properties.h>
#include <dom/nsIDOMNSHTMLElement.h>
#include <nsIDOMElement.h>

#include "mozilla-event.h"

nsresult
evaluate_dom_event (gpointer dom_event,
		    PRBool *isImage,
		    PRBool *isCtrlKey,
		    PRBool *isAnchored,
		    PRInt32 *x, PRInt32 *y,
		    char **imgSrc)
{
	nsresult rv;

	nsCOMPtr <nsIDOMMouseEvent> ev =
				static_cast <nsIDOMMouseEvent*> (dom_event);
	NS_ENSURE_TRUE (ev, NS_ERROR_FAILURE);

	ev->GetCtrlKey (isCtrlKey);

	nsCOMPtr <nsIDOMEventTarget> target;
	rv = ev->GetTarget (getter_AddRefs (target));
	if (NS_FAILED (rv) || !target) return NS_ERROR_FAILURE;

	nsCOMPtr <nsIDOMNode> node = do_QueryInterface (target, &rv);
	if (NS_FAILED (rv) || !node) return NS_ERROR_FAILURE;

	/* check whether parent is anchor */
	nsCOMPtr <nsIDOMNode> parentNode;
	rv = node->GetParentNode (getter_AddRefs (parentNode));
	if (NS_FAILED (rv) || !parentNode) return NS_ERROR_FAILURE;

	if (isCtrlKey)
	{
		nsCOMPtr <nsIDOMHTMLAnchorElement> parentAnchor =
					do_QueryInterface (parentNode, &rv);
		if (!NS_FAILED (rv) && parentAnchor)
		{
			(*isAnchored) = PR_TRUE;
		}
	}

	PRUint16 type;
	rv = node->GetNodeType (&type);
	if (NS_FAILED (rv)) return NS_ERROR_FAILURE;

	nsCOMPtr <nsIDOMHTMLElement> element = do_QueryInterface(node);
	if ((nsIDOMNode::ELEMENT_NODE != type) || !element)
		return NS_ERROR_FAILURE;

	nsEmbedString uTag;
	rv = element->GetLocalName (uTag);
	if (NS_FAILED (rv)) return NS_ERROR_FAILURE;

	nsEmbedCString tag;
	NS_UTF16ToCString (uTag, NS_CSTRING_ENCODING_UTF8, tag);

	if (g_ascii_strcasecmp (tag.get(), "img") == 0)
	{
		nsCOMPtr <nsIDOMHTMLImageElement> image =
					do_QueryInterface (node, &rv);
		if (NS_FAILED (rv) || !image) return NS_ERROR_FAILURE;

		(*isImage) = PR_TRUE;

		/* image url to download */
		nsEmbedString uImg;
		rv = image->GetSrc (uImg);
		if (NS_FAILED (rv)) return NS_ERROR_FAILURE;

		/* TODO: resolving with base url */

		nsEmbedCString img;
		NS_UTF16ToCString (uImg, NS_CSTRING_ENCODING_UTF8, img);
		(*imgSrc) = g_strdup (img.get());

		/* image offset */
		PRInt32 tmpTop, tmpLeft, allTop, allLeft;
		nsCOMPtr <nsIDOMNSHTMLElement> nsElement =
					do_QueryInterface (element, &rv);
		if (NS_FAILED (rv)) return NS_ERROR_FAILURE;
		nsElement->GetOffsetTop (&tmpTop);
		nsElement->GetOffsetLeft (&tmpLeft);

		allTop = tmpTop;
		allLeft = tmpLeft;

		/* scroll offset */
		nsCOMPtr <nsIDOMElement> offsetParent;
		rv = nsElement->GetOffsetParent (getter_AddRefs (offsetParent));
		while (!NS_FAILED (rv) && offsetParent != NULL)
		{
			nsElement = do_QueryInterface (offsetParent, &rv);
			nsElement->GetOffsetTop  (&tmpTop);
			nsElement->GetOffsetLeft (&tmpLeft);

			allTop  += tmpTop;
			allLeft += tmpLeft;

			nsElement->GetScrollTop (&tmpTop);
			nsElement->GetScrollLeft (&tmpLeft);

			allTop  -= tmpTop;
			allLeft -= tmpLeft;

			rv = nsElement->GetOffsetParent
						(getter_AddRefs (offsetParent));
		}

		/* plus all the offsetTop from all the offsetParent
		 * (http://slayeroffice.com/tools/modi/v2.0/modi_help.html)
		 * and minus scrollX, scrollY from window
		 */
		(*x) = allLeft;
		(*y) = allTop;
	} else {
		(*isImage) = PR_FALSE;
		(*x) = -1;
		(*y) = -1;
	}

	return NS_OK;
}
