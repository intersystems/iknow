#!/usr/bin/env bash

# Build wheels for Python 3.6 through Python 3.9, compatible with
# Mac OS X >= 10.9 x86_64. This script must be executed with the repository root
# as the working directory.
#
# Usage: actions/build_osx.sh
#
# Required Environment Variables:
# - ICU_URL is the URL to a .zip source release of ICU
# - ICUDIR is the directory to install ICU
# - JSON_URL is the URL of the C++ JSON project on Github
# - JSONDIR is the directory to install the JSON header
# - MACOSX_DEPLOYMENT_TARGET is the minimum supported Mac OS X version

set -euxo pipefail


##### Build ICU if it's not cached #####
if ! [ -f "$ICUDIR/iknow_icu_url.txt" ] || [ $(cat "$ICUDIR/iknow_icu_url.txt") != "$ICU_URL" ]; then
  rm -rf "$ICUDIR"
  curl -L -o icu4c-src.zip "$ICU_URL"
  unzip -q icu4c-src.zip
  cd icu/source
  dos2unix -f *.m4 config.* configure* *.in install-sh mkinstalldirs runConfigureICU
  export CXXFLAGS="-std=c++11"
  export LDFLAGS="-headerpad_max_install_names"
  ./runConfigureICU MacOSX --prefix="$ICUDIR"
  make -j $(sysctl -n hw.logicalcpu)
  make install
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
export IKNOWPLAT=macx64
cd "$GITHUB_WORKSPACE"
make -j $(sysctl -n hw.logicalcpu) test


##### Build iknowpy wheels #####
cd modules/iknowpy
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
for PYTHON in python3.{6..9}; do
  "$PYTHON" setup.py bdist_wheel --plat-name=macosx-10.9-x86_64 --no-dependencies
done
"$PYTHON" setup.py merge
rm -r dist/cache


##### Report cache statistics #####
ccache -s
