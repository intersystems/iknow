#!/usr/bin/env bash

# Build manylinux wheels for Python 3.5 through Python 3.9. This script must be
# executed inside a manylinux container in which /iknow is the root of the
# repository.
#
# Usage: /iknow/.github/workflows/build_manylinux.sh
#
# Required Environment Variables:
# - CCACHE_MAXSIZE is the size limit for files held with ccache
# - PIP_CACHE_DIR is the location that pip caches files
# - ICU_URL is the URL to a .zip source release of ICU

set -euxo pipefail


##### Install and configure dependencies #####
# epel-release
#   Needed on some platforms to install ccache.
# dos2unix
#   Give ICU build scripts Unix line endings so that they can be executed. For
#   some reason, ICU source releases use Windows line endings.
# ccache
#   Speed up build times by caching results from previous builds.
PROCESSOR="$(uname -p)"
if [ "$PROCESSOR" = aarch64 ] || [ "$PROCESSOR" = ppc64le ]; then
  yum install -y epel-release
fi
if [ "$PROCESSOR" = aarch64 ] || [ "$PROCESSOR" = ppc64le ]; then
  # this mirror is often slow, so disable it
  echo "exclude=csc.mcs.sdsmt.edu" >> /etc/yum/pluginconf.d/fastestmirror.conf
fi
yum install -y dos2unix ccache
mkdir -p /opt/ccache
ln -s /usr/bin/ccache /opt/ccache/cc
ln -s /usr/bin/ccache /opt/ccache/c++
ln -s /usr/bin/ccache /opt/ccache/gcc
ln -s /usr/bin/ccache /opt/ccache/g++
export PATH="/opt/ccache:$PATH"
if [ "$PROCESSOR" = x86_64 ]; then
  # On manylinux2010_x86_64, the version of ccache is too old to recognize the
  # CCACHE_MAXSIZE environment variable, so set the max cache size manually.
  ccache --max-size "$CCACHE_MAXSIZE"
fi


##### Build ICU if it's not cached #####
export ICUDIR=/iknow/thirdparty/icu
if ! [ -f "$ICUDIR/iknow_icu_url.txt" ] || [ $(cat "$ICUDIR/iknow_icu_url.txt") != "$ICU_URL" ]; then
  rm -rf "$ICUDIR"
  curl -L -o icu4c-src.zip "$ICU_URL"
  unzip -q icu4c-src.zip
  cd icu/source

  dos2unix -f *.m4 config.* configure* *.in install-sh mkinstalldirs runConfigureICU
  export CXXFLAGS="-std=c++11"
  PYTHON=/opt/python/cp39-cp39/bin/python ./runConfigureICU Linux --prefix="$ICUDIR"
  gmake -j $(nproc)
  gmake install
  echo "$ICU_URL" > "$ICUDIR/iknow_icu_url.txt"
fi


##### Build iKnow engine #####
cd /iknow

case "$PROCESSOR" in
  x86_64)
    export IKNOWPLAT=lnxrhx64
    ;;
  aarch64)
    export IKNOWPLAT=lnxrharm64
    ;;
  ppc64le)
    export IKNOWPLAT=lnxrhppc64le
    ;;
  *)
    echo "Processor type $PROCESSOR is not supported"
    exit 1
    ;;
esac

make -j $(nproc)


##### Build iknowpy wheels #####
cd modules/iknowpy
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/iknow/kit/$IKNOWPLAT/release/bin:$ICUDIR/lib"

# install Python package dependencies and build initial wheels
chown -R root "$PIP_CACHE_DIR"
for PYTHON in /opt/python/{cp35-cp35m,cp36-cp36m,cp37-cp37m,cp38-cp38,cp39-cp39}/bin/python
do
  "$PYTHON" -m pip install --user cython=="$CYTHON_VERSION" setuptools wheel --no-warn-script-location
  "$PYTHON" setup.py bdist_wheel --no-dependencies
done
"$PYTHON" setup.py merge --no-dependencies
chmod -R a+rw "$PIP_CACHE_DIR"

# repair wheel using auditwheel to convert to manylinux wheel
auditwheel repair dist/merged/iknowpy-*.whl


##### Report cache statistics #####
ccache -s
