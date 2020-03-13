#!/bin/bash

./create-root-cert-ec.sh ca/ "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Root CA"
./create-intermediate-cert-ec.sh ca/ "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Intermediate CA 1"
