#!/usr/bin/python3

from setuptools import setup

setup(name='qdsp',
      version='1.3.0',
      description='Lightweight, dynamic plotting library',
      url='https://github.com/matt2718/qdsp',
      author='Matt Mitchell',
      author_email='matthew.s.mitchell@colorado.edu',
      license='LGPL',
      packages=['qdsp'],
      install_requires=[
          'numpy',
      ],
      zip_safe=False)
