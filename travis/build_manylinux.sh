#!/usr/bin/env bash

# Build manylinux wheels for Python 3.5 through Python 3.8. Upload the wheels to
# PyPI if appropriate. This script must be executed inside a manylinux
# container.
#
# Usage: ./build_manylinux.sh TAG ICU_SRC_URL TOKEN
# - TAG is the manylinux platform tag. Supported tags are
#     manylinux2010_x86_64
#     manylinux2010_i686
#     manylinux2014_x86_64
#     manylinux2014_i686
#     manylinux2014_aarch64
#     manylinux2014_ppc64le
# - ICU_SRC_URL is the URL to a .zip source release of ICU
# - TOKEN is an API token to the iknowpy repository on PyPI.

set -euxo pipefail
TAG="$1"
URL="$2"
{ set +x; } 2>/dev/null  # don't save token to build log
echo '+ TOKEN="$3"'
TOKEN="$3"
set -x

SUPPORTEDTAGS="manylinux2010_x86_64 manylinux2010_i686 manylinux2014_x86_64 manylinux2014_i686 manylinux2014_aarch64 manylinux2014_ppc64le"

if [[ " $SUPPORTEDTAGS " != *" $TAG "* ]]; then
  echo "Tag \"$TAG\" is not supported"
  exit 1
fi


##### Install dependencies #####
# dos2unix is needed to give ICU build scripts Unix line endings so that they
# can be executed. For some reason, ICU source releases use Windows line
# endings. On some platforms, openssl-devel is needed to install twine.
yum install -y dos2unix openssl-devel


##### Build ICU #####
curl -L -o icu4c-src.zip "$URL"
unzip icu4c-src.zip
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
  if [[ "$PYTHON" == *"/cp38-cp38/"* ]]; then
    PACKAGES="$PACKAGES twine"
  fi
  "$PYTHON" -m pip install -U $PACKAGES
  "$PYTHON" setup.py bdist_wheel --no-dependencies
done

# aarch64 and ppc64le platforms often have a large page size of 64KiB. We need
# to redirect all invocations of patchelf so that when auditwheel invokes it, it
# produces ELFs with the proper page alignment.
if [[ "$TAG" == *"_aarch64" || "$TAG" == *"_ppc64le" ]]; then
  mv /usr/local/bin/patchelf /usr/local/bin/_patchelf
  printf '#!/usr/bin/env bash\n\n_patchelf --page-size 65536 "$@"\n' >> /usr/local/bin/patchelf
  chmod +x /usr/local/bin/patchelf
fi

# repair wheels using auditwheel
for WHEEL in dist/iknowpy-*.whl; do
  auditwheel repair -w dist2 $WHEEL
done

# restore patchelf on aarch64 and ppc64le
if [[ "$TAG" == *"_aarch64" || "$TAG" == *"_ppc64le" ]]; then
  rm -f /usr/local/bin/patchelf
  mv /usr/local/bin/_patchelf /usr/local/bin/patchelf
fi


##### Upload iknowpy wheels if version was bumped #####
if [[ "$(/iknow/travis/deploy_check.sh)" == "1" ]]; then
  { set +x; } 2>/dev/null  # don't save token to build log
  echo '+ /opt/python/cp38-cp38/bin/python -m twine upload -u "__token__" -p "$TOKEN" dist2/iknowpy-*manylinux*.whl'
  /opt/python/cp38-cp38/bin/python -m twine upload -u "__token__" -p "$TOKEN" dist2/iknowpy-*manylinux*.whl
  set -x
else
  echo "Deployment skipped"
fi
