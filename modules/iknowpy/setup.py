"""Setup script for iknowpy.

The working directory must be the directory containing this script, and the
iKnow engine must already be built before running this script.

USAGE:
python setup.py build_ext -i
    Build the extension module in the ./iknowpy directory. This builds the
    Python interface component only.
python setup.py build_ext -i -f
    Rebuild the extension module in the ./iknowpy directory. This builds the
    Python interface component only.
python setup.py install
    Build and install the module into your instance of Python.
python setup.py bdist_wheel
    Create a wheel containing the extension including the iKnow and ICU
    dependencies.
python setup.py bdist_wheel --no-dependencies
    Create a wheel containing the extension without the iKnow and ICU
    dependencies. (Useful if you are using other tools like auditwheel to take
    care of dependencies)
python setup.py merge
    Merge all the wheels in dist/ into a single wheel containing all
    dependencies. The merged wheel is placed into dist/merged. It is assumed
    that the original wheels were created with the --no-dependencies flag and
    that they all have the same platform tag.
python setup.py merge --no-dependencies
    Same as above, except the merged wheel does not contain the iKnow and ICU
    dependencies.
python setup.py clean
    Clean all build files.
"""

import base64
import fnmatch
import glob
import hashlib
import os
import platform
import shutil
import subprocess
import sys
import zipfile
from setuptools import setup, Extension, Command
from Cython.Build import cythonize
if sys.platform == 'win32':
    import pefile
    import machomachomangler.pe


class BuildError(Exception):
    """Exception that is raised if a build-related error occurs."""
    pass


