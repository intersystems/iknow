#!/usr/bin/env bash

# Check whether we should deploy iknowpy after building it. We deploy iknowpy
# if and only if 3 criteria hold.
#   1. The build is from the master branch.
#   2. The build is not associated with an open pull request.
#   3. The commit that the current build is testing contains a change to
#      /modules/iknowpy/iknowpy/version.py.
# Write 1 to stdout if we should deploy. Write 0 to stdout otherwise.

set -euxo pipefail

if [[ "$TRAVIS_BRANCH" == "master" ]] && \
    [[ "$TRAVIS_PULL_REQUEST" == "false" ]] && \
    [ -n "$(git diff-tree --no-commit-id --name-only -r $TRAVIS_COMMIT | grep modules/iknowpy/iknowpy/version.py)" ]
then
  echo 1
else
  echo 0
fi
