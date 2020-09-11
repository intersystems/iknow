#!/usr/bin/env bash

# Build manylinux wheels for Python 3.5 through Python 3.9. This script must be
# executed inside a manylinux container in which /iknow is the root of the
# repository.
#
# Usage: /iknow/travis/build_manylinux.sh TAG ICU_SRC_URL PYPI_TOKEN TESTPYPI_TOKEN
# - TAG is the manylinux platform tag. Supported tags are
#     manylinux2010_x86_64
#     manylinux2010_i686
#     manylinux2014_x86_64
#     manylinux2014_i686
#     manylinux2014_aarch64
#     manylinux2014_ppc64le
# - ICU_SRC_URL is the URL to a .zip source release of ICU
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI

set -euxo pipefail
TAG="$1"
URL="$2"
{ set +x; } 2>/dev/null  # don't save token to build log
echo '+ PYPI_TOKEN="$3"'
PYPI_TOKEN="$3"
echo '+ TESTPYPI_TOKEN="$4"'
TESTPYPI_TOKEN="$4"
set -x

SUPPORTEDTAGS="manylinux2010_x86_64 manylinux2010_i686 manylinux2014_x86_64 manylinux2014_i686 manylinux2014_aarch64 manylinux2014_ppc64le"

if [[ " $SUPPORTEDTAGS " != *" $TAG "* ]]; then
  echo "Tag \"$TAG\" is not supported"
  exit 1
fi


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

# ICU build environment requires that /usr/bin/python be at least version 2.7.
# manylinux2010 images have version 2.6, so create symlink to version 2.7 binary
if [[ "$TAG" == "manylinux2010_"* ]]; then
  mv /usr/bin/python /usr/bin/_python
  ln -s /opt/python/cp27-cp27m/bin/python /usr/bin/python
fi

dos2unix -f *.m4 config.* configure* *.in install-sh mkinstalldirs runConfigureICU
export CXXFLAGS="-std=c++11"
export ICUDIR=/iknow/thirdparty/icu
./runConfigureICU Linux --prefix="$ICUDIR"
gmake -j $(nproc)
gmake install

# restore system Python on manylinux2010
if [[ "$TAG" == "manylinux2010_"* ]]; then
  rm -f /usr/bin/python
  mv /usr/bin/_python /usr/bin/python
fi


##### Build iKnow engine #####
cd /iknow

if [[ "$TAG" == *"_x86_64" ]]; then
  export IKNOWPLAT=lnxrhx64
elif [[ "$TAG" == *"_i686" ]]; then
  export IKNOWPLAT=lnxrhx86
elif [[ "$TAG" == *"_aarch64" ]]; then
  export IKNOWPLAT=lnxrharm64
elif [[ "$TAG" == *"_ppc64le" ]]; then
  export IKNOWPLAT=lnxrhppc64le
else
  echo "Unknown platform"
  exit 1
fi

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
