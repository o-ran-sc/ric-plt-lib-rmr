# vim: ts=4 noet sw=4:
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

# This is a stand-alone make file as the RTD documentation is NOT built and packaged
# with the normal CMake process and thus isn't covered in the CMake files.

# {X}fm is required to compile the source. 


%.rst:: %.xfm
	OUTPUT_TYPE=rst tfm $< | sed 's/^ //' >$@

%.ps:: %.xfm
	OUTPUT_TYPE=ps pfm $< $@

%.txt:: %.xfm
	OUTPUT_TYPE=txt tfm $< $@

%.md:: %.xfm
	OUTPUT_TYPE=markdown tfm $<  | sed 's/^ //' >$@

docs = config-deploy developer-guide user-guide rel-notes overview

all:: $(docs:%=%.rst) $(docs:%=%.txt) $(docs:%=%.md) 

rel-notes.xfm:
	ksh fmt_changes.ksh >rel-notes.xfm

# copy the .rst files which have changed into the docs (plural) directory at the root of the repo
publish : $(docs:%=%.rst)
	for f in *.rst;\
	do\
		if ! diff -N -q $$f ../../../docs/$$f >/dev/null 2>&1;\
		then\
			cp -p $$f ../../../docs/;\
		fi;\
	done

# ditch any intermediate files
clean:
	rm -f rel-notes.xfm *.sp *.ca

# ditch anything that can be rebuilt
nuke: clean
	rm -f *.ps *.pdf *.rst *.md

