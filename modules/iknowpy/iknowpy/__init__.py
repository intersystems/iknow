"""Natural Language Processing Engine"""

# provide useful error message when accidentally imported from source directory
import os
import inspect
file_directory = os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda: 0)))
if os.path.isfile(os.path.join(file_directory, 'SOURCE')):
    raise ImportError(
        f'You have imported the source package {file_directory} instead of the '
        'installed package, which is not allowed. This occurred because the '
        '`iknowpy\' package source is in the directory where the import '
        'occurred and took precedence over the installed package. If you tried '
        'importing `iknowpy\' from the Python interactive console, change your '
        'working directory and try again. If you tried importing `iknowpy\' '
        'within a Python script, move the script to a different directory.'
    )
del os, inspect, file_directory

# export public variables and classes
from .version import __version__
from .labels import Labels
from .engine import iKnowEngine, UserDictionary
