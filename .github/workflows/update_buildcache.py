#!/usr/bin/env python3
"""Search for an update to buildcache.

Usage: update_buildcache.py GITHUB_TOKEN"""

import updatelib

import json
import re
import urllib.request
import sys


vars = updatelib.get_vars()
buildcache_name_old = vars['BUILDCACHE_NAME']

# send GitHub API request and parse response
headers = {'Authorization': 'Bearer ' + sys.argv[1]}
request = urllib.request.Request('https://api.github.com/repos/mbitsnbites/buildcache/releases/latest', headers=headers)
url_data = urllib.request.urlopen(request)
json_data = json.load(url_data)
for asset in json_data['assets']:
    if re.match(r'^.+-win-msvc\.zip$', asset['name']):
        download_url = asset['browser_download_url']
        break
else:
    raise ValueError(f'buildcache URL not found in {json_data["assets"]}')

# set variables
vars['BUILDCACHE_NAME'] = json_data['name']
vars['BUILDCACHE_URL_WIN'] = download_url
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('BUILDCACHE_NAME_OLD', buildcache_name_old)
updatelib.setenv('BUILDCACHE_NAME_LATEST', vars['BUILDCACHE_NAME'])
updatelib.setenv('BUILDCACHE_PUBLISHED_AT', json_data['published_at'])
updatelib.setenv('BUILDCACHE_HTML_URL', json_data['html_url'])
