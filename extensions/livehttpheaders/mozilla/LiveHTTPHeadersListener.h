/*
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2003 Marco Pesenti Gritti
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

#ifndef EPHY_LIVEHTTPHEADERS_H
#define EPHY_LIVEHTTPHEADERS_H

#include <nsIObserver.h>
#include <glib.h>

class HeaderVisitor;

class LiveHTTPHeadersListener : public nsIObserver
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSIOBSERVER

	LiveHTTPHeadersListener();

	GSList *GetFrames (void) { return mHeaders; }

	void ClearFrames (void);
	
private:
	~LiveHTTPHeadersListener();

	HeaderVisitor *mVisitor;

	GSList     *mHeaders;
	GHashTable *mUrls;
};

#endif
