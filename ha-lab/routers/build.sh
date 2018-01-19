#!/bin/bash

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n router-a
sudo cp router-a.config /var/lib/lxc/router-a/config
sudo lxc-start -n router-a
sudo ifconfig router-a-uplink 100.64.0.1/24
# Configure masquerading on the host if necessary
sudo iptables -P INPUT ACCEPT
sudo iptables -P FORWARD ACCEPT
sudo iptables -P OUTPUT ACCEPT
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo sed -i 's/^#net\.ipv4\.ip_forward.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sudo sysctl -p
#
sudo lxc-attach -n router-a /bin/bash -l -s << EOF
ifdown eth0
cat > /etc/network/interfaces << FOE
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
	address 100.64.0.2
	netmask 255.255.255.0
	gateway 100.64.0.1
FOE
ifup eth0
apt-get update && apt-get -y dist-upgrade
apt-get -y install bird bird-bgp iputils-ping gzip wget less vim
EOF
sudo lxc-attach -n router-a /bin/bash -l -s << EOF
cat > /etc/bird/bird.conf << FOE
router id 100.64.0.1;

protocol device {
}

# The Kernel protocol is not a real routing protocol. Instead of communicating
# with other routers in the network, it performs synchronization of BIRD's
# routing tables with the OS kernel.
protocol kernel {
        metric 64;      # Use explicit kernel route metric to avoid collisions
                        # with non-BIRD routes in the kernel routing table
        import all;
       export all;     # Actually insert routes into the kernel routing table
}

protocol bgp {
        description "My BGP uplink";
        local as 65000;
        neighbor 100.64.0.1 as 64512;
}
FOE
/etc/init.d/bird reload
EOF
