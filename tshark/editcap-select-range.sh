#!/bin/bash

if [ $# -ne 2 ]; then
    echo "No arguments provided"
    exit 1
fi

editcap -r "$1" "$2" 1-1000 2001-3000
