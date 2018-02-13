#!/bin/bash

TOPOLOGY="$1"

ASN=4200000000
V4_SUB="192.0.2."
V6_SUB="2001:DB8::"
INDEX="0"

HOSTS=`mktemp`

cat "$TOPOLOGY" | sed 's/#.*//' | awk '{print $1}' | grep eth | awk -F':' '{print $1}' | sed 's/\"//g' | sort | uniq > $HOSTS
cat "$TOPOLOGY" | sed 's/*.*//' | awk '{print $3}' | grep eth | awk -F':' '{print $1}' | sed 's/\"//g' | sort | uniq >> $HOSTS

for HOST in `cat $HOSTS | sort | uniq`
do
    HOST_VARS_FILE="host_vars/$HOST"
    cat > $HOST_VARS_FILE << EOF
---
fabric:
  asn: $ASN
  router_id: ${V4_SUB}${INDEX}
  loopback: ${V4_SUB}${INDEX}
  loopbackv6: ${V6_SUB}${INDEX}

interfaces:
EOF
    IFS_BACK="$IFS"
    IFS='
'
    for CONNECTION in `grep "\"$HOST\"" "$TOPOLOGY" | grep -v '#'`
    do
        SRC_HOST=`echo "$CONNECTION" | awk '{print $1}' | awk -F':' '{print $1}' | sed 's/\"//g'`
        SRC_INT=`echo "$CONNECTION" | awk '{print $1}' | awk -F':' '{print $2}' | sed 's/\"//g' | sed 's/eth//'`
        DEST_HOST=`echo "$CONNECTION" | awk '{print $3}' | awk -F':' '{print $1}' | sed 's/\"//g'`
        DEST_INT=`echo "$CONNECTION" | awk '{print $3}' | awk -F':' '{print $2}' | sed 's/\"//g' | sed 's/eth//' | sed 's/;//'`

        if [ "$SRC_HOST" == "$HOST" ]; then
            cat >> $HOST_VARS_FILE << EOF
  ${SRC_INT}:
    link: br${SRC_HOST}_${SRC_INT}
EOF
        else
            cat >> $HOST_VARS_FILE << EOF
  ${DEST_INT}:
    link: br${SRC_HOST}_${SRC_INT}
EOF
        fi
    done
    IFS="$IFS_BACK"
    ASN=$(($ASN + 1))
    INDEX=$(($INDEX + 1))
done

rm $HOSTS
