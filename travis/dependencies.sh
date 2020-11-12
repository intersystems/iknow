# This file is used as a central location for managing build-time dependencies.
#
# Usage: . travis/dependencies.sh

set -x


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
if [ "$TRAVIS_OS_NAME" = windows ]; then
  # URL to .zip pre-built release of ICU for Windows x86_64
  export ICU_URL="$ICU_URL_WIN"
else
  # URL to .zip source release of ICU
  export ICU_URL="$ICU_URL_SRC"
fi

# Python versions
# Availability of certain versions can differ between NuGet and pyenv.
if [ "$TRAVIS_OS_NAME" = windows ]; then
  # space-delimited Python versions to install with NuGet, old to new
  export PYVERSIONS="$PYVERSIONS_WIN"
elif [ "$TRAVIS_OS_NAME" = osx ]; then
  # space-delimited Python versions to install with pyenv, old to new
  export PYVERSIONS="$PYVERSIONS_OSX"

  # Given a Python version (MAJOR.MINOR only), print the URL for a compatible
  # official package installer. Return 1 if the version is not supported.
  pyinstall_fallback () {
    case "$1" in
      3.5)
        echo https://www.python.org/ftp/python/3.5.4/python-3.5.4-macosx10.6.pkg
        ;;
      3.6)
        echo https://www.python.org/ftp/python/3.6.8/python-3.6.8-macosx10.9.pkg
        ;;
      3.7)
        echo https://www.python.org/ftp/python/3.7.9/python-3.7.9-macosx10.9.pkg
        ;;
      3.8)
        echo https://www.python.org/ftp/python/3.8.6/python-3.8.6-macosx10.9.pkg
        ;;
      3.9)
        echo https://www.python.org/ftp/python/3.9.0/python-3.9.0-macosx10.9.pkg
        ;;
      *)
        echo "Version $1 is not supported"
        return 1
        ;;
    esac
  }
  export -f pyinstall_fallback
fi

# buildcache
if [ "$TRAVIS_OS_NAME" = windows ]; then
  # URL to .zip release of buildcache
  export BUILDCACHE_URL="$BUILDCACHE_URL_WIN"
fi


# unset variables that are no longer needed and export those that will be
# needed in child shells
unset ICU_NAME
unset ICU_URL_WIN
unset ICU_URL_SRC
unset PYVERSIONS_WIN
unset PYVERSIONS_OSX
export PYENV_TOOL_VERSION
unset BUILDCACHE_TAG_NAME
unset BUILDCACHE_URL_WIN
export CYTHON_VERSION

{ set +x; } 2>/dev/null
