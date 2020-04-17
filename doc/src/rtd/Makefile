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


%.rst:: %.xfm	man_list.im
	OUTPUT_TYPE=rst tfm $< | sed 's/^ //' >$@

%.ps:: %.xfm	man_list.im
	OUTPUT_TYPE=ps pfm $< $@

%.txt:: %.xfm	man_list.im
	OUTPUT_TYPE=txt tfm $< $@

%.md:: %.xfm	man_list.im
	OUTPUT_TYPE=markdown tfm $<  | sed 's/^ //' >$@

docs = config-deploy developer-guide user-guide rel-notes overview

all:: $(docs:%=%.rst) $(docs:%=%.txt) $(docs:%=%.md)

man_list.im:: always
	bash gen_man_list.sh >man_list.im

rel-notes.xfm:: always
	ksh fmt_changes.ksh >rel-notes.xfm


# we force the docs to always be out of date so that we don't have to
# manage the list of man pages and other files that are read to generate the
# output needed for RTD.
#
$(docs:%=%.rst): always
$(docs:%=%.txt): always
$(docs:%=%.md): always


# copy the .rst files which have changed into the docs (plural) directory at the root of the repo
publish : $(docs:%=%.rst)
	bash publish_man.sh;\
	bash publish_lib.sh;\
	for f in *.rst;\
	do\
		if ! diff -N -q $$f ../../../docs/$$f >/dev/null 2>&1;\
		then\
			cp -p $$f ../../../docs/;\
		fi;\
	done

# just list what would be published
verify : $(docs:%=%.rst)
	for f in *.rst;\
	do\
		if ! diff -N -q $$f ../../../docs/$$f >/dev/null 2>&1;\
		then\
			echo "$$f would be published";\
		fi;\
	done

test:
	bash publish_lib.sh;

# ditch any intermediate files
clean:
	rm -f rel-notes.xfm *.sp *.ca

# ditch anything that can be rebuilt
nuke: clean
	rm -f *.ps *.pdf *.rst *.md *.txt

# make hack to force a rule to always be out of date
always:
