#!/usr/bin/bash

#./pretty_print_debug_file.pl debug_* 2> /dev/null | grep xmlns | sed 's/.*xmlns=\"\([^\"]\+\)\".*/\1/' | sort | uniq | sed 's/http[^\.]\+\.\([^\.]\+\)\.[^\/]\+\(\/.*\)$/\1\2/' | sed 's/urn:ietf:params:xml:/ietf:/' | sed 's/[\/:]ns//'
./pretty_print_debug_file.pl debug_* 2> /dev/null | grep xmlns | sed 's/.*xmlns=\"\([^\"]\+\)\".*/\1/' | sort | uniq
