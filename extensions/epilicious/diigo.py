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

import httplib2
import urllib
import urllib2
import simplejson
import time

# {{{1 Constants
_REALM = 'Application'
_BASE_URI = 'http://api2.diigo.com/'
_API_URI = _BASE_URI

class DiigoError(Exception): # {{{1
    def __init__(self, status, msg):
        self.status = status
        self.msg = msg

    def __str__(self):
        return 'DiigoError(status=%(status)i,msg=%(msg)s)' % self.__dict__

def _handle_http_error(function): # {{{1

    def decorate(*args, **kwargs):
        tries = 0
        while tries < 5:
            try:
                return function(*args, **kwargs)
            except DiigoError, e:
                if e.status == 503:
                    time.sleep(3 * tries)
                    tries += 1
                else:
                    raise e

    # It'd be a shame to lose the docstrings, wouldn't it?
    decorate.__doc__ = function.__doc__
    return decorate

class Diigo2(object): # {{{1
    '''Class for accessing Diigo's webservice API version 2.'''

    def __init__(self, user_name, password):
        self.__user = user_name
        self.__passwd = password

    @_handle_http_error
    def __do_request(self, function, method='GET', body=None, **kwargs):
        params = urllib.urlencode(kwargs)
        h = httplib2.Http()
        h.add_credentials(self.__user, self.__passwd)
        resp, cont = h.request(_API_URI + function + '?' + params, method=method, body=body)
        if resp['status'] != '200':
            raise DiigoError(int(resp['status']), function)
        return simplejson.loads(cont)

    def all_bookmarks(self, users=None, filter='all'):
        '''Retrieve all bookmarks.

        @param users  : limit to specific users (defaults to yourself)
        @param filter : all/public (default 'all')
        '''
        if users:
            u = users
        else:
            u = self.__user
        res = []
        idx = 0
        l = 100
        while l == 100:
            tmp = self.__do_request('bookmarks', users=u, filter=filter, start=idx, rows=100)
            res, l, idx = res + tmp, len(tmp), idx+100
        return res

    @_handle_http_error
    def delete_bookmark(self, url):
        '''Delete a bookmark.

        @param url : the URL
        '''
        return self.__do_request('bookmarks', method='DELETE', url=url)

    @_handle_http_error
    def add_bookmark(self, title, url, desc='', tags='', shared='yes'):
        '''Add/update a bookmark.

          @param url    : the URL
          @param title  : title for the bookmark
          @param desc   : description of the bookmark (default "")
          @param tags   : string of tags for the URL (separated by spaces, default "")
          @param shared : whether the URL should be shared or not (default "yes")
        '''
        # ToDo: diigo seems to barf on 'dangerous' URLs like file://, come up
        # with a way to deal with it!
        return self.__do_request('bookmarks', method='POST', body=' ', \
                title=title, url=url, desc=desc, tags=tags, shared=shared)
