#!/usr/bin/env bash

# Check whether and where we should deploy iknowpy after building it. We deploy
# iknowpy if and only if 3 criteria hold.
#   1. The build is from the master branch.
#   2. The build is not associated with an open pull request.
#   3. The commit that the current build is testing contains a change to
#      /modules/iknowpy/iknowpy/version.py.
#
# If the version specified in version.py is a developmental release version as
# defined in PEP 440, then we should deploy to TestPyPI. Otherwise, we should
# deploy to PyPI. A developmental release version ends with ".devN", where N is
# the development release number. For example, "0.0.10.dev0" is a development
# release number, while "0.0.10" is not.
#
# Required environment variables
# TRAVIS_BRANCH: branch for which the build is occurring
# TRAVIS_PULL_REQUEST: pull request number for current build, false if it's not a pull request
# TRAVIS_COMMIT: commit hash for current build
# REPO_ROOT: root of Git repository
#
# Output
# Write PyPI to stdout if we should deploy to PyPI.
# Write TestPyPI to stdout if we should deploy to TestPyPI.
# Write 0 to stdout otherwise.

set -euxo pipefail

if [[ "$TRAVIS_BRANCH" == "master" ]] && \
    [[ "$TRAVIS_PULL_REQUEST" == "false" ]] && \
    git diff-tree --no-commit-id --name-only -r $TRAVIS_COMMIT | grep modules/iknowpy/iknowpy/version.py > /dev/null
then
  if grep ".dev[0-9][0-9]*'" "$REPO_ROOT/modules/iknowpy/iknowpy/version.py"; then
    echo TestPyPI
  else
    echo PyPI
  fi
else
  echo 0
fi
