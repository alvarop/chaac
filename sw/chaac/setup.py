"""
Testing
"""
from setuptools import setup, find_packages

setup(
    name="chaac",
    version="0.1",
    description="Chaac python libraries",
    url="https://github.com/alvarop/chaac",
    author="Alvaro Prieto",
    author_email="source@alvaroprieto.com",
    license="MIT",
    packages=find_packages(),
    include_package_data=True,
    zip_safe=False,
)
