````
su -
apt-get install vde2 apt-move
mkdir -p /var/local/uml
cat - > /etc/apt-move.conf << EOF
LOCALDIR=/var/local/mirrors
DIST=squeeze
COPYONLY=yes
EOF
cd /var/local/uml
mkdir firewall
debootstrap --arch amd64 --variant minbase stable firewall 	http://ftp.us.debian.org/debian/
real    2m43.374s
user    1m0.545s
sys     0m15.783s

````
