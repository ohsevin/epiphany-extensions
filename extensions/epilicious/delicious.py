# -*- coding: utf-8 -*-

# Copyright (C) 2006 by Magnus Therning

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

# History: First epilicious used Frank Timmermann's pydelicious module. It
# worked fine for a long time (up to and including release 0.9) but there were
# a few things that irritated me:
#  1. pydelicious includes more than epilicious needs, meaning that I had to
#     cut a bit to keep dependencies reasonable
#  2. when it broke it broke in strange ways
#  3. there are bugs in pydelicious, some so basic that I have serious doubts
#     about the quality of the entire module
# Given this and the fact that epilicious needs only a small part of the
# pydelicious functionality I decided to write a custom del.icio.us module for
# epilicious.

import urllib
import urllib2
import sys
import time

# ElementTree is a standard lib from Python 2.5 {{{1
if sys.version_info[0:2] >= (2, 5):
    from xml.etree.ElementTree import parse as xml_parse
else:
    from elementtree.ElementTree import parse as xml_parse


def _handle_throttle(function): # {{{1
    # Decorator to deal with the throttling (503) that might happen when
    # talking to del.icio.us.
    # The documentation for the del.icio.us API isn't very clear on this.  They
    # make it sound as if it's alright to make the request and if you get a 503
    # back then you have to wait.  This isn't strictly true.  In some cases
    # you'll receive a 500 (internal error), suggesting that their server has a
    # bug, and if you push it a bit and only wait after receiving a 503 you run
    # the chance of getting your account temporarily blocked (999).  So, to be
    # on the safe side we _always_ wait a second before sending off a request,
    # silly I know!

    def decorate(*args, **kwargs):
        attempt = 0
        while 1:
            try:
                time.sleep(1)
                return function(*args, **kwargs)
            except urllib2.HTTPError, e:
                attempt += 1
                if attempt > 3: raise e # fed up trying
                if e.code == 503:
                    print '503'
                    time.sleep(30)
                elif e.code == 500:
                    print '500'
                    time.sleep(120)
                else:
                    raise e

    # It'd be a shame to lose the docstrings, wouldn't it?
    decorate.__doc__ = function.__doc__
    return decorate


class _DeliciousParser(object): # {{{1
    # Class for parsing the responses from del.icio.us. All methods expect an
    # ElementTree.

    def __init__(self):
        pass

    def parse_result(self, xmltree):
        # Parses a result tree. Returns 1 if done, 0 otherwise.
        if xmltree.getroot().attrib.get('code', '') == 'done':
            return 1
        else:
            return 0

    def parse_posts(self, xmltree):
        # Parses a <posts> tree into a list of dictionaries.
        keys = [ 'href', 'description', 'hash', 'tag', 'time', 'extended', \
                'count', ]
        l = []
        for n in xmltree.findall('post'):
            e = dict([[k, n.attrib.get(k, '')] for k in keys])
            l.append(e)
        return l


# {{{1 Constants
_DWS_REALM = 'del.icio.us API'
_DWS_BASE_URI = 'https://api.del.icio.us/'
_DWS_API_URI = 'https://api.del.icio.us/v1/'
_DWS_USER_AGENT = 'epilicious/0.9 (magnus@therning.org)'


class Delicious(object): # {{{1
    def __init__(self, user_name, password):
        self.__user = user_name
        self.__passwd = password

    def __do_request(self, method, **kwargs):
        # Do the request to del.icio.us. Pass off the result to
        # __parse_result().
        for k in kwargs:
            if isinstance(kwargs[k], unicode):
                kwargs[k] = kwargs[k].encode('utf-8')
        params = urllib.urlencode(kwargs)
        authinfo = urllib2.HTTPBasicAuthHandler()
        authinfo.add_password(realm=_DWS_REALM, uri=_DWS_BASE_URI, user=self.__user, passwd=self.__passwd)
        opener = urllib2.build_opener(authinfo)
        request = urllib2.Request(_DWS_API_URI + method + '?' + params)
        request.add_header('User-Agent', _DWS_USER_AGENT)
        return self.__parse_result(xml_parse(opener.open(request)))

    def __parse_result(self, xmltree):
        # Pass on the actual parsing to a _DeliciousParser instance.
        fname = 'parse_' + xmltree.getroot().tag
        return getattr(_DeliciousParser(), fname)(xmltree)

    def all_bookmarks(self, tag=''):
        '''Get all bookmarks from del.icio.us.

        tag: (required) limits the response to bookmarks with that tag

        Returns a list of dictionaries:
            [{'count': ..., 'description': ..., 'extended': ..., 'hash': ...,
              'href': ..., 'tag': ..., 'time': ...},
            ]
        '''
        return self.__do_request('posts/all', tag=tag)
    all_bookmarks = _handle_throttle(all_bookmarks)

    def delete_bookmark(self, url):
        '''Delete bookmark from del.icio.us.

        url: (required) the URL

        Returns 1 on success, 0 on failure.
        '''
        return self.__do_request('posts/delete', url=url)
    delete_bookmark = _handle_throttle(delete_bookmark)

    def add_bookmark(self, url, description, notes='', tags='', dt='', replace='yes', shared='yes'):
        '''Add a bookmark to del.icio.us.

        url: (required) the URL
        description: (required) description of the URL
        notes: notes for the URL (default "")
        tags: string of tags for the URL (separated by spaces, default "")
        replace: whether an already existing bookmark should be replaced ("yes"
                 or "no", default "yes")
        shared: whether the URL should be shared or not ("yes" or "no", default
                 "no")

        Returns 1 on success, 0 on failure.
        '''
        return self.__do_request('posts/add', url=url, \
                description=description, extended=notes, tags=tags,
                dt=dt, replace=replace, shared=shared)
    add_bookmark = _handle_throttle(add_bookmark)
