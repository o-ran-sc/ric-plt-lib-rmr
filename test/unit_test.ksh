#!/usr/bin/env ksh
# this has been hacked to work with bash; ksh is preferred

#==================================================================================
#        Copyright (c) 2019 Nokia
#        Copyright (c) 2018-2019 AT&T Intellectual Property.
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


#
#	Mnemonic:	unit_test.ksh
#	Abstract:	Execute unit test(s) in the directory and produce a more
#				meaningful summary than gcov gives by default (exclude
#				coverage  on the unit test functions).
#
#				Test files must be named *_test.c, or must explicitly be
#				supplied on the command line. Functions in the test
#				files will not be reported on provided that they have
#				their prototype (all on the SAME line) as:
#					static type name() {
#
#				Functions with coverage less than 80% will be reported as
#				[LOW] in the output.  A file is considered to pass if the
#				overall execution percentage for the file is >= 80% regardless
#				of the number of functions that reported low.
#
#				Test programmes are built prior to execution. Plan-9 mk is
#				the preferred builder, but as it's not widly adopted (sigh)
#				make is assumed and -M will shift to Plan-9. Use -C xxx to
#				invoke a customised builder.
#
#				For a module which does not pass, we will attempt to boost
#				the coverage by discounting the unexecuted lines which are
#				inside of if() statements that are checking return from
#				(m)alloc() calls or are checking for nil pointers as these
#				cases are likely impossible to drive. When discount testing
#				is done both the failure message from the original analysis
#				and a pass/fail message from the discount test are listed,
#				but only the result of the discount test is taken into
#				consideration with regard to overall success.
#
#				Overall Pass/Fail
#				By default the overall state is based only on the success
#				or failure of the unit tests and NOT on the perceived
#				state of coverage.  If the -s (strict) option is given, then
#				overall state will be failure if code coverage expectations
#				are not met.
#
#	Date:		16 January 2018
#	Author:		E. Scott Daniels
# -------------------------------------------------------------------------

function usage {
	echo "usage: $0 [-G|-M|-C custom-command-string] [-a] [-c cov-target]  [-f] [-F] [-v] [-x]  [files]"
	echo "  if -C is used to provide a custom build command then it must "
	echo "  contain a %s which will be replaced with the unit test file name."
	echo '  e.g.:  -C "mk -a %s"'
	echo "  -a always run coverage (even on failed modules)"
	echo "  -c allows user to set the target coverage for a module to pass; default is 80"
	echo "  -f forces a discount check (normally done only if coverage < target)"
	echo "  -F show only failures at the function level"
	echo "  -Q turns off quiet mode. Quiet mode (default) is less chatty about intermediate"
	echo "     coverage results and test programme output when failures detected"
	echo "  -q enable quiet mode (default, so this is no longer needed)"
	echo "  -s strict mode; code coverage must also pass to result in a good exit code"
	echo "  -v will write additional information to the tty and save the disccounted file if discount run or -f given"
	echo "  -x generates the coverage XML files for Sonar (implies -f)"
}

# read through the given file and add any functions that are static to the
# ignored list.  Only test and test tools files should be parsed.
#
function add_ignored_func {
	if [[ ! -r $1 ]]
	then
		echo ">>>> can't find file to ignore: $1"
		return
	fi

	typeset f=""
	goop=$(
		grep "^static.*(.*).*{" $1 | awk '		# get list of test functions to ignore
			{
				gsub( "[(].*", "" )
				gsub( "[*]", "" )
				if( $2 == "struct" ) {			# static struct goober function
					printf( "%s ", $4 )
				} else {
					printf( "%s ", $3 )			# static goober-type funct
				}
			}
		' )

	iflist="$iflist $goop"			# this goop hack because bash can't read from a loop
}


# Merge two coverage files to preserve the total lines covered by different
# test programmes.
#
function merge_cov {
	if [[ -z $1 || -z $2 ]]
	then
		return
	fi

	if [[ ! -e $1 || ! -e $2 ]]
	then
		return
	fi

	(
		cat $1
		echo "==merge=="
		cat $2
	) | awk '
		/^==merge==/ {
			merge = 1
			next
		}

		merge && /#####:/ {
			line = $2+0
			if( executed[line] ) {
				$1 = sprintf( "%9d:", executed[line] )
			}
		}

		merge {
			print
			next
		}

		{
			line = $2+0
			if( $1+0 > 0 ) {
				executed[line] = $1+0
			}
		}
	'
}

