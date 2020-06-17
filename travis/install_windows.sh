#!/usr/bin/env bash

# Install build dependencies on Windows

set -euxo pipefail

# Visual Studio 2019
choco install visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

# Python
wget https://www.python.org/ftp/python/3.5.4/python-3.5.4-amd64-webinstall.exe
wget https://www.python.org/ftp/python/3.6.8/python-3.6.8-amd64-webinstall.exe
wget https://www.python.org/ftp/python/3.7.7/python-3.7.7-amd64-webinstall.exe
wget https://www.python.org/ftp/python/3.8.3/python-3.8.3-amd64-webinstall.exe

for PYINSTALL in python-*.exe; do
  "./$PYINSTALL" -quiet InstallAllUsers=1 Shortcuts=0 Include_doc=0 Include_launcher=0 Include_tcltk=0 Include_test=0
done

# Python packages
for PYTHON in /c/"Program Files"/Python3*/python.exe; do
  "$PYTHON" -m pip install -U pip
  PACKAGES="cython setuptools wheel"
  if [[ "$PYTHON" == *"Python38"* ]]; then
    PACKAGES="$PACKAGES twine"
  fi
  "$PYTHON" -m pip install -U $PACKAGES --no-warn-script-location
done
