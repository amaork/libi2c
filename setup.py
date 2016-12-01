#!/usr/bin/env python

"""
setup.py file for pylibi2c
"""

from distutils.core import setup, Extension

pylibi2c_module = Extension('pylibi2c', sources = ['src/i2c.c', 'src/pyi2c.c'],)

setup (
	name = 'pylibi2c',
	version = '0.1',
	author = "amaork@gmail.com",
	description = """Linux userspace i2c operation library""",
	ext_modules = [pylibi2c_module],
	py_modules = ["pylibi2c"],
)