#
#	Parse the .gcov file and discount any unexecuted lines which are in if()
#	blocks that are testing the result of alloc/malloc calls, or testing for
#	nil pointers.  The feeling is that these might not be possible to drive
#	and shoudn't contribute to coverage deficiencies.
#
#	In verbose mode, the .gcov file is written to stdout and any unexecuted
#	line which is discounted is marked with ===== replacing the ##### marking
#	that gcov wrote.
#
#	The return value is 0 for pass; non-zero for fail.
function discount_an_checks {
	typeset f="$1"

	mct=$( get_mct ${1%.gcov} )			# see if a special coverage target is defined for this

	if [[ ! -f $1 ]]
	then
		if [[ -f ${1##*/} ]]
		then
			f=${1##*/}
		else
			echo "cant find: $f"
			return
		fi
	fi

	awk -v module_cov_target=$mct \
		-v cfail=${cfail:-WARN} \
		-v show_all=$show_all \
		-v full_name="${1}"  \
		-v module="${f%.*}"  \
		-v chatty=$chatty \
		-v replace_flags=$replace_flags \
	'
	function spit_line( ) {
		if( chatty ) {
			printf( "%s\n", $0 )
		}
	}

	/-:/ {				# skip unexecutable lines
		spit_line()
		seq++					# allow blank lines in a sequence group
		next
	}

	{
		nexec++			# number of executable lines
	}

	/#####:/ {
		unexec++;
		if( $2+0 != seq+1 ) {
			prev_malloc = 0
			prev_if = 0
			seq = 0
			spit_line()
			next
		}

		if( prev_if && prev_malloc ) {
			if( prev_malloc ) {
				#printf( "allow discount: %s\n", $0 )
				if( replace_flags ) {
					gsub( "#####", "    1", $0 )
				}
				discount++;
			}
		}

		seq++;;
		spit_line()
		next;
	}

	/if[(].*alloc.*{/ {			# if( (x = malloc( ... )) != NULL ) or if( (p = sym_alloc(...)) != NULL )
		seq = $2+0
		prev_malloc = 1
		prev_if = 1
		spit_line()
		next
	}

	/if[(].* == NULL/ {				# a nil check likely not easily forced if it wasnt driven
		prev_malloc = 1
		prev_if = 1
		spit_line()
		seq = $2+0
		next
	}

	/if[(]/ {
		if( seq+1 == $2+0 && prev_malloc ) {		// malloc on previous line
			prev_if = 1
		} else {
			prev_malloc = 0
			prev_if = 0
		}
		spit_line()
		next
	}

	/alloc[(]/ {
		seq = $2+0
		prev_malloc = 1
		spit_line()
		next
	}

	{
		spit_line()
	}

	END {
		net = unexec - discount
		orig_cov = ((nexec-unexec)/nexec)*100		# original coverage
		adj_cov = ((nexec-net)/nexec)*100			# coverage after discount
		pass_fail = adj_cov < module_cov_target ? cfail : "PASS"
		rc = adj_cov < module_cov_target ? 1 : 0
		if( pass_fail == cfail || show_all ) {
			if( chatty ) {
				printf( "[%s] %s executable=%d unexecuted=%d discounted=%d net_unex=%d  cov=%d%% ==> %d%%  target=%d%%\n",
					pass_fail, full_name ? full_name : module, nexec, unexec, discount, net, orig_cov, adj_cov, module_cov_target )
			} else {
				printf( "[%s] %d%% (%d%%) %s\n", pass_fail, adj_cov, orig_cov, full_name ? full_name : module )
			}
		}

		exit( rc )
	}
	' $f
}

# Given a file name ($1) see if it is in the ./.targets file. If it is
# return the coverage listed, else return (echo)  the default $module_cov_target
#
function get_mct {
	typeset v=$module_cov_target

	if [[ -f ./.targets ]]
	then
		grep "^$1 " ./.targets | head -1 | read stuff
		tv="${stuff##* }"					# assume junk tv; ditch junk
	fi

	echo ${tv:-$v}
}

# Remove unneeded coverage files, then generate the xml files that can be given
# to sonar.  gcov.xml is based on the "raw" coverage and dcov.xml is based on
# the discounted coverage.
#
function mk_xml {
	rm -fr *_test.c.gcov test_*.c.gcov *_test.c.dcov test_*.c.dcov		# we don't report on the unit test code, so ditch
	cat *.gcov | cov2xml.ksh >gcov.xml
	cat *.dcov | cov2xml.ksh >dcov.xml
}


# -----------------------------------------------------------------------------------------------------------------

if [[ -z $BUILD_PATH ]]
then

	# we assume that the project has been built in the ../[.]build directory
	if [[ -d ../build ]]
	then
		export BUILD_PATH=../build
	else
		if [[ -d ../.build ]]
		then
			export BUILD_PATH=../.build
		else
			echo "[WARN] cannot find build directory (tried ../build and ../.build); things might not work"
			echo ""
		fi
	fi
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$BUILD_PATH/lib:$BUILD_PATH/lib64
export C_INCLUDE_PATH=$C_INCLUDE_PATH:$BUILD_PATH/include
export LIBRARY_PATH=$LD_LIBRARY_PATH

# The Makefile sets specific includes for things
#export C_INCLUDE_PATH="../src/rmr/common/include:../src/rmr/si/include:$C_INCLUDE_PATH"

module_cov_target=80
builder="make -B %s"							# default to plain ole make
verbose=0
show_all=1										# show all things -F sets to show failures only
strict=0										# -s (strict) will set; when off, coverage state ignored in final pass/fail
show_output=0									# show output from each test execution (-S)
quiet=1											# less chatty with result output (only overall coverage, no intermediate coverage) -Q turns off
gen_xml=0
replace_flags=1									# replace ##### in gcov for discounted lines
run_nano_tests=0								# can nolonger be turned on
run_nng_tests=0									# -N will enable
always_gcov=0									# -a sets to always run gcov even if failure
save_gcov=1										# -o turns this off
out_dir=${UT_COVERAGE_DIR:-/tmp/rmr_gcov}		# -O changes output directory

export RMR_WARNING=1							# turn on warnings

ulimit -c unlimited

while [[ $1 == "-"* ]]
do
	case $1 in
		-C)	builder="$2"; shift;;		# custom build command
		-G)	builder="gmake %s";;
		-M)	builder="mk -a %s";;		# use plan-9 mk (better, but sadly not widly used)
		-N)	run_nng_tests=1;;
		-O)	out_dir=$2; shift;;

		-a)	always_gcov=1;;
		-c)	module_cov_target=$2; shift;;
		-e)	capture_file=$2; >$capture_file; shift;;		# capture errors from failed tests rather than spewing on tty
		-f)	force_discounting=1;
			trigger_discount_str="WARN|FAIL|PASS"		# check all outcomes for each module
			;;

		-F)	show_all=0;;

		-n)	noexec=1;;
		-o)	save_gcov=0;;
		-s)	strict=1;;					# coverage counts toward pass/fail state
		-S)	show_output=1;;				# test output shown even on success
		-v)	(( verbose++ ));;
		-q)	quiet=1;;					# less chatty when spilling error log files
		-Q)	quiet=0;;					# disable quiet mode
		-x)	gen_xml=1
			force_discounting=1
			trigger_discount_str="WARN|FAIL|PASS"		# check all outcomes for each module
			rm -fr *cov.xml
			;;


		-h)		usage; exit 0;;
		--help) usage; exit 0;;
		-\?)	usage; exit 0;;

		*)	echo "unrecognised option: $1" >&2
			usage >&2
			exit 1
			;;
	esac

	shift
