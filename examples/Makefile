# The demo programmes assume that RMr (along with nng) is installed
# It may be necessary to set LD_LIBRARY_PATH=/usr/local/lib


.PHONY: all
all: sender receiver

receiver: receiver.c
        gcc $< -g -o $@ -lrmr_nng -lnng -lpthread -lm

sender: sender.c
        gcc $< -g -o $@ -lrmr_nng -lnng -lpthread -lm


