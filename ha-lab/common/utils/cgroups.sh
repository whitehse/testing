#!/bin/bash

sudo sh -c 'echo 1 > /sys/fs/cgroup/cpuset/cgroup.clone_children'
sudo sh -c 'echo 1 > /proc/sys/kernel/unprivileged_userns_clone'
