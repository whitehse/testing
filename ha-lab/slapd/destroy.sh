#!/bin/bash

sudo lxc-stop -n slapd-1
sudo lxc-destroy -n slapd-1
sudo lxc-stop -n slapd-2
sudo lxc-destroy -n slapd-2
