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

raw.lxc: |-
  lxc.aa_profile=unconfined
  lxc.cgroup.devices.allow=a
  lxc.mount.auto=proc:rw sys:ro cgroup:ro
  lxc.kmsg=0
  lxc.autodev=1

adduser frr frrvty
