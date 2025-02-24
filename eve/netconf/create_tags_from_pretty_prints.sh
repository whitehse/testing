#!/usr/bin/bash

grep -v '<\/' | grep '<[a-zA-Z0-9_\-]\+' | sed 's/.*<\([a-zA-Z0-9_\-]\+\).*/\1/' | sort | uniq <&0
