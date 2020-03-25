import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

if 'ICUDIR' in os.environ:
	icudir = os.environ['ICUDIR']
else:
	icudir = '../../thirdparty/icu'

setup(
	ext_modules=cythonize(
		[Extension(
			'iknowpy',
			sources=['iknowpy.pyx'],
			include_dirs=['../engine/src', '../core/src/headers', '../base/src/headers', os.path.join(icudir, 'include')],
			library_dirs=['../../kit/x64/Release/bin'],
			libraries=['iKnowEngine'],
		)],
		compiler_directives={'language_level': '3'}
	)
)
