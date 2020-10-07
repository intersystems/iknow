#!/usr/bin/env bash

# Build Windows x86_64 wheels for Python 3.5 through Python 3.9. Upload the
# wheels to PyPI if appropriate. This script must be executed with the
# repository root as the working directory.
#
# Usage: travis/build_windows.sh
#
# Required Environment Variables:
# - ICU_URL is the URL to a .zip pre-built release of ICU for Windows x86_64
# - ICUDIR is the directory to install ICU
# - BUILDCACHE_DIR is the directory where buildcache stores its cache
# - PYINSTALL_DIR is the directory where Python instances are installed
#
# Optional Environment Variables:
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI

set -euxo pipefail


##### Install ICU if it's not cached #####
if ! [ -f "$ICUDIR/iknow_icu_url.txt" ] || [ $(cat "$ICUDIR/iknow_icu_url.txt") != "$ICU_URL" ]; then
  rm -rf "$ICUDIR"
  wget -nv -O icu4c.zip "$ICU_URL"
  mkdir -p "$ICUDIR"
  unzip -q icu4c.zip -d "$ICUDIR"
  echo "$ICU_URL" > "$ICUDIR/iknow_icu_url.txt"
fi


##### Build iKnow engine #####
cd modules
MSBUILD_PATH="/c/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin"
BUILDCACHE_IMPERSONATE=cl.exe PATH="$MSBUILD_PATH:$PATH" \
  MSBuild.exe iKnowEngine.sln -p:Configuration=Release -p:Platform=x64 \
    -maxcpucount \
    -p:ForceImportBeforeCppTargets="$(pwd)/EnableBuildCache.props" \
    -p:TrackFileAccess=false \
    -p:CLToolExe=buildcache.exe \
    -p:CLToolPath="$BUILDCACHE_EXE_DIR"


##### Build iknowpy wheels #####
cd iknowpy
for PYTHON in "$PYINSTALL_DIR"/python.*/tools/python.exe; do
  "$PYTHON" setup.py bdist_wheel
done


##### Report cache statistics #####
"$BUILDCACHE_EXE_DIR/buildcache.exe" -s
