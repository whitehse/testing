---
fabric:
  asn: 4200000018
  router_id: 192.0.2.18
  loopback: 192.0.2.18
  loopbackv6: 2001:DB8::18

interfaces:
  0:
    link: spine-a.x_0
  1:
    link: spine-b.x_0
  2:
    link: leaf-a.x_2
  3:
    link: leaf-a.x_3
