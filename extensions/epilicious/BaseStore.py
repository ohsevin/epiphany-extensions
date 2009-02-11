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

class BaseStore(object):
    '''The base of all stores. Never to be instantiated.'''

    def __init__(self):
        self.store = 1
        self.fstore = []

    def perform_calls(self):
        for f, s, args, kwargs in self.fstore:
            yield f(s, *args, **kwargs)
        self.fstore = []

    @classmethod
    def store_call(cls, func):
        def inner(self, *args, **kwargs):
            if self.store:
                self.fstore.append((func, self, args, kwargs))
            else:
                func(self, *args, **kwargs)

        return inner

    def get_snapshot(self):
        '''Calculates a snapshot of the store's bookmarks.

        The format of the snapshot is::
         { <url> : [ <description>, [tag, tag, ...]],
           <url> : [ <description>, [<tags>*]],
           ... }

        @return: the snapshot
        '''
        pass


    def url_delete(self, url):
        '''Deletes a URL form the store.

        @type url: string
        @param url: URL to delete
        '''
        pass


    def url_sync(self, url, desc, to_del, to_add):
        '''Synchronises a URL's tags. The URL is added if it doesn't exist.

        @param url: The URL
        @param to_del: Set of tags to delete
        @param to_add: Set of tags to add
        '''
        pass
