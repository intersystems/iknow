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
import re
import subprocess
import sys
import urllib.request


def compare_versions(a, b):
    """Compare 2 Python version strings. Return -1 if a < b, 0 if a == b, and
    1 if a > b. Version string must match the regex VERSION_REGEX."""
    a = [int(i) for i in a.split('.')]
    b = [int(i) for i in b.split('.')]
    if a < b:
        return -1
    elif a == b:
        return 0
    return 1


platform = sys.argv[1]
assert platform in ('win64', 'osx'), f'{platform} is not supported'
VERSION_REGEX = r'^([0-9]+\.){2,}[0-9]+$'

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
