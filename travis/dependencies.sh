# This file is used as a central location for managing build-time dependencies.

set -x

# International Components for Unicode
if [ "$TRAVIS_OS_NAME" = windows ]; then
  # URL to .zip pre-built release of ICU for Windows x86_64
  export ICU_URL=https://github.com/unicode-org/icu/releases/download/release-67-1/icu4c-67_1-Win64-MSVC2017.zip
else
  # URL to .zip source release of ICU
  export ICU_URL=https://github.com/unicode-org/icu/releases/download/release-67-1/icu4c-67_1-src.zip
fi

# Python versions
# Availability of certain versions can differ between NuGet and pyenv.
if [ "$TRAVIS_OS_NAME" = windows ]; then
  # space-delimited Python versions to install with NuGet, old to new
  export PYVERSIONS="3.5.4 3.6.8 3.7.9 3.8.6 3.9.0"
elif [ "$TRAVIS_OS_NAME" = osx ]; then
  # space-delimited Python versions to install with pyenv, old to new
  export PYVERSIONS="3.5.9 3.6.11 3.7.8 3.8.5 3.9.0b5"

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
  export BUILDCACHE_URL=https://github.com/mbitsnbites/buildcache/releases/download/v0.22.3/buildcache-win-msvc.zip
fi

{ set +x; } 2>/dev/null
