#!/usr/bin/env bash

# Install build dependencies on Mac OS X

set -euxo pipefail

# Homebrew packages
brew install dos2unix

# download Python
curl -L -O https://www.python.org/ftp/python/3.5.4/python-3.5.4-macosx10.6.pkg
curl -L -O https://www.python.org/ftp/python/3.6.8/python-3.6.8-macosx10.9.pkg
curl -L -O https://www.python.org/ftp/python/3.7.8/python-3.7.8-macosx10.9.pkg
curl -L -O https://www.python.org/ftp/python/3.8.5/python-3.8.5-macosx10.9.pkg
curl -L -O https://www.python.org/ftp/python/3.9.0/python-3.9.0rc2-macosx10.9.pkg

# install Python
for PACKAGE in python-*.pkg; do
  sudo installer -pkg "$PACKAGE" -target /
done

# install Python packages
curl -L -O https://bootstrap.pypa.io/get-pip.py
for PYTHON in python3.5 python3.6 python3.7 python3.8 python3.9; do
  "$PYTHON" get-pip.py
  PACKAGES="cython setuptools wheel"
  if [[ "$PYTHON" == "python3.9" ]]; then
    PACKAGES="$PACKAGES twine"
  fi
  "$PYTHON" -m pip install -U $PACKAGES --no-warn-script-location
done
