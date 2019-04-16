

CC = gcc
coverage_opts = -ftest-coverage -fprofile-arcs

#libs = ../build/librmr_nng.a -L ../build/lib -lnng -lpthread -lm
libs =  -L ../build/lib -lnng -lpthread -lm

#sa_tests = sa_tools_test.o

%.o:: %.c
	$(CC) -g $< -c 

%:: %.c
	$(CC) -I ../src/common/src/ -I ../src/common/include -I ../src/nng/include $(coverage_opts) -fPIC -g $< -o $@  $(libs)

# catch all 
all:
	echo "run unit_test.ksh to make and run things here"


#sa_tools_test.o: sa_tools_test.c
#	$(CC) -I ../src/common/src/ -I ../src/common/include -I $(coverage_opts) ../src/nng/include  -fPIC -g $< -c 

#.PHONY: sa_tests
#sa_tests:	$(sa_tests)

# remove intermediates
clean:
	rm -f *.gcov *.gcda *.dcov *.gcno

# remove anything that can be builts
nuke: clean
	rm -f ring_test symtab_test 
