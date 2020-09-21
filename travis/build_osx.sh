#!/usr/bin/env bash

# Build wheels for Python 3.5 through Python 3.9, compatible with
# Mac OS X >= 10.9 x86_64. Upload the wheels to PyPI if appropriate. This
# script must be executed with the repository root as the working directory.
#
# Usage: travis/build_osx.sh
#
# Required Environment Variables:
# - ICU_SRC_URL is the URL to a .zip source release of ICU
#
# Optional Environment Variables:
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI

set -euxo pipefail


##### Build ICU #####
export REPO_ROOT=$(pwd)
curl -L -o icu4c-src.zip "$ICU_SRC_URL"
unzip -q icu4c-src.zip
cd icu/source
dos2unix -f *.m4 config.* configure* *.in install-sh mkinstalldirs runConfigureICU
export CXXFLAGS="-std=c++11"
export LDFLAGS="-headerpad_max_install_names"
export MACOSX_DEPLOYMENT_TARGET=10.9
export ICUDIR=$REPO_ROOT/thirdparty/icu
./runConfigureICU MacOSX --prefix="$ICUDIR"
make -j $(sysctl -n hw.logicalcpu)
make install


##### Build iKnow engine #####
export IKNOWPLAT=macx64
cd "$REPO_ROOT"
make -j $(sysctl -n hw.logicalcpu)


##### Build iknowpy wheels #####
cd modules/iknowpy
for PYTHON in python3.5 python3.6 python3.7 python3.8 python3.9; do
  "$PYTHON" setup.py bdist_wheel --plat-name=macosx-10.9-x86_64
done


##### Report cache statistics #####
ccache -s
