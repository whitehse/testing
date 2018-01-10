$ttl 172800
$ORIGIN example.com.
@       IN      SOA     ns.example.com. root.example.com. (
                        2017122100
                        10800
                        3600
                        432000
                        3600 )

;; Nameservers
                IN      NS      ns
ns              IN      A       198.51.100.1

kdc		IN	A	198.51.100.7
kerberos-1	IN	A	198.51.100.7
ldap27		IN	A	198.51.100.8
ldap26		IN	A	198.51.100.9
ldap23		IN	A	198.51.100.10
imap		IN	A	198.51.100.11
smtp		IN	A	198.51.100.11

;; Kerberos records
_kerberos               TXT             "EXAMPLE.COM"
_kerberos._udp          SRV 0 0 88      kerberos-1
_kerberos._tcp          SRV 0 0 88      kerberos-1
_kerberos-master._udp   SRV 0 0 88      kerberos-1
_kerberos-adm._tcp      SRV 0 0 749     kerberos-1
_kpasswd._udp           SRV 0 0 464     kerberos-1
