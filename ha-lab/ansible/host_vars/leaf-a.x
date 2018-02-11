---
fabric:
  asn: 4200000020
  router_id: 192.0.2.20
  loopback: 192.0.2.20
  loopbackv6: 2001:DB8::20

interfaces:
  0:
    link: spine-a.x_0
  1:
    link: spine-b.x_0
  2:
    link: leaf-a.x_2
  3:
    link: leaf-a.x_3
