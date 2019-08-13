rmr-python
===========

Please see `docs/source/index.rst`

Generating Documentation
========================

To generate `rmr-python` documentation:

::

    # from the root of rmr-python, not here
    docker build -t rmrpythondocs:latest -f Dockerfile-DocGen  .
    docker run -v ~/desired/target/dir/:/tmp/docs/build/text rmrpythondocs:latest

After running this, `/desired/target/dir/` will contain `index.rst` and `module_api.txt`
