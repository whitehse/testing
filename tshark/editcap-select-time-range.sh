#!/bin/bash

if [ $# -ne 2 ]; then
    echo "No arguments provided"
    exit 1
fi

editcap -A "2008-01-17 11:40:00" -B "2008-01-17 11:49:59" "$1" "$2"
