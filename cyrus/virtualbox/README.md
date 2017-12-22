# Firewall

````
# Internal Networks are transient
# Create the Firewall, which performs the following functions:
#   Acts as a router between the example.com and example.org networks
#   Acts as a NAT router between the example.com, example.org networks
#     and the public internet
#   Is the DNS authority for example.com and example.org
#   Is a certificate authority
# example_com - 198.51.100.0/24 - MIT KDC
# example_org - 203.0.113.0/24 - Heimdal KDC

vboxmanage createvm --name cyrus-firewall --ostype Debian_64 --register
vboxmanage modifyvm cyrus-firewall --memory 512 --boot1 disk --boot2 dvd --boot3 net --rtcuseutc on
vboxmanage modifyvm cyrus-firewall --nic1 bridged --bridgeadapter1 eth1
vboxmanage modifyvm cyrus-firewall --nic2 intnet
vboxmanage modifyvm cyrus-firewall --intnet2 "example_com"
vboxmanage modifyvm cyrus-firewall --nic3 intnet
vboxmanage modifyvm cyrus-firewall --intnet3 "example_org"
vboxmanage createhd --size 10000 --filename $HOME/.VirtualBox/HardDisks/cyrus-firewall
vboxmanage storagectl cyrus-firewall --name sata0 --add sata
vboxmanage storageattach cyrus-firewall --storagectl sata0 --device 0 --port 0 --type hdd --medium $HOME/.VirtualBox/HardDisks/cyrus-firewall.vdi

wget https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-9.3.0-amd64-netinst.iso

vboxmanage storagectl cyrus-firewall --name ide0 --add ide
vboxmanage storageattach cyrus-firewall --storagectl ide0 --device 0 --port 0 --type dvddrive --medium $HOME/debian-9.3.0-amd64-netinst.iso

vboxmanage modifyvm cyrus-firewall --vrde on
vboxmanage modifyvm cyrus-firewall --vrdeport 42000
vboxmanage startvm cyrus-firewall --type=headless
rdesktop localhost:42000

Select DHCP (or static as required by the local network)
selected tasksel tasks: SSH server, standard system utilities
apt-get install vim iptables-persistent bind9 dnsutils
set 'PermitRootLogin yes' in /etc/ssh/sshd_config
/etc/init.d/ssh reload
sed -i 's/^#net\.ipv4\.ip_forward.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sysctl -p
iptables -P OUTPUT ACCEPT
iptables -t nat -A POSTROUTING -o enp0s3 -j MASQUERADE
/sbin/iptables-save > /etc/iptables/rules.v4
add 'recusion yes;' to /etc/bind/named.conf.options
Create bind zones for example.com, example.org, '100.51.198.in-addr.arpa.', and
'113.0.203.in-addr.arpa.'.

Certificate Authority:
#http://albertech.blogspot.com/2015/06/iamaca-become-your-own-certificate.html
https://jamielinux.com/docs/openssl-certificate-authority/create-the-root-pair.html

mkdir /root/ca
cd /root/ca
mkdir certs crl newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
wget -O /root/ca/openssl.cnf https://jamielinux.com/docs/openssl-certificate-authority/_downloads/root-config.txt
openssl genrsa -aes256 -out private/ca.key.pem 4096
chmod 400 private/ca.key.pem
openssl req -config openssl.cnf \
      -key private/ca.key.pem \
      -new -x509 -days 7300 -sha256 -extensions v3_ca \
      -out certs/ca.cert.pem
chmod 444 certs/ca.cert.pem
mkdir /root/ca/intermediate
cd /root/ca/intermediate
mkdir certs crl csr newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
echo 1000 > /root/ca/intermediate/crlnumber
wget -O /root/ca/intermediate/openssl.cnf https://jamielinux.com/docs/openssl-certificate-authority/_downloads/intermediate-config.txt
cd /root/ca
openssl genrsa -aes256 \
      -out intermediate/private/intermediate.key.pem 4096
chmod 400 intermediate/private/intermediate.key.pem
openssl req -config intermediate/openssl.cnf -new -sha256 \
      -key intermediate/private/intermediate.key.pem \
      -out intermediate/csr/intermediate.csr.pem
openssl ca -config openssl.cnf -extensions v3_intermediate_ca \
      -days 3650 -notext -md sha256 \
      -in intermediate/csr/intermediate.csr.pem \
      -out intermediate/certs/intermediate.cert.pem
# ignore the index.txt.attr error
chmod 444 intermediate/certs/intermediate.cert.pem
openssl x509 -noout -text \
      -in intermediate/certs/intermediate.cert.pem
openssl verify -CAfile certs/ca.cert.pem \
      intermediate/certs/intermediate.cert.pem
