/*
 *  Copyright (C) 2004 Adam Hooper
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
 */

#ifndef MOZ_AD_BLOCKER_H
#define MOZ_AD_BLOCKER_H

#include <nsIContentPolicy.h>

/*
 * This class is intended to include the bare minimum of code necessary to call
 * ad_blocker_test() on requested URIs.
 */
class MozAdBlocker : public nsIContentPolicy
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSICONTENTPOLICY
	/* additional members */

private:
	nsresult ShouldLoadURI (nsIURI *uri, nsISupports *aContext, PRUint32 aContenType, PRBool *_retval);
};

#define G_MOZADBLOCKER_CONTRACTID "@gnome.org/projects/epiphany/epiphany-extensions/adblock/moz-add-blocker;1"

#define G_MOZADBLOCKER_CID					\
{	/* 86f60bca-4211-4eeb-aaf1-5a74e426bc26 */		\
	0x86f60bca,						\
	0x4211,							\
	0x4eeb,							\
	{0xaa, 0xfa, 0x5a, 0x74, 0xe4, 0x26, 0xbc, 0x26}	\
}

#define G_MOZADBLOCKER_CLASSNAME "Ad Blocker"

class nsIFactory;

extern nsresult NS_NewMozAdBlockerFactory(nsIFactory** aFactory);

#endif /* MOZ_AD_BLOCKER_H */
