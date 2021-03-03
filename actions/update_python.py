#!/usr/bin/env python3
"""Search for an update to Python for a given OS.

For Windows, we search for updates using NuGet.

For Mac OS X, we search for updates using pyenv.

Usage:
    update_python.py win64
        (can be run on any platform)
    update_python.py osx
        (must be run on Mac OS X)
"""

import updatelib

import json
import os
import re
import subprocess
import sys
import urllib.request


def get_first_numeric(s):
    """Return the index of the first numeric character in s, -1 if none
    exists."""
    for i in range(len(s)):
        if s[i] in '0123456789':
            return i
    return -1


def get_first_nonnumeric(s):
    """Return the index of the first non-numeric character in s, -1 if all
    characters are numeric."""
    for i in range(len(s)):
        if s[i] not in '0123456789':
            return i
    return -1


def compare_versions(a, b):
    """Compare 2 Python version strings. Return -1 if a < b, 0 if a == b, and
    1 if a > b. Version string must match the regex VERSION_REGEX."""
    a = a.split('.')
    b = b.split('.')
    a[0] = int(a[0])
    b[0] = int(b[0])
    a[1] = int(a[1])
    b[1] = int(b[1])
    nonnumeric_i = get_first_nonnumeric(a[2])
    if nonnumeric_i == -1:
        a[2] = int(a[2])
        a.append('zz')
    else:
        micro = a[2]
        a[2] = int(micro[:nonnumeric_i])
        nonnumeric_i += micro[nonnumeric_i] == '-'
        after_micro = micro[nonnumeric_i:]
        numeric_i = get_first_numeric(after_micro)
        a.append(after_micro[:numeric_i])
        a.append(int(after_micro[numeric_i:]))
    nonnumeric_i = get_first_nonnumeric(b[2])
    if nonnumeric_i == -1:
        b[2] = int(b[2])
        b.append('zz')
    else:
        micro = b[2]
        b[2] = int(micro[:nonnumeric_i])
        nonnumeric_i += micro[nonnumeric_i] == '-'
        after_micro = micro[nonnumeric_i:]
        numeric_i = get_first_numeric(after_micro)
        b.append(after_micro[:numeric_i])
        b.append(int(after_micro[numeric_i:]))
    if a < b:
        return -1
    elif a == b:
        return 0
    return 1


ALLOW_PRERELEASE = False  # flag for whether to allow updates to prerelease Python versions

platform = sys.argv[1]
assert platform in ('win64', 'osx'), f'{platform} is not supported'
if platform == 'win64' and ALLOW_PRERELEASE:
    VERSION_REGEX = r'^([0-9]+\.){2}[0-9]+(-(a|b|rc)[0-9]+)?$'
elif platform == 'osx' and ALLOW_PRERELEASE:
    VERSION_REGEX = r'^([0-9]+\.){2}[0-9]+((a|b|rc)[0-9]+)?$'
else:
    VERSION_REGEX = r'^([0-9]+\.){2}[0-9]+$'

vars = updatelib.get_vars()
if platform == 'win64':
    current_versions = vars['PYVERSIONS_WIN'].split()
else:
    current_versions = vars['PYVERSIONS_OSX'].split()

# Get list of available Python versions.
if platform == 'win64':
    url_data = urllib.request.urlopen('https://api.nuget.org/v3/index.json')
    json_data = json.load(url_data)
    for item in json_data['resources']:
        if item['@type'] == 'PackageBaseAddress/3.0.0':
            base_url = item['@id']
            break
    else:
        raise ValueError('PackageBaseAddress/3.0.0 not found')
    url_data = urllib.request.urlopen(f'{base_url}python/index.json')
    json_data = json.load(url_data)
    available_versions = [version for version in json_data['versions'] if re.match(VERSION_REGEX, version)]
else:  # platform == 'osx'
    if sys.platform != 'darwin':
        raise EnvironmentError('Must be run on Mac OS X')
    subprocess.run(['brew', 'update'], check=True)
    subprocess.run(['brew', 'install', 'pyenv'], check=True)
    p = subprocess.run(['pyenv', 'install', '--list'],
                       stdout=subprocess.PIPE, universal_newlines=True,
                       check=True)
    available_versions = [version for version in p.stdout.split() if re.match(VERSION_REGEX, version)]

# find updates to current versions
update_info = []
latest_versions = []
for current_version in current_versions:
    latest_version_found = current_version
    for available_version in available_versions:
        if current_version.split('.')[:2] == available_version.split('.')[:2] and \
                compare_versions(latest_version_found, available_version) == -1:
            latest_version_found = available_version
    latest_versions.append(latest_version_found)
    if current_version != latest_version_found:
        update_info.append([current_version, latest_version_found])

# set variables
if platform == 'win64':
    vars['PYVERSIONS_WIN'] = ' '.join(latest_versions)
else:
    vars['PYVERSIONS_OSX'] = ' '.join(latest_versions)
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('PYTHON_UPDATE_INFO_ONELINE', ', '.join('→'.join(x) for x in update_info))
for i in range(len(update_info)):
    update_info[i] = ' can be updated to '.join(update_info[i])
    update_info[i] = f'- {update_info[i]}'
updatelib.setenv('PYTHON_UPDATE_INFO_MULTILINE', '\n'.join(update_info))
if platform == 'osx':
    p = subprocess.run(['brew', 'list', '--versions', 'pyenv'],
                       stdout=subprocess.PIPE, universal_newlines=True,
                       check=True)
    pyenv_tool_version = p.stdout.split()[1]
    updatelib.setenv('PYENV_TOOL_VERSION', pyenv_tool_version)