class PatchLib:
    """An instance represents a shared library file patcher."""

    def __init__(self):
        """Check that the tools needed to patch libraries are present, then
        configure tools to patch libraries correctly. Raise an exception if
        tools are not present.

        On Linux, we require patchelf >=0.9. On Mac OS, we require otool and
        install_name_tool. On Windows, no external programs are needed."""
        if sys.platform == 'darwin':
            if not shutil.which('otool'):
                raise BuildError('otool not found')
            if not shutil.which('install_name_tool'):
                raise BuildError('install_name_tool not found')
        elif sys.platform == 'linux':
            if not shutil.which('patchelf'):
                raise BuildError('patchelf not found')
            p = subprocess.run(['patchelf', '--version'],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True,
                universal_newlines=True)
            try:
                version = tuple(int(x) for x in p.stdout.split()[1].split('.'))
            except ValueError:
                raise BuildError('Unable to parse patchelf version {!r}'.format(p.stdout.rstrip()))
            if version < (0, 9):
                raise BuildError('patchelf >=0.9 is needed, but found version {!r}'.format(p.stdout.rstrip()))
            if platform.processor() == 'aarch64' and version < (0, 12):
                # work around patchelf bug (https://github.com/NixOS/patchelf/pull/216)
                self._patchelf = ['patchelf', '--page-size', '65536']
            else:
                self._patchelf = ['patchelf']
        # do nothing on Windows

    def setrpath(self, lib_path):
        """Given the path to a shared library, set its rpath to '$ORIGIN' so
        that it can find libraries in its own directory. Linux only."""
        if sys.platform != 'linux':
            raise NotImplementedError('PatchLib.setrpath is Linux only')
        subprocess.run(self._patchelf + ['--set-rpath', '$ORIGIN', lib_path],
                       check=True)

    def getneeded(self, lib_path):
        """Given a path to a shared library, return a list containing the ICU
        and iKnow engine direct dependencies of that shared library. The list
        may contain the names of the dependencies or paths to the dependencies,
        depending on what is embedded in the executable."""
        if sys.platform == 'darwin':
            cmd = ['otool', '-L', lib_path]
            p = subprocess.run(cmd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               check=True, universal_newlines=True)
            lib_name = os.path.split(lib_path)[1]
            needed = map(up_to_first_space, p.stdout.rstrip().split('\n'))
            needed = (s.strip() for s in needed)
            return [s for s in needed
                    if os.path.split(s)[1] != lib_name and
                    (fnmatch.fnmatch(s, '*' + iculibs_name_pattern) or
                     fnmatch.fnmatch(s, '*' + enginelibs_name_pattern))]
        elif sys.platform == 'linux':
            cmd = self._patchelf + ['--print-needed', lib_path]
            p = subprocess.run(cmd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               check=True, universal_newlines=True)
            return [s for s in p.stdout.split()
                    if fnmatch.fnmatch(s, iculibs_name_pattern) or
                    fnmatch.fnmatch(s, enginelibs_name_pattern)]
        else:  # win32
            pe = pefile.PE(lib_path)
            needed = []
            if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
                for entry in pe.DIRECTORY_ENTRY_IMPORT:
                    dll_name = entry.dll.decode('utf-8')
                    if (fnmatch.fnmatch(dll_name, iculibs_name_pattern) or
                            fnmatch.fnmatch(dll_name, enginelibs_name_pattern)):
                        needed.append(dll_name)
            pe.close()
            return needed

    def replaceneeded(self, lib_path, old_deps, name_map):
        """For the shared library at lib_path, replace its declared dependencies
        on old_dep_names with those in name_map.

        old_deps: a nonempty list of dependencies
            (either by name or path, depending on the value currently in the
            shared library) that the shared library has
        name_map: a dict that maps an old dependency NAME to a new NAME"""
        if sys.platform == 'darwin':
            cmd = ['install_name_tool']
            for old_dep in old_deps:
                cmd.append('-change')
                cmd.append(old_dep)
                old_dep_name = os.path.split(old_dep)[1]
                new_dep_name = name_map[old_dep_name]
                new_dep = os.path.join('@loader_path', new_dep_name)
                cmd.append(new_dep)
            cmd.append(lib_path)
            subprocess.run(cmd, check=True)
        elif sys.platform == 'linux':
            cmd = self._patchelf[:]
            for old_dep in old_deps:
                cmd.append('--replace-needed')
                cmd.append(old_dep)
                cmd.append(name_map[old_dep])
            cmd.append(lib_path)
            subprocess.run(cmd, check=True)
        else:  # win32
            with open(lib_path, 'rb') as f:
                buf = f.read()
            buf = machomachomangler.pe.redll(buf, {dep.encode('utf-8'): name_map[dep].encode('utf-8') for dep in old_deps})
            with open(lib_path, 'wb') as f:
                f.write(buf)
            pe = pefile.PE(lib_path)
            pe.OPTIONAL_HEADER.CheckSum = pe.generate_checksum()
            pe.write(lib_path)
            pe.close()

    def setname(self, lib_path, name):
        """Set the logical name of a shared library. Does not rename the
        file."""
        if sys.platform == 'darwin':
            cmd = ['install_name_tool', '-id', name, lib_path]
            subprocess.run(cmd, check=True)
        elif sys.platform == 'linux':
            cmd = self._patchelf + ['--set-soname', name, lib_path]
            subprocess.run(cmd, check=True)
        # do nothing on Windows


class CleanCommand(Command):
    """Command for cleaning all build files"""
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        rmtree('build')
        rmtree('dist')
        rmtree('iknowpy.egg-info')
        rmtree('iknowpy/__pycache__')
        remove('iknowpy/engine.cpp')
        for p in glob.iglob(os.path.join('iknowpy', module_name_pattern)):
            remove(p)


