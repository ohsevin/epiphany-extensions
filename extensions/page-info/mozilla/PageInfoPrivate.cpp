/*
 *  Copyright © 2003, 2004 Marco Pesenti Gritti
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

#include <nscore.h>

#define MOZILLA_INTERNAL_API 1
#include <nsString.h>

#include <nsCOMPtr.h>
#include <nsIDOMDocument.h>
#ifdef HAVE_GECKO_1_9
#include <nsIDocument.h>
#else
#include <nsIHTMLDocument.h>
#endif

#include "PageInfoPrivate.h"

EmbedPageRenderMode
PageInfoPrivate::GetRenderMode (nsIDOMDocument *aDocument)
{
#ifdef HAVE_GECKO_1_9
  nsCOMPtr<nsIDocument> doc (do_QueryInterface (aDocument));
#else
  nsCOMPtr<nsIHTMLDocument> doc (do_QueryInterface (aDocument));
#endif

  EmbedPageRenderMode mode = EMBED_RENDER_UNKNOWN;
  if (doc)
    {
      mode = (EmbedPageRenderMode) doc->GetCompatibilityMode ();
    }

  return mode;
}
