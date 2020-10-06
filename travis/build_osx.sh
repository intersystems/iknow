#!/usr/bin/env bash

# Build wheels for Python 3.5 through Python 3.9, compatible with
# Mac OS X >= 10.9 x86_64. Upload the wheels to PyPI if appropriate. This
# script must be executed with the repository root as the working directory.
#
# Usage: travis/build_osx.sh
#
# Required Environment Variables:
# - ICU_URL is the URL to a .zip source release of ICU
# - ICUDIR is the directory to install ICU
# - MACOSX_DEPLOYMENT_TARGET is the minimum supported Mac OS X version
#
# Optional Environment Variables:
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI

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


##### Build iKnow engine #####
export IKNOWPLAT=macx64
cd "$TRAVIS_BUILD_DIR"
make -j $(sysctl -n hw.logicalcpu)


##### Build iknowpy wheels #####
cd modules/iknowpy
for PYVERSION in $PYVERSIONS; do
  PYMAJORMINOR=$(echo "$PYVERSION" | awk -F '.' '{print $1"."$2}')
  python$PYMAJORMINOR setup.py bdist_wheel --plat-name=macosx-10.9-x86_64
done


##### Report cache statistics #####
ccache -s
