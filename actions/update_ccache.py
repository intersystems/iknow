#!/usr/bin/env python3
"""Search for an update to ccache.

Usage: update_ccache.py GITHUB_TOKEN"""

import updatelib

import bs4
import json
import re
import sys
import urllib.request


vars = updatelib.get_vars()
ccache_version_old = vars['CCACHE_VERSION']

# send GitHub API request and parse response
headers = {'Authorization': 'Bearer ' + sys.argv[1]}
request = urllib.request.Request('https://api.github.com/repos/ccache/ccache/releases/latest', headers=headers)
url_data = urllib.request.urlopen(request)
json_data = json.load(url_data)

for asset in json_data['assets']:
    if asset['name'].endswith('windows-x86_64.zip'):
        download_url = asset['browser_download_url']
        break
else:
    raise ValueError(f'ccache URL not found in {json_data["assets"]}')
version = json_data['name']

# obtain changelog
if match := re.search(r'\[Release notes]\((.+?)\)', json_data['body']):
    changelog_url = match[1]
    request = urllib.request.urlopen(changelog_url)
    soup = bs4.BeautifulSoup(request, 'html.parser')
    for h2_element in soup.find_all('h2'):
        if any(s == f'Ccache {version}' for s in h2_element.stripped_strings):
            changelog_element = h2_element.parent
            # remove links, classes, and ids
            elements = [changelog_element]
            elements.extend(changelog_element.find_all())
            for element in elements:
                if element.get('href') is not None:
                    del element['href']
                if element.get('class') is not None:
                    del element['class']
                if element.get('id') is not None:
                    del element['id']
            changelog_text = str(changelog_element)
            break
    else:
        print(soup)
        changelog_text = 'Error parsing changelog; click the above link to view release information.'
else:
    changelog_url = 'https://ccache.dev/releasenotes.html'
    changelog_text = 'Error parsing changelog; click the above link to view release information.'

# set variables
vars['CCACHE_VERSION'] = version
vars['CCACHE_URL'] = download_url
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('CCACHE_VERSION_OLD', ccache_version_old)
updatelib.setenv('CCACHE_VERSION', version)
updatelib.setenv('CCACHE_PUBLISHED_AT', json_data['published_at'])
updatelib.setenv('CCACHE_CHANGELOG_URL', changelog_url)
updatelib.setenv('CCACHE_CHANGELOG', changelog_text)
updatelib.setenv('CCACHE_HTML_URL', json_data['html_url'])
