#!/bin/bash

# Uncomment the below if new modules are added; this regenerates .rst files
# For a minimalist approach, I simply edited index.rst with the modules we have now as it's very unlikely to grow
#sphinx-apidoc -f -o source/ ../rmr

make text
