#!/usr/bin/env bash

# After the build is complete, execute this script on Linux to deploy the wheels
# if appropriate.
#
# Wheels are assumed to be in ~/wheels/*.whl.
#
# We deploy iknowpy if and only if 3 criteria hold.
#   1. The build is due to a push to the master branch.
#   2. Variables PYPI_TOKEN and TESTPYPI_TOKEN are nonempty. 
#   3. The commit that the current build is testing contains a change to
#      modules/iknowpy/iknowpy/version.py, or FORCE_DEPLOY=1.
#
# If the version specified in version.py is a developmental release version as
# defined in PEP 440, then we deploy to TestPyPI. Otherwise, we deploy to PyPI.
# A developmental release version ends with ".devN", where N is the development
# release number. For example, "0.0.10.dev0" is a development release number,
# while "0.0.10" is not.
#
# Required Environment Variables:
# - GITHUB_EVENT_NAME is the event that triggered the workflow
# - GITHUB_REF is the branch ref that triggered the workflow
# - GITHUB_SHA is the commit hash for current build
# - COMMIT_RANGE is a space-separated string of up to 20 commit hashes for the
#   commits in this push
#
# Optional Environment Variables:
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI. If unset,
#   deployment is skipped.
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI. If
#   unset, deployment is skipped.
# - FORCE_DEPLOY, if set to 1, indicates that deployment should occur even
#   if no change to version.py occurred.
#
# Output Variables:
# - DEPLOY_OCCURRED: 0 or 1, indicating whether deployment occurred
# - WARN_COMMIT: the commit hash for the commit that changed version.py
#     (present only if version.py was changed in the last <= 20 commits in this
#      push and was not changed in the final commit)

set -euxo pipefail

EXPECTED_WHEEL_COUNT=5  # 5 platforms

if [ "$GITHUB_EVENT_NAME" = push ] || [ "$GITHUB_EVENT_NAME" = workflow_dispatch ] && \
    [ "$GITHUB_REF" = refs/heads/master ] && \
    [ -n "${PYPI_TOKEN-}" ] && [ -n "${TESTPYPI_TOKEN-}" ]
then
  if git diff-tree --no-commit-id --name-only -r "$GITHUB_SHA" | grep modules/iknowpy/iknowpy/version.py > /dev/null || \
      [ "${FORCE_DEPLOY-}" = 1 ]
  then
    WHEELS=~/wheels/*.whl
    if [ $(echo $WHEELS | wc -w) -ne $EXPECTED_WHEEL_COUNT ]; then
      echo "Error: Expected $EXPECTED_WHEEL_COUNT wheels"
      exit 1
    fi
    pip3 install --user twine --no-warn-script-location
    if grep ".dev[0-9][0-9]*'" "modules/iknowpy/iknowpy/version.py"; then
      python3 -m twine upload -r testpypi --skip-existing -u "__token__" -p "$TESTPYPI_TOKEN" $WHEELS
    else
      python3 -m twine upload --skip-existing -u "__token__" -p "$PYPI_TOKEN" $WHEELS
    fi
    echo "DEPLOY_OCCURRED=1" >> $GITHUB_ENV
  else
    if [ "$GITHUB_REPOSITORY" = intersystems/iknow ]; then
      for SHA in $COMMIT_RANGE; do
        if git diff-tree --no-commit-id --name-only -r "$SHA" | grep modules/iknowpy/iknowpy/version.py > /dev/null
        then
          echo "WARN_COMMIT=$SHA" >> $GITHUB_ENV
          break
        fi
      done
    fi
    echo "Deployment skipped"
    echo "DEPLOY_OCCURRED=0" >> $GITHUB_ENV
  fi
else
  echo "Deployment skipped"
  echo "DEPLOY_OCCURRED=0" >> $GITHUB_ENV
fi
