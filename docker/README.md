apt-get install docker.io

docker pull debian

docker image list

#docker start debian
docker run -i -t debian /bin/bash

apt-get update
apt-get install procps git sudo vim
mkdir -p /usr/src/git
cd /usr/src/git
git clone https://github.com/whitehse/lab-oss.git
cd lab-oss
apt-get install slapd bind9-dyndb-ldap ldap-utils python-ldap dnsutils
cd ldap
./build-ldap.sh
sed -i 's/^#BASE.*/BASE     dc=example,dc=net/' /etc/ldap/ldap.conf
sed -i 's/^#URI.*/URI     ldapi:\/\//' /etc/ldap/ldap.conf
cat > ~/.ldaprc << EOF
SASL_MECH EXTERNAL
EOF
cd ..
cd nslcd
apt-get install libpam-ldapd
./build-nslcd.sh
cd ..
cd freeradius
apt-get install freeradius freeradius-ldap
./build-freeradius.sh
cd ..
cd bind-dyndb-ldap
./build-bind-ldap.sh
cd
#apt-get install libpyside2-py3-5.11 pyside2-tools python3-pyside2.qtwidgets python3-pyside2.qtgui python3-pyside2.qtcore
