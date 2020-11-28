#!/usr/bin/env python
import setuptools


REQ = ['quickjs', ]

setuptools.setup(
    name = "streamfinder",
    version = "0.1.0",
    author="IsoaSFlus",
    author_email="me@isoasflus.com",
    packages = setuptools.find_packages(),
    requires = REQ,
    install_requires = REQ,
    platforms = 'any',
    package_data = {
        'streamfinder': ['*.js', ],
    },
    entry_points={
        "console_scripts": ["streamfinder=streamfinder.__main__:entry"]
    },
)
