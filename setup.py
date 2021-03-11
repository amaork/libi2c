#!/usr/bin/env python

"""
setup.py file for pylibi2c
"""

import os
from distutils.core import setup, Extension

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
INC_DIR = os.path.join(THIS_DIR, 'include')
VERSION = open('VERSION').read().strip()

pylibi2c_module = Extension('pylibi2c',
  sources=['src/i2c.c', 'src/pyi2c.c'],
  extra_compile_args=['-DLIBI2C_VERSION="' + VERSION + '"'],
  include_dirs=[INC_DIR],
)

setup(
    name='pylibi2c',
    version=VERSION,
    license='MIT',
    author='Amaork',
    author_email="amaork@gmail.com",
    url='https://github.com/amaork/libi2c',
    description="Linux userspace i2c operation library",
    long_description=open('README.md').read(),
    ext_modules=[pylibi2c_module],
    py_modules=["pylibi2c"],
    classifiers=[
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
    ],
)