class MergeCommand(Command):
    """Command for merging all wheels in dist/ into a single wheel. The merged
    wheel is placed in dist/merged."""
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        python_tags = []
        abi_tags = []
        platform_tag = None
        extracted_dirs = []
        tmp_dir_base = 'dist{}temp'.format(os.sep)
        rmtree(tmp_dir_base)

        # extract wheels
        whl_paths = glob.glob('dist/iknowpy-{}-*.whl'.format(version))
        if len(whl_paths) < 2:
            raise BuildError('Fewer than 2 wheels were found in dist/')
        if len(set(map(lambda s: s[s.rindex('-') + 1:], whl_paths))) > 1:
            raise BuildError('Multiple platform tags were found in the wheels in dist/. '
                             'Only wheels with the same platform tag can be merged.')
        for whl_path in whl_paths:
            whl_name = os.path.split(whl_path)[1]
            whl_name = whl_name[:-len('.whl')]  # strip off the extension
            whl_name_split = whl_name.split('-')
            # whl_name_split has the form [name, version, python_tag, abi_tag, platform_tag]
            python_tags.append(whl_name_split[2])
            abi_tags.append(whl_name_split[3])
            platform_tag = whl_name_split[4]
            extracted_dir = os.path.join(tmp_dir_base, whl_name)
            extracted_dirs.append(extracted_dir)
            extract_wheel(whl_path, extracted_dir)
            if glob.glob(os.path.join(extracted_dir, 'iknowpy', enginelibs_name_pattern)):
                raise BuildError('{} was not built with --no-dependencies and cannot be merged'.format(whl_path))
        python_tags.sort()
        abi_tags.sort()

        # copy all engine modules into extracted contents of first wheel
        for extracted_dir in extracted_dirs[1:]:
            for module_path in glob.iglob(os.path.join(extracted_dir, 'iknowpy', module_name_pattern)):
                print('copying {} -> {}'.format(module_path, os.path.join(extracted_dirs[0], 'iknowpy', os.path.split(module_path)[1])))
                shutil.copy2(module_path, os.path.join(extracted_dirs[0], 'iknowpy'))

        # fix up the wheel
        if no_dependencies:
            update_wheel_record(extracted_dirs[0])
        else:
            patch_wheel(extracted_dirs[0], True)

        # create the merged wheel
        merged_whl_name = 'iknowpy-{}-{}-{}-{}.whl'.format(version, '.'.join(python_tags), '.'.join(abi_tags), platform_tag)
        merged_whl_path = os.path.join('dist', 'merged', merged_whl_name)
        print('creating merged wheel {}'.format(merged_whl_path))
        repackage_wheel(merged_whl_path, extracted_dirs[0])

        # delete temp files
        rmtree(tmp_dir_base)


def rmtree(path):
    """Like shutil.rmtree() but is silent if path does not exist."""
    try:
        shutil.rmtree(path)
    except FileNotFoundError:
        pass


def remove(path):
    """Like os.remove() but is silent if path does not exist."""
    try:
        os.remove(path)
    except FileNotFoundError:
        pass


def up_to_first_space(s):
    """Return the substring of s up to the first space, or all of s if it does
    not contain a space."""
    i = s.find(' ')
    if i == -1:
        i = len(s)
    return s[:i]


def rehash(file_path):
    """Return (hash, size) for a file with path file_path. The hash and size
    are used by pip to verify the integrity of the contents of a wheel."""
    with open(file_path, 'rb') as file:
        contents = file.read()
        hash = base64.urlsafe_b64encode(hashlib.sha256(contents).digest()).decode('latin1').rstrip('=')
        size = len(contents)
        return hash, size


def hashfile(afile, blocksize=65536, length=8):
    """Hash the contents of an open file handle with SHA256. Return the first
    length characters of the hash."""
    hasher = hashlib.sha256()
    buf = afile.read(blocksize)
    while len(buf) > 0:
        hasher.update(buf)
        buf = afile.read(blocksize)
    return hasher.hexdigest()[:length]


def extract_wheel(whl_path, dest):
    """Extract a wheel to the directory dest.

    Parameter whl_path: The path to the wheel to extract
    Precondition: whl_path is the path to an existing wheel

    Parameter dest: The destination to extract the wheel
    Precondition: dest is an existing directory"""
    print('extracting {} to {}'.format(whl_path, dest))
    with zipfile.ZipFile(whl_path) as whl_file:
        whl_file.extractall(dest)


