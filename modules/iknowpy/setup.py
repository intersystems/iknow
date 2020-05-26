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

import base64
import fnmatch
import glob
import hashlib
import os
import random
import shutil
import string
import subprocess
import sys
import zipfile
from setuptools import setup, Extension
from Cython.Build import cythonize


class BuildError(Exception):
	pass


def patchelf_check_version():
	"""We require patchelf to be at least version 0.9. Raise exception if
	patchelf is not present or has version < 0.9. Do nothing if the version
	check passes."""
	if not shutil.which('patchelf'):
		raise BuildError('patchelf was not found')
	p = subprocess.run(
		['patchelf', '--version'],
		stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True,
		universal_newlines=True)
	try:
		version = float(p.stdout.split()[1])
	except ValueError:
		raise BuildError('Unable to parse patchelf version {!r}'.format(p.stdout.rstrip()))
	if version < 0.9:
		raise BuildError('patchelf >=0.9 is needed, but found version {!r}'.format(p.stdout.rstrip()))


def patchelf_setrpath(lib_path):
	"""Given the path to a shared library, set its rpath to '$ORIGIN' so that it
	can find libraries in its own directory."""
	subprocess.run(['patchelf', '--set-rpath', '$ORIGIN', lib_path], check=True)


def patchelf_getneeded(lib_path):
	"""Given a path to a shared library, return a list containing the NAMES of
	the ICU and iKnow engine direct dependencies of that shared library. Unix
	only."""
	p = subprocess.run(
		['patchelf', '--print-needed', lib_path],
		stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True,
		universal_newlines=True)
	return [s for s in p.stdout.split()
			if fnmatch.fnmatch(s, iculibs_name_pattern) or
			fnmatch.fnmatch(s, enginelibs_name_pattern)]


def patchelf_replaceneeded(lib_path, old_dep_names, name_map):
	"""For the shared library at lib_path, replace its declared dependencies on
	old_dep_names with those in name_map.

	old_dep_names: a nonempty list of old dependency names that the shared library has
	name_map: a dict that maps an old dependency name to a new name"""
	cmd = ['patchelf']
	for old_dep_name in old_dep_names:
		cmd.append('--replace-needed')
		cmd.append(old_dep_name)
		cmd.append(name_map[old_dep_name])
	cmd.append(lib_path)
	subprocess.run(cmd, check=True)


def patchelf_setsoname(lib_path, name):
	"""Set the name of a shared library. Does not rename the file."""
	subprocess.run(['patchelf', '--set-soname', name, lib_path], check=True)


def rehash(file_path):
	"""Return (hash, size) for a file with path file_path."""
	with open(file_path, 'rb') as file:
		contents = file.read()
		hash = base64.urlsafe_b64encode(hashlib.sha256(contents).digest()).decode('latin1').rstrip('=')
		size = len(contents)
		return hash, size


def rand_alphanumeric(length=8):
	"""Return a random alphanumeric string with a given length."""
	return ''.join(random.choice(ALPHANUMERIC) for _ in range(length))


def patch_wheel(whl_path):
	"""Patch a wheel in a manner similar to auditwheel. On Unix, this is
	necessary prior to packaging the ICU and iKnow engine shared libraries.
	There are two reasons for patching the libraries.

	We need to be able to load the libraries no matter where they are installed.
	We accomplish this by patching RPATH so that a library can find a dependency
	in its own directory.

	We don't want a system library with the same name to interfere with loading.
	We accomplish this by tagging each library with a random alphanumeric
	string."""

	# extract wheel
	tmp_dir = 'dist/temp'
	print('extracting {} to {}'.format(whl_path, tmp_dir))
	whl_file = zipfile.ZipFile(whl_path)
	try:
		shutil.rmtree(tmp_dir)
	except FileNotFoundError:
		pass
	os.mkdir(tmp_dir)
	whl_file.extractall(tmp_dir)
	whl_file.close()

	# create list of libraries to repair
	repair_lib_dir = os.path.join(tmp_dir, 'iknowpy')
	repair_lib_paths = glob.glob(os.path.join(repair_lib_dir, 'engine.*.so'))
	if len(repair_lib_paths) == 0:
		raise BuildError('Unable to find module matching pattern {!r}'.format(os.path.join(repair_lib_dir, 'engine.*.so')))
	elif len(repair_lib_paths) > 1:
		raise BuildError('Found multiple modules matching pattern {!r}'.format(os.path.join(repair_lib_dir, 'engine.*.so')))
	repair_lib_paths.extend(glob.iglob(os.path.join(repair_lib_dir, iculibs_name_pattern)))
	repair_lib_paths.extend(glob.iglob(os.path.join(repair_lib_dir, enginelibs_name_pattern)))

	# repair libraries by setting RPATH and renaming them
	lib_rename = {}  # dict from old lib name to new lib name
	for lib_path in repair_lib_paths:
		lib_name = os.path.split(lib_path)[1]
		if lib_name.startswith('engine.'):  # don't rename main module file
			lib_rename[lib_name] = lib_name
		else:
			lib_name_split = lib_name.split('.')
			lib_name_split[0] += '-' + rand_alphanumeric()
			lib_name_new = '.'.join(lib_name_split)
			lib_rename[lib_name] = lib_name_new
	for lib_path in repair_lib_paths:
		lib_dir, lib_name = os.path.split(lib_path)
		print('repairing {} -> {}'.format(lib_path, os.path.join(lib_dir, lib_rename[lib_name])))
		patchelf_setsoname(lib_path, lib_rename[lib_name])
		dep_lib_names = patchelf_getneeded(lib_path)
		if dep_lib_names:
			patchelf_setrpath(lib_path)
			patchelf_replaceneeded(lib_path, dep_lib_names, lib_rename)
		os.rename(lib_path, os.path.join(lib_dir, lib_rename[lib_name]))

	# update record file
	record_filepath = os.path.join(tmp_dir, 'iknowpy-{}.dist-info'.format(version), 'RECORD')
	print('updating {}'.format(record_filepath))
	record_file = open(record_filepath, 'w')
	filepath_list = []
	for root, _, files in os.walk(tmp_dir):
		for file in files:
			filepath_list.append(os.path.join(root, file))
	for file_path in filepath_list:
		if file_path == record_filepath:
			record_file.write(os.path.relpath(record_filepath, tmp_dir))
			record_file.write(',,\n')
		else:
			record_line = '{},sha256={},{}\n'.format(os.path.relpath(file_path, tmp_dir), *rehash(file_path))
			record_file.write(record_line)
	record_file.close()

	# repackage wheel
	print('repackaging {}'.format(whl_path))
	whl_file = zipfile.ZipFile(whl_path, 'w', zipfile.ZIP_DEFLATED)
	for file_path in filepath_list:
		print('adding {}'.format(file_path))
		whl_file.write(file_path, os.path.relpath(file_path, tmp_dir))
	whl_file.close()

	# remove temporary files
	print('removing {}'.format(tmp_dir))
	shutil.rmtree(tmp_dir)


