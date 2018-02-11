---

fabric:
  asn: 4200000002
  uplink_port_start: 1
  uplink_port_end: 6
  router_id: "192.0.2.2"
  loopback: 192.0.2.2
  loopbackv6: 2001:DB8::2/128

interfaces:
  0:
    link: spine-a.y_0

  1:
    link: spine-a.y_1

