from setuptools import setup, find_packages
from setuptools.extension import Extension

hail_ext = Extension(
    name='_hail',
    sources=[
        'libhail/module.c',
        'libhail/types.c'
    ])

setup(
    name='hail',
    version='3.0.0',
    packages=find_packages('.'),
    ext_modules=[hail_ext])