def find_wheel():
	"""Return the path to the wheel file that this script created. Raise
	exception if wheel cannot be found."""
	wheel_pattern = 'dist/iknowpy-{}-*{}{}-*.whl'.format(version, sys.version_info.major, sys.version_info.minor)
	wheel_pattern_matches = glob.glob(wheel_pattern)
	if len(wheel_pattern_matches) == 0:
		raise BuildError('Unable to find wheel matching pattern {!r}'.format(wheel_pattern))
	elif len(wheel_pattern_matches) > 1:
		raise BuildError('Found multiple wheels matching pattern {!r}'.format(wheel_pattern))
	return wheel_pattern_matches[0]


ALPHANUMERIC = string.ascii_letters + string.digits

if 'ICUDIR' in os.environ:
	icudir = os.environ['ICUDIR']
else:
	icudir = '../../thirdparty/icu'

version = '0.0.2'

# platform-specific settings
install_wheel = False
if sys.platform == 'win32':
	library_dirs = ['../../kit/x64/Release/bin']
	iculibs_name_pattern = 'icu*.dll'
	iculibs_path_pattern = os.path.join(icudir, 'bin64', iculibs_name_pattern)
	enginelibs_name_pattern = 'iKnow*.dll'
	enginelibs_path_pattern = os.path.join('../../kit/x64/Release/bin', enginelibs_name_pattern)
	extra_compile_args = []
else:
	if len(sys.argv) > 1 and sys.argv[1] == 'install':
		sys.argv[1] = 'bdist_wheel'
		install_wheel = True
	if 'IKNOWPLAT' in os.environ:
		iknowplat = os.environ['IKNOWPLAT']
	else:
		raise BuildError("'IKNOWPLAT' is not defined")
	library_dirs = ['../../kit/{}/release/bin'.format(iknowplat)]
	if sys.platform == 'darwin':
		iculibs_name_pattern = 'libicu*.dylib'
		enginelibs_name_pattern = 'libiknow*.dylib'
		os.environ['CC'] = 'clang++'
		os.environ['CXX'] = 'clang++'
		extra_compile_args = ['-std=c++11']
	else:
		iculibs_name_pattern = 'libicu*.so*'
		enginelibs_name_pattern = 'libiknow*.so'
		extra_compile_args = []
	iculibs_path_pattern = os.path.join(icudir, 'lib', iculibs_name_pattern)
	enginelibs_path_pattern = os.path.join('../../kit/{}/release/bin'.format(iknowplat), enginelibs_name_pattern)

# Unless the '--no-dependencies' flag is specified, temporarily copy ICU and
# iKnow engine libraries into package source to include in distribution
if '--no-dependencies' in sys.argv:
	no_dependencies = True
	sys.argv.remove('--no-dependencies')
else:
	no_dependencies = False
	iculibs_list = glob.glob(iculibs_path_pattern)
	enginelibs_list = glob.glob(enginelibs_path_pattern)
	if not iculibs_list:
		raise BuildError('ICU libraries not found: {}'.format(iculibs_path_pattern))
	if not enginelibs_list:
		raise BuildError('iKnow engine libraries not found: {}'.format(enginelibs_path_pattern))
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
		package_data={'iknowpy': [iculibs_name_pattern, enginelibs_name_pattern]},
		version=version,
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
	for lib in glob.iglob(os.path.join('iknowpy', iculibs_name_pattern)):
		os.remove(lib)
	for lib in glob.iglob(os.path.join('iknowpy', enginelibs_name_pattern)):
		os.remove(lib)

if 'bdist_wheel' in sys.argv and not no_dependencies and sys.platform == 'linux':
	print('repairing wheel')
	patchelf_check_version()
	patch_wheel(find_wheel())

if install_wheel:
	subprocess.run(
		[sys.executable, '-m', 'pip', 'install', '--upgrade', '--force-reinstall', find_wheel()],
		check=True)
