.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. Copyright (C) 2020 AT&T Intellectual Property


Installation Guide
==================

The RMR shared-object library and supporting files including
C-language header files for development are published as Debian (.deb)
and Redhat Package Manager (.rpm) package files to `PackageCloud
<https://packagecloud.io/app/o-ran-sc/release/search?q=rmr>`_ .
Follow the instructions there to download and install the required
version. The commands to install a recent version on a Debian system
such as Ubuntu appear next::

    wget -nv --content-disposition https://packagecloud.io/o-ran-sc/release/packages/debian/stretch/rmr_4.0.5_amd64.deb/download.deb
    sudo dpkg -i rmr_4.0.5_amd64.deb
    wget -nv --content-disposition https://packagecloud.io/o-ran-sc/release/packages/debian/stretch/rmr-dev_4.0.5_amd64.deb/download.deb
    sudo dpkg -i rmr-dev_4.0.5_amd64.deb