def update_wheel_record(whl_dir):
    """Given a directory containing the extracted contents of a wheel, update
    the RECORD file to account for any changes in the contents."""
    record_filepath = os.path.join(whl_dir, 'iknowpy-{}.dist-info'.format(version), 'RECORD')
    print('updating {}'.format(record_filepath))
    filepath_list = []
    for root, _, files in os.walk(whl_dir):
        for file in files:
            filepath_list.append(os.path.join(root, file))
    with open(record_filepath, 'w') as record_file:
        for file_path in filepath_list:
            if file_path == record_filepath:
                record_file.write(os.path.relpath(record_filepath, whl_dir))
                record_file.write(',,\n')
            else:
                record_line = '{},sha256={},{}\n'.format(
                    os.path.relpath(file_path, whl_dir), *rehash(file_path))
                record_file.write(record_line)


def repackage_wheel(whl_path, whl_dir):
    """Create or replace a wheel at whl_path by packaging the files in
    whl_dir.
    Precondition: whl_dir is the directory containing the extracted wheel
    contents"""
    print('repackaging {}'.format(whl_path))
    filepath_list = []
    for root, _, files in os.walk(whl_dir):
        for file in files:
            filepath_list.append(os.path.join(root, file))
    os.makedirs(os.path.split(whl_path)[0], exist_ok=True)
    with zipfile.ZipFile(whl_path, 'w', zipfile.ZIP_DEFLATED) as whl_file:
        for file_path in filepath_list:
            print('adding {}'.format(file_path))
            whl_file.write(file_path, os.path.relpath(file_path, whl_dir))


def fix_wheel_ppc64le(whl_path, extracted=False):
    """Fix a ppc64le wheel so that it is compatible with Python distributions
    using both 'ppc64le' and 'powerpc64le' platform tags. Linux for ppc64le
    only. This serves as a workaround for
    https://github.com/pypa/manylinux/issues/687.

    If extracted is False, whl_path must be the path to a .whl file, and this
    file is replaced with a fixed version. If extracted is True, then whl_path
    must be a directory containing the extracted contents of the wheel to fix,
    and the contents of the directory are replaced with the contents of a fixed
    version (the contents are not zipped up)."""
    print('patching ppc64le wheel')

    if extracted:
        extracted_dir = whl_path
    else:
        # extract wheel
        extracted_dir = 'dist{}temp'.format(os.sep)
        rmtree(extracted_dir)
        os.mkdir(extracted_dir)
        extract_wheel(whl_path, extracted_dir)

    # add compatibility with different platform tag
    module_pattern = os.path.join(extracted_dir, 'iknowpy', 'engine.*pc64le-*.so')
    module_paths = glob.glob(module_pattern)
    if not module_paths:
        raise BuildError('Unable to find module matching pattern {!r}'.format(module_pattern))
    for module_path in module_paths:
        module_dir, module_name = os.path.split(module_path)
        if '-powerpc64le-' in module_name:
            other_module_name = module_name.replace('-powerpc64le-', '-ppc64le-')
        elif '-ppc64le-' in module_name:
            other_module_name = module_name.replace('-ppc64le-', '-powerpc64le-')
        else:
            raise BuildError("Module {} contains neither '-powerpc64le-' nor '-ppc64le-'".format(module_path))
        other_module_path = os.path.join(module_dir, other_module_name)
        print('copying {} -> {}'.format(module_path, other_module_path))
        shutil.copy2(module_path, other_module_path)

    # update record
    update_wheel_record(extracted_dir)

    if not extracted:
        # repackage wheel
        repackage_wheel(whl_path, extracted_dir)

        # remove extracted files
        print('removing {}'.format(extracted_dir))
        rmtree(extracted_dir)


