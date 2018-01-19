#!/bin/bash

if [ ! -d ~/lxc ]; then
	mkdir -m 700 ~/lxc
fi

if [ ! -f ~/.config/lxc/lxc.conf ]; then
	echo "lxc.lxcpath=/$HOME/lxc" >> ~/.config/lxc/lxc.conf
	exit 0
fi

if grep lxc\.lxcpath ~/.config/lxc/lxc.conf; then
	echo "lxc.lxcpath=$HOME/lxc" >> ~/.config/lxc/lxc.conf
fi
