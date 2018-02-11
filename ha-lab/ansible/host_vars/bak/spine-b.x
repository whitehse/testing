---

fabric:
  asn: 4200000005
  uplink_port_start: 1
  uplink_port_end: 6
  router_id: "192.0.2.5"
  loopback: 192.0.2.5
  loopbackv6: 2001:DB8::5/128

interfaces:
  0:
    link: spine-b.x_0

  1:
    link: spine-b.x_1

