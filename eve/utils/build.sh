#!/usr/bin/bash
gcc -I /usr/local/include -o echo-client echo-client.c -lsodium -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -I /usr/local/include -o echo-server echo-server.c -lsodium -L /usr/local/lib/ -lcbor -lcjson -lm
