# This file is used as a central location for managing build-time dependencies.
#
# Usage: actions/dependencies.sh

set -euxo pipefail


# If you edit the dependency-autoupdate section manually, you may also need to
# change actions/updatelib.py.

# START DEPENDENCY-AUTOUPDATE SECTION
ICU_NAME="ICU 69.1"
ICU_URL_WIN=https://github.com/unicode-org/icu/releases/download/release-69-1/icu4c-69_1-Win64-MSVC2019.zip
ICU_URL_SRC=https://github.com/unicode-org/icu/releases/download/release-69-1/icu4c-69_1-src.zip
PYVERSIONS_WIN="3.6.8 3.7.9 3.8.10 3.9.6"
PYVERSIONS_OSX="3.6.14 3.7.11 3.8.11 3.9.6"
BUILDCACHE_NAME="Release v0.27.1"
BUILDCACHE_URL_WIN=https://github.com/mbitsnbites/buildcache/releases/download/v0.27.1/buildcache-windows.zip
CYTHON_VERSION=0.29.24
MANYLINUX2010_X86_64_TAG=2021-07-04-1e3ce39
MANYLINUX2014_AARCH64_TAG=2021-07-04-1e3ce39
MANYLINUX2014_PPC64LE_TAG=2021-07-04-1e3ce39
# END DEPENDENCY-AUTOUPDATE SECTION


# International Components for Unicode
if [ "$RUNNER_OS" = Windows ]; then
  # URL to .zip pre-built release of ICU for Windows x86_64
  echo "ICU_URL=$ICU_URL_WIN" >> $GITHUB_ENV
else
  # URL to .zip source release of ICU
  echo "ICU_URL=$ICU_URL_SRC" >> $GITHUB_ENV
fi

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
echo "MANYLINUX2010_X86_64_TAG=$MANYLINUX2010_X86_64_TAG" >> $GITHUB_ENV
echo "MANYLINUX2014_AARCH64_TAG=$MANYLINUX2014_AARCH64_TAG" >> $GITHUB_ENV
echo "MANYLINUX2014_PPC64LE_TAG=$MANYLINUX2014_PPC64LE_TAG" >> $GITHUB_ENV
