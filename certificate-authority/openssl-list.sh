#!/bin/bash

#standard-commands | digest-commands | cipher-commands | cipher-algorithms | digest-algorithms |
#       public-key-algorithms

# -help                   Display this summary
# -1                      List in one column
# -commands               List of standard commands
# -digest-commands        List of message digest commands
# -digest-algorithms      List of message digest algorithms
# -cipher-commands        List of cipher commands
# -cipher-algorithms      List of cipher algorithms
# -public-key-algorithms  List of public key algorithms
# -public-key-methods     List of public key methods
# -disabled               List of disabled features
# -missing-help           List missing detailed help strings
# -options val            List options for specified command

echo "================="
echo "standard-commands"
echo "================="
openssl list -commands
echo ""

echo "================="
echo "digest-commands"
echo "================="
openssl list -digest-commands
echo ""

echo "================="
echo "cipher-commands"
echo "================="
openssl list -cipher-commands
echo ""

echo "================="
echo "cipher-algorithms"
echo "================="
openssl list -cipher-algorithms
echo ""

echo "================="
echo "digest-algorithms"
echo "================="
openssl list -digest-algorithms
echo ""

echo "====================="
echo "public-key-algorithms"
echo "====================="
openssl list -public-key-algorithms
echo ""

echo "=================="
echo "public-key-methods"
echo "=================="
openssl list -public-key-methods
echo ""

echo "================="
echo "disabled-features"
echo "================="
openssl list -disabled
echo ""

echo "=============="
echo "EC List Curves"
echo "=============="
openssl ecparam -list_curves
echo ""
