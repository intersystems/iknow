#!/usr/bin/env bash

# Build Windows x86_64 wheels for Python 3.6 through Python 3.9. This script
# must be executed with the repository root as the working directory.
#
# Usage: actions/build_windows.sh
#
# Required Environment Variables:
# - ICU_URL is the URL to a .zip pre-built release of ICU for Windows x86_64
# - ICUDIR is the directory to install ICU
# - JSON_URL is the URL of the C++ JSON project on Github
# - JSONDIR is the directory to install the JSON header
# - BUILDCACHE_DIR is the directory where buildcache stores its cache
# - PYINSTALL_DIR is the directory where Python instances are installed

set -euxo pipefail


##### Install ICU if it's not cached #####
if ! [ -f "$ICUDIR/iknow_icu_url.txt" ] || [ $(cat "$ICUDIR/iknow_icu_url.txt") != "$ICU_URL" ]; then
  rm -rf "$ICUDIR"
  curl -L -o icu4c.zip "$ICU_URL"
  mkdir -p "$ICUDIR"
  unzip -q icu4c.zip -d "$ICUDIR"
  echo "$ICU_URL" > "$ICUDIR/iknow_icu_url.txt"
fi

##### Build JSON C++
if ! [ -f "$JSONDIR/iknow_json_url.txt" ] || [ $(cat "$JSONDIR/iknow_json_url.txt") != "$JSON_URL" ]; then
    rm -rf "$JSONDIR"
    cd "$GITHUB_WORKSPACE/thirdparty"
    git clone "$JSON_URL"
    cd json
    git checkout v3.10.2
    echo "$JSON_URL" > "$JSONDIR/iknow_json_url.txt"
fi
export JSON_INCLUDE="$JSONDIR/single_include"

##### Build iKnow engine and run C++ unit tests #####
cd "$GITHUB_WORKSPACE/modules"
MSBUILD_PATH="/c/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/MSBuild/Current/Bin"
BUILDCACHE_IMPERSONATE=cl.exe PATH="$MSBUILD_PATH:$PATH" \
  MSBuild.exe iKnowEngine.sln -p:Configuration=Release -p:Platform=x64 \
    -maxcpucount \
    -p:ForceImportBeforeCppTargets="$(pwd)/EnableBuildCache.props" \
    -p:TrackFileAccess=false \
    -p:CLToolExe=buildcache.exe \
    -p:CLToolPath="$BUILDCACHE_EXE_DIR"
PATH="$ICUDIR/bin64:$PATH" ../kit/x64/Release/bin/iKnowEngineTest.exe


##### Build iknowpy wheels #####
cd iknowpy
for PYTHON in "$PYINSTALL_DIR"/python.*/tools/python.exe; do
  "$PYTHON" setup.py bdist_wheel --no-dependencies
done
"$PYTHON" setup.py merge --no-dependencies
"$PYTHON" -m delvewheel repair dist/merged/iknowpy-*.whl --add-path "$ICUDIR/bin64;../../kit/x64/Release/bin"


##### Report cache statistics #####
"$BUILDCACHE_EXE_DIR/buildcache.exe" -s
