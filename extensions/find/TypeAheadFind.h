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

#ifndef TYPEAHEADFIND_H
#define TYPEAHEADFIND_H

#include <epiphany/ephy-embed.h>

#include <nsCOMPtr.h>
#include <nsITypeAheadFind.h>

typedef enum
  {
    FIND_FOUND    = 0U,
    FIND_NOTFOUND = 1U,
    FIND_WRAPPED  = 2U,
    FIND_ERROR    = 65535U
} FindResult;

class TypeAheadFind
{
  public:
    TypeAheadFind ();
    ~TypeAheadFind ();

    nsresult Init ();
    nsresult SetEmbed (EphyEmbed *aEmbed);
    void SetFocusLinks (PRBool aFocusLinks);
    void SetCaseSensitive (PRBool aCaseSensitive);

    FindResult Find (const char *aSearchString,
                     PRBool aLinksOnly);
    FindResult FindNext ();
    FindResult FindPrevious ();

  private:
    nsCOMPtr<nsITypeAheadFind> mFind;
    EphyEmbed *mCurrentEmbed;
    PRBool mLinksOnly;
    PRBool mInitialised;
};

#endif /* !TYPEAHEADFIND_H */
