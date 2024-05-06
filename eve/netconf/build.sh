#!/usr/bin/bash
gcc -I. -o netconf netconf.c -lcurl -lassh -lev -luring -lsodium -lcbor -lcjson -L/usr/local/lib/ -lwebsockets

gcc -I /usr/local/include -o echo-client echo-client.c -lsodium -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -o create_keys create-keys.c -l sodium
