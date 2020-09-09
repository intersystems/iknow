#!/usr/bin/env bash

# After build_manylinux.sh successfully runs inside a manylinux container, run
# this script outside the container to deploy the wheels if appropriate. The
# working directory must be the root of the GitHub repository. 

set -euxo pipefail

export REPO_ROOT=$(pwd)
DEPLOY=$(travis/deploy_check.sh)
if [[ "$DEPLOY" == "0" ]]; then
  echo "Deployment skipped"
else
  pip3 install --user twine
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
  echo '+ python3 -m twine upload -u "__token__" -p "$TOKEN" modules/iknowpy/dist2/iknowpy-*manylinux*.whl'
  python3 -m twine upload -u "__token__" -p "$TOKEN" modules/iknowpy/dist2/iknowpy-*manylinux*.whl
  set -x
fi
