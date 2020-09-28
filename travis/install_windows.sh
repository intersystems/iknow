#!/usr/bin/env bash

# Install build dependencies on Windows

set -euxo pipefail

# Visual Studio 2019
choco install visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

# buildcache
wget -nv https://github.com/mbitsnbites/buildcache/releases/latest/download/buildcache-win-msvc.zip
unzip -qj buildcache-win-msvc.zip

# Python
choco install nuget.commandline
nuget.exe install python -Version 3.5.4
nuget.exe install python -Version 3.6.8
nuget.exe install python -Version 3.7.8
nuget.exe install python -Version 3.8.6
nuget.exe install python -Version 3.9.0-rc2

# Python packages
for PYTHON in python.*/tools/python.exe; do
  "$PYTHON" -m pip install -U pip
  "$PYTHON" -m pip install -U cython setuptools wheel --no-warn-script-location
done
