all:
	clang -I /usr/src/linux-headers-6.1.0-29-common/arch/x86/include/uapi/ -I /usr/src/linux-headers-6.1.0-29-amd64/arch/x86/include/generated/uapi/ -O2 -target bpf -c test_ebpf.c -o test_ebpf.o
clean:
	rm test_ebpf.o
