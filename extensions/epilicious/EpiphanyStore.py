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

from sets import Set
import epiphany
import libepilicious
from libepilicious.BaseStore import BaseStore


class EpiphanyStore(BaseStore):

    def __init__(self, keyword = None, exclude = False):
        BaseStore.__init__(self)
        # TODO: add a check for the priority of the keyword and set it to 0, at
        # the moment it's not possible since there's no method for it
        # (ephy_node_set_property() isn't in the Python API)
        self.__kw = keyword
        self.__excl = exclude

        self.__bms = epiphany.ephy_shell_get_default().get_bookmarks()
        if not self.__bms.find_keyword(self.__kw, False):
            self.__bms.add_keyword(self.__kw)

    def __get_all_shared_bookmarks(self):
        # Return all shared bookmarks
        # Two possibilities:
        #  1. Take all bookmarks we have and remove the ones in __kw
        #  2. All bookmarks are in __kw
        key = self.__bms.find_keyword(self.__kw, False)
        if self.__excl:
            tmp = self.__bms.get_bookmarks().get_children()
            res = []
            for b in tmp:
                if not self.__bms.has_keyword(key, b):
                    res.append(b)
            return res
        else:
            return key.get_children()

    def __get_all_keywords(self):
        # Return a list containing all interesting keywords. All special
        # keywords (priority==1) and 'All' (id==0) are uninteresting.
        kw_node = self.__bms.find_keyword(self.__kw, False)
        return [kw for kw in self.__bms.get_keywords().get_children() \
                if kw.get_id() != 0 and \
                kw.get_property_int(epiphany.NODE_KEYWORD_PROP_PRIORITY) != 1 and \
                kw != kw_node]

    def get_snapshot(self):
        '''Calculates a snapshot of the Epiphany bookmarks.

        @note: L{BaseStore.get_snapshot} documents the format of the return
        value.
        '''
        all_shared_bms = self.__get_all_shared_bookmarks()
        all_keywords = self.__get_all_keywords()

        res = {}
        for bm in all_shared_bms:
            res[bm.get_property_string(epiphany.NODE_BMK_PROP_LOCATION)] = \
                    [bm.get_property_string(epiphany.NODE_BMK_PROP_TITLE)]
            keywords = []
            for key in all_keywords:
                if self.__bms.has_keyword(key, bm):
                    keywords.append(key.get_property_string(epiphany.NODE_KEYWORD_PROP_NAME))
            res[bm.get_property_string(epiphany.NODE_BMK_PROP_LOCATION)].append(keywords)

        return res

    @BaseStore.store_call
    def url_delete(self, url):
        '''Delete a bookmark.

        :type url: string
        :param url: The URL of the bookmark
        '''
        # This is brute force. It seems the easiest way of removing a bookmark
        # is by removing every keyword there is from it.
        bm = self.__bms.find_bookmark(url)
        if bm:
            for kw in self.__bms.get_keywords().get_children():
                self.__bms.unset_keyword(kw, bm)

    @BaseStore.store_call
    def url_sync(self, url, desc, to_del, to_add):
        '''Synchronise a bookmark.

        The bookmark with the given URL is created if needed and its keywords
        are adjusted.

        :type url: string
        :param url: The URL of the bookmark
        :type desc: string
        :param desc: One-line description of the bookmark
        :type to_del: list
        :param to_del: The keywords (as strings) to delete from the bookmark
        :type to_add: list
        :param to_add: The keywords (as strings) to add to the bookmark
        '''
        bm = self.__bms.find_bookmark(url)
        sharekw = self.__bms.find_keyword(self.__kw, False)
        if not bm:
            bm = self.__bms.add(desc, url)

        if self.__excl:
            if self.__bms.has_keyword(sharekw, bm):
                self.__bms.unset_keyword(sharekw, bm)
        else:
            if not self.__bms.has_keyword(sharekw, bm):
                self.__bms.set_keyword(sharekw, bm)

        for k in to_del:
            kw = self.__bms.find_keyword(k, False)
            if kw: self.__bms.unset_keyword(kw, bm)

        for k in to_add:
            kw = self.__bms.find_keyword(k, False)
            if not kw: kw = self.__bms.add_keyword(k)
            self.__bms.set_keyword(kw, bm)
