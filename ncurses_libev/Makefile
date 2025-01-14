#!/usr/bin/bash

all:
	#gcc -o tui_client tui_client.c -lcurses -lev
	/usr/bin/arm-linux-gnueabihf-gcc-12 -o tui_client_arm tui_client.c -L/lib/arm-linux-gnueabihf/ -lcurses -ltermcap -static

clean:
	rm tui_client
