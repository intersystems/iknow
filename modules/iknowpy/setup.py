"""Setup script for iknowpy.

python setup.py build_ext -i
	Build the extension module in the ./iknowpy directory.
python setup.py build_ext -i -f
	Rebuild the extension module in the ./iknowpy directory.
python setup.py install
	Build and install the module.
python setup.py bdist_wheel
	Create a wheel containing the extension with dependencies.
python setup.py bdist_wheel --no-dependencies
	Create a wheel without the dependencies. (Useful if you are using other
	tools to take care of dependencies)
"""

import glob
import os
import shutil
import sys
from setuptools import setup, Extension
from Cython.Build import cythonize


class BuildError(Exception):
	pass


if 'ICUDIR' in os.environ:
	icudir = os.environ['ICUDIR']
else:
	icudir = '../../thirdparty/icu'

# platform-specific settings
if sys.platform == 'win32':
	library_dirs = ['../../kit/x64/Release/bin']
	lib_ext_pattern = '*.dll'
	iculibs_pattern = os.path.join(icudir, 'bin64/icu*.dll')
	enginelibs_pattern = '../../kit/x64/Release/bin/iKnow*.dll'
	extra_compile_args = []
else:
	if 'IKNOWPLAT' in os.environ:
		iknowplat = os.environ['IKNOWPLAT']
	else:
		raise BuildError("'IKNOWPLAT' is not defined")
	library_dirs = ['../../kit/{}/release/bin'.format(iknowplat)]
	if sys.platform == 'darwin':
		lib_ext_pattern = '*.dylib'
		iculibs_pattern = os.path.join(icudir, 'lib/libicu*.dylib')
		enginelibs_pattern = '../../kit/{}/release/bin/libiknow*.dylib'.format(iknowplat)
		os.environ['CC'] = 'clang++'
		os.environ['CXX'] = 'clang++'
		extra_compile_args = ['-std=c++11']
	else:
		lib_ext_pattern = '*.so*'
		iculibs_pattern = os.path.join(icudir, 'lib/libicu*.so*')
		enginelibs_pattern = '../../kit/{}/release/bin/libiknow*.so'.format(iknowplat)
		extra_compile_args = []

# Unless the '--no-dependencies' flag is specified, temporarily copy ICU and
# iKnow engine libraries into package source to include in distribution
if '--no-dependencies' in sys.argv:
	sys.argv.remove('--no-dependencies')
else:
	iculibs_list = glob.glob(iculibs_pattern)
	enginelibs_list = glob.glob(enginelibs_pattern)
	if not iculibs_list:
		raise BuildError('ICU libraries not found: {}'.format(iculibs_pattern))
	if not enginelibs_list:
		raise BuildError('iKnow engine libraries not found: {}'.format(enginelibs_pattern))
	for lib in iculibs_list:
		shutil.copy2(lib, 'iknowpy')
	for lib in enginelibs_list:
		shutil.copy2(lib, 'iknowpy')

try:
	setup(
		name='iknowpy',
		description='iKnow natural language processing engine',
		long_description='iKnow natural language processing engine',
		url='https://github.com/intersystems/iknow',
		author='InterSystems Corporation',
		license='MIT',
		classifiers=[
			'Development Status :: 3 - Alpha',
			'License :: OSI Approved :: MIT License',
			'Programming Language :: Python :: 3',
		],
		keywords='NLP',
		project_urls={
			'Source': 'https://github.com/intersystems/iknow',
			'Tracker': 'https://github.com/intersystems/iknow/issues',
		},
		packages=['iknowpy'],
		package_data={'iknowpy': [lib_ext_pattern]},
		version='0.0.2',
		python_requires='>=3.5',
		setup_requires=['cython', 'wheel'],
		zip_safe=False,
		ext_modules=cythonize(
			[Extension(
				'iknowpy.engine',
				sources=['iknowpy/engine.pyx'],
				include_dirs=['../engine/src', '../core/src/headers', '../base/src/headers', os.path.join(icudir, 'include')],
				library_dirs=library_dirs,
				libraries=['iknowengine'],
				extra_compile_args=extra_compile_args
			)],
			compiler_directives={'language_level': '3'}
		)
	)
finally:
	# remove ICU and iKnow engine libraries from package source
	for lib in glob.iglob(os.path.join('iknowpy', lib_ext_pattern)):
		try:
			os.remove(lib)
		except FileNotFoundError:
			pass
