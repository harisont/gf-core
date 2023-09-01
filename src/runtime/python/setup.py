from setuptools import setup, Extension
import os
import platform

on_windows = platform.system() == 'Windows'

includes = os.getenv('EXTRA_INCLUDE_DIRS','').split(':')
if includes==['']:
    includes=[]
libraries = os.getenv('EXTRA_LIB_DIRS','').split(':')
if libraries==['']:
    libraries=[]

if on_windows:
    cpath = '../c/pgf/'
    extra_sources = [cpath+f for f in os.listdir(cpath) if f.endswith('.cxx')]
    includes+=["../c"]
    flags = ['/DCOMPILING_STATIC_PGF=1']
else:
    extra_sources = []
    flags = ['-std=c99', '-Werror', '-Wno-error=unused-variable', '-Wno-comment']

pgf_module = Extension(
    'pgf',
    sources = [
        'pypgf.c',
        'expr.c',
        'ffi.c',
        'transactions.c'
    ]+extra_sources,
    extra_compile_args = flags,
    include_dirs = includes,
    library_dirs = libraries,
    libraries = [] if on_windows else ['pgf'])

setup(
    name = 'pgf',
    version = '2.4',
    description = 'Python bindings to the Grammatical Framework\'s PGF runtime',
    long_description="""\
Grammatical Framework (GF) is a programming language for multilingual grammar applications.
This package provides Python bindings to GF runtime, which allows you to \
parse and generate text using GF grammars compiled into the PGF format.
""",
    url='https://www.grammaticalframework.org/',
    author='Krasimir Angelov',
    author_email='kr.angelov@gmail.com',
    license='BSD',
    ext_modules = [pgf_module],
    data_files = ["pgf.pyi"])
