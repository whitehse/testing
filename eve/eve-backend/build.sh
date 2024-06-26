#!/usr/bin/bash

gcc -I. -o sodium.o sodium.c -c -I.
gcc -I. -o unix_server.o unix_server.c -c -I.
gcc -I. -o curl.o curl.c -c -I.
gcc -I. -o iou.o iou.c -c -I.
gcc -I. -o websockets.o websockets.c -c -I. -I/usr/local/include
gcc -I. -o assh.o assh.c -c -I.
gcc -I. -o eve eve.c assh.o sodium.o curl.o unix_server.o iou.o websockets.o -lcurl -lassh -lev -luring -lsodium -lcbor -lcjson -L/usr/local/lib/ -lwebsockets
gcc -I /usr/local/include -o echo-client echo-client.c -lsodium -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -I /usr/local/include -o unix-echo-client unix-echo-client.c -L /usr/local/lib/ -lcbor -lcjson -lm
gcc -o create_keys create-keys.c -l sodium
gcc -o tui_client tui-client.c -lcurses -lev

gcc -o pqsql pqsql.c -I /usr/include/postgresql/ -lpq -lev
#gcc -o eve eve.c unix_server.o sodium.o curl.o -I. -lcurl -lev -lassh -lsodium -lcbor -lcjson -lwebsockets -luring

gcc -o eve_db eve_db.c -I. -lev -luring -lcbor -lcjson

gcc -o echo_client_monocypher third_party/monocypher/src/monocypher.c echo_client_monocypher.c -Ithird_party/monocypher/src -I. -lev
#x86_64-w64-mingw32-gcc -o echo_client_monocypher.exe third_party/monocypher/src/monocypher.c echo_client_monocypher.c -Ithird_party/monocypher/src -I. -I/tmp/libev -lws2_32 -lrt

gcc -o echo_server_monocypher third_party/monocypher/src/monocypher.c echo_server_monocypher.c -Ithird_party/monocypher/src -I. -lev -lsodium
