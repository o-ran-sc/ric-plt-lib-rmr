# ==================================================================================
#       Copyright (c) 2019 Nokia
#       Copyright (c) 2018-2019 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# ==================================================================================
from os.path import dirname, abspath, join as path_join
from setuptools import setup, find_packages

SETUP_DIR = abspath(dirname(__file__))


def _long_descr():
    """Yields the content of documentation files for the long description"""
    try:
        doc_path = path_join(SETUP_DIR, "docs/source/index.rst")
        with open(doc_path) as f:
            return f.read()
    except FileNotFoundError:  # this happens during unit testing, we don't need it
        return ""


setup(
    name="rmr",
    version="0.13.4",
    packages=find_packages(),
    author="Tommy Carpenter, E. Scott Daniels",
    description="Python wrapper for RIC RMR",
    url="https://gerrit.o-ran-sc.org/r/admin/repos/ric-plt/lib/rmr",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Telecommunications Industry",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: POSIX :: Linux",
        "Topic :: Communications",
    ],
    python_requires=">=3.7",
    keywords="RIC rmr",
    license="Apache 2.0",
    data_files=[("", ["LICENSE.txt"])],
    install_requires=[],
    long_description=_long_descr(),
    long_description_content_type="text/x-rst",
)
