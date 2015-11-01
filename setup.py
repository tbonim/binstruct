#! /usr/bin/env python
from setuptools import setup, Extension

binstruct = Extension('_binstruct2', sources=['_binstruct2.c'])

setup(
    name='binstruct',
    version='0.1',
    description='Python binary struct extension',
    author='Thomas Bonim',
    author_email='thomas.bonim@googlemail.com',
    long_description='A Python module for binary data struct',
    url='https://github.com/tbonim/binstruct',
    license="Public domain",
    ext_modules=[binstruct],
    packages=['binstruct'],
    test_suite='test',
    platforms='Cross Platform',
    classifiers=[
        'Programming Language :: Python :: 2',
    ],
)
