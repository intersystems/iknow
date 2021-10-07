#!/usr/bin/env python3
"""Search for an update to JSON for Modern C++.

Usage: update_json.py GITHUB_TOKEN"""

import updatelib

import json
import re
import urllib.request
import sys


vars = updatelib.get_vars()
json_version_old = vars['JSON_VERSION']

# send GitHub API request and parse response
headers = {'Authorization': 'Bearer ' + sys.argv[1]}
request = urllib.request.Request('https://api.github.com/repos/nlohmann/json/releases/latest', headers=headers)
url_data = urllib.request.urlopen(request)
json_data = json.load(url_data)
for asset in json_data['assets']:
    if asset['name'] == 'include.zip':
        download_url = asset['browser_download_url']
        break
else:
    raise ValueError(f'JSON for Modern C++ URL not found in {json_data["assets"]}')

# set variables
try:
    vars['JSON_VERSION'] = json_data['name'][json_data['name'].rindex(' ') + 1:]
except ValueError:
    raise ValueError(f'Release name {json_data["name"]} has unexpected format') from None
vars['JSON_URL'] = download_url
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('JSON_VERSION_OLD', json_version_old)
updatelib.setenv('JSON_VERSION_LATEST', vars['JSON_VERSION'])
updatelib.setenv('JSON_PUBLISHED_AT', json_data['published_at'])
updatelib.setenv('JSON_HTML_URL', json_data['html_url'])
updatelib.setenv('JSON_RELEASE_INFO', json_data['body'])
