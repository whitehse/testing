---
fabric:
  asn: 4200000021
  router_id: 192.0.2.21
  loopback: 192.0.2.21
  #loopbackv6: 2001:DB8::21

interfaces:
  0:
    link: brspine-a.x_0
  1:
    link: brspine-b.x_0
  2:
    link: brleaf-a.x_2
  3:
    link: brleaf-a.x_3
