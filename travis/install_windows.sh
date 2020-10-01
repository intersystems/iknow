#!/usr/bin/env bash

# Install build dependencies on Windows

set -euxo pipefail

# Visual Studio 2019
choco install visualstudio2019buildtools --limit-output --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --quiet"

# buildcache
wget -nv https://github.com/mbitsnbites/buildcache/releases/latest/download/buildcache-win-msvc.zip
unzip -qj buildcache-win-msvc.zip

# Python
choco install nuget.commandline --limit-output 
for PYVERSION in 3.5.4 3.6.8 3.7.9 3.8.6 3.9.0-rc2; do
  nuget.exe install python -Version "$PYVERSION" -NonInteractive -Verbosity quiet
done

# Python packages
for PYTHON in python.*/tools/python.exe; do
  "$PYTHON" -m pip install -U pip
  "$PYTHON" -m pip install -U cython setuptools wheel --no-warn-script-location
done
