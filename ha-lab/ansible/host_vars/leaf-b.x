---
fabric:
  asn: 4200000022
  router_id: 192.0.2.22
  loopback: 192.0.2.22
  loopbackv6: 2001:DB8::22

interfaces:
  0:
    link: spine-a.x_1
  1:
    link: spine-b.x_1
  2:
    link: leaf-b.x_2
  3:
    link: leaf-b.x_3
