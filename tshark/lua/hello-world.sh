#!/bin/bash

tshark -X lua_script:hello-world.lua -c 0
