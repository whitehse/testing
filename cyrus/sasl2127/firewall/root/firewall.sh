#!/bin/bash

# Definitions
IP6TABLES='/sbin/ip6tables'
IPTABLES='/sbin/iptables'
EXTERNAL_INT='eth0'
EXTERNAL_IP='192.168.27.2'
EXAMPLE_COM_INT='eth1'
EXAMPLE_ORG_INT='eth2'

# Static NATs
# kdc.example.com        - 192.168.27.10 to 198.51.100.7
# ldap27.example.com     - 192.168.27.11 to 198.51.100.8
# ldap26.example.com     - 192.168.27.12 to 198.51.100.9
# ldap23.example.com     - 192.168.27.13 to 198.51.100.10
# imap.example.com       - 192.168.27.14 to 198.51.100.11
# kdc.example.org        - 192.168.27.20 to 203.0.113.7
# ldap27.example.org     - 192.168.27.21 to 203.0.113.8
# ldap26.example.org     - 192.168.27.22 to 203.0.113.9
# ldap23.example.org     - 192.168.27.23 to 203.0.113.10
# imap.example.org       - 192.168.27.24 to 203.0.113.11

# ipv4 rules

$IPTABLES -F
$IPTABLES -X
$IPTABLES -t nat -F
$IPTABLES -t nat -X
$IPTABLES -t mangle -F
$IPTABLES -t mangle -X
$IPTABLES -F INPUT
$IPTABLES -F OUTPUT
$IPTABLES -F FORWARD
$IPTABLES -P INPUT ACCEPT
$IPTABLES -P FORWARD ACCEPT
$IPTABLES -P OUTPUT ACCEPT

#$IPTABLES -t nat -A POSTROUTING -o $EXTERNAL_INT -j MASQUERADE

# Configure Static NAT rules to allow traffic into internal hosts by way of the
# firewall's 192.168.27.x IPs.

$IPTABLES -t nat -A PREROUTING -d 192.168.27.10 -i $EXTERNAL_INT -j DNAT --to-destination 198.51.100.7
$IPTABLES -t nat -A POSTROUTING -s 198.51.100.7 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.10
$IPTABLES -t nat -A PREROUTING -d 192.168.27.11 -i $EXTERNAL_INT -j DNAT --to-destination 198.51.100.8
$IPTABLES -t nat -A POSTROUTING -s 198.51.100.8 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.11
$IPTABLES -t nat -A PREROUTING -d 192.168.27.12 -i $EXTERNAL_INT -j DNAT --to-destination 198.51.100.9
$IPTABLES -t nat -A POSTROUTING -s 198.51.100.9 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.12
$IPTABLES -t nat -A PREROUTING -d 192.168.27.13 -i $EXTERNAL_INT -j DNAT --to-destination 198.51.100.10
$IPTABLES -t nat -A POSTROUTING -s 198.51.100.10 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.13
$IPTABLES -t nat -A PREROUTING -d 192.168.27.14 -i $EXTERNAL_INT -j DNAT --to-destination 198.51.100.11
$IPTABLES -t nat -A POSTROUTING -s 198.51.100.11 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.14
$IPTABLES -t nat -A PREROUTING -d 192.168.27.20 -i $EXTERNAL_INT -j DNAT --to-destination 203.0.113.7
$IPTABLES -t nat -A POSTROUTING -s 203.0.113.7 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.20
$IPTABLES -t nat -A PREROUTING -d 192.168.27.21 -i $EXTERNAL_INT -j DNAT --to-destination 203.0.113.8
$IPTABLES -t nat -A POSTROUTING -s 203.0.113.8 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.21
$IPTABLES -t nat -A PREROUTING -d 192.168.27.22 -i $EXTERNAL_INT -j DNAT --to-destination 203.0.113.9
$IPTABLES -t nat -A POSTROUTING -s 203.0.113.9 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.22
$IPTABLES -t nat -A PREROUTING -d 192.168.27.23 -i $EXTERNAL_INT -j DNAT --to-destination 203.0.113.10
$IPTABLES -t nat -A POSTROUTING -s 203.0.113.10 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.23
$IPTABLES -t nat -A PREROUTING -d 192.168.27.24 -i $EXTERNAL_INT -j DNAT --to-destination 203.0.113.11
$IPTABLES -t nat -A POSTROUTING -s 203.0.113.11 -o $EXTERNAL_INT -j SNAT --to-source 198.168.27.24

