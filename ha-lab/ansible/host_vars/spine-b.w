---
fabric:
  asn: 4200000045
  router_id: 192.0.2.45
  loopback: 192.0.2.45
  loopbackv6: 2001:DB8::45

interfaces:
  23:
    link: isp-pe-1_
  0:
    link: spine-b.w_0
  1:
    link: spine-b.w_1
