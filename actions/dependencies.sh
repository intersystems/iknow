# This file is used as a central location for managing build-time dependencies.
#
# Usage: actions/dependencies.sh

set -euxo pipefail


# If you add a variable to or remove a variable from the dependency-autoupdate
# section manually, you may also need to edit the EXPECTED_VARNAMES dictionary
# in actions/updatelib.py.

# START DEPENDENCY-AUTOUPDATE SECTION
ICU_NAME="ICU 71.1"
ICU_URL_WIN=https://github.com/unicode-org/icu/releases/download/release-71-1/icu4c-71_1-Win64-MSVC2019.zip
ICU_URL_SRC=https://github.com/unicode-org/icu/releases/download/release-71-1/icu4c-71_1-src.zip
JSON_VERSION=3.10.5
JSON_URL=https://github.com/nlohmann/json/releases/download/v3.10.5/include.zip
PYVERSIONS_WIN="3.6.8 3.7.9 3.8.10 3.9.13 3.10.4"
PYVERSIONS_OSX="3.6.15 3.7.13 3.8.13 3.9.12 3.10.4"
BUILDCACHE_NAME="Release v0.27.6"
BUILDCACHE_URL_WIN=https://github.com/mbitsnbites/buildcache/releases/download/v0.27.6/buildcache-windows.zip
CYTHON_VERSION=0.29.30
MANYLINUX2014_X86_64_TAG=2022-05-22-fbe07ea
MANYLINUX2014_AARCH64_TAG=2022-05-22-fbe07ea
MANYLINUX2014_PPC64LE_TAG=2022-05-22-fbe07ea
# END DEPENDENCY-AUTOUPDATE SECTION


# International Components for Unicode
if [ "$RUNNER_OS" = Windows ]; then
  # URL to .zip pre-built release of ICU for Windows x86_64
  echo "ICU_URL=$ICU_URL_WIN" >> $GITHUB_ENV
else
  # URL to .zip source release of ICU
  echo "ICU_URL=$ICU_URL_SRC" >> $GITHUB_ENV
fi

# JSON for Modern C++
echo "JSON_URL=$JSON_URL" >> $GITHUB_ENV

# Python versions
# Availability of certain versions can differ between NuGet and pyenv.
if [ "$RUNNER_OS" = Windows ]; then
  # space-delimited Python versions to install with NuGet, old to new
  echo "PYVERSIONS=$PYVERSIONS_WIN" >> $GITHUB_ENV
elif [ "$RUNNER_OS" = macOS ]; then
  # space-delimited Python versions to install with pyenv, old to new
  echo "PYVERSIONS=$PYVERSIONS_OSX" >> $GITHUB_ENV
fi

# buildcache
if [ "$RUNNER_OS" = Windows ]; then
  # URL to .zip release of buildcache
  echo "BUILDCACHE_URL=$BUILDCACHE_URL_WIN" >> $GITHUB_ENV
fi


# set variables that will be needed in later steps
echo "CYTHON_VERSION=$CYTHON_VERSION" >> $GITHUB_ENV
echo "MANYLINUX2014_X86_64_TAG=$MANYLINUX2014_X86_64_TAG" >> $GITHUB_ENV
echo "MANYLINUX2014_AARCH64_TAG=$MANYLINUX2014_AARCH64_TAG" >> $GITHUB_ENV
echo "MANYLINUX2014_PPC64LE_TAG=$MANYLINUX2014_PPC64LE_TAG" >> $GITHUB_ENV