cat intermediate/certs/intermediate.cert.pem \
      certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem
chmod 444 intermediate/certs/ca-chain.cert.pem
openssl genrsa \
      -out intermediate/private/ldap.example.com.key.pem 2048
chmod 400 intermediate/private/ldap.example.com.key.pem
openssl req -config intermediate/openssl.cnf \
      -key intermediate/private/ldap.example.com.key.pem \
      -new -sha256 -out intermediate/csr/ldap.example.com.csr.pem
openssl ca -config intermediate/openssl.cnf \
      -extensions server_cert -days 375 -notext -md sha256 \
      -in intermediate/csr/ldap.example.com.csr.pem \
      -out intermediate/certs/ldap.example.com.cert.pem
openssl x509 -noout -text \
      -in intermediate/certs/ldap.example.com.cert.pem
openssl verify -CAfile intermediate/certs/ca-chain.cert.pem \
      intermediate/certs/ldap.example.com.cert.pem







openssl genrsa -out /root/exampleCA.key 2048
openssl req -x509 -sha256 -nodes -days 1024 -newkey rsa:2048 -key /root/exampleCA.key -out /root/exampleCA.crt

openssl genrsa -out /root/exampleInt.key 2048
openssl req -new -key /root/exampleInt.key -out exampleInt.csr
openssl x509 -extensions v3_intermediate_ca -req -in exampleInt.csr -CA /root/exampleCA.crt -CAkey /root/exampleCA.key -CAcreateserial -out exampleInt.crt -days 730

cat exampleInt.crt exampleCA.crt > examplecachain.crt

openssl genrsa -out ldap.example.com.key 2048
openssl req -new -key ldap.example.com.key -out ldap.example.com.csr
openssl x509 -req -in ldap.example.com.csr -CA /root/exampleInt.crt -CAkey /root/exampleInt.key -CAcreateserial -out ldap.example.com.crt -days 365
openssl x509 -in ldap.example.com.crt -text -noout | grep CN
       Subject: C = US, ST = OK, O = Example, CN = ldap.example.com

openssl verify -CAfile exampleCA.crt -untrusted exampleInt.crt ldap.example.com.crt

#openssl genrsa -out dwhite.example.com.key 2048
#openssl req -new -key dwhite.example.com.key -out dwhite.example.com.csr
#openssl x509 -req -in dwhite.example.com.csr -CA /root/exampleCA.crt -CAkey /root/exampleCA.key -CAcreateserial -out dwhite.example.com.crt -days 365
#openssl x509 -in dwhite.example.com.crt -text -noout | grep CN
        Subject: C = US, ST = OK, O = Example, CN = dwhite@example.com, emailAddress = dwhite@example.com

scp exampleCA.crt root@ldap.example.com:/etc/ssl/certs/
scp exampleCA.crt root@client.example.com:/etc/ssl/certs/
scp ldap.example.com.key ldap.example.com.crt root@ldap.example.com:
scp dwhite.example.com.key dwhite.example.com.crt dwhite@client.example.com:
````

# sans-sasl

````
vboxmanage createvm --name sans-sasl --ostype Debian_64 --register
vboxmanage modifyvm sans-sasl --memory 512 --boot1 disk --boot2 dvd --boot3 net --rtcuseutc on
vboxmanage modifyvm sans-sasl --nic1 intnet
vboxmanage modifyvm sans-sasl --intnet1 "example_com"
vboxmanage createhd --size 10000 --filename $HOME/.VirtualBox/HardDisks/sans-sasl
vboxmanage storagectl sans-sasl --name sata0 --add sata
vboxmanage storageattach sans-sasl --storagectl sata0 --device 0 --port 0 --type hdd --medium $HOME/.VirtualBox/HardDisks/sans-sasl.vdi
vboxmanage storagectl sans-sasl --name ide0 --add ide
vboxmanage storageattach sans-sasl --storagectl ide0 --device 0 --port 0 --type dvddrive --medium $HOME/debian-9.3.0-amd64-netinst.iso
vboxmanage modifyvm sans-sasl --vrde on
vboxmanage modifyvm sans-sasl --vrdeport 42001
vboxmanage startvm sans-sasl --type=headless
rdesktop localhost:42001

