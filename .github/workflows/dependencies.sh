# This file is used as a central location for managing build-time dependencies.
#
# Usage: .github/workflows/dependencies.sh

set -euxo pipefail


# If you edit the dependency-autoupdate section manually, you may also need to
# change .github/workflows/updatelib.py.

# START DEPENDENCY-AUTOUPDATE SECTION
ICU_NAME="ICU 68.1"
ICU_URL_WIN=https://github.com/unicode-org/icu/releases/download/release-68-1/icu4c-68_1-Win64-MSVC2019.zip
ICU_URL_SRC=https://github.com/unicode-org/icu/releases/download/release-68-1/icu4c-68_1-src.zip
PYVERSIONS_WIN="3.5.4 3.6.8 3.7.9 3.8.6 3.9.0"
PYVERSIONS_OSX="3.5.10 3.6.12 3.7.9 3.8.6 3.9.0"
PYENV_TOOL_VERSION=1.2.21
BUILDCACHE_NAME="Release v0.22.3"
BUILDCACHE_URL_WIN=https://github.com/mbitsnbites/buildcache/releases/download/v0.22.3/buildcache-win-msvc.zip
CYTHON_VERSION=0.29.21
MANYLINUX2010_X86_64_TAG=2020-11-11-201fb79
MANYLINUX2014_AARCH64_TAG=2020-11-11-bc8ce45
MANYLINUX2014_PPC64LE_TAG=2020-11-11-bc8ce45
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
echo "PYENV_TOOL_VERSION=$PYENV_TOOL_VERSION" >> $GITHUB_ENV
echo "CYTHON_VERSION=$CYTHON_VERSION" >> $GITHUB_ENV
echo "MANYLINUX2010_X86_64_TAG=$MANYLINUX2010_X86_64_TAG" >> $GITHUB_ENV
echo "MANYLINUX2014_AARCH64_TAG=$MANYLINUX2014_AARCH64_TAG" >> $GITHUB_ENV
echo "MANYLINUX2014_PPC64LE_TAG=$MANYLINUX2014_PPC64LE_TAG" >> $GITHUB_ENV
