

CC = gcc
coverage_opts = -ftest-coverage -fprofile-arcs

libs = ../build/librmr_nng.a -L ../build/lib -lnng -lpthread -lm

%.o:: %.c
	$(CC) -g $< -c 

%:: %.c
	$(CC) $(coverage_opts) -fPIC -g $< -o $@  $(libs)

# catch all 
all:
	echo "run unit_test.ksh to make and run things here"

# remove intermediates
clean:
	rm -f *.gcov *.gcda *.dcov *.gcno

# remove anything that can be builts
nuke: clean
	rm -f ring_test symtab_test 
