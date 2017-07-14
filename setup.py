#!/usr/bin/env python

"""
setup.py file for pylibi2c
"""

from distutils.core import setup, Extension

pylibi2c_module = Extension('pylibi2c', sources=['src/i2c.c', 'src/pyi2c.c'],)

setup(
    name='pylibi2c',
    version='0.2',
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


