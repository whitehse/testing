#!/usr/bin/bash

gcc -I. -o sodium.o sodium.c -c -I.
gcc -I. -o unix_server.o unix_server.c -c -I.
gcc -I. -o curl.o curl.c -c -I.
gcc -I. -o iou.o iou.c -c -I.
gcc -I. -o eve eve.c sodium.o curl.o unix_server.o iou.o -lcurl -lassh -lev -luring -lsodium -lcbor -lcjson -L/usr/local/lib/ -lwebsockets
gcc -I /usr/local/include -o echo-client echo-client.c -lsodium -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -I /usr/local/include -o unix-echo-client unix-echo-client.c -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -o create_keys create-keys.c -l sodium
gcc -o tui_client tui-client.c -lcurses -lev

#gcc -o eve eve.c unix_server.o sodium.o curl.o -I. -lcurl -lev -lassh -lsodium -lcbor -lcjson -lwebsockets -luring
