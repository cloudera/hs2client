# Copyright 2016 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Some pieces adapted from the Apache Kudu (incubating) Python setup.py

from Cython.Distutils import build_ext
from Cython.Build import cythonize
import Cython

import numpy as np

import sys
from setuptools import setup
from distutils.command.clean import clean as _clean
from distutils.extension import Extension
import os
import platform

if Cython.__version__ < '0.19.1':
    raise Exception('Please upgrade to Cython 0.19.1 or newer')

MAJOR = 0
MINOR = 1
MICRO = 0
VERSION = '%d.%d.%d' % (MAJOR, MINOR, MICRO)
ISRELEASED = False

setup_dir = os.path.abspath(os.path.dirname(__file__))


def write_version_py(filename=os.path.join(setup_dir, 'hs2client/version.py')):
    version = VERSION
    if not ISRELEASED:
        version += '.dev'

    a = open(filename, 'w')
    file_content = "\n".join(["",
                              "# THIS FILE IS GENERATED FROM SETUP.PY",
                              "version = '%(version)s'",
                              "isrelease = '%(isrelease)s'"])

    a.write(file_content % {'version': VERSION,
                            'isrelease': str(ISRELEASED)})
    a.close()


class clean(_clean):
    def run(self):
        _clean.run(self)
        for x in ['hs2client/ext.cpp']:
            try:
                os.remove(x)
            except OSError:
                pass


if 'HS2CLIENT_HOME' in os.environ:
    hs2client_home = os.environ['HS2CLIENT_HOME']
    sys.stderr.write("Using HS2CLIENT_HOME directory: %s\n" %
                     (hs2client_home,))
    if not os.path.isdir(hs2client_home):
        sys.stderr.write("%s is not a valid HS2CLIENT_HOME directory" %
                         (hs2client_home,))
        sys.exit(1)

    hs2client_include_dirs = [os.path.join(hs2client_home, 'src')]
    hs2client_include_dirs.append(os.path.join(hs2client_home, 'include'))
    hs2client_lib_dir = os.path.join(hs2client_home, 'lib')
else:
    raise KeyError('HS2CLIENT_HOME')

INCLUDE_PATHS = hs2client_include_dirs + [np.get_include()]

LIBRARY_DIRS = [hs2client_lib_dir]
RT_LIBRARY_DIRS = LIBRARY_DIRS

EXTRA_COMPILE_ARGS = []
if platform.system() != 'Windows':
    # EXTRA_COMPILE_ARGS = ['-std=c++11', '-g', '-O0']
    EXTRA_COMPILE_ARGS = ['-std=c++11']

ext_submodules = ['ext']

extensions = []

for submodule_name in ext_submodules:
    ext = Extension('hs2client.{0}'.format(submodule_name),
                    ['hs2client/{0}.pyx'.format(submodule_name)],
                    libraries=['hs2client'],
                    include_dirs=INCLUDE_PATHS,
                    library_dirs=LIBRARY_DIRS,
                    extra_compile_args=EXTRA_COMPILE_ARGS,
                    runtime_library_dirs=RT_LIBRARY_DIRS)
    extensions.append(ext)

extensions = cythonize(extensions)

write_version_py()

LONG_DESCRIPTION = open(os.path.join(setup_dir, "README.md")).read()
DESCRIPTION = "Python interface to hs2client HiveServer 2 library"

CLASSIFIERS = [
    'Development Status :: 3 - Alpha',
    'Environment :: Console',
    'Programming Language :: Python',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 2.7',
    'Programming Language :: Python :: 3.4',
    'Programming Language :: Python :: 3.5',
    'Programming Language :: Cython'
]

URL = 'http://github.com/cloudera/hs2client'

setup(
    name="hs2client",
    packages=['hs2client', 'hs2client.tests'],
    version=VERSION,
    package_data={'hs2client': ['*.pxd', '*.pyx']},
    ext_modules=extensions,
    cmdclass={
        'clean': clean,
        'build_ext': build_ext
    },
    install_requires=['cython >= 0.21'],
    description=DESCRIPTION,
    long_description=LONG_DESCRIPTION,
    license='Apache License, Version 2.0',
    classifiers=CLASSIFIERS,
    maintainer="Wes McKinney",
    maintainer_email="wesm@apache.org",
    url=URL,
    test_suite="hs2client.tests"
)
