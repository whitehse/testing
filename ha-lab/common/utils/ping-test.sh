#!/bin/bash
 
# Do not run this against any hosts, or on any networks, which you do not own.
# Running this against, say, Google may result in your ISP being ICMP throttled
# by the remote, or an intermediate, network. You have been warned.

if [ -z "$1" ]; then
	echo "A host must be provided."
	exit 1
fi

if [ -z "$2" ]; then
	echo "An interval must be provided, in seconds."
	exit 1
fi

# Resolve ARP at the remote network
ping -q -c 2 $1 > /dev/null

ping -q -i $2 $1 &

while [ 1 ]
do
	sleep 1
	kill -SIGQUIT $!
done
