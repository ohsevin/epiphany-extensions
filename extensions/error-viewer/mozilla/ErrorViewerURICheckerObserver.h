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
 *
 *  $Id$
 */

#include "link-checker.h"

#include <nsIRequestObserver.h>

/* Header file */
class ErrorViewerURICheckerObserver : public nsIRequestObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVER

  ErrorViewerURICheckerObserver();
  virtual ~ErrorViewerURICheckerObserver();
  /* additional members */

  nsresult Init (LinkChecker *aChecker, const char *aFilename);

  char *mFilename;
  PRUint32 mNumLinksChecked;
  PRUint32 mNumLinksInvalid;
  PRUint32 mNumLinksTotal;

private:
  LinkChecker *mChecker;
};

#define G_ERRORVIEWERURICHECKEROBSERVER_CONTRACTID "@gnome.org/projects/epiphany/epiphany-extensions/error-viewer/error-viewer-uri-checker-observer;1"

#define G_ERRORVIEWERURICHECKEROBSERVER_CID 		\
{ /* 562ad140-49f5-4c25-a14b-977f7e4ddd58 */		\
    0x562ad140,						\
    0x49f5,    						\
    0x4c25,						\
    {0xa1, 0x4b, 0x97, 0x7f, 0x7e, 0x4d, 0xdd, 0x58}	\
}
#define G_ERRORVIEWERURICHECKEROBSERVER_CLASSNAME "Error Viewer's URI Checker Observer"                                                                                
class nsIFactory;
                                                                                
extern nsresult NS_NewErrorViewerURICheckerObserverFactory(nsIFactory** aFactory);
