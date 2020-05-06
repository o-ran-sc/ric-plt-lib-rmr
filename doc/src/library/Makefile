#=================================================================================2
#	Copyright (c) 2019-2020 Nokia
#	Copyright (c) 2018-2020 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#==================================================================================

XP = XFM_PATH=".:.."
P1 = $(XP) XFM_PASS=1
P2 = $(XP) XFM_PASS=2

# ----- generics  --------------------------------------------------------
# two pass builds in order to forward reference variables for figures and to insert
# table of contents in postscript output
#
%.ps: %.xfm
	$(P1) pfm $< /dev/null
	$(P2) pfm $< $@

%.md: %.xfm
	$(P1) OUTPUT_TYPE=markdown tfm $< /dev/null
	$(P2) OUTPUT_TYPE=markdown tfm $< stdout | sed 's/^ //' >$@

%.rst: %.xfm
	$(P1) OUTPUT_TYPE=rst tfm $< /dev/null
	$(P2) OUTPUT_TYPE=rst tfm $< stdout | sed 's/^ //' >$@

%.txt: %.xfm
	$(P1) OUTPUT_TYPE=txt tfm  $< /dev/null
	$(P2) OUTPUT_TYPE=txt tfm  $< $@

%.html: %.xfm
	$(P1) OUTPUT_TYPE=html hfm  $< /dev/null
	$(P2) OUTPUT_TYPE=html hfm  $< $@

%.pdf: %.ps
	gs -dQUIET -dBATCH -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=$@ $<


# ----- lists  --------------------------------------------------------
# source common to all documents
common_src = index_snatch.im setup.im mbuf.im front_junk.im license.im

user_src = user.xfm advanced_use.im api_qref.im code_appendix.im code_send.im \
	config.im failures.im general_use.im glossary.im

# user guide code examples
code_src = $(shell ls code_*.im)

rt_src = rt_tables.xfm

# output types for different builds
all_types = pdf md rst txt html
rtd_types = rst


# ------ rules --------------------------------------------------------
all:	$(all_types:%=user.%) $(all_types:%=rt_tables.%)
	mv user.rst user-guide.rst

# just the things which are pushed to the docs scraper dirctory,
# renames to expected filenames where required.
docs:	$(rtd_types:%=user.%) $(rtd_types:%=rt_tables.%)
	mv user.rst user-guide.rst

# dependencies; actual recipes are generic
$(all_types:%=user.%): $(user_src) $(code_src) $(common_src)
$(all_types:%=rt_tables.%): $(rt_src)  $(common_src)


# ---------- housekeeping ----------------------------------------------
# remove only intermeidate junk that might straggle; leave output
clean:
	rm -fr *.bcnfile *.ca *.ecnfile *.sp *.toc

# Remove anything that can be built
nuke: clean
	rm -fr *.ps *.pdf *.txt *.rst *.md *.html

