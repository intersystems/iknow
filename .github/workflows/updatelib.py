"""Common functions for autoupdate bot. Requires Python 3.6 or higher."""

import os
import random
import string
import sys


def _rand_string(length):
    """Return a random alphanumeric string with a given length."""
    return ''.join(random.choice(ALPHANUMERIC) for _ in range(length))


def get_vars():
    """Return a dict of name:value pairs from the dependencies file."""
    vars = {}  # name:value dict for variables
    _lines_before.clear()
    _lines_after.clear()
    with open(DEPENDENCIES_FILENAME) as dependencies_file:
        start_reading = False
        stop_reading = False
        for line in dependencies_file:
            if start_reading:
                if line == '# END DEPENDENCY-AUTOUPDATE SECTION\n':
                    stop_reading = True
                if stop_reading:
                    _lines_after.append(line)
                else:
                    pair = line.rstrip().split('=')
                    if len(pair) != 2:
                        raise ValueError(f'Invalid name-value pair {pair!r}')
                    pair[1] = pair[1].strip('"')
                    vars[pair[0]] = pair[1]
            else:
                _lines_before.append(line)
                if line == '# START DEPENDENCY-AUTOUPDATE SECTION\n':
                    start_reading = True
    if set(vars) != EXPECTED_VARNAMES:
        raise ValueError(f'Variables in {DEPENDENCIES_FILENAME} do not match the set {EXPECTED_VARNAMES}')
    return vars


def set_vars(vars):
    """Set a dict of name:value pairs to the dependencies file. Do not call this
    function before calling get_vars()."""
    assert _lines_before and _lines_after, 'Need to call get_vars()'
    with open(DEPENDENCIES_FILENAME, 'w') as dependencies_file:
        dependencies_file.write(''.join(_lines_before))
        for name, value in vars.items():
            if ' ' in value:
                value = f'"{value}"'
            dependencies_file.write(f'{name}={value}\n')
        dependencies_file.write(''.join(_lines_after))


def setenv(name, value):
    """Set the value of an environment variable for use in GitHub actions."""
    with open(os.environ['GITHUB_ENV'], 'a') as env_file:
        if '\n' in value:
            length = 8
            end_marker = _rand_string(length)
            while end_marker in value:
                length *= 2
                end_marker = _rand_string(length)
            env_file.write(f'{name}<<{end_marker}\n{value}\n{end_marker}\n')
        else:
            env_file.write(f'{name}={value}\n')


assert sys.version_info >= (3, 6), 'Python >=3.6 is required'

ALPHANUMERIC = string.digits + string.ascii_letters
DEPENDENCIES_FILENAME = os.path.join(os.environ['GITHUB_WORKSPACE'], 'travis/dependencies.sh')
EXPECTED_VARNAMES = {
    'ICU_NAME',
    'ICU_URL_WIN',
    'ICU_URL_SRC',
    'PYVERSIONS_WIN',
    'PYVERSIONS_OSX',
    'PYENV_TOOL_VERSION',
    'BUILDCACHE_NAME',
    'BUILDCACHE_URL_WIN',
    'CYTHON_VERSION',
    'MANYLINUX2010_X86_64_TAG',
    'MANYLINUX2014_AARCH64_TAG',
    'MANYLINUX2014_PPC64LE_TAG'
}

_lines_before = []
_lines_after = []
