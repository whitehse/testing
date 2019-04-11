#!/bin/bash

tshark -X lua_script:tap.lua -c 25
