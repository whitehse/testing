#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

if [ -z "$2" ]; then
  cat - << EOF
A package name must be specified
EOF
  exit 1
fi

export HOST="$1"
export PACKAGE="$2"

TEMPDIR=`mktemp -d`

cd $TEMPDIR
apt-cache show "$PACKAGE" | grep -v Depends > $TEMPDIR/control
equivs-build control
FILELIST=`ls | grep -v control`
cd -

for foo in "$FILELIST"
do
  scp -F conf/ssh_config $TEMPDIR/$foo root@${HOST}:
  ssh -F conf/ssh_config root@${HOST} 'bash -l -s' << EOF
dpkg -i "$foo"
rm "$foo"
EOF
done

rm -rf "$TEMPDIR"
