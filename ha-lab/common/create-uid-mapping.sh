#!/bin/bash

if [ -f ~/.config/lxc/default.conf ]; then
	echo '~/.config/lxc/default.conf exists. Exiting'
	exit 1
fi

if [ ! -d ~/.config/lxc ]; then
	mkdir -p ~/.config/lxc
fi

cat > ~/.config/lxc/default.conf << EOF
lxc.include = /etc/lxc/default.conf
lxc.id_map = u 0 1279648 65536
lxc.id_map = g 0 1279648 65536
EOF