198.51.100.5/24
gateway: 198.51.100.1
dns server: 198.51.100.1
selected tasksel tasks: SSH server, standard system utilities
apt-get install vim aptitude
set 'PermitRootLogin yes' in /etc/ssh/sshd_config
/etc/init.d/ssh reload
aptitude remove libsasl2-2 libldap-2.4-2 libcurl3-gnutls python3-debianbts python3-pycurl python3-pysimplesoap python3-reportbug reportbug
dpkg --purge libsasl2-modules
cd /usr/src
wget http://www.cyrusimap.org/releases/cyrus-sasl-2.1.27-rc6.tar.gz
wget ftp://ftp.openldap.org/pub/OpenLDAP/openldap-release/openldap-2.4.45.tgz
tar -xvzf cyrus-sasl-2.1.27-rc6.tar.gz
tar -xvzf openldap-2.4.45.tgz
cd openldap-2.4.45/
apt-get install build-essential
#apt-get build-dep slapd
apt-get install autoconf automake autopoint autotools-dev comerr-dev debhelper dh-autoreconf dh-strip-nondeterminism gettext intltool-debian libarchive-zip-perl libcroco3 libdb5.3-dev libfile-stripnondeterminism-perl libglib2.0-0 libgmp-dev libgmpxx4ldbl libgnutls-dane0 libidn11-dev libltdl-dev libltdl7 libodbc1 libp11-kit-dev libperl-dev libsigsegv2 libtasn1-6-dev libtool libunbound2 libwrap0-dev m4 nettle-dev odbcinst odbcinst1debian2 pkg-config po-debconf time unixodbc-dev zlib1g-dev libssl-dev
# Library installation only, without libsasl
./configure \
--prefix=/usr \
--libexecdir=/usr/lib \
--sysconfdir=/etc \
--localstatedir=/var \
--mandir=/usr/share/man \
--enable-dynamic \
--enable-slapd=no \
--enable-crypt=yes \
--enable-spasswd=no \
--disable-backends \
--enable-overlays=no \
--with-cyrus-sasl=no \
--with-threads \
--with-tls=openssl
make depend
make && make install
halt
vboxmanage list runningvms
# If necessary
vboxmanage controlvm sans-sasl poweroff
````

# Generic MIT Host

````
vboxmanage clonevm sans-sasl --name mit-host --register
vboxmanage modifyvm mit-host --vrdeport 42002
vboxmanage startvm mit-host --type=headless
rdesktop localhost:42002

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 198.51.100.6/24
gateway: 198.51.100.1
Hostname: mit-host.example.com
reboot

apt-get install libkrb5-dev
#apt-get build-dep libsasl2-2
apt-get install chrpath default-libmysqlclient-dev diffstat docbook docbook-to-man libdb-dev libmariadbclient-dev libmariadbclient-dev-compat libmariadbclient18 libosp5 libpam0g-dev libsqlite3-dev mysql-common opensp quilt sgml-data
cd /usr/src/cyrus-sasl-2.1.27
./configure \
--prefix=/usr \
--mandir=/usr/share/man \
--infodir=/usr/share/info \
--enable-alwaystrue \
--enable-srp \
--enable-srp-setpass \
--enable-login \
--enable-ntlm \
--enable-passdss \
--enable-share \
--with-saslauthd=/var/run/saslauthd \
--with-authdaemond=/var/run/courier \
--sysconfdir=/etc \
--enable-httpform \
--with-devrandom=/dev/urandom \
--enable-gssapi \
--enable-gss_mutexes
#--enable-sql \
make && make install
ldconfig

apt-get install krb5-user
specify no default realm, and specify no servers for it
remove the default realm from /etc/krb5.conf

verify no debian ldap or sasl packages have been installed as a dependency
along the way:

dpkg -l "*ldap*"
dpkg -l "*sasl*"

These responses are expected at this point:

~$ ldapsearch -Y GSSAPI
ldapsearch: not compiled with SASL support

