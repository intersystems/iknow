#!/usr/bin/env python3
"""Search for an update to ICU.

Usage: update_icu.py GITHUB_TOKEN"""

import updatelib

import json
import re
import urllib.request
import sys


vars = updatelib.get_vars()
icu_name_old = vars['ICU_NAME']

# send GitHub API request and parse response
headers = {'Authorization': 'Bearer ' + sys.argv[1]}
request = urllib.request.Request('https://api.github.com/repos/unicode-org/icu/releases/latest', headers=headers)
url_data = urllib.request.urlopen(request)
json_data = json.load(url_data)
win_url = None
src_url = None
for asset in json_data['assets']:
    if re.match(r'^icu4c-.+-Win64-.+\.zip$', asset['name']):
        win_url = asset['browser_download_url']
    elif re.match(r'^icu4c-.+-src\.zip$', asset['name']):
        src_url = asset['browser_download_url']
if win_url is None:
    print('Warning: ICU Win64 URL for latest version was not found')
if src_url is None:
    print('Warning: ICU source URL for latest version was not found')

# set variables to latest ICU version
vars['ICU_URL_WIN'] = win_url
vars['ICU_URL_SRC'] = src_url
vars['ICU_NAME'] = json_data['name']
if win_url is not None and src_url is not None:
    updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('ICU_NAME_OLD', icu_name_old)
updatelib.setenv('ICU_NAME_LATEST', vars['ICU_NAME'])
updatelib.setenv('ICU_PUBLISHED_AT', json_data['published_at'])
updatelib.setenv('ICU_HTML_URL', json_data['html_url'])
updatelib.setenv('ICU_RELEASE_INFO', json_data['body'])