# Explicit deny

$IPTABLES -P INPUT DROP
$IPTABLES -P OUTPUT DROP

# Filter all packets that have RH0 headers:
#$IPTABLES -A INPUT -m rt --rt-type 0 -j DROP
#$IPTABLES -A FORWARD -m rt --rt-type 0 -j DROP
#$IPTABLES -A OUTPUT -m rt --rt-type 0 -j DROP

# Allow anything on the local link
$IPTABLES -A INPUT  -i lo -j ACCEPT
$IPTABLES -A OUTPUT -o lo -j ACCEPT

# Allow anything out to the internet
$IPTABLES -A OUTPUT -o ${EXTERNAL_INT} -j ACCEPT
# Allow established, related packets back in
$IPTABLES -A INPUT  -i ${EXTERNAL_INT} -m state --state ESTABLISHED,RELATED -j ACCEPT

# Allow the localnet access to us:
$IPTABLES -A INPUT    -i ${EXAMPLE_COM_INT}  -j ACCEPT
$IPTABLES -A OUTPUT   -o ${EXAMPLE_COM_INT}  -j ACCEPT
$IPTABLES -A INPUT    -i ${EXAMPLE_ORG_INT}  -j ACCEPT
$IPTABLES -A OUTPUT   -o ${EXAMPLE_ORG_INT}  -j ACCEPT

# IMAP
# imap.example.com       - 192.168.27.14 to 198.51.100.11
# imap.example.org       - 192.168.27.24 to 203.0.113.11
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.14 --dport 143 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.14 --dport 993 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.24 --dport 143 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.24 --dport 993 -j ACCEPT

# SMTP
# imap.example.com       - 192.168.27.14 to 198.51.100.11
# imap.example.org       - 192.168.27.24 to 203.0.113.11
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.14 --dport 25 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.14 --dport 587 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.24 --dport 25 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.24 --dport 587 -j ACCEPT

# Port 80

# Port 443

# SSH
# kdc.example.com        - 192.168.27.10 to 198.51.100.7
# ldap27.example.com     - 192.168.27.11 to 198.51.100.8
# ldap26.example.com     - 192.168.27.12 to 198.51.100.9
# ldap23.example.com     - 192.168.27.13 to 198.51.100.10
# imap.example.com       - 192.168.27.14 to 198.51.100.11
# kdc.example.org        - 192.168.27.20 to 203.0.113.7
# ldap27.example.org     - 192.168.27.21 to 203.0.113.8
# ldap26.example.org     - 192.168.27.22 to 203.0.113.9
# ldap23.example.org     - 192.168.27.23 to 203.0.113.10
# imap.example.org       - 192.168.27.24 to 203.0.113.11
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.10 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.11 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.12 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.13 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_COM_INT} -d 192.167.27.14 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.20 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.21 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.22 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.23 --dport 22 -j ACCEPT
$IPTABLES -A INPUT -p tcp -i ${EXAMPLE_ORG_INT} -d 192.167.27.24 --dport 22 -j ACCEPT

# ssh to firewall
$IPTABLES -A INPUT -p tcp -i ${EXTERNAL_INT} -d ${EXTERNAL_IP} --dport 22 -j ACCEPT

# DNS
$IPTABLES -A INPUT -p tcp -i ${EXTERNAL_INT} -d ${EXTERNAL_IP} --dport 53 -j ACCEPT
$IPTABLES -A INPUT -p udp -i ${EXTERNAL_INT} -d ${EXTERNAL_IP} --dport 53 -j ACCEPT

# Paranoia setting on ipv4 interface  
#$IPTABLES -I INPUT -i eth0 -p tcp --syn -j DROP
#$IPTABLES -I FORWARD -i eth0 -p tcp --syn -j DROP
#$IPTABLES -I INPUT -i eth0 -p udp  -j DROP
#$IPTABLES -I FORWARD -i eth0 -p udp  -j DROP

# Allow ICMP everwhere
$IPTABLES -A INPUT -p icmp -j ACCEPT
$IPTABLES -I OUTPUT -p icmp -j ACCEPT
$IPTABLES -I FORWARD -p icmp -j ACCEPT

# Log 
$IPTABLES -A INPUT -j LOG --log-prefix "INPUT-v4:"
$IPTABLES -A FORWARD -j LOG  --log-prefix "FORWARD-v4:"
$IPTABLES -A OUTPUT -j LOG  --log-prefix "OUTPUT-v4:"
