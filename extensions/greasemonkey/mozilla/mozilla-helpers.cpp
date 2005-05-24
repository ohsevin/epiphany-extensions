/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#include "mozilla-helpers.h"

#include <glib.h>

#include <nsEmbedString.h>
#include <nsCOMPtr.h>
#include <nsIDOMEvent.h>
#include <nsIDOMEventTarget.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMHTMLElement.h>
#include <nsIDOMHTMLScriptElement.h>
#include <nsMemory.h>

gboolean
mozilla_evaluate_js (gpointer event,
		     const char *script)
{
	nsresult rv;

	nsCOMPtr<nsIDOMEvent> ev;
	ev = static_cast<nsIDOMEvent*>(event);
	NS_ENSURE_TRUE (ev, FALSE);

	nsCOMPtr<nsIDOMEventTarget> evTarget;
	ev->GetTarget(getter_AddRefs (evTarget));
	NS_ENSURE_TRUE (evTarget, FALSE);

	nsCOMPtr<nsIDOMDocument> doc (do_QueryInterface (evTarget));
	NS_ENSURE_TRUE (doc, FALSE);

	nsCOMPtr<nsIDOMElement> body;
	nsCOMPtr<nsIDOMHTMLDocument> htmlDoc (do_QueryInterface (doc));
	if (htmlDoc)
	{
		nsCOMPtr<nsIDOMHTMLElement> htmlBody;
		htmlDoc->GetBody (getter_AddRefs (htmlBody));
		body = do_QueryInterface (htmlBody);
		NS_ENSURE_TRUE (body, FALSE);
	}
	else
	{
		rv = doc->GetDocumentElement (getter_AddRefs (body));
		NS_ENSURE_SUCCESS (rv, FALSE);
	}

	// Create the script tag to be inserted
	static const PRUnichar kScriptLiteral[] = { 's', 'c', 'r', 'i', 'p',
						    't', '\0' };
	static const PRUnichar kJSTypeLiteral[] = { 't', 'e', 'x', 't', '/',
						    'j', 'a', 'v', 'a', 's',
						    'c', 'r', 'i', 'p', 't',
						    '\0' };

	nsCOMPtr<nsIDOMElement> elem;
	rv = doc->CreateElement (nsEmbedString (kScriptLiteral),
				 getter_AddRefs (elem));
	NS_ENSURE_SUCCESS (rv, FALSE);

	nsCOMPtr<nsIDOMHTMLScriptElement> scriptTag = do_QueryInterface (elem);
	NS_ENSURE_TRUE (scriptTag, FALSE);

	nsEmbedString aScript;
	NS_CStringToUTF16 (nsEmbedCString (script), NS_CSTRING_ENCODING_UTF8,
			   aScript);
	scriptTag->SetText (aScript);
	scriptTag->SetType (nsEmbedString (kJSTypeLiteral));

	// Insert our script within the DOM
	nsCOMPtr<nsIDOMNode> dummy;
	rv = body->AppendChild (scriptTag, getter_AddRefs (dummy));
	NS_ENSURE_SUCCESS (rv, FALSE);

	// And remove it, since we don't need it any more
	rv = body->RemoveChild (scriptTag, getter_AddRefs (dummy));
	NS_ENSURE_SUCCESS (rv, FALSE);

	return TRUE;
}
