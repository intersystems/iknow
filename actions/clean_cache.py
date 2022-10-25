#!/usr/bin/env python3
"""Delete cache items that will never be used again, including the following
items.
- All but the most recent single-use cache per ref
- Caches associated with closed pull requests
- Caches associated with deleted branches
This reduces the chance that GitHub Actions will evict items that we still
need.

Usage: clean_cache.py GITHUB_TOKEN"""

import functools
import itertools
import os
import re
import sys

import requests


def cache_sort_key(cache: dict):
    match = re.fullmatch(r'(.*-run-)(\d+)', cache['key'])
    return cache['ref'], match[1], int(match[2])


def cache_group_key(cache: dict):
    match = re.fullmatch(r'(.*-run-)\d+', cache['key'])
    return cache['ref'], match[1]


@functools.lru_cache(maxsize=None)
def get(url: str):
    """Wrapper for requests.get() that caches results and uses HEADERS"""
    return requests.get(url, headers=HEADERS)


HEADERS = {'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + sys.argv[1]}

run_caches = []
page = 1
total_count = 0
to_delete = []
while True:
    json_data = requests.get(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/actions/caches?per_page=100&page={page}', headers=HEADERS).json()
    new_caches = json_data['actions_caches']
    for cache in new_caches:
        if match := re.fullmatch(r'refs/pull/(\d+)/merge', cache['ref']):
            # find caches associated with closed pull requests
            req = get(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/pulls/{match[1]}')
            if req.status_code == 200:
                if req.json()['state'] == 'closed':
                    to_delete.append(cache)
            else:
                print(f'Failed to obtain pull request information for cache: {cache}')
        elif match := re.fullmatch('refs/heads/(.+)', cache['ref']):
            # find caches associated with deleted branches
            req = get(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/branches/{match[1]}')
            if req.status_code == 404:
                to_delete.append(cache)
            elif req.status_code != 200:
                print(f'Failed to obtain branch information for cache: {cache}')
        if re.fullmatch(r'.*-run-\d+', cache['key']):
            run_caches.append(cache)
    if not new_caches or (total_count := total_count + len(new_caches)) >= json_data['total_count']:
        break
    page += 1

run_caches.sort(key=cache_sort_key)
for _, group in itertools.groupby(run_caches, cache_group_key):
    for cache in list(group)[:-1]:
        if cache not in to_delete:
            to_delete.append(cache)

size = 0
for cache in to_delete:
    req = requests.delete(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/actions/caches/{cache["id"]}', headers=HEADERS)
    if req.status_code == 204:
        print('Deleted cache', cache)
        size += cache['size_in_bytes']
    else:
        print('Failed to delete cache', cache)
print(f'Freed {size:,} bytes')
