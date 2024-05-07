#!/usr/bin/bash

gcc -o eve eve.c -I /usr/local/include -L /usr/local/lib -l websockets -l ev
