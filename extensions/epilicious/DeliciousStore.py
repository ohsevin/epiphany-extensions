# -*- coding: utf-8 -*-

# Copyright (C) 2005 by Magnus Therning

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

from urllib2 import URLError
from sets import Set
import libepilicious
from libepilicious.BaseStore import BaseStore
from libepilicious.backend import create_backend


class DeliciousStore(BaseStore):
    '''The class representing the storage of Delicious bookmarks.'''

    def __init__(self, user, pwd, space_repl, backend):
        '''Constructor.

        @param user: Delicious username
        @param pwd: Delicious password
        @param space_repl: Character that replaces space
        '''
        self.__un = user
        self.__pwd = pwd
        self.__sr = space_repl
        self.__d = create_backend(backend)(user, pwd)
        self.__snap_utd = 0

    def get_snapshot(self):
        '''Calculates a snapshot of the del.icio.us bookmarks.

        @note: L{BaseStore.get_snapshot} documents the format of the return
        value.
        '''
        if self.__snap_utd:
            return self.__snap
        all = self.__d.all_bookmarks()
        res = {}
        for p in all:
            # Delicious prepends "dangerous" links
            if p['href'][:33] == 'http://del.icio.us/doc/dangerous#':
                url = p['href'][33:]
            else:
                url = p['href']
            res[url] = [p['description'], \
                    [t.replace(self.__sr, ' ') for t in p['tag'].split(' ')]]

        self.__snap = res
        self.__snap_utd = 1
        return res

    def url_delete(self, url):
        '''Deletes a URL from the storage.

        @param url: The URL to delete
        '''
        # Delicious prepends "dangerous" links
        if url[:7] == 'file://':
            url = 'http://del.icio.us/doc/dangerous#' + url
        try:
            self.__d.delete_bookmark(url)
            self.__snap_utd = 0
        except:
            libepilicious.get_logger().exception('Failed to delete URL %s' % url)

    def url_sync(self, url, desc, to_del, to_add):
        '''Synchronises a URL's tags. The URL is added if it doesn't exist.

        @param url: The URL
        @param to_del: Set of tags to delete
        @param to_add: Set of tags to add
        '''
        if to_del or to_add:
            snap = self.get_snapshot()
            if snap.has_key(url):
                tags = (Set(snap[url][1]) | to_add) - to_del
            else:
                tags = to_add
            tag_str = ' '.join([t.replace(' ', self.__sr) for t in tags])
            try:
                self.__d.add_bookmark(url, description=desc, \
                        tags=tag_str)
                self.__snap_utd = 0
            except:
                libepilicious.get_logger().exception('Failed to synchronise URL %s' % url)