def patch_wheel(whl_path, extracted=False):
    """Patch a wheel in a manner similar to auditwheel.

    If extracted is False, whl_path must be the path to a .whl file, and this
    file is replaced with a fixed version. If extracted is True, then whl_path
    must be a directory containing the extracted contents of the wheel to patch,
    and the contents of the directory are replaced with the contents of a
    patched version (the contents are not zipped up).

    This is necessary for packaging the ICU and iKnow engine shared libraries in
    a way that avoids dependency hell and ensures that Python packages are
    self-contained and isolated. There are a few reasons for patching the
    libraries.

    1. We need to be able to load the libraries no matter where they are
    installed.
    2. We don't want a system library with the same name to interfere with
    loading.
    3. We don't want two Python packages containing libraries with the same name
    to interfere with each other.

    To patch the wheel, we rename the iKnow engine and ICU shared libraries by
    adding a hash to the file names, and then we re-link them to the newly
    renamed libraries. This way, we can guarantee that the correct library is
    loaded when iknowpy is imported."""

    print('repairing wheel')

    # extract wheel
    if extracted:
        extracted_dir = whl_path
    else:
        extracted_dir = 'dist{}temp'.format(os.sep)
        rmtree(extracted_dir)
        os.mkdir(extracted_dir)
        extract_wheel(whl_path, extracted_dir)

    # copy ICU and iKnow engine library files
    iculib_map = {}  # name of symlink to ICU library -> name of actual ICU library file
    repair_lib_dir = os.path.join(extracted_dir, 'iknowpy')
    for lib_path in iculib_paths:
        if os.path.islink(lib_path):
            iculib_map[os.path.split(lib_path)[1]] = os.path.split(os.path.realpath(lib_path))[1]
        else:
            shutil.copy2(lib_path, repair_lib_dir)
    for lib_path in enginelib_paths:
        shutil.copy2(lib_path, repair_lib_dir)

    # create list of libraries to repair
    module_pattern = os.path.join(repair_lib_dir, module_name_pattern)
    repair_lib_paths = glob.glob(module_pattern)
    if not repair_lib_paths:
        raise BuildError('Unable to find module matching pattern {!r}'.format(module_pattern))
    repair_lib_paths.extend(glob.iglob(os.path.join(repair_lib_dir, iculibs_name_pattern)))
    repair_lib_paths.extend(glob.iglob(os.path.join(repair_lib_dir, enginelibs_name_pattern)))

    # repair libraries by setting dependency paths and renaming them
    patcher = PatchLib()
    lib_rename = {}  # dict from old lib name to new lib name
    for lib_path in repair_lib_paths:
        lib_name = os.path.split(lib_path)[1]
        if lib_name.startswith('engine.'):  # don't rename main module file
            lib_rename[lib_name] = lib_name
        else:
            lib_name_split = lib_name.split('.')
            with open(lib_path, 'rb') as lib_file:
                lib_name_split[0] += '-' + hashfile(lib_file)
            lib_name_new = '.'.join(lib_name_split)
            lib_rename[lib_name] = lib_name_new
    for lib_name in iculib_map:
        # replace dependency on symlink to dependency on actual library file
        lib_rename[lib_name] = lib_rename[iculib_map[lib_name]]

    os.makedirs(CACHE_DIR, exist_ok=True)
    for lib_path in repair_lib_paths:
        lib_dir, lib_name = os.path.split(lib_path)
        print('repairing {} -> {}'.format(lib_path, os.path.join(lib_dir, lib_rename[lib_name])), end='')
        if not lib_name.startswith('engine.') and os.path.isfile(os.path.join('dist/cache', lib_rename[lib_name])):
            # copy patched library from cache
            os.remove(lib_path)
            shutil.copy2(os.path.join(CACHE_DIR, lib_rename[lib_name]), lib_dir)
            print(' (cached)')
        else:
            dep_libs = patcher.getneeded(lib_path)
            patcher.setname(lib_path, lib_rename[lib_name])
            if dep_libs:
                if sys.platform == 'linux':
                    patcher.setrpath(lib_path)
                patcher.replaceneeded(lib_path, dep_libs, lib_rename)
            os.rename(lib_path, os.path.join(lib_dir, lib_rename[lib_name]))
            if not lib_name.startswith('engine.'):
                # copy patched library into cache
                shutil.copy2(os.path.join(lib_dir, lib_rename[lib_name]), CACHE_DIR)
            print()

    # update record file, which tracks wheel contents and their checksums
    update_wheel_record(extracted_dir)

    if not extracted:
        # repackage wheel
        repackage_wheel(whl_path, extracted_dir)

        # remove extracted files
        print('removing {}'.format(extracted_dir))
        rmtree(extracted_dir)


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


