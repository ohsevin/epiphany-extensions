/*
 *  Copyright (C) 2004 Crispin Flowerday
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

#ifndef MOZ_EPHY_SIDEBAR_PROXY_H
#define MOZ_EPHY_SIDEBAR_PROXY_H

#define EPHY_SIDEBAR_CLASSNAME "Epiphany Sidebar Extension Implementation"

/* 0a9fa68e-49dc-4fb1-ac61-cdd8568a741b */
#define EPHY_SIDEBAR_CID \
{ 0x0a9fa68e, 0x49dc, 0x4fb1, { 0xac, 0x61, 0xcd, 0xd8, 0x56, 0x8a, 0x74, 0x1b } }

#include <nsISidebar.h>
#include <glib-object.h>

class SidebarProxy : public nsISidebar,
		     public nsIClassInfo
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSISIDEBAR
	NS_DECL_NSICLASSINFO
     
	SidebarProxy();

	static void SetSignalObject (GObject *aObject);
 private:

	static GObject *mSignalObject;
};

#endif /* MOZ_EPHY_SIDEBAR_PROXY_H */
