#!/usr/bin/env python3
"""Search for an update to the manylinux Docker images.

Usage: update_manylinux.py"""

import updatelib
import requests


vars = updatelib.get_vars()
manylinux2010_x86_64_tag = vars['MANYLINUX2010_X86_64_TAG']
manylinux2014_aarch64_tag = vars['MANYLINUX2014_AARCH64_TAG']
manylinux2014_ppc64le_tag = vars['MANYLINUX2014_PPC64LE_TAG']

# send Quay API requests and parse responses
tags = []
for repo in ('manylinux2010_x86_64', 'manylinux2014_aarch64', 'manylinux2014_ppc64le'):
    r = requests.get(f'https://quay.io/api/v1/repository/pypa/{repo}/image')
    json_data = r.json()
    for image in reversed(json_data['images']):
        if 'latest' in image['tags']:
            break
    else:
        raise ValueError(f'latest tag not found: {json_data["images"]}')
    for tag in image['tags']:
        if tag != 'latest':
            break
    else:
        raise ValueError(f'failed to find tag for latest image: {image}')
    tags.append(tag)

# set variables to latest ICU version
vars['MANYLINUX2010_X86_64_TAG'] = tags[0]
vars['MANYLINUX2014_AARCH64_TAG'] = tags[1]
vars['MANYLINUX2014_PPC64LE_TAG'] = tags[2]
updatelib.set_vars(vars)

# set environment variables for next GitHub actions step
message = []
if manylinux2010_x86_64_tag != tags[0]:
    message.append(['manylinux2010_x86_64', manylinux2010_x86_64_tag, tags[0]])
if manylinux2014_aarch64_tag != tags[1]:
    message.append(['manylinux2014_aarch64', manylinux2014_aarch64_tag, tags[1]])
if manylinux2014_ppc64le_tag != tags[1]:
    message.append(['manylinux2014_ppc64le', manylinux2014_ppc64le_tag, tags[2]])
updatelib.setenv('MANYLINUX_UPDATE_INFO_ONELINE', ', '.join(f'{repo}[{old_tag}→{new_tag}]' for repo,old_tag,new_tag in message))
updatelib.setenv('MANYLINUX_UPDATE_INFO_MULTILINE', '\n'.join(f'- {repo}: update tag `{old_tag}` to `{new_tag}`' for repo,old_tag,new_tag in message))
