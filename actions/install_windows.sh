#!/usr/bin/env bash

# Install build dependencies on Windows
#
# Required Environment Variables:
# - PYVERSIONS is a space-delimited string of Python versions to install with
#   NuGet
# - PYINSTALL_DIR is the directory where Python instances are installed
# - CYTHON_VERSION is the version of Cython to install
# - JSON_URL is the URL of .zip release of JSON for Modern C++
# - JSONDIR is the directory to install JSON for Modern C++
# - CCACHE_URL is the URL to a .zip release of ccache
# - CCACHE_EXE_DIR is the directory containing ccache versions of cl.exe and
#   link.exe

set -euxo pipefail

# ccache
if ! [ -f "$CCACHE_EXE_DIR/iknow_cache_url.txt" ] || [ $(cat "$CCACHE_EXE_DIR/iknow_ccache_url.txt") != "$CCACHE_URL" ]; then
  rm -rf "$CCACHE_EXE_DIR"
  curl -L -o ccache.zip "$CCACHE_URL"
  unzip -qj ccache.zip -d "$CCACHE_EXE_DIR"
  cp "$CCACHE_EXE_DIR/ccache.exe" "$CCACHE_EXE_DIR/cl.exe"
  cp "$CCACHE_EXE_DIR/ccache.exe" "$CCACHE_EXE_DIR/link.exe"
  echo "$CCACHE_URL" > "$CCACHE_EXE_DIR/iknow_ccache_url.txt"
fi

# Python
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
for PYVERSION in $PYVERSIONS; do
  PYTHON="$PYINSTALL_DIR/python.$PYVERSION/tools/python.exe"
  "$PYTHON" -m pip install -U pip --no-warn-script-location
  "$PYTHON" -m pip install -U setuptools wheel==0.37.1 --no-warn-script-location
  "$PYTHON" -m pip install -U cython=="$CYTHON_VERSION" pefile machomachomangler --no-warn-script-location
done
"$PYTHON" -m pip install -U delvewheel

# JSON for Modern C++
if ! [ -f "$JSONDIR/iknow_json_url.txt" ] || [ $(cat "$JSONDIR/iknow_json_url.txt") != "$JSON_URL" ]; then
    rm -rf "$JSONDIR"
    mkdir -p "$JSONDIR"
    curl -L -o json_for_modern_cpp.zip "$JSON_URL"
    unzip -q -d "$JSONDIR" json_for_modern_cpp.zip
    echo "$JSON_URL" > "$JSONDIR/iknow_json_url.txt"
fi
