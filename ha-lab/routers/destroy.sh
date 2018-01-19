#!/bin/bash

sudo lxc-stop -n router-a
sudo lxc-destroy -n router-a
sudo lxc-stop -n router-b
sudo lxc-destroy -n router-b
