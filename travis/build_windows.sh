#!/usr/bin/env bash

# Build Windows x86_64 wheels for Python 3.5 through Python 3.8. Upload the
# wheels to PyPI if appropriate.
#
# Usage: ./build_windows.sh ICU_WIN_URL TOKEN
# - ICU_WIN_URL is the URL to a .zip pre-built release of ICU for Windows x86_64
# - TOKEN is an API token to the iknowpy repository on PyPI.

set -euxo pipefail
URL="$1"
{ set +x; } 2>/dev/null  # don't save token to build log
echo '+ TOKEN="$2"'
TOKEN="$2"
set -x
MSBUILD_PATH="/c/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin"
export PATH=$MSBUILD_PATH:$PATH


##### Install ICU #####
REPO_ROOT=$(pwd)
wget -O icu4c.zip "$URL"
export ICUDIR=$REPO_ROOT/thirdparty/icu
mkdir -p "$ICUDIR"
unzip icu4c.zip -d "$ICUDIR"


##### Build iKnow engine #####
cd modules
MSBuild.exe iKnowEngine.sln -p:Configuration="Release" -p:Platform="x64" -maxcpucount


##### Build iknowpy wheels #####
cd iknowpy
for PYTHON in /c/"Program Files"/Python3*/python.exe; do
  "$PYTHON" setup.py bdist_wheel
done


##### Upload iknowpy wheels if version was bumped #####
if [[ "$($REPO_ROOT/travis/deploy_check.sh)" == "1" ]]; then
  { set +x; } 2>/dev/null  # don't save token to build log
  echo '+ /c/"Program Files"/Python38/python.exe -m twine upload -u "__token__" -p "$TOKEN" dist/iknowpy-*.whl'
  /c/"Program Files"/Python38/python.exe -m twine upload -u "__token__" -p "$TOKEN" dist/iknowpy-*.whl
  set -x
else
  echo "Deployment skipped"
fi
