#!/usr/bin/env bash

# Install build dependencies on Mac OS X
#
# Required environment variables:
# - MACOSX_DEPLOYMENT_TARGET is the minimum supported Mac OS X version
# - PYINSTALL_DIR is the location that Python installations are installed and
#   cached

set -euxo pipefail

# Given a Python version (MAJOR.MINOR only), print the URL for a compatible
# official package installer. Return 1 if the version is not supported.
pyinstall_fallback () {
  case "$1" in
    3.5)
      echo https://www.python.org/ftp/python/3.5.4/python-3.5.4-macosx10.6.pkg
      ;;
    3.6)
      echo https://www.python.org/ftp/python/3.6.8/python-3.6.8-macosx10.9.pkg
      ;;
    3.7)
      echo https://www.python.org/ftp/python/3.7.9/python-3.7.9-macosx10.9.pkg
      ;;
    3.8)
      echo https://www.python.org/ftp/python/3.8.6/python-3.8.6-macosx10.9.pkg
      ;;
    3.9)
      echo https://www.python.org/ftp/python/3.9.0/python-3.9.0rc2-macosx10.9.pkg
      ;;
    *)
      echo "Version $1 is not supported"
      return 1
      ;;
  esac
}

# Homebrew packages
brew install dos2unix ccache

# Python
# We must handle a performance tradeoff when handling the installation of
# Python. There are 2 easy ways to install Python on Mac OS: use pyenv or
# download the official installer.
# pyenv pro:
#   installation can be cached to speed up future builds
# pyenv con:
#   takes a long time to install initially because it builds Python from source
# official installer pro:
#   relatively quick to install initially
# official installer con:
#   installation cannot be cached for future builds
# When the cache is cold, it is quicker to use the official installer, and when
# the cache is warm, it is quicker to use pyenv. When the cache is cold, it is
# necessary to use pyenv to speed up future builds, but if we use pyenv
# exclusively, we will almost certainly hit the 50-minute time limit enforced by
# Travis CI. To account for this, we follow a simple rule: for each build,
# use pyenv to install the first instance of Python not found in the cache, then
# use the official installer for any remaining not-yet-installed instances. This
# way, we add one Python installation to the cache during every run until all
# installations are in the cache, all while keeping a reasonable runtime before
# all installations are in the cache.
PYVERSIONS="3.5.9 3.6.11 3.7.8 3.8.5 3.9.0b5"

# print list of Python versions available for installing using pyenv
pyenv install --list | paste -s -d '\0' -

if [ -d "$PYINSTALL_DIR" ]; then
  # delete unused Python versions from cache
  cd "$PYINSTALL_DIR"
  for PYVERSION in *; do
    [ -d "$PYVERSION" ] || continue
    if ! [[ "$PYVERSIONS" == *"$PYVERSION"* ]]; then
      rm -rf "$PYVERSION"
    fi
  done
  cd -
fi

PYENV_INSTALL_OCCURRED=0
PYENV_INSTALLED_VERSIONS=""
PYENV_INSTALLED_CMDS=""
PKG_INSTALLED_CMDS=""
for PYVERSION in $PYVERSIONS; do
  # extract Python version in MAJOR.MINOR form
  PYMAJORMINOR=$(echo "$PYVERSION" | awk -F '.' '{print $1"."$2}')
  if [ -d "$PYINSTALL_DIR/$PYVERSION" ]; then
    # found in pyenv cache
    echo "Python $PYVERSION found in cache"
    PYENV_INSTALLED_CMDS="$PYENV_INSTALLED_CMDS python$PYMAJORMINOR"
    PYENV_INSTALLED_VERSIONS="$PYENV_INSTALLED_VERSIONS $PYVERSION"
  elif [ "$PYENV_INSTALL_OCCURRED" = 0 ]; then
    # first pyenv install this run
    pyenv install "$PYVERSION"
    PYENV_INSTALL_OCCURRED=1
    PYENV_INSTALLED_CMDS="$PYENV_INSTALLED_CMDS python$PYMAJORMINOR"
    PYENV_INSTALLED_VERSIONS="$PYENV_INSTALLED_VERSIONS $PYVERSION"
  else
    # pyenv install already occurred this run; fall back to official installer
    curl -L $(pyinstall_fallback "$PYMAJORMINOR") -o pyinstaller.pkg
    sudo installer -pkg pyinstaller.pkg -target /
    PKG_INSTALLED_CMDS="$PKG_INSTALLED_CMDS python$PYMAJORMINOR"
  fi
done
pyenv global $PYENV_INSTALLED_VERSIONS

# Python packages
for PYTHON in $PYENV_INSTALLED_CMDS; do
  "$PYTHON" -m pip install -U pip
  "$PYTHON" -m pip install -U cython setuptools wheel --no-warn-script-location
done
for PYTHON in $PKG_INSTALLED_CMDS; do
  if ! [ -f get-pip.py ]; then
    curl -L -O https://bootstrap.pypa.io/get-pip.py
  fi
  "$PYTHON" get-pip.py
  "$PYTHON" -m pip install -U cython setuptools wheel --no-warn-script-location
done
