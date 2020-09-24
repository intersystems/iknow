#!/usr/bin/env bash

# Install build dependencies on Windows

set -euxo pipefail

# Visual Studio 2019
choco install visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

# buildcache
wget -nv https://github.com/mbitsnbites/buildcache/releases/latest/download/buildcache-win-msvc.zip
unzip -qj buildcache-win-msvc.zip

# Python
wget -nv https://www.python.org/ftp/python/3.5.4/python-3.5.4-amd64-webinstall.exe
wget -nv https://www.python.org/ftp/python/3.6.8/python-3.6.8-amd64-webinstall.exe
wget -nv https://www.python.org/ftp/python/3.7.8/python-3.7.8-amd64-webinstall.exe
wget -nv https://www.python.org/ftp/python/3.8.6/python-3.8.6-amd64-webinstall.exe
wget -nv https://www.python.org/ftp/python/3.9.0/python-3.9.0rc2-amd64-webinstall.exe

for PYINSTALL in python-*.exe; do
  "./$PYINSTALL" -quiet InstallAllUsers=1 Shortcuts=0 Include_doc=0 Include_launcher=0 Include_tcltk=0 Include_test=0
done

# Python packages
for PYTHON in /c/"Program Files"/Python3*/python.exe; do
  "$PYTHON" -m pip install -U pip
  "$PYTHON" -m pip install -U cython setuptools wheel --no-warn-script-location
done
