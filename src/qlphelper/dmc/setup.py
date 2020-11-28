#!/usr/bin/env python3
import setuptools

setuptools.setup(
    name = "dmc",
    version = "0.1.0",
    author="IsoaSFlus",
    author_email="me@isoasflus.com",
    packages = setuptools.find_packages(),
    install_requires = [
        'urllib3',
    ],
    platforms = ['any', ],
    entry_points={
        "console_scripts": ["dmc=dmc.__main__:entry"]
    },
)
