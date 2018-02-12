````
 bgp bestpath as-path multipath-relax
 bgp bestpath compare-routerid
 neighbor fabric peer-group
 neighbor fabric remote-as external
 neighbor fabric description Internal Fabric Network
 neighbor fabric capability extended-nexthop
 neighbor swp1 interface peer-group fabric
 neighbor swp2 interface peer-group fabric
 neighbor swp3 interface peer-group fabric
 neighbor swp4 interface peer-group fabric
 neighbor swp29 interface peer-group fabric
 neighbor swp30 interface peer-group fabric

#raw.lxc: |-
#  lxc.aa_profile=unconfined
#  lxc.cgroup.devices.allow=a
#  lxc.mount.auto=proc:rw sys:ro cgroup:ro
#  lxc.kmsg=0
#  lxc.autodev=1

adduser frr frrvty
cp /usr/share/doc/frr/examples/bgpd.conf.sample /etc/frr/bgpd.conf
cp /usr/share/doc/frr/examples/zebra.conf.sample /etc/frr/zebra.conf

service frr start

vtysh
sh mem
sh bpg mem


ASNs:

Private:
  64512 - 65534
  4200000000 - 4294967294

Documentation:
  64496 - 64511
  65536 - 65551

IPs:

Private Subnets:
  10.0.0.0 – 10.255.255.255
  172.16.0.0 – 172.31.255.255
  192.168.0.0 – 192.168.255.255
  fc00::/7

Documentation:
  192.0.2.0/24
  198.51.100.0/24
  203.0.113.0/24
  2001:DB8::/32
````
