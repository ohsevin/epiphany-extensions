# -*- coding: utf-8 -*-

# Copyright (C) 2009 by Magnus Therning

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from sets import Set

import libepilicious
from libepilicious.BaseStore import BaseStore
from libepilicious.diigo import Diigo2

class DiigoStore(BaseStore):
    '''A class representing the storage of bookmarks on a Diigo backend.
    '''

    def __init__(self, user, pwd, space_repl):
        BaseStore.__init__(self)
        self.__un = user
        self.__pwd = pwd
        self.__sr = space_repl
        self.__bkend = Diigo2(user, pwd)
        self.__snap_utd = False
        self.__snap = None

    def get_snapshot(self):
        '''Retrieves a snapshot of bookmarks.

        @note: L{BaseStore.get_snapshot} documents the format of the return
        value.
        '''
        def fixup_bm(b):
            url = b['url']
            title = b['title']
            tags = [t.replace(self.__sr, ' ') for t in b['tags'].split(',')]
            return (url, [title, tags])

        if self.__snap_utd:
            return self.__snap
        bms = self.__bkend.all_bookmarks()
        self.__snap = dict([fixup_bm(b) for b in bms])
        self.__snap_utd = True
        return self.__snap

    @BaseStore.store_call
    def url_delete(self, url):
        '''Deletes a URL from storage.

        @param url: The URL to delete
        '''
        try:
            self.__bkend.delete_bookmark(url)
            self.__snap_utd = False
        except:
            print 'DiigoStore: Failed to delete URL %s' % url
            libepilicious.get_logger().exception('DiigoStore: Failed to delete URL %s' % url)

    @BaseStore.store_call
    def url_sync(self, url, desc, to_del, to_add):
        if to_del or to_add:
            snap = self.get_snapshot()
            if snap.has_key(url):
                tags = (Set(snap[url][1]) | to_add) - to_del
            else:
                tags = to_add
            tag_str = ' '.join([t.replace(' ', self.__sr) for t in tags])
            try:
                self.__bkend.add_bookmark(url=url, title=desc, tags=tag_str)
                self.__snap_utd = False
            except:
                print 'DiigoStore: Failed to synchronise URL %s' % url
                libepilicious.get_logger().exception('DiigoStore: Failed to synchronise URL %s' % url)