# set constants
CACHE_DIR = 'dist/cache'

# obtain version
version = {}
with open('iknowpy/version.py') as version_file:
    exec(version_file.read(), version)
version = version['__version__']

if 'ICUDIR' in os.environ:
    icudir = os.environ['ICUDIR']
else:
    icudir = '../../thirdparty/icu'

# Do not allow creation of a source distribution, as building iknowpy and its
# dependencies is too complex to be encoded in a source distribution. If you
# want to build iknowpy yourself, use the GitHub repository.
if 'sdist' in sys.argv:
    raise BuildError('Creation of a source distribution is not supported.')

# If installation is requested, do not perform a direct installation. Create a
# wheel instead and install the wheel. This is necessary to perform the wheel
# repair procedure prior to installation.
install_wheel = False
if len(sys.argv) > 1 and sys.argv[1] == 'install':
    sys.argv[1] = 'bdist_wheel'
    install_wheel = True

# platform-specific settings
setup_requires = ['cython', 'wheel']
if sys.platform == 'win32':
    setup_requires.extend(['pefile', 'machomachomangler'])
    library_dirs = ['../../kit/x64/Release/bin']
    iculibs_name_pattern = 'icu*.dll'
    iculibs_path_pattern = os.path.join(icudir, 'bin64', iculibs_name_pattern)
    module_name_pattern = 'engine.*.pyd'
    enginelibs_name_pattern = 'iKnow*.dll'
    enginelibs_path_pattern = os.path.join('../../kit/x64/Release/bin', enginelibs_name_pattern)
    extra_compile_args = []
    extra_link_args = []
else:
    if 'IKNOWPLAT' in os.environ:
        iknowplat = os.environ['IKNOWPLAT']
    else:
        raise BuildError("'IKNOWPLAT' is not defined")
    library_dirs = ['../../kit/{}/release/bin'.format(iknowplat)]
    module_name_pattern = 'engine.*.so'
    if sys.platform == 'darwin':
        iculibs_name_pattern = 'libicu*.dylib'
        enginelibs_name_pattern = 'libiknow*.dylib'
        os.environ['CC'] = 'clang++'  # workaround to force setuptools to invoke C++ compiler
        os.environ['CXX'] = 'clang++'
        extra_compile_args = ['-std=c++11']
        extra_link_args = ['-headerpad_max_install_names']
        if install_wheel:
            # set wheel target platform to that of the build platform
            macosx_version = '.'.join(platform.mac_ver()[0].split('.')[:2])
            os.environ['MACOSX_DEPLOYMENT_TARGET'] = macosx_version
            sys.argv.append('--plat-name=macosx-{}-x86_64'.format(macosx_version))
    else:
        iculibs_name_pattern = 'libicu*.so*'
        enginelibs_name_pattern = 'libiknow*.so'
        os.environ['CC'] = 'g++'  # workaround to force setuptools to invoke C++ compiler
        os.environ['CXX'] = 'g++'
        extra_compile_args = []
        extra_link_args = []
    iculibs_path_pattern = os.path.join(icudir, 'lib', iculibs_name_pattern)
    enginelibs_path_pattern = os.path.join('../../kit/{}/release/bin'.format(iknowplat), enginelibs_name_pattern)

