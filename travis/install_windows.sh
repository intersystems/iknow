#!/usr/bin/env bash

# Install build dependencies on Windows
#
# Required Environment Variables:
# - PYVERSIONS is a space-delimited string of Python versions to install with
#   nuget
# - PYINSTALL_DIR is the directory where Python instances are installed

set -euxo pipefail

# Visual Studio 2019
choco install visualstudio2019buildtools --limit-output --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --quiet"

# buildcache
wget -nv "$BUILDCACHE_URL"
unzip -qj buildcache-win-msvc.zip -d "$HOME"

# Python
choco install nuget.commandline --limit-output
if [ -d "$PYINSTALL_DIR" ]; then
  # delete unused Python versions from cache
  cd "$PYINSTALL_DIR"
  for PYINSTALLATION in *; do
    [ -d "$PYINSTALLATION" ] || continue
    if ! [[ "$PYVERSIONS" == *"${PYINSTALLATION#python.}"* ]]; then
      rm -rf "$PYINSTALLATION"
    fi
  done
  cd -
fi
for PYVERSION in $PYVERSIONS; do
  nuget.exe install python -Version "$PYVERSION" -NonInteractive -Verbosity quiet -OutputDirectory "$PYINSTALL_DIR"
done

# Python packages
for PYTHON in "$PYINSTALL_DIR"/python.*/tools/python.exe; do
  "$PYTHON" -m pip install -U pip
  "$PYTHON" -m pip install -U cython setuptools wheel --no-warn-script-location
done
