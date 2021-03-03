#!/usr/bin/env python3
"""Search for an update to Cython.

Usage: update_cython.py"""

import updatelib

import bs4
import requests

vars = updatelib.get_vars()
old_version = vars['CYTHON_VERSION']

# find latest Cython version
json_data = requests.get('https://pypi.org/pypi/Cython/json').json()
latest_version = json_data['info']['version']

# apply latest version
vars['CYTHON_VERSION'] = latest_version
updatelib.set_vars(vars)

# obtain changelog
r = requests.get('https://cython.readthedocs.io/en/latest/src/changes.html')
soup = bs4.BeautifulSoup(r.text, 'html.parser')
for h2_element in soup.find_all('h2'):
    if any(s.startswith(latest_version) for s in h2_element.stripped_strings):
        changelog_element = h2_element.parent
        # remove permalink anchors
        for a_element in changelog_element.find_all('a'):
            if a_element.get('class') is not None and 'headerlink' in a_element.get('class'):
                a_element.decompose()
        # remove classes and ids
        elements = [changelog_element]
        elements.extend(changelog_element.find_all())
        for element in elements:
            if element.get('class') is not None:
                del element['class']
            if element.get('id') is not None:
                del element['id']
        changelog_text = str(changelog_element)
        break
else:
    print(soup)
    changelog_text = 'Error parsing changelog; click the above link to view release information.'

# set environment variables for next GitHub actions step
updatelib.setenv('CYTHON_VERSION_OLD', old_version)
updatelib.setenv('CYTHON_VERSION_LATEST', latest_version)
updatelib.setenv('CYTHON_CHANGELOG', changelog_text)
