#!/bin/sh

CONTAINER=$1

PID=`cat /sys/fs/cgroup/memory/${CONTAINER}/tasks | tail -1`
sudo nsenter --target $PID -m -u -i -n -p
