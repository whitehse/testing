---
fabric:
  asn: 4200000021
  router_id: 192.0.2.21
  loopback: 192.0.2.21
  loopbackv6: 2001:DB8::21

interfaces:
  0:
    link: spine-a.w_1
  1:
    link: spine-b.w_1
  2:
    link: leaf-b.w_2
  3:
    link: leaf-b.w_3
  4:
    link: leaf-b.w_4
  5:
    link: leaf-b.w_5
  6:
    link: leaf-b.w_6
  7:
    link: leaf-b.w_7
