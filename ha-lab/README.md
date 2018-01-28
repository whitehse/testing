apt-get install kpartx
#mount vmware-server-flat.vmdk /tmp/test/ -o ro,loop=/dev/loop1,offset=32768
kpartx -av <image-flat.vmdk>; mount -o /dev/mapper/loop0p1 /mnt/vmdk
kpartx -dv cumulus-linux-3.5.1-vx-amd64-1515733196.bab9bc5z95389d3-disk1.vmdk
sudo onie-select -f -i
sudo reboot

sudo addgroup --system pcap
sudo adduser cumulus pcap
newgrp pcap
sudo chgrp pcap /usr/sbin/tcpdump
sudo setcap cap_net_raw,cap_net_admin=eip /usr/sbin/tcpdump

apt-get -y install vagrant

vagrant init debian/jessie64
vagrant up

vagrant init CumulusCommunity/cumulus-vx
vagrant up
