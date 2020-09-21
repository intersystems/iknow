#!/usr/bin/env bash

# Build manylinux wheels for Python 3.5 through Python 3.9. This script must be
# executed inside a manylinux container in which /iknow is the root of the
# repository.
#
# Usage: /iknow/travis/build_manylinux.sh ICU_SRC_URL
# - ICU_SRC_URL is the URL to a .zip source release of ICU

set -euxo pipefail
URL="$1"


##### Install dependencies #####
# epel-release
#   Needed on some platforms to install ccache.
# dos2unix
#   Give ICU build scripts Unix line endings so that they can be executed. For
#   some reason, ICU source releases use Windows line endings.
# ccache
#   Speed up build times by caching results from previous builds.
yum install -y epel-release
yum install -y dos2unix ccache
mkdir -p /opt/ccache
ln -s /usr/bin/ccache /opt/ccache/cc
ln -s /usr/bin/ccache /opt/ccache/c++
ln -s /usr/bin/ccache /opt/ccache/gcc
ln -s /usr/bin/ccache /opt/ccache/g++
export PATH="/opt/ccache:$PATH"


##### Build ICU #####
curl -L -o icu4c-src.zip "$URL"
unzip -q icu4c-src.zip
cd icu/source

dos2unix -f *.m4 config.* configure* *.in install-sh mkinstalldirs runConfigureICU
export CXXFLAGS="-std=c++11"
export ICUDIR=/iknow/thirdparty/icu
PYTHON=/opt/python/cp39-cp39/bin/python ./runConfigureICU Linux --prefix="$ICUDIR"
gmake -j $(nproc)
gmake install


##### Build iKnow engine #####
cd /iknow

case $(uname -p) in
  x86_64)
    export IKNOWPLAT=lnxrhx64
    ;;
  i686)
    export IKNOWPLAT=lnxrhx86
    ;;
  aarch64)
    export IKNOWPLAT=lnxrharm64
    ;;
  ppc64le)
    export IKNOWPLAT=lnxrhppc64le
    ;;
  *)
    echo "Processor type $(uname -p) is not supported"
    exit 1
    ;;
esac

make -j $(nproc)


##### Build iknowpy wheels #####
cd modules/iknowpy
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/iknow/kit/$IKNOWPLAT/release/bin:$ICUDIR/lib

# install Python package dependencies and build initial wheels
for PYTHON in /opt/python/cp3*/bin/python; do
  PACKAGES="cython setuptools wheel"
  "$PYTHON" -m pip install --user --no-warn-script-location $PACKAGES
  "$PYTHON" setup.py bdist_wheel --no-dependencies
done

# repair wheels using auditwheel to convert to manylinux wheels
for WHEEL in dist/iknowpy-*.whl; do
  auditwheel repair -w dist2 $WHEEL
done


##### Report cache statistics #####
ccache -s
