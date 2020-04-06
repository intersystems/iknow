import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

if 'ICUDIR' in os.environ:
	icudir = os.environ['ICUDIR']
else:
	icudir = '../../thirdparty/icu'

if os.name == 'nt':
	library_dirs = ['../../kit/x64/Release/bin']
else:
	if 'IKNOWPLAT' in os.environ:
		iknowplat = os.environ['IKNOWPLAT']
	else:
		raise RuntimeError("'IKNOWPLAT' is not defined")
	library_dirs = ['../../kit/{}/release/bin'.format(iknowplat)]

setup(
	ext_modules=cythonize(
		[Extension(
			'iknowpy',
			sources=['iknowpy.pyx'],
			include_dirs=['../engine/src', '../core/src/headers', '../base/src/headers', os.path.join(icudir, 'include')],
			library_dirs=library_dirs,
			libraries=['iknowengine'],
		)],
		compiler_directives={'language_level': '3'}
	)
)
