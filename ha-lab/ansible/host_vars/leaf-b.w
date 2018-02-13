---
fabric:
  asn: 4200000023
  router_id: 192.0.2.23
  loopback: 192.0.2.23
  #loopbackv6: 2001:DB8::23

interfaces:
  0:
    link: brspine-a.w_1
  1:
    link: brspine-b.w_1
  2:
    link: brleaf-b.w_2
  3:
    link: brleaf-b.w_3
  4:
    link: brleaf-b.w_4
  5:
    link: brleaf-b.w_5
  6:
    link: brleaf-b.w_6
  7:
    link: brleaf-b.w_7
