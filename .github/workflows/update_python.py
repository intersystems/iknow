#!/usr/bin/env python3
"""Search for an update to Python for a given OS.

This update script can be executed on any platform (not necessarily the OS for
which we're searching for a Python update).

For Windows, we search for updates using NuGet.

For Mac OS X, we search for updates using pyenv. The caller specifies the
version of pyenv to use to search for Python updates.

Usage: update_python.py win64
       update_python.py osx PYENV_TOOL_VERSION_CURRENT GITHUB_TOKEN
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


platform = sys.argv[1]
assert platform in ('win64', 'osx'), f'{platform} is not supported'
if platform == 'win64':
    VERSION_REGEX = r'^([0-9]+\.){2}[0-9]+(-(a|b|rc)[0-9]+)?$'
else:
    VERSION_REGEX = r'^([0-9]+\.){2}[0-9]+((a|b|rc)[0-9]+)?$'

vars = updatelib.get_vars()
if platform == 'win64':
    current_versions = vars['PYVERSIONS_WIN'].split()
else:
    current_versions = vars['PYVERSIONS_OSX'].split()

# Get list of available Python versions.
pyenv_tool_version_current = None
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
    pyenv_tool_version_current = sys.argv[2]
    headers = {'Authorization': 'Bearer ' + sys.argv[3]}
    request = urllib.request.Request(f'https://api.github.com/repos/pyenv/pyenv/zipball/v{pyenv_tool_version_current}', headers=headers)
    url_data = urllib.request.urlopen(request)
    home_dir = os.environ['HOME']
    with open(os.path.join(home_dir, 'pyenv-src.zip'), 'wb') as file:
        file.write(url_data.read())
    subprocess.run(['unzip', '-q', os.path.join(home_dir, 'pyenv-src.zip'), '-d', home_dir], check=True)
    for item in os.listdir(home_dir):
        if os.path.isdir(os.path.join(home_dir, item)) and item.startswith('pyenv-'):
            pyenv_folder_name = item
            break
    else:
        raise ValueError('Extracted contents of pyenv-src.zip not found')
    os.environ['PYENV_ROOT'] = os.path.join(home_dir, pyenv_folder_name)
    pyenv_binary_path = os.path.join(os.environ['PYENV_ROOT'], 'bin/pyenv')
    p = subprocess.run([pyenv_binary_path, 'install', '--list'],
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
    vars['PYENV_TOOL_VERSION'] = pyenv_tool_version_current
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
updatelib.setenv('PYTHON_UPDATE_INFO_ONELINE', ', '.join('→'.join(x) for x in update_info))
for i in range(len(update_info)):
    update_info[i] = ' can be updated to '.join(update_info[i])
    update_info[i] = f'- {update_info[i]}'
updatelib.setenv('PYTHON_UPDATE_INFO_MULTILINE', '\n'.join(update_info))
