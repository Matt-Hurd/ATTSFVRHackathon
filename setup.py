from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy

setup(
	name = 'bp',
    ext_modules=[
        Extension("bp", ["bp.c"],
            include_dirs=[numpy.get_include()]),
    ],
)