#!/usr/bin/env python3

"""Check whether the iknowpy project on PyPI has nearly reached the 10GiB limit.

If we are near the limit and there is no open issue saying that we are near the
limit, create the file $HOME/issue.md that says that we are near the limit.
Otherwise, do nothing.

Usage: check_pypi_limit.py GITHUB_TOKEN"""

import json
import os
import sys
import urllib.request


SIZE_THRESHOLD = 8589934592  # 8 GiB

# determine whether the size of the iknowpy project on PyPI has exceeded the
# threshold
size = 0
url_data = urllib.request.urlopen('https://pypi.org/pypi/iknowpy/json')
json_data = json.load(url_data)
for release in json_data['releases'].values():
    for file in release:
        size += file['size']
if size <= SIZE_THRESHOLD:
    print(f'Size of iknowpy is {size:,} bytes, which is lower than the threshold of {SIZE_THRESHOLD:,} bytes')
    sys.exit(0)

# determine whether there is an open issue saying we're near the limit
headers = {'Authorization': 'Bearer ' + sys.argv[1]}
request = urllib.request.Request(f'https://api.github.com/repos/{os.environ["GITHUB_REPOSITORY"]}/issues?state=open&creator=github-actions%5Bbot%5D', headers=headers)
url_data = urllib.request.urlopen(request)
json_data = json.load(url_data)
for issue in json_data:
    if issue['title'].startswith('[check-pypi-limit]'):
        print(f'Issue already exists at {issue["html_url"]}')
        sys.exit(0)

# create file containing issue details
issue_text = f"""\
The `iknowpy` project at https://pypi.org/project/iknowpy/ is near the 10GiB \
limit. Current size is {size:,} bytes ({size/1024**3:.02f}GiB). Once the limit \
is reached, you will no longer be able to produce another release. Consider \
deleting old releases that are no longer relevant.

*I am a bot, and this action was performed automatically.*\
"""
with open(os.path.join(os.environ["HOME"], 'issue.md'), 'w') as issue_file:
    issue_file.write(issue_text)