# Find ICU and iKnow engine libraries. We do not copy the libraries into the
# package source at this stage because the libraries are added during the wheel
# repair step.
iculib_paths = []  # paths to original ICU libraries
enginelib_paths = []  # paths to original iKnow engine libraries
if '--no-dependencies' in sys.argv:
    no_dependencies = True
    sys.argv.remove('--no-dependencies')
elif 'install' in sys.argv or 'bdist_wheel' in sys.argv or 'merge' in sys.argv:
    no_dependencies = False
    iculib_paths = glob.glob(iculibs_path_pattern)
    enginelib_paths = glob.glob(enginelibs_path_pattern)
    if not iculib_paths:
        raise BuildError('ICU libraries not found: {}'.format(iculibs_path_pattern))
    if not enginelib_paths:
        raise BuildError('iKnow engine libraries not found: {}'.format(enginelibs_path_pattern))
else:
    no_dependencies = True

# include MIT license in distribution
shutil.copy2('../../LICENSE', '.')

# include ICU license in distribution
icu_license_found = False
for root, _, files in os.walk(icudir):
    for filename in files:
        if filename == 'LICENSE':
            icu_license_found = True
            shutil.copy2(os.path.join(root, filename), 'LICENSE_ICU')
            break
    if icu_license_found:
        break
if not icu_license_found:
    raise BuildError('ICU license not found in {}'.format(icudir))

with open('../../README.md', encoding='utf-8') as readme_file:
    long_description = readme_file.read()
    # Strip off badges. They belong in the GitHub version of the README file but
    # are less appropriate on PyPI.
    long_description = long_description[long_description.index('# iKnow\n'):]

try:
    setup(
        name='iknowpy',
        description='iKnow Natural Language Processing engine',
        long_description=long_description,
        long_description_content_type='text/markdown',
        url='https://github.com/intersystems/iknow',
        author='InterSystems Corporation',
        license='MIT',
        classifiers=[
            'Development Status :: 4 - Beta',
            'License :: OSI Approved :: MIT License',
            'Topic :: Scientific/Engineering :: Information Analysis',
            'Programming Language :: C++',
            'Programming Language :: Cython',
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3 :: Only',
            'Programming Language :: Python :: 3.5',
            'Programming Language :: Python :: 3.6',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: 3.8',
            'Programming Language :: Python :: 3.9',
            'Programming Language :: Python :: Implementation :: CPython',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: Microsoft :: Windows',
            'Operating System :: POSIX'
        ],
        keywords='NLP',
        project_urls={
            'Source': 'https://github.com/intersystems/iknow',
            'Tracker': 'https://github.com/intersystems/iknow/issues',
            'Wiki': 'https://github.com/intersystems/iknow/wiki'
        },
        packages=['iknowpy'],
        version=version,
        python_requires='>=3.5',
        setup_requires=setup_requires,
        zip_safe=False,
        ext_modules=cythonize(
            [Extension(
                'iknowpy.engine',
                sources=['iknowpy/engine.pyx'],
                include_dirs=['../engine/src', '../core/src/headers', '../base/src/headers', os.path.join(icudir, 'include')],
                library_dirs=library_dirs,
                libraries=['iknowengine'],
                extra_compile_args=extra_compile_args,
                extra_link_args=extra_link_args
            )],
            compiler_directives={'language_level': '3', 'binding': True}
        ),
        cmdclass={
            'clean': CleanCommand,
            'merge': MergeCommand
        }
    )
finally:
    # remove licenses from package source
    remove('LICENSE')
    remove('LICENSE_ICU')

if 'bdist_wheel' in sys.argv and platform.processor() == 'ppc64le':
    fix_wheel_ppc64le(find_wheel())

if 'bdist_wheel' in sys.argv and not no_dependencies:
    patch_wheel(find_wheel())

if install_wheel:
    subprocess.run(
        [sys.executable, '-m', 'pip', 'install', '--upgrade', '--force-reinstall', find_wheel()],
        check=True)
