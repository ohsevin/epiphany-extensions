/*
 *  Copyright (C) 2003, 2004 Marco Pesenti Gritti
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
#include <nsIHTMLDocument.h>

#include "PageInfoPrivate.h"

EmbedPageRenderMode
PageInfoPrivate::GetRenderMode (nsIDOMDocument *aDocument)
{
  nsCOMPtr<nsIHTMLDocument> htmlDoc (do_QueryInterface (aDocument));

  EmbedPageRenderMode mode = EMBED_RENDER_UNKNOWN;
  if (htmlDoc)
    {
      mode = (EmbedPageRenderMode) htmlDoc->GetCompatibilityMode ();
    }

  return mode;
}
