#!/bin/sh

grep -q LXC_CACHE_PATH ~/.profile

if [ $? -ne 0 ]; then
	cat >> ~/.bashrc << EOF

LXC_CACHE_PROFILE=~/.cache/lxc
EOF

fi

if [ ! -d ~/.cache/lxc ]; then
	mkdir -p ~/.cache/lxc
fi
