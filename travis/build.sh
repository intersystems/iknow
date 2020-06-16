#!/usr/bin/env bash

# Run the correct build script for the current platform.

set -euxo pipefail

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  docker run --rm -e TRAVIS_COMMIT -v $(pwd):/iknow $DOCKER_IMAGE ${PRE_CMD-} /iknow/travis/build_manylinux.sh $TAG $ICU_URL $PYPI_TOKEN
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  travis/build_osx.sh
fi
