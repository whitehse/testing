````
# example_com - 198.51.100.0/24 - MIT KDC
# example_org - 203.0.113.0/24 - Heimdal KDC

su -
apt-get install user-mode-linux uml-utilities vde2 debootstrap
mkdir -p /var/local/uml
cd /var/local/uml

vde_switch -daemon -s ./example.com
vde_switch -daemon -s ./example.org

tunctl -t tap27
ip addr add 192.168.27.1/24 dev tap27
ip link set	tap27 up

#for foo in firewall kdc.example.com ldap27.example.com ldap26.example.com ldap23.example.com imap.example.com kdc.example.rg ldap27.example.org ldap26.example.org ldap23.example.org imap.example.org
for foo in firewall
do
  mkdir $foo
  truncate -s 10G ${foo}.img
  mke2fs -t ext3 ${foo}.img
done

mount firewall.img firewall
debootstrap --arch amd64 --variant minbase --include less,vim,sudo,dropbear-run,ifupdown,iproute2,iputils-ping,iptables stable firewall http://ftp.us.debian.org/debian/
umount firewall

sed -i 's/^#net\.ipv4\.ip_forward.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sysctl -p
#iptables -P OUTPUT ACCEPT
# -o should use your publically facing interface
iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
iptables -I FORWARD -i tap27 -j ACCEPT
iptables -I FORWARD -o tap27 -j ACCEPT

linux ubd0=firewall.img mem=128M con=pty con0=fd:0,fd:1 eth0=tuntap,tap27,f0:00:00:00:01,192.168.27.1 eth1=vde,./example.com eth2=vde,./example.org umid=firewall
ip addr add 192.168.27.2/24 dev eth0
ip link set eth0 up
ip route add default via 192.168.27.1 dev eth0
echo "root:passw0rd"|chpasswd
````