~$ ldapsearch -VV
ldapsearch: @(#) $OpenLDAP: ldapsearch 2.4.45 (Dec 21 2017 13:08:25) $
        root@sans-sasl:/usr/src/openldap-2.4.45/clients/tools
        (LDAP library: OpenLDAP 20445)

~$ strings /usr/sbin/pluginviewer | grep 2\.1\.27
/usr/src/cyrus-sasl-2.1.27/utils

~$ ldd /usr/sbin/pluginviewer | grep 'krb5\|heimdal'
        libgssapi_krb5.so.2 => /usr/lib/x86_64-linux-gnu/libgssapi_krb5.so.2 (0x00007f4298df0000)
        libkrb5.so.3 => /usr/lib/x86_64-linux-gnu/libkrb5.so.3 (0x00007f4298b16000)
        libkrb5support.so.0 => /usr/lib/x86_64-linux-gnu/libkrb5support.so.0 (0x00007f4297f17000)

~$ pluginviewer
Installed and properly configured auxprop mechanisms are:
sasldb
List of auxprop plugins follows
Plugin "sasldb" ,       API version: 8
        supports store: yes

Installed and properly configured SASL (server side) mechanisms are:
  SRP PASSDSS-3DES-1 GS2-KRB5 GS2-IAKERB SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 EXTERNAL OTP CRAM-MD5 NTLM PLAIN LOGIN ANONYMOUS
Available SASL (server side) mechanisms matching your criteria are:
  SRP PASSDSS-3DES-1 GS2-KRB5 GS2-IAKERB SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 OTP CRAM-MD5 NTLM PLAIN LOGIN ANONYMOUS
List of server plugins follows
Plugin "srp" [loaded],  API version: 4
        SASL mechanism: SRP, best SSF: 128, supports setpass: yes
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|NO_DICTIONARY|FORWARD_SECRECY|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "passdss" [loaded],      API version: 4
        SASL mechanism: PASSDSS-3DES-1, best SSF: 112, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|NO_DICTIONARY|FORWARD_SECRECY|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "gs2" [loaded],  API version: 4
        SASL mechanism: GS2-KRB5, best SSF: 0, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|GSS_FRAMING|CHANNEL_BINDING
Plugin "gs2" [loaded],  API version: 4
        SASL mechanism: GS2-IAKERB, best SSF: 0, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|GSS_FRAMING|CHANNEL_BINDING
Plugin "scram" [loaded],        API version: 4
        SASL mechanism: SCRAM-SHA-256, best SSF: 0, supports setpass: yes
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|MUTUAL_AUTH
        features: PROXY_AUTHENTICATION|CHANNEL_BINDING|SUPPORTS_HTTP
Plugin "scram" [loaded],        API version: 4
        SASL mechanism: SCRAM-SHA-1, best SSF: 0, supports setpass: yes
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|MUTUAL_AUTH
        features: PROXY_AUTHENTICATION|CHANNEL_BINDING|SUPPORTS_HTTP
Plugin "gssapiv2" [loaded],     API version: 4
        SASL mechanism: GSSAPI, best SSF: 256, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION|DONTUSE_USERPASSWD
Plugin "gssapiv2" [loaded],     API version: 4
        SASL mechanism: GSS-SPNEGO, best SSF: 256, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION|DONTUSE_USERPASSWD|SUPPORTS_HTTP
Plugin "digestmd5" [loaded],    API version: 4
        SASL mechanism: DIGEST-MD5, best SSF: 128, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|MUTUAL_AUTH
        features: PROXY_AUTHENTICATION|SUPPORTS_HTTP
Plugin "otp" [loaded],  API version: 4
        SASL mechanism: OTP, best SSF: 0, supports setpass: yes
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|FORWARD_SECRECY
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "crammd5" [loaded],      API version: 4
        SASL mechanism: CRAM-MD5, best SSF: 0, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT
        features: SERVER_FIRST
Plugin "ntlm" [loaded],         API version: 4
        SASL mechanism: NTLM, best SSF: 0, supports setpass: no
        security flags: NO_ANONYMOUS|NO_PLAINTEXT
        features: WANT_CLIENT_FIRST|SUPPORTS_HTTP
Plugin "plain" [loaded],        API version: 4
        SASL mechanism: PLAIN, best SSF: 0, supports setpass: no
        security flags: NO_ANONYMOUS|PASS_CREDENTIALS
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "login" [loaded],        API version: 4
        SASL mechanism: LOGIN, best SSF: 0, supports setpass: no
        security flags: NO_ANONYMOUS|PASS_CREDENTIALS
        features:
Plugin "anonymous" [loaded],    API version: 4
        SASL mechanism: ANONYMOUS, best SSF: 0, supports setpass: no
        security flags: NO_PLAINTEXT
        features: WANT_CLIENT_FIRST|DONTUSE_USERPASSWD
Installed and properly configured SASL (client side) mechanisms are:
  SRP PASSDSS-3DES-1 GS2-KRB5 GS2-IAKERB SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 EXTERNAL OTP CRAM-MD5 NTLM PLAIN LOGIN ANONYMOUS
Available SASL (client side) mechanisms matching your criteria are:
  SRP PASSDSS-3DES-1 GS2-KRB5 GS2-IAKERB SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 EXTERNAL OTP CRAM-MD5 NTLM PLAIN LOGIN ANONYMOUS
List of client plugins follows
Plugin "srp" [loaded],  API version: 4
        SASL mechanism: SRP, best SSF: 128
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|NO_DICTIONARY|FORWARD_SECRECY|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "passdss" [loaded],      API version: 4
        SASL mechanism: PASSDSS-3DES-1, best SSF: 112
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|NO_DICTIONARY|FORWARD_SECRECY|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "gs2" [loaded],  API version: 4
        SASL mechanism: GS2-KRB5, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|NEED_SERVER_FQDN|GSS_FRAMING|CHANNEL_BINDING
Plugin "gs2" [loaded],  API version: 4
        SASL mechanism: GS2-IAKERB, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|NEED_SERVER_FQDN|GSS_FRAMING|CHANNEL_BINDING
Plugin "scram" [loaded],        API version: 4
        SASL mechanism: SCRAM-SHA-256, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|MUTUAL_AUTH
        features: PROXY_AUTHENTICATION|CHANNEL_BINDING|SUPPORTS_HTTP
Plugin "scram" [loaded],        API version: 4
        SASL mechanism: SCRAM-SHA-1, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|MUTUAL_AUTH
        features: PROXY_AUTHENTICATION|CHANNEL_BINDING|SUPPORTS_HTTP
Plugin "gssapiv2" [loaded],     API version: 4
        SASL mechanism: GSSAPI, best SSF: 256
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION|NEED_SERVER_FQDN
Plugin "gssapiv2" [loaded],     API version: 4
        SASL mechanism: GSS-SPNEGO, best SSF: 256
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_ACTIVE|PASS_CREDENTIALS|MUTUAL_AUTH
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION|NEED_SERVER_FQDN|SUPPORTS_HTTP
Plugin "digestmd5" [loaded],    API version: 4
        SASL mechanism: DIGEST-MD5, best SSF: 128
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|MUTUAL_AUTH
        features: PROXY_AUTHENTICATION|NEED_SERVER_FQDN|SUPPORTS_HTTP
Plugin "EXTERNAL" [loaded],     API version: 4
        SASL mechanism: EXTERNAL, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|NO_DICTIONARY
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "otp" [loaded],  API version: 4
        SASL mechanism: OTP, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT|FORWARD_SECRECY
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "crammd5" [loaded],      API version: 4
        SASL mechanism: CRAM-MD5, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT
        features: SERVER_FIRST
Plugin "ntlm" [loaded],         API version: 4
        SASL mechanism: NTLM, best SSF: 0
        security flags: NO_ANONYMOUS|NO_PLAINTEXT
        features: WANT_CLIENT_FIRST|SUPPORTS_HTTP
Plugin "plain" [loaded],        API version: 4
        SASL mechanism: PLAIN, best SSF: 0
        security flags: NO_ANONYMOUS|PASS_CREDENTIALS
        features: WANT_CLIENT_FIRST|PROXY_AUTHENTICATION
Plugin "login" [loaded],        API version: 4
        SASL mechanism: LOGIN, best SSF: 0
        security flags: NO_ANONYMOUS|PASS_CREDENTIALS
        features: SERVER_FIRST
Plugin "anonymous" [loaded],    API version: 4
        SASL mechanism: ANONYMOUS, best SSF: 0
        security flags: NO_PLAINTEXT
        features: WANT_CLIENT_FIRST

Recompile libldap with sasl support, and compile slapd

cd /usr/src
mv openldap-2.4.45 openldap-2.4.45-sans-sasl
tar -xvzf openldap-2.4.45.tgz
cd openldap-2.4.45

./configure \
--prefix=/usr \
--libexecdir=/usr/lib \
--sysconfdir=/etc \
--localstatedir=/var \
--mandir=/usr/share/man \
--enable-dynamic \
--enable-syslog \
--enable-proctitle \
--enable-ipv6 \
--enable-local \
--enable-slapd \
--enable-dynacl \
--enable-aci \
--enable-cleartext \
--enable-crypt \
--disable-lmpasswd \
--enable-spasswd \
--enable-modules \
--enable-rewrite \
--enable-rlookups \
--enable-slapi \
--enable-wrappers \
--enable-backends=mod \
--disable-ndb \
--enable-overlays=mod \
--with-cyrus-sasl \
--with-threads \
--with-tls=openssl \
--with-odbc=unixodbc

make depend
make && make install

Expected output:

~$ ldapsearch -VV
ldapsearch: @(#) $OpenLDAP: ldapsearch 2.4.45 (Dec 21 2017 15:21:18) $
        root@mit-host:/usr/src/openldap-2.4.45/clients/tools
        (LDAP library: OpenLDAP 20445)
~$ ldapsearch -Y GSSAPI
ldap_sasl_interactive_bind_s: Can't contact LDAP server (-1)
~$ /usr/lib/slapd -d 2
5a3c2ae8 @(#) $OpenLDAP: slapd 2.4.45 (Dec 21 2017 15:22:17) $
        root@mit-host:/usr/src/openldap-2.4.45/servers/slapd
Unrecognized database type (mdb)
5a3c2ae8 /etc/openldap/slapd.conf: line 52: <database> failed init (mdb)
5a3c2ae8 slapd stopped.
5a3c2ae8 connections_destroy: nothing to destroy.

halt
vboxmanage list runningvms
# If necessary
vboxmanage controlvm mit-host poweroff
````

# kdc.example.com

````
vboxmanage clonevm mit-host --name kdc-example-com --register
vboxmanage modifyvm kdc-example-com --vrdeport 42003
vboxmanage startvm kdc-example-com --type=headless
rdesktop localhost:42003

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 198.51.100.7/24
gateway: 198.51.100.1
Hostname: kdc.example.com
reboot

apt-get install krb5-{admin-server,kdc}
specify default realm (default_realm = EXAMPLE.COM) in /etc/krb5.conf

krb5_newrealm
Enter KDC database master key: passw0rd
Re-enter KDC database master key to verify: passw0rd

~$ kadmin.local
kadmin.local:  addprinc dwhite@EXAMPLE.COM
WARNING: no policy specified for dwhite@EXAMPLE.COM; defaulting to no policy
Enter password for principal "dwhite@EXAMPLE.COM": passw0rd
Re-enter password for principal "dwhite@EXAMPLE.COM": passw0rd
Principal "dwhite@EXAMPLE.COM" created.
kadmin.local:  addprinc -randkey host/ldap.example.com
kadmin.local:  addprinc -randkey ldap/ldap.example.com
kadmin.local:  ktadd -k ldap.keytab host/ldap.example.com
kadmin.local:  ktadd -k slapd.keytab ldap/ldap.example.com
kadmin.local:  q

scp ldap.keytab slapd.keytab root@ldap.example.com:
````

# ldap.example.com

````
vboxmanage clonevm mit-host --name ldap-example-com --register
vboxmanage modifyvm ldap-example-com --vrdeport 42004
vboxmanage startvm ldap-example-com --type=headless
rdesktop localhost:42004

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 198.51.100.8/24
Gateway: 198.51.100.1
Hostname: ldap.example.com
reboot

kinit -k -t /root/ldap.keytab host/ldap.example.com@EXAMPLE.COM
echo "pidfile /var/run/slapd.pid" > /etc/slapd.conf
cat - << EOF > /etc/slapd.conf
pidfile /var/run/slapd.pid
TLSCACertificateFile /etc/ssl/certs/exampleCA.crt
TLSCertificateFile /root/ldap.example.com.crt
TLSCertificateKeyFile /root/ldap.example.com.key
EOF

mkdir /etc/sasl2
echo "keytab: /root/slapd.keytab" > /etc/sasl2/slapd.conf
/usr/lib/slapd -f /etc/slapd.conf
````

# client.example.com

````
vboxmanage clonevm mit-host --name client-example-com --register
vboxmanage modifyvm client-example-com --vrdeport 42005
vboxmanage startvm client-example-com --type=headless
rdesktop localhost:42005

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 198.51.100.9/24
Gateway: 198.51.100.1
Hostname: client.example.com
reboot

su - dwhite
kinit dwhite@EXAMPLE.COM

ldapsearch -H ldap://ldap.example.com -x -b "" -s base -LLL supportedSASLMechanisms
dn:
supportedSASLMechanisms: SRP
supportedSASLMechanisms: PASSDSS-3DES-1
supportedSASLMechanisms: GS2-KRB5
supportedSASLMechanisms: GS2-IAKERB
supportedSASLMechanisms: SCRAM-SHA-256
supportedSASLMechanisms: SCRAM-SHA-1
supportedSASLMechanisms: GSSAPI
supportedSASLMechanisms: GSS-SPNEGO
supportedSASLMechanisms: DIGEST-MD5
supportedSASLMechanisms: OTP
supportedSASLMechanisms: CRAM-MD5
supportedSASLMechanisms: NTLM

ldapwhoami -Y GSSAPI -H ldap://ldap.example.com
SASL/GSSAPI authentication started
SASL username: dwhite@EXAMPLE.COM
SASL SSF: 56
SASL data security layer installed.
dn:uid=dwhite@example.com,cn=gssapi,cn=auth

ldapwhoami -Y GS2-KRB5 -H ldap://ldap.example.com
SASL/GS2-KRB5 authentication started
SASL username: dwhite@EXAMPLE.COM
SASL SSF: 0
dn:uid=dwhite@example.com,cn=gs2-krb5,cn=auth

ldapwhoami -Y GS2-IAKERB -H ldap://ldap.example.com
SASL/GS2-IAKERB authentication started
SASL username: dwhite@EXAMPLE.COM
SASL SSF: 0
dn:uid=dwhite@example.com,cn=gs2-iakerb,cn=auth

ldapwhoami -Y GSS-SPNEGO -H ldap://ldap.example.com
SASL/GSS-SPNEGO authentication started
SASL username: dwhite@EXAMPLE.COM
SASL SSF: 56
SASL data security layer installed.
dn:uid=dwhite@example.com,cn=gss-spnego,cn=auth
````

# Generic Heimdal Host

````
vboxmanage clonevm sans-sasl --name heimdal-host --register
vboxmanage modifyvm heimdal-host --nic1 intnet
vboxmanage modifyvm heimdal-host --intnet1 "example_org"
vboxmanage modifyvm heimdal-host --vrdeport 42006
vboxmanage startvm heimdal-host --type=headless
rdesktop localhost:42006

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 203.0.113.6/24
gateway: 203.0.113.1
Hostname: heimdal-host.example.org
reboot

#apt-get install heimdal-dev
# Heimdal, on debian, depends on both libldap and libsasl. Build from
source, and hope that it doesn't balk on missing libsasl.

#apt-get build-dep heimdal
apt-get install bison flex libbison-dev libbsd-dev libcap-ng-dev libdb-dev libedit-dev libhesiod-dev libhesiod0 libice-dev libice6 libjson-perl libncurses5-dev libperl4-corelibs-perl libpthread-stubs0-dev libsm-dev libsm6 libsqlite3-dev libtext-unidecode-perl libtinfo-dev libx11-dev libxau-dev libxcb1-dev libxdmcp-dev libxml-libxml-perl libxml-namespacesupport-perl libxml-sax-base-perl libxml-sax-perl libxt-dev libxt6 ss-dev tex-common texinfo unzip x11-common x11proto-core-dev x11proto-input-dev x11proto-kb-dev xorg-sgml-doctools xtrans-dev

cd /usr/src/
wget https://github.com/heimdal/heimdal/releases/download/heimdal-7.5.0/heimdal-7.5.0.tar.gz
tar -xvzf heimdal-7.5.0.tar.gz
cd heimdal-7.5.0
./configure \
--prefix=/usr \
--libexecdir=/usr/sbin \
--enable-shared \
--includedir=/usr/include \
--with-openldap=/usr \
--with-sqlite3=/usr \
--with-libedit=/usr \
--enable-kcm \
--with-hdbdir=/var/lib/heimdal-kdc \
--with-openssl=/usr \
--infodir=/usr/share/info \
--datarootdir=/usr/share \
--libdir=/usr/lib \
--without-krb4

make && make install

apt-get install krb5-config
remove the default realm from /etc/krb5.conf

#apt-get build-dep libsasl2-2
apt-get install chrpath default-libmysqlclient-dev diffstat docbook docbook-to-man libdb-dev libmariadbclient-dev libmariadbclient-dev-compat libmariadbclient18 libosp5 libpam0g-dev libsqlite3-dev mysql-common opensp quilt sgml-data
cd /usr/src/cyrus-sasl-2.1.27
./configure \
--prefix=/usr \
--mandir=/usr/share/man \
--infodir=/usr/share/info \
--enable-alwaystrue \
--enable-srp \
--enable-srp-setpass \
--enable-login \
--enable-ntlm \
--enable-passdss \
--enable-share \
--with-saslauthd=/var/run/saslauthd \
--with-authdaemond=/var/run/courier \
--sysconfdir=/etc \
--enable-httpform \
--with-devrandom=/dev/urandom \
--enable-gssapi \
--enable-gss_mutexes
#--enable-sql \
make && make install
ldconfig

verify no debian ldap or sasl packages have been installed as a dependency
along the way:

dpkg -l "*ldap*"
dpkg -l "*sasl*"

These responses are expected at this point:

~$ ldapsearch -Y GSSAPI
~$ ldapsearch -VV
~$ strings /usr/sbin/pluginviewer | grep 2\.1\.27
~$ ldd /usr/sbin/pluginviewer | grep 'krb5\|heim'
        libheimntlm.so.0 => /usr/lib/libheimntlm.so.0 (0x00007fbb7698b000)
        libkrb5.so.26 => /usr/lib/libkrb5.so.26 (0x00007fbb766fc000)
        libheimbase.so.1 => /usr/lib/libheimbase.so.1 (0x00007fbb75b37000)

~$ pluginviewer | grep GS
pluginviewer | grep GS
  SRP PASSDSS-3DES-1 SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 EXTERNAL OTP CRAM-MD5 NTLM PLAIN LOGIN GS2-KRB5 ANONYMOUS
  SRP PASSDSS-3DES-1 SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 OTP CRAM-MD5 NTLM PLAIN LOGIN GS2-KRB5 ANONYMOUS
        SASL mechanism: GSSAPI, best SSF: 256, supports setpass: no
        SASL mechanism: GSS-SPNEGO, best SSF: 256, supports setpass: no
        SASL mechanism: GS2-KRB5, best SSF: 0, supports setpass: no
        features: WANT_CLIENT_FIRST|GSS_FRAMING|CHANNEL_BINDING
  SRP PASSDSS-3DES-1 SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 EXTERNAL OTP CRAM-MD5 NTLM PLAIN LOGIN GS2-KRB5 ANONYMOUS
  SRP PASSDSS-3DES-1 SCRAM-SHA-256 SCRAM-SHA-1 GSSAPI GSS-SPNEGO DIGEST-MD5 EXTERNAL OTP CRAM-MD5 NTLM PLAIN LOGIN GS2-KRB5 ANONYMOUS
        SASL mechanism: GSSAPI, best SSF: 256
        SASL mechanism: GSS-SPNEGO, best SSF: 256
        SASL mechanism: GS2-KRB5, best SSF: 0
        features: WANT_CLIENT_FIRST|NEED_SERVER_FQDN|GSS_FRAMING|CHANNEL_BINDING

Recompile libldap with sasl support, and compile slapd

cd /usr/src
mv openldap-2.4.45 openldap-2.4.45-sans-sasl
tar -xvzf openldap-2.4.45.tgz
cd openldap-2.4.45

./configure \
--prefix=/usr \
--libexecdir=/usr/lib \
--sysconfdir=/etc \
--localstatedir=/var \
--mandir=/usr/share/man \
--enable-dynamic \
--enable-syslog \
--enable-proctitle \
--enable-ipv6 \
--enable-local \
--enable-slapd \
--enable-dynacl \
--enable-aci \
--enable-cleartext \
--enable-crypt \
--disable-lmpasswd \
--enable-spasswd \
--enable-modules \
--enable-rewrite \
--enable-rlookups \
--enable-slapi \
--enable-wrappers \
--enable-backends=mod \
--disable-ndb \
--enable-overlays=mod \
--with-cyrus-sasl \
--with-threads \
--with-tls=openssl \
--with-odbc=unixodbc

make depend
make && make install

Expected output:

~$ ldapsearch -VV
~$ ldapsearch -Y GSSAPI
~$ /usr/lib/slapd -d 2

halt
vboxmanage list runningvms
# If necessary
vboxmanage controlvm heimdal-host poweroff
`````

# kdc.example.org

````
vboxmanage clonevm heimdal-host --name kdc-example-org --register
vboxmanage modifyvm kdc-example-org --vrdeport 42007
vboxmanage startvm kdc-example-org --type=headless
rdesktop localhost:42007

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 203.0.113.7/24
gateway: 203.0.113.1
Hostname: kdc.example.org
reboot

specify default realm (default_realm = EXAMPLE.ORG) in /etc/krb5.conf

install -d -m 755 /var/lib/heimdal-kdc
kstash
Master key: passw0rd
Verify password - Master key: passw0rd
kstash: writing key to `/var/lib/heimdal-kdc/m-key'

install -d -m 755 /etc/heimdal
~$ kadmin -l
kadmin> init EXAMPLE.ORG
Realm max ticket life [unlimited]:
Realm max renewable ticket life [unlimited]:
kadmin> add dwhite
Max ticket life [1 day]:
Max renewable life [1 week]:
Principal expiration time [never]:
Password expiration time [never]:
Attributes []:
Policy [default]:
dwhite@EXAMPLE.ORG's Password: passw0rd
Verify password - dwhite@EXAMPLE.ORG's Password: passw0rd
kadmin> add --random-key host/ldap.example.org
kadmin> add --random-key ldap/ldap.example.org
kadmin> ext --keytab=/root/ldap.keytab host/ldap.example.org
kadmin> ext --keytab=/root/slapd.keytab ldap/ldap.example.org

at the end of /etc/krb5.conf, add:

[logging]
    kdc = FILE:/var/log/kdc.log
    admin_server = FILE:/var/log/kadmin.log
    default = FILE:/var/log/krb.log

/usr/sbin/kdc &

scp ldap.keytab slapd.keytab root@ldap.example.org:
````

# ldap.example.org

````
vboxmanage clonevm heimdal-host --name ldap-example-org --register
vboxmanage modifyvm ldap-example-org --vrdeport 42008
vboxmanage startvm ldap-example-org --type=headless
rdesktop localhost:42008

Modify /etc/network/interfaces, /etc/host, and /etc/hostname to reflect the following:
IP: 203.0.113.8/24
Gateway: 203.0.113.1
Hostname: ldap.example.org
reboot

kinit -k -t /root/ldap.keytab host/ldap.example.org@EXAMPLE.ORG
echo "pidfile /var/run/slapd.pid" > /etc/slapd.conf
mkdir /etc/sasl2
echo "keytab: /root/slapd.keytab" > /etc/sasl2/slapd.conf
/usr/lib/slapd -f /etc/slapd.conf
````
