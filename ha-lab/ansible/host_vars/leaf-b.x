---
fabric:
  asn: 4200000025
  router_id: 192.0.2.25
  loopback: 192.0.2.25
  #loopbackv6: 2001:DB8::25

interfaces:
  0:
    link: brspine-a.x_1
  1:
    link: brspine-b.x_1
  2:
    link: brleaf-b.x_2
  3:
    link: brleaf-b.x_3
