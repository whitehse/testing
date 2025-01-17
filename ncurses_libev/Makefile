#!/usr/bin/bash

all:
	clang -I/usr/src/linux-headers-6.1.0-29-common/arch/x86/include/uapi/ -I/usr/src/linux-headers-6.1.0-29-amd64/arch/x86/include/generated/uapi/ -O2 -g -Wall -target bpf -c bpf_test.c -o bpf_test.o
	llvm-strip -g bpf_test.o
	/usr/sbin/bpftool gen skeleton bpf_test.o > bpf_test.h
	gcc -o tui_client tui_client.c -lcurses -lev -lbpf
	/usr/bin/arm-linux-gnueabihf-gcc-12 -o tui_client_arm tui_client.c -L/lib/arm-linux-gnueabihf/ -lcurses -ltermcap -lbpf -lelf -lz -static

clean:
	rm tui_client tui_client_arm *.o bpf_test.h
