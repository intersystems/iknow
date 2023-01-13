#!/usr/bin/env bash

# Install build dependencies on Mac OS X
#
# Required environment variables:
#
# - XCODE_SELECTED is the path to the Xcode application to use
# - MACOSX_DEPLOYMENT_TARGET is the minimum supported Mac OS X version
# - GITHUB_EVENT_NAME is the event that triggered the workflow
# - GITHUB_REF is the branch ref that triggered the workflow
# - GITHUB_REPOSITORY is the name of the repository
# - CYTHON_VERSION is the version of Cython to install
# - JSON_URL is the URL of .zip release of JSON for Modern C++
# - JSONDIR is the directory to install JSON for Modern C++

set -euxo pipefail

# choose XCode version
sudo xcode-select -s "$XCODE_SELECTED"

# Homebrew packages
brew install ccache
echo /usr/local/opt/ccache/libexec >> $GITHUB_PATH

# Python packages
pip install -U cython=="$CYTHON_VERSION" setuptools wheel

# JSON for Modern C++
if ! [ -f "$JSONDIR/iknow_json_url.txt" ] || [ $(cat "$JSONDIR/iknow_json_url.txt") != "$JSON_URL" ]; then
    rm -rf "$JSONDIR"
    mkdir -p "$JSONDIR"
    curl -L -o json_for_modern_cpp.zip "$JSON_URL"
    unzip -q -d "$JSONDIR" json_for_modern_cpp.zip
    echo "$JSON_URL" > "$JSONDIR/iknow_json_url.txt"
fi
