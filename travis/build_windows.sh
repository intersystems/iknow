#!/usr/bin/env bash

# Build Windows x86_64 wheels for Python 3.5 through Python 3.9. Upload the
# wheels to PyPI if appropriate. This script must be executed with the
# repository root as the working directory.
#
# Usage: travis/build_windows.sh
#
# Required Environment Variables:
# - ICU_WIN_URL is the URL to a .zip pre-built release of ICU for Windows x86_64
# - BUILDCACHE_DIR is the directory where buildcache stores its cache
#
# Optional Environment Variables:
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI

set -euxo pipefail
MSBUILD_PATH="/c/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin"
export PATH="$MSBUILD_PATH:$PATH"


##### Install ICU #####
export REPO_ROOT="$(pwd)"
wget -nv -O icu4c.zip "$ICU_WIN_URL"
export ICUDIR="$REPO_ROOT/thirdparty/icu"
mkdir -p "$ICUDIR"
unzip -q icu4c.zip -d "$ICUDIR"


##### Build iKnow engine #####
cd modules
BUILDCACHE_IMPERSONATE=cl.exe \
  MSBuild.exe iKnowEngine.sln -p:Configuration=Release -p:Platform=x64 \
    -maxcpucount \
    -p:ForceImportBeforeCppTargets="$(pwd)/EnableBuildCache.props" \
    -p:TrackFileAccess=false \
    -p:CLToolExe=buildcache.exe \
    -p:CLToolPath="$TRAVIS_BUILD_DIR"


##### Build iknowpy wheels #####
cd iknowpy
for PYTHON in /c/"Program Files"/Python3*/python.exe; do
  "$PYTHON" setup.py bdist_wheel
done


##### Report cache statistics #####
"$TRAVIS_BUILD_DIR/buildcache.exe" -s
