/*
 *  Copyright (C) 2005 Christian Persch
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

#include "TypeAheadFind.h"

#include "ephy-debug.h"

#define MOZILLA_STRICT_API
#include <nsEmbedString.h>
#undef MOZILLA_STRICT_API

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>
#include <nsCOMPtr.h>
#include <nsIServiceManager.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowser.h>
#include <nsIDocShell.h>

#include <glib.h>

#define NS_TYPEAHEADFIND_CONTRACTID "@mozilla.org/typeaheadfind;1"

TypeAheadFind::TypeAheadFind ()
: mCurrentEmbed(nsnull)
, mLinksOnly(PR_FALSE)
, mInitialised(PR_FALSE)
{
  LOG ("TypeAheadFind ctor [%p]", this);
}

TypeAheadFind::~TypeAheadFind ()
{
  LOG ("TypeAheadFind dtor [%p]", this);
}

nsresult
TypeAheadFind::Init ()
{
  nsresult rv;
  mFind = do_CreateInstance (NS_TYPEAHEADFIND_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS (rv, rv);

  return rv;
}

nsresult
TypeAheadFind::SetEmbed (EphyEmbed *aEmbed)
{
  nsresult rv = NS_ERROR_FAILURE;
  NS_ENSURE_TRUE (mFind, rv);

  rv = NS_OK;
  if (aEmbed == mCurrentEmbed) return rv;

  mCurrentEmbed = nsnull;

  rv = NS_ERROR_FAILURE;
  nsCOMPtr<nsIWebBrowser> webBrowser;
  gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (aEmbed),
				   getter_AddRefs (webBrowser));
  NS_ENSURE_TRUE (webBrowser, rv);

  nsCOMPtr<nsIDocShell> docShell (do_GetInterface (webBrowser, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  if (!mInitialised)
    {
      mInitialised = PR_TRUE;
      rv = mFind->Init (docShell);
    }
  else
    {
      rv = mFind->SetDocShell (docShell);
    }
  NS_ENSURE_SUCCESS (rv, rv);

  mCurrentEmbed = aEmbed;

  SetFocusLinks (PR_TRUE);

  return rv;
}

void
TypeAheadFind::SetFocusLinks (PRBool aFocusLinks)
{
  NS_ENSURE_TRUE (mInitialised, );

  nsresult rv;
  rv = mFind->SetFocusLinks (aFocusLinks);
}

void
TypeAheadFind::SetCaseSensitive (PRBool aCaseSensitive)
{
  NS_ENSURE_TRUE (mInitialised, );

  nsresult rv;
  rv = mFind->SetCaseSensitive (aCaseSensitive);
}

FindResult
TypeAheadFind::Find (const char *aSearchString,
                     PRBool aLinksOnly)
{
  NS_ENSURE_TRUE (mInitialised, FIND_NOTFOUND);

  nsEmbedString uSearchString;
  NS_CStringToUTF16 (nsEmbedCString (aSearchString ? aSearchString : ""),
  		     NS_CSTRING_ENCODING_UTF8, uSearchString);

  nsresult rv;
  PRUint16 found = FIND_ERROR;
  rv = mFind->Find (uSearchString, aLinksOnly, &found);

  return NS_SUCCEEDED (rv) ? (FindResult) found : FIND_ERROR;
}

FindResult
TypeAheadFind::FindNext ()
{
  NS_ENSURE_TRUE (mInitialised, FIND_NOTFOUND);

  nsresult rv;
  PRUint16 found = FIND_ERROR;
  rv = mFind->FindNext (&found);

  return NS_SUCCEEDED (rv) ? (FindResult) found : FIND_ERROR;
}

FindResult
TypeAheadFind::FindPrevious ()
{
  NS_ENSURE_TRUE (mInitialised, FIND_NOTFOUND);

  nsresult rv;
  PRUint16 found = FIND_ERROR;
  rv = mFind->FindPrevious (&found);

  return NS_SUCCEEDED (rv) ? (FindResult) found : FIND_ERROR;
}
