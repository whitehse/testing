apt-get install docker.io

docker pull debian

docker image list

#docker start debian
docker run -i -t debian /bin/bash

apt-get update
apt-get install procps git sudo
mkdir -p /usr/src/git
cd /usr/src/git
git clone https://github.com/whitehse/lab-oss.git
cd lab-oss
apt-get install slapd bind9-dyndb-ldap ldap-utils python-ldap
./build-ldap.sh
cd ..
cd nslcd
apt-get install libpam-ldapd
./build-nslcd.sh
cd ..
cd freeradius
apt-get install freeradius freeradius-ldap
./build-freeradius.sh
