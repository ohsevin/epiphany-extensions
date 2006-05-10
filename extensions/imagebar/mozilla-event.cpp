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

#include "mozilla-config.h"
#include "config.h"

#include <nsStringAPI.h>

#include <nsCOMPtr.h>
#include <nsIDOMCSS2Properties.h>
#include <nsIDOMElement.h>
#include <nsIDOMEvent.h>
#include <nsIDOMEventTarget.h>
#include <nsIDOMHTMLAnchorElement.h>
#include <nsIDOMHTMLImageElement.h>
#include <nsIDOMMouseEvent.h>
#include <nsIDOMNode.h>
#include <nsIDOMNSHTMLElement.h>

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

	nsCOMPtr <nsIDOMMouseEvent> ev = NS_STATIC_CAST (nsIDOMMouseEvent*, dom_event);
	NS_ENSURE_TRUE (ev, NS_ERROR_FAILURE);

	nsCOMPtr <nsIDOMEventTarget> target;
	rv = ev->GetTarget (getter_AddRefs (target));
	if (NS_FAILED (rv) || !target) return NS_ERROR_FAILURE;

	nsCOMPtr <nsIDOMNode> node = do_QueryInterface (target, &rv);
	if (NS_FAILED (rv) || !node) return NS_ERROR_FAILURE;

	ev->GetCtrlKey (isCtrlKey);
	if (*isCtrlKey)
	{
		nsCOMPtr <nsIDOMNode> parentNode;
		node->GetParentNode (getter_AddRefs (parentNode));

		nsCOMPtr <nsIDOMHTMLAnchorElement> parentAnchor (do_QueryInterface (parentNode));
		*isAnchored = parentAnchor != nsnull;
	}

	nsCOMPtr <nsIDOMHTMLImageElement> image (do_QueryInterface (node));
	if (!image)
	{
		*isImage = PR_FALSE;
		*x = -1;
		*y = -1;

		return NS_OK;
	}

	*isImage = PR_TRUE;

	/* image url to download */
	nsString uImg;
	image->GetSrc (uImg);

	/* FIXME: resolving with base url */

	nsCString img;
	NS_UTF16ToCString (uImg, NS_CSTRING_ENCODING_UTF8, img);
	*imgSrc = g_strdup (img.get());

	/* image offset */
	PRInt32 tmpTop, tmpLeft, allTop, allLeft;
	nsCOMPtr <nsIDOMNSHTMLElement> nsElement (do_QueryInterface (node));
	if (!nsElement) return NS_ERROR_FAILURE;

	nsElement->GetOffsetTop (&tmpTop);
	nsElement->GetOffsetLeft (&tmpLeft);

	allTop = tmpTop;
	allLeft = tmpLeft;

	/* scroll offset */
	nsCOMPtr <nsIDOMElement> offsetParent;
	rv = nsElement->GetOffsetParent (getter_AddRefs (offsetParent));
	while (NS_SUCCEEDED (rv) && offsetParent)
	{
		nsElement = do_QueryInterface (offsetParent);
		if (!nsElement) break;

		nsElement->GetOffsetTop  (&tmpTop);
		nsElement->GetOffsetLeft (&tmpLeft);

		allTop  += tmpTop;
		allLeft += tmpLeft;

		nsElement->GetScrollTop (&tmpTop);
		nsElement->GetScrollLeft (&tmpLeft);

		allTop  -= tmpTop;
		allLeft -= tmpLeft;

		rv = nsElement->GetOffsetParent (getter_AddRefs (offsetParent));
	}

	/* plus all the offsetTop from all the offsetParent
	 * (http://slayeroffice.com/tools/modi/v2.0/modi_help.html)
	 * and minus scrollX, scrollY from window
	 */
	*x = allLeft;
	*y = allTop;

	return NS_OK;
}
