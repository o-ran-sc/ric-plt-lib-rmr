#==================================================================================
#	Copyright (c) 2019 Nokia
#	Copyright (c) 2018-2019 AT&T Intellectual Property.
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

# most things here require a two pass build, but these might be useful

%.ps: %.xfm
	pfm $< $@

%.md: %.xfm
	OUTPUT_TYPE=markdown tfm $< stdout | sed 's/^ //' >$@

%.rst: %.xfm
	OUTPUT_TYPE=rst tfm $< stdout | sed 's/^ //' >$@

%.txt: %.xfm
	OUTPUT_TYPE=txt tfm  $< stdout
	OUTPUT_TYPE=txt tfm  $< $@

%.pdf: %.ps
	gs -dQUIET -dBATCH -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=$@ $<


src = user.xfm advanced_use.im api_qref.im code_appendix.im code_send.im config.im failures.im general_use.im glossary.im index_snatch.im setup.im mbuf.im front_junk.im license.im
code_src = $(shell ls code_*.im)

all:	user.pdf  user.txt user.rst user.md

# two pass build for pfm to allow for toc insertion
user.ps: $(src) $(code_src)
	XFM_PASS=1 pfm $< /dev/null
	XFM_PASS=2 pfm $< $@

user.md: $(src) $(code_src)
	OUTPUT_TYPE=markdown XFM_PASS=1 tfm $< /dev/null
	OUTPUT_TYPE=markdown XFM_PASS=2 tfm $< | sed 's/^ //' >$@

user.rst: $(src) $(code_src)
	OUTPUT_TYPE=rst XFM_PASS=1 tfm $< /dev/null
	GEN_TITLE=1 OUTPUT_TYPE=rst XFM_PASS=2 tfm $< | sed 's/^ //' >$@

user.txt: $(src) $(code_src)
	OUTPUT_TYPE=txt XFM_PASS=1 tfm $< /dev/null
	OUTPUT_TYPE=txt XFM_PASS=2 tfm $< $@

# intermeidate junk that might straggle
clean:
	rm -fr *.bcnfile *.ca *.ecnfile

# Anything that can be built
nuke: clean
	rm -fr *.ps *.pdf *.txt *.rst

