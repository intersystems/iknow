#!/usr/bin/env bash

# Build wheels for Python 3.5 through Python 3.8, compatible with
# Mac OS X >= 10.9 x86_64. Upload the wheels to PyPI if appropriate.
#
# Usage: ./build_osx.sh ICU_SRC_URL PYPI_TOKEN TESTPYPI_TOKEN
# - ICU_SRC_URL is the URL to a .zip source release of ICU
# - PYPI_TOKEN is an API token to the iknowpy repository on PyPI
# - TESTPYPI_TOKEN is an API token to the iknowpy repository on TestPyPI

set -euxo pipefail
URL="$1"
{ set +x; } 2>/dev/null  # don't save token to build log
echo '+ PYPI_TOKEN="$2"'
PYPI_TOKEN="$2"
echo '+ TESTPYPI_TOKEN="$3"'
TESTPYPI_TOKEN="$3"
set -x


##### Build ICU #####
export REPO_ROOT=$(pwd)
curl -L -o icu4c-src.zip "$URL"
unzip icu4c-src.zip
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
for PYTHON in python3.5 python3.6 python3.7 python3.8; do
  "$PYTHON" setup.py bdist_wheel --plat-name=macosx-10.9-x86_64
done


##### Upload iknowpy wheels if appropriate #####
DEPLOY=$($REPO_ROOT/travis/deploy_check.sh)
if [[ "$DEPLOY" == "0" ]]; then
  echo "Deployment skipped"
else
  if [[ "$DEPLOY" == "PyPI" ]]; then
    export TWINE_REPOSITORY=pypi
    { set +x; } 2>/dev/null  # don't save token to build log
    echo '+ TOKEN="$PYPI_TOKEN"'
    TOKEN="$PYPI_TOKEN"
    set -x
  else
    export TWINE_REPOSITORY=testpypi
    { set +x; } 2>/dev/null  # don't save token to build log
    echo '+ TOKEN="$TESTPYPI_TOKEN"'
    TOKEN="$TESTPYPI_TOKEN"
    set -x
  fi
  { set +x; } 2>/dev/null  # don't save token to build log
  echo '+ python3.8 -m twine upload -u "__token__" -p "$TOKEN" dist/iknowpy-*.whl'
  python3.8 -m twine upload -u "__token__" -p "$TOKEN" dist/iknowpy-*.whl
  set -x
fi
