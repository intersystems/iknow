#!/usr/bin/env bash

# Build Windows x86_64 wheels for Python 3.6 through Python 3.11. This script
# must be executed with the repository root as the working directory.
#
# Usage: actions/build_windows.sh
#
# Required Environment Variables:
# - ICU_URL is the URL to a .zip pre-built release of ICU for Windows x86_64
# - ICUDIR is the directory to install ICU
# - JSON_URL is the URL of the C++ JSON project on Github
# - JSONDIR is the directory to install the JSON header
# - CCACHE_DIR is the directory where ccache stores its cache
# - PYINSTALL_DIR is the directory where Python instances are installed
# - JSON_INCLUDE is the directory containing JSON for Modern C++ headers
# - PYVERSIONS is a space-delimited string of Python versions to install with
#   NuGet

set -euxo pipefail


##### Install ICU if it's not cached #####
if ! [ -f "$ICUDIR/iknow_icu_url.txt" ] || [ $(cat "$ICUDIR/iknow_icu_url.txt") != "$ICU_URL" ]; then
  rm -rf "$ICUDIR"
  curl -L -o icu4c.zip "$ICU_URL"
  mkdir .icu_extract
  unzip -q icu4c.zip -d .icu_extract
  mkdir -p "$ICUDIR"
  if compgen -G ".icu_extract/*/icu-windows.zip" > /dev/null; then
    unzip -q .icu_extract/*/icu-windows.zip -d "$ICUDIR"
  else
    mv .icu_extract/* "$ICUDIR"
  fi
  echo "$ICU_URL" > "$ICUDIR/iknow_icu_url.txt"
fi

##### Build iKnow engine and run C++ unit tests #####
cd "$GITHUB_WORKSPACE/modules"
MSBUILD_PATH="/c/Program Files/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin"
PATH="$MSBUILD_PATH:$PATH" \
  MSBuild.exe iKnowEngine.sln -p:Configuration=Release -p:Platform=x64 \
    -maxcpucount \
    -p:ForceImportBeforeCppTargets="$(pwd)/EnableCcache.props" \
    -p:TrackFileAccess=false \
    -p:CLToolPath="$CCACHE_EXE_DIR" \
    -p:LinkToolPath="$CCACHE_EXE_DIR"
PATH="$ICUDIR/bin64:$PATH" ../kit/x64/Release/bin/iKnowEngineTest.exe


##### Build iknowpy wheels #####
cd iknowpy
for PYVERSION in $PYVERSIONS; do
  PYTHON="$PYINSTALL_DIR/python.$PYVERSION/tools/python.exe"
  "$PYTHON" setup.py bdist_wheel --no-dependencies
done
"$PYTHON" setup.py merge --no-dependencies
"$PYTHON" -m delvewheel repair dist/merged/iknowpy-*.whl --add-path "$ICUDIR/bin64;../../kit/x64/Release/bin"


##### Report cache statistics #####
"$CCACHE_EXE_DIR/ccache.exe" -s
