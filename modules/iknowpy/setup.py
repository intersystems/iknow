import os
import sys
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

if sys.version_info.major < 3:
	raise RuntimeError('Python 3 or higher is required')

if 'ICUDIR' in os.environ:
	icudir = os.environ['ICUDIR']
else:
	icudir = '../../thirdparty/icu'

if sys.platform == 'win32':
	library_dirs = ['../../kit/x64/Release/bin']
	extra_compile_args = []
else:
	if 'IKNOWPLAT' in os.environ:
		iknowplat = os.environ['IKNOWPLAT']
	else:
		raise RuntimeError("'IKNOWPLAT' is not defined")
	library_dirs = ['../../kit/{}/release/bin'.format(iknowplat)]

	if sys.platform == 'darwin':
		os.environ['CC'] = 'clang++'
		extra_compile_args = ['-std=c++11']
	else:
		extra_compile_args = []

setup(
	name='iknowpy',
	description='Python interface to the InterSystems iKnow engine',
	url='https://github.com/intersystems/iknow',
	ext_modules=cythonize(
		[Extension(
			'iknowpy',
			sources=['iknowpy.pyx'],
			include_dirs=['../engine/src', '../core/src/headers', '../base/src/headers', os.path.join(icudir, 'include')],
			library_dirs=library_dirs,
			libraries=['iknowengine'],
			extra_compile_args=extra_compile_args
		)],
		compiler_directives={'language_level': '3'}
	)
)