done


if (( strict ))			# if in strict mode, coverage shortcomings are failures
then
	cfail="FAIL"
else
	cfail="WARN"
fi
if [[ -z $trigger_discount_str ]]
then
	trigger_discount_str="$cfail"
fi


if [[ -z $1 ]]
then
	flist=""
	for tfile in *_test.c
	do
		if [[ $tfile != *"static_test.c" ]]
		then
			if (( ! run_nng_tests )) && [[ $tfile == *"nng"* ]]		# drop any nng file unless -N given
			then
				continue
			fi
			if [[ $tfile == *"nano"* ]]			# no longer support nano tests; drop regardless
			then
				continue
			fi

			if (( ! quiet ))
			then
				echo "<INFO> add test: $tfile" >&2
			fi
			flist="${flist}$tfile "
		fi
	done
else
	flist="$@"
fi

if (( noexec ))
then
	echo "no exec mode; would test these:"
	for tf in $flist
	do
		echo "	$tf"
	done
	exit 0
fi

rm -fr *.gcov			# ditch the previous coverage files
ut_errors=0			# unit test errors (not coverage errors)
errors=0

if ! touch /tmp/PPID$$.noise
then
	echo "<ERR> unable to write to /tmp???"
fi

for tfile in $flist
do
	for x in *.gcov
	do
		if [[ -e $x ]]
		then
			cp $x $x-
		fi
	done

	echo "$tfile --------------------------------------"
	(	# all noise is now captured into a tmp file to support quiet mode
		bcmd=$( printf "$builder" "${tfile%.c}" )
		if ! $bcmd >/tmp/PID$$.log 2>&1
		then
			echo "[FAIL] cannot build $tfile"
			cat /tmp/PID$$.log
			# do NOT remove tmp files; bash seens to not gen a new PID for subshells
			exit 1
		fi

		iflist="main sig_clean_exit "		# ignore external functions from our tools
		add_ignored_func $tfile				# ignore all static functions in our test driver
		add_ignored_func test_support.c		# ignore all static functions in our test tools
		add_ignored_func test_nng_em.c		# the nng/nano emulated things
		add_ignored_func test_si95_em.c		# the si emulated things
		add_ignored_func test_common_em.c	# the common emulation functions
		for f in *_static_test.c			# all static modules here
		do
			if(( ! run_nano_tests )) && [[ $f == *"nano"* ]]
			then
				continue
			fi

			add_ignored_func $f
		done

		if ! ./${tfile%.c} >/tmp/PID$$.log 2>&1
		then
			echo "[FAIL] unit test failed for: $tfile"
			if [[ -n $capture_file ]]
			then
				echo "all errors captured in $capture_file, listing only fail message on tty"
				echo "$tfile --------------------------------------" >>$capture_file
				cat /tmp/PID$$.log >>$capture_file
				grep "^<FAIL>" /tmp/PID$$.log
				echo ""
			else
				if (( quiet ))
				then
					grep "^<" /tmp/PID$$.log|egrep -v "^<SIEM>|^<EM>"	# in quiet mode just dump <...> messages which are assumed from the test programme not appl
				else
					cat /tmp/PID$$.log
				fi
			fi
			(( ut_errors++ ))				# cause failure even if not in strict mode
			if (( ! always_gcov ))
			then
				exit 1						# we are in a subshell, must exit bad
			fi
		else
			if (( show_output ))
			then
				printf "\n============= test programme output =======================\n"
				cat /tmp/PID$$.log
				printf "===========================================================\n"
			else
				grep "SUMMARY" /tmp/PID$$.log
			fi
		fi

		(
			touch ./.targets
			sed '/^#/ d; /^$/ d; s/^/TARGET: /' ./.targets
			gcov -f ${tfile%.c} | sed "s/'//g"
		) | awk \
			-v cfail=$cfail \
			-v show_all=$show_all \
			-v ignore_list="$iflist" \
			-v module_cov_target=$module_cov_target \
			-v chatty=$verbose \
			'
			BEGIN {
				announce_target = 1;
				nignore = split( ignore_list, ignore, " " )
				for( i = 1; i <= nignore; i++ ) {
					imap[ignore[i]] = 1
				}

				exit_code = 0		# assume good
			}

			/^TARGET:/ {
				if( NF > 1 ) {
					target[$2] = $NF
				}
				next;
			}

			/File.*_test/ || /File.*test_/ {		# dont report on test files
				skip = 1
				file = 1
				fname = $2
				next
			}

			/File/ {
				skip = 0
				file = 1
				fname = $2
				next
			}

			/Function/ {
				fname = $2
				file = 0
				if( imap[fname] ) {
					fname = "skipped: " fname		# should never see and make it smell if we do
					skip = 1
				} else {
					skip = 0
				}
				next
			}

			skip { next }

			/Lines executed/ {
				split( $0, a, ":" )
				pct = a[2]+0

				if( file ) {
					if( announce_target ) {				# announce default once at start
						announce_target = 0;
						printf( "\n[INFO] default target coverage for modules is %d%%\n", module_cov_target )
					}

					if( target[fname] ) {
						mct = target[fname]
						announce_target = 1;
					} else {
						mct = module_cov_target
					}

					if( announce_target ) {					# annoucne for module if different from default
						printf( "[INFO] target coverage for %s is %d%%\n", fname, mct )
					}

					if( pct < mct ) {
						printf( "[%s] %3d%% %s\n", cfail, pct, fname )	# CAUTION: write only 3 things  here
						exit_code = 1
					} else {
						printf( "[PASS] %3d%% %s\n", pct, fname )
					}

					announce_target = 0;
				} else {
					if( pct < 70 ) {
						printf( "[LOW]  %3d%% %s\n", pct, fname )
					} else {
						if( pct < 80 ) {
							printf( "[MARG] %3d%% %s\n", pct, fname )
						} else {
							if( show_all ) {
								printf( "[OK]   %3d%% %s\n", pct, fname )
							}
						}
					}
				}
			}

			END {
				printf( "\n" );
				exit( exit_code )
			}
		' >/tmp/PID$$.log					# capture output to run discount on failures
		rc=$?
		cat /tmp/PID$$.log

		if (( rc  || force_discounting ))	# didn't pass, or forcing, see if discounting helps
		then
			if (( ! verbose ))		# if verbose is on we don't need this (! is not a mistake!!)
			then
				echo "[INFO] checking to see if discounting improves coverage for failures listed above"
			fi

			# preferred, but breaks under bash
			#egrep "$trigger_discount_str"  /tmp/PID$$.log | while read state junk  name
			egrep "$trigger_discount_str"  /tmp/PID$$.log | while read stuff
			do
				set stuff			# this hack required because bash cant read into mult vars
				state="$1"
				name="$3"

				if ! discount_an_checks $name.gcov >/tmp/PID$$.disc
				then
					(( errors++ ))
				fi

				tail -1 /tmp/PID$$.disc | grep '\['

				if (( verbose > 1 ))			# updated file was generated, keep here
				then
					echo "[INFO] discounted coverage info in: ${tfile##*/}.dcov"
				fi

				mv /tmp/PID$$.disc ${name##*/}.dcov
			done
		fi
	)>/tmp/PID$$.noise 2>&1
	if (( $? != 0 ))
	then
		(( ut_errors++ ))
		cat /tmp/PID$$.noise
		continue
	fi

	for x in *.gcov							# merge any previous coverage file with this one
	do
		if [[ -e $x && -e $x- ]]
		then
			merge_cov $x $x- >/tmp/PID$$.mc
			cp /tmp/PID$$.mc $x
			rm $x-
		fi
	done

	if (( ! quiet ))
	then
		cat /tmp/PID$$.noise
	else
		grep "SUMMARY" /tmp/PID$$.noise
	fi
done

echo ""
echo "[INFO] final discount checks on merged gcov files"
show_all=1
for xx in *.gcov
do
	if [[ $xx != *"test"* ]]
	then
		of=${xx%.gcov}.dcov
		discount_an_checks $xx  >$of
		if [[ -n $of ]]
		then
			tail -1 $of |  grep '\['
		fi
	fi
done

if (( save_gcov ))
then
	echo ""
	ok=1
	if [[ ! -d $outdir ]]
	then
		if ! mkdir -p $out_dir
		then
			echo "[WARN] unable to save .gcov files in $out_dir"
			ok=0
		fi
	fi

	if (( ok ))
	then
		rm -fr $out_dir/*
		echo "[INFO] gcov files saved in $out_dir for push to remote system(s)"
		cp *.gcov $out_dir/
		rm -f $out_dir/*_test.c.gcov $out_dir/test_*.c.gcov
		rm -f ./*_test.c.gcov ./test_*.c.gcov
	fi
else
	echo "[INFO] .gcov files were not saved for remote system"
fi

state=0						# final state
rm -f /tmp/PID$$.*
if (( strict ))				# fail if some coverage failed too
then
	if (( errors + ut_errors ))
	then
		state=1
	fi
else						# not strict; fail only if unit tests themselves failed
	if (( ut_errors ))
	then
		state=1
	fi
fi

# finally run any "vetters" which run programmes and analyse the output
echo "[INFO] running vetters...."
if ! make vet
then
	echo "[FAIL] one or more vetters failed"
	state=1
else
	echo "[INFO] vetters all passed"
fi

echo""
if (( state ))
then
	echo "[FAIL] overall unit testing fails: coverage errors=$errors   unit test errors=$ut_errors"
else
	echo "[PASS] overall unit testing passes"
	if (( gen_xml ))
	then
		mk_xml
	fi
fi

exit $state

