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

XP = XFM_PATH=".:.."
P1 = $(XP) XFM_PASS=1
P2 = $(XP) XFM_PASS=2

# two pass builds in order to forward reference variables for figures and to insert
# table of contents in postscript output
#
#%.ps: %.xfm
#	pfm $< $@
%.ps: %.xfm
	$(P1) pfm $< /dev/null
	$(P2) pfm $< $@

%.md: %.xfm
	$(P1) OUTPUT_TYPE=markdown tfm $< /dev/null
	$(P2) OUTPUT_TYPE=markdown tfm $< stdout | sed 's/^ //' >$@

#%.rst: %.xfm
#	OUTPUT_TYPE=rst tfm $< stdout | sed 's/^ //' >$@
%.rst: %.xfm
	$(P1) OUTPUT_TYPE=rst tfm $< /dev/null
	$(P2) OUTPUT_TYPE=rst tfm $< stdout | sed 's/^ //' >$@

%.txt: %.xfm
	$(P1) OUTPUT_TYPE=txt tfm  $< /dev/null
	$(P2) OUTPUT_TYPE=txt tfm  $< $@

%.pdf: %.ps
	gs -dQUIET -dBATCH -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=$@ $<


src = user.xfm advanced_use.im api_qref.im code_appendix.im code_send.im config.im failures.im general_use.im glossary.im index_snatch.im setup.im mbuf.im front_junk.im license.im
code_src = $(shell ls code_*.im)

all:	user.pdf  user.txt user.rst user.md

# two pass build for pfm to allow for toc insertion
user.ps: $(src) $(code_src)
	$(P1) pfm $< /dev/null
	$(P2) pfm $< $@

user.md: $(src) $(code_src)
	$(P1) OUTPUT_TYPE=markdown tfm $< /dev/null
	$(P2) OUTPUT_TYPE=markdown tfm $< | sed 's/^ //' >$@

user.rst: $(src) $(code_src)
	$(P1) OUTPUT_TYPE=rst tfm $< /dev/null
	$(P2) OUTPUT_TYPE=rst GEN_TITLE=1 tfm $< | sed 's/^ //' >$@

user.txt: $(src) $(code_src)
	$(P1) OUTPUT_TYPE=txt tfm $< /dev/null
	$(P2) OUTPUT_TYPE=txt tfm $< $@

# intermeidate junk that might straggle
clean:
	rm -fr *.bcnfile *.ca *.ecnfile

# Anything that can be built
nuke: clean
	rm -fr *.ps *.pdf *.txt *.rst

