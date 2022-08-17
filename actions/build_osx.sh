#!/usr/bin/env bash

# Build wheels for Python 3.6 through Python 3.11, compatible with
# Mac OS X >= 10.9 x86_64. This script must be executed with the repository root
# as the working directory.
#
# Usage: actions/build_osx.sh
#
# Required Environment Variables:
# - ICU_URL is the URL to a .tgz source release of ICU
# - ICUDIR is the directory to install ICU
# - MACOSX_DEPLOYMENT_TARGET is the minimum supported Mac OS X version
# - JSON_INCLUDE is the directory containing JSON for Modern C++ headers

set -euxo pipefail


##### Build ICU if it's not cached #####
if ! [ -f "$ICUDIR/iknow_icu_url.txt" ] || [ $(cat "$ICUDIR/iknow_icu_url.txt") != "$ICU_URL" ]; then
  rm -rf "$ICUDIR"
  curl -L -o icu4c-src.tgz "$ICU_URL"
  tar xfz icu4c-src.tgz
  cd icu/source
  export CXXFLAGS="-std=c++11"
  export LDFLAGS="-headerpad_max_install_names"
  ./runConfigureICU MacOSX --prefix="$ICUDIR"
  make -j $(sysctl -n hw.logicalcpu)
  make install
  echo "$ICU_URL" > "$ICUDIR/iknow_icu_url.txt"
  unset CXXFLAGS LDFLAGS
fi


##### Build iKnow engine and run C++ unit tests #####
export IKNOWPLAT=macx64
cd "$GITHUB_WORKSPACE"
make -j $(sysctl -n hw.logicalcpu) test


##### Build iknowpy wheels #####
cd modules/iknowpy
eval "$(pyenv init -)"
for PYTHON in python3.{6..11}; do
  "$PYTHON" setup.py bdist_wheel --plat-name=macosx-$MACOSX_DEPLOYMENT_TARGET-x86_64 --no-dependencies
done
"$PYTHON" setup.py merge
rm -r dist/cache


##### Report cache statistics #####
ccache -s
