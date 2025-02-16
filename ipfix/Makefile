all:
	gcc -I. -o ipfix_server ipfix_server.c -lev -lcjson -lcbor 

prep:
	./process_ipfix_iana.pl > ipfix_iana.h

clean:
	rm ipfix_server
