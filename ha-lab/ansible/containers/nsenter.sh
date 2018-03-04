#!/bin/sh

PID=$1

sudo nsenter --target $PID -m -u -i -n -p
