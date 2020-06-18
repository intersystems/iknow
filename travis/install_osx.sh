#!/usr/bin/env bash

# Install build dependencies on Mac OS X

set -euxo pipefail


# Given a string of PIDs of background jobs, wait for all of them to terminate.
wait_all () {
  local PID
  for PID in $1; do
    wait "$PID"
  done
}


# Homebrew packages
brew install dos2unix &
PIDS="$!"

# download Python
curl -L -s -O https://www.python.org/ftp/python/3.5.4/python-3.5.4-macosx10.6.pkg &
PIDS="$PIDS $!"
curl -L -s -O https://www.python.org/ftp/python/3.6.8/python-3.6.8-macosx10.9.pkg &
PIDS="$PIDS $!"
curl -L -s -O https://www.python.org/ftp/python/3.7.7/python-3.7.7-macosx10.9.pkg &
PIDS="$PIDS $!"
curl -L -s -O https://www.python.org/ftp/python/3.8.3/python-3.8.3-macosx10.9.pkg &
PIDS="$PIDS $!"
wait_all "$PIDS"

# install Python
PIDS=""
for PACKAGE in python-*.pkg; do
  sudo installer -pkg "$PACKAGE" -target / &
  PIDS="$PIDS $!"
done
wait_all "$PIDS"

# install Python packages
PIDS=""
for PYTHON in python3.5 python3.6 python3.7 python3.8; do
  (
    curl -s https://bootstrap.pypa.io/get-pip.py | "$PYTHON"
    PACKAGES="cython setuptools wheel"
    if [[ "$PYTHON" == "python3.8" ]]; then
      PACKAGES="$PACKAGES twine"
    fi
    "$PYTHON" -m pip install -U $PACKAGES --no-warn-script-location
  ) &
done
wait_all "$PIDS"
