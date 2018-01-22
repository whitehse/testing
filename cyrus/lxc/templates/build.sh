#!/bin/bash

lxc-create -t /usr/src/git/testing/cyrus/lxc/templates/lxc-mit-sasl2123 -n test-mit-sasl2123 | tee /tmp/output-mit-sasl2123.txt
lxc-create -t /usr/src/git/testing/cyrus/lxc/templates/lxc-mit-sasl2126 -n test-mit-sasl2126 | tee /tmp/output-mit-sasl2126.txt
lxc-create -t /usr/src/git/testing/cyrus/lxc/templates/lxc-mit-saslmaster -n test-mit-saslmaster | tee /tmp/output-mit-saslmaster.txt

lxc-create -t /usr/src/git/testing/cyrus/lxc/templates/lxc-heimdal-sasl2123 -n test-heimdal-sasl2123 | tee /tmp/output-heimdal-sasl2123.txt
lxc-create -t /usr/src/git/testing/cyrus/lxc/templates/lxc-heimdal-sasl2126 -n test-heimdal-sasl2126 | tee /tmp/output-heimdal-sasl2126.txt
lxc-create -t /usr/src/git/testing/cyrus/lxc/templates/lxc-heimdal-saslmaster -n test-heimdal-saslmaster | tee /tmp/output-saslmaster.txt
