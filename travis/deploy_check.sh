#!/usr/bin/env bash

# Check whether we should deploy iknowpy after building it. We deploy iknowpy
# if and only if the commit that the current build is testing contains a change
# to the file /modules/iknowpy/iknowpy/version.py. Write 1 to stdout if we
# should deploy. Write 0 to stdout otherwise.

set -euxo pipefail

if [ -n "$(git diff-tree --no-commit-id --name-only -r $TRAVIS_COMMIT | grep modules/iknowpy/iknowpy/version.py)" ]; then
  echo 1
else
  echo 0
fi
