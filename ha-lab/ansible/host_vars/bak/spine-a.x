---

fabric:
  asn: 4200000004
  uplink_port_start: 1
  uplink_port_end: 6
  router_id: "192.0.2.4"
  loopback: 192.0.2.4
  loopbackv6: 2001:DB8::4/128

interfaces:
  0:
    link: spine-a.x_0

  1:
    link: spine-a.x_1

