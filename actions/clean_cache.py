#!/usr/bin/env python3
"""Delete all but the most recent single-use caches. This keeps the cache clear
of items that we know we will never use again, reducing the chance that GitHub
Actions will evict items that we still need.

Usage: clean_cache.py GITHUB_TOKEN"""

import itertools
import os
import re
import sys

import requests


def cache_sort_key(cache: dict):
    match = re.match(r'(.*-run-)(\d+)', cache['key'])
    return cache['ref'], match[1], int(match[2])


def cache_group_key(cache: dict):
    match = re.match(r'(.*-run-)\d+', cache['key'])
    return cache['ref'], match[1]


headers = {'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + sys.argv[1]}
caches = []
page = 1
total_count = 0
while True:
    json_data = requests.get(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/actions/caches?per_page=100&page={page}', headers=headers).json()
    new_caches = json_data['actions_caches']
    caches.extend(filter(lambda cache: re.match(r'.*-run-\d+', cache['key']), new_caches))
    if not new_caches or (total_count := total_count + len(new_caches)) >= json_data['total_count']:
        break
    page += 1

caches.sort(key=cache_sort_key)
deleted = False
for _, group in itertools.groupby(caches, cache_group_key):
    for cache in list(group)[:-1]:
        req = requests.delete(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/actions/caches/{cache["id"]}', headers=headers)
        if req.status_code == 204:
            print('Deleted cache', cache)
            deleted = True
        else:
            print('Failed to delete cache', cache)
if not deleted:
    print('No caches were deleted')
