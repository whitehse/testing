---
fabric:
  asn: 4200000040
  router_id: 192.0.2.40
  loopback: 192.0.2.40
  loopbackv6: 2001:DB8::40

interfaces:
  24:
    link: isp-pe-3_
  0:
    link: spine-a.x_0
  1:
    link: spine-a.x_1
