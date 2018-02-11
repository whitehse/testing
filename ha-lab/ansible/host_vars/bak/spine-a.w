---

fabric:
  asn: 4200000006
  uplink_port_start: 1
  uplink_port_end: 6
  router_id: "192.0.2.6"
  loopback: 192.0.2.6
  loopbackv6: 2001:DB8::6/128

interfaces:
  0:
    link: spine-a.w_0

  1:
    link: spine-a.w_1

