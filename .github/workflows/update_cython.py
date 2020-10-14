#!/usr/bin/env python3
"""Search for an update to Cython.

Usage: update_cython.py"""

import updatelib

import json
import urllib.request

vars = updatelib.get_vars()
old_version = vars['CYTHON_VERSION']

# find latest Cython version
url_data = urllib.request.urlopen('https://pypi.org/pypi/Cython/json')
json_data = json.load(url_data)
latest_version = json_data['info']['version']

# apply latest version
vars['CYTHON_VERSION'] = latest_version
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('CYTHON_VERSION_OLD', old_version)
updatelib.setenv('CYTHON_VERSION_LATEST', latest_version)
