# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [0.10.2] - 7/31/2019
    * Include license here per Jira RICPLT-1855

## [0.10.0] - 5/15/2019
    * Fix a bug in rmr mock that prevented it for being used for rmr_rcv (was only usable for rmr_torcv)
    * Add more unit tests, esp for message summary
    * Remove meid truncation in the case where a nil is present mid string
    * Change the defaul mock of meid and get_src to something more useful

## [0.9.0] - 5/13/2019
    * Add a new module for mocking out rmr-python, useful for other packages that depend on rmr-python

## [0.8.4] - 5/10/2019
    * Add some unit tests; more to come

## [0.8.3] - 5/8/2019
    * Better loop indexing in meid string handling

## [0.8.2] - 5/8/2019
    * Fix examples bug
    * add liscneses for LF push

## [0.8.1] - 5/7/2019
    * Better andling of meid in message summary

## [0.8.0] - 5/7/2019
    * Refactor some code to be more functional
    * Put back RMR_MAX_RCV_BYTES as a constant
    * Add tox.ini, although right now it only LINTs

## [0.7.0] - 5/6/2019
    * Add constant fetching from RMr library

## [0.6.0] - 5/6/2019
    * Add a new field to rmr_mbuf_t: sub_id
    * Fix prior commits lint-ailing python style

## [0.5.0] - 5/3/2019
    * Add errno access via new function: rmr.errno()
    * Add new functions to access new RMr header fields: get_src, get_meid, rmr_bytes2meid
    * Add new RMr constants for error states

## [0.4.1] - 4/8/2019
    * Fix a non-ascii encoding issue

## [0.4.0] - 3/28/2019
    * Greatly imroved test sender/receiver
    * Three new functions implemented (rmr_close, rmr_set_stimeout, rmr_payload_size)

## [0.3.0] - 3/26/2019
    * Support a new receive function that (hurray!) has a timeout

## [0.2.1] - 3/25/2019
    * Add two new MR states

## [0.2.0] - 3/25/2019
    * Switch to NNG from nanomessage

## [0.1.0] - 3/14/2019
    * Initial Creation
