#!/usr/bin/bash

gcc -I. -o eve sodium.c curl.c unix_server.c eve.c -lcurl -lassh -lev -luring -lsodium -lcbor -lcjson -L/usr/local/lib/ -lwebsockets
gcc -I /usr/local/include -o echo-client echo-client.c -lsodium -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -I /usr/local/include -o unix-echo-client unix-echo-client.c -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -o create_keys create-keys.c -l sodium
gcc -o tui_client tui-client.c -lcurses -lev
