#!/bin/bash

if [ -f ~/.config/lxc/default.conf ]; then
	echo '~/.config/lxc/default.conf exists. Exiting'
	exit 1
fi

if [ ! -d ~/.config/lxc ]; then
	mkdir -p ~/.config/lxc
fi

SUBUID=`cat /etc/subuid | grep "^$USER:" | awk -F ':' '{print $2, $3}'`
SUBGID=`cat /etc/subgid | grep "^$USER:" | awk -F ':' '{print $2, $3}'`
cat > ~/.config/lxc/default.conf << EOF
lxc.include = /etc/lxc/default.conf
lxc.id_map = u 0 $SUBUID
lxc.id_map = g 0 $SUBGID
EOF
