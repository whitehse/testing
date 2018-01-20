#!/bin/bash

../common/utils/stop-openldap-server.sh slapd-1
../common/utils/stop-openldap-server.sh slapd-2
sudo lxc-stop -n slapd-1
sudo lxc-destroy -n slapd-1
sudo lxc-stop -n slapd-2
sudo lxc-destroy -n slapd-2
