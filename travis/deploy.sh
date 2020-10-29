#!/usr/bin/env bash

# After the build is complete, execute this script to deploy the wheels if
# appropriate. The working directory must be the root of the repository.
#
# We deploy iknowpy if and only if 4 criteria hold.
#   1. The build is from the master branch.
#   2. The build is not associated with an open pull request.
#   3. Variables PYPI_TOKEN and TESTPYPI_TOKEN are set. 
#   4. The commit that the current build is testing contains a change to
#      /modules/iknowpy/iknowpy/version.py, OR the build was triggered manually
#      via the Travis UI with variable FORCE_DEPLOY=1.
#
# If the version specified in version.py is a developmental release version as
# defined in PEP 440, then we deploy to TestPyPI. Otherwise, we deploy to PyPI.
# A developmental release version ends with ".devN", where N is the development
# release number. For example, "0.0.10.dev0" is a development release number,
# while "0.0.10" is not.
#
# Required Environment Variables:
# - TRAVIS_OS_NAME is the name of the platform (linux, osx, windows)
# - TRAVIS_BRANCH is the branch for which the build is occurring
# - TRAVIS_PULL_REQUEST is the pull request number for current build, or false
#   if it's not a pull request
# - TRAVIS_COMMIT is the commit hash for current build
# - TRAVIS_EVENT_TYPE indicates how the build was triggered
# - TRAVIS_COMMIT_MESSAGE is the commit message
# - PYVERSIONS (windows and osx only) is the space-delimited Python versions
#   present, ordered old to new
# - PYINSTALL_DIR (windows only) is the location Python is installed
#
# Optional Environment Variables:
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI. If unset,
#   deployment is skipped.
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI. If
#   unset, deployment is skipped.

set -euxo pipefail

if [ "$TRAVIS_BRANCH" = master ] && \
    [ "$TRAVIS_PULL_REQUEST" = false ] && \
    [ -n "${PYPI_TOKEN+x}" ] && [ -n "${TESTPYPI_TOKEN+x}" ] && \
    (git diff-tree --no-commit-id --name-only -r "$TRAVIS_COMMIT" | grep modules/iknowpy/iknowpy/version.py > /dev/null || \
      ([ "$TRAVIS_EVENT_TYPE" = api ] && [ "${FORCE_DEPLOY-}" = 1 ]))
then
  if [ "$TRAVIS_OS_NAME" = linux ]; then
    WHEELS=modules/iknowpy/wheelhouse/iknowpy-*manylinux*.whl
    PYTHON=python3
  else
    WHEELS=modules/iknowpy/dist/iknowpy-*.whl
    if [ "$TRAVIS_OS_NAME" = windows ]; then
      PYTHON="$PYINSTALL_DIR/python.${PYVERSIONS##* }/tools/python.exe"
    else  # osx
      PYTHON=python$(echo "${PYVERSIONS##* }" | awk -F '.' '{print $1"."$2}')
    fi
  fi
  "$PYTHON" -m pip install --user twine --no-warn-script-location
  if grep ".dev[0-9][0-9]*'" "modules/iknowpy/iknowpy/version.py"; then
    export TWINE_REPOSITORY=testpypi
    { set +x; } 2>/dev/null  # don't save token to build log
    echo '+ "$PYTHON" -m twine upload -u "__token__" -p "$TESTPYPI_TOKEN" $WHEELS'
    "$PYTHON" -m twine upload --skip-existing -u "__token__" -p "$TESTPYPI_TOKEN" $WHEELS
    set -x
  else
    export TWINE_REPOSITORY=pypi
    { set +x; } 2>/dev/null  # don't save token to build log
    echo '+ "$PYTHON" -m twine upload -u "__token__" -p "$PYPI_TOKEN" $WHEELS'
    "$PYTHON" -m twine upload --skip-existing -u "__token__" -p "$PYPI_TOKEN" $WHEELS
    set -x
  fi
else
  echo "Deployment skipped"
fi
