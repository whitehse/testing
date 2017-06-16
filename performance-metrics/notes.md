~~~~
top
iotop
iostat -x 1
netstat -s
netstat -i
ss [-n]
# -c - enable cpu stats (system, user, idle, wait), for more CPU related stats also see --cpu-adv and --cpu-use
# -d - enable disk stats (read, write), for more disk related stats look into the other --disk plugins
dstat -c -d -g -i -l -m -n -p -r -s -t -y --aio --cpu-adv --fs --ipc --lock --mem-adv --raw --socket --tcp --udp --unix --vm --vm-adv --zones --nocolor --noheaders --output /tmp/dstat.out


dstat --nocolor --output /tmp/dstat.csv \
-T \
-d -D xvdb \
-g \
-i \ 
--mem-adv --swap \
-n -N eth0 \
-p \
-r \
-y \
--aio \
--cpu-adv \
--fs \
--ipc \
--socket \
--tcp \
--udp \
--unix \
--vm --vm-adv

sed -i '1,6d' /tmp/dstat.csv
echo "timestamp,xvdb-read,xvdb-write,paging-in,paging-out,interrupt-48,interrupt-49,interrupt-50,total,used,free,buff,cach,dirty,shmem,recl,swap-used,swap-free,eth0-recv,eth0-send,run,blk,new,io-total-read,io-total-write,interrupts,context-switches,aio,cpu0-usr,cpu0-sys,cpu0-idl,cpu0-wai,cpu0-hiq,cpu0-siq,cpu0-stl,cpu1-usr,cpu1-sys,cpu1-idl,cpu1-wai,cpu1-hiq,cpu1-siq,cpu1-stl,cpu2-usr,cpu2-sys,cpu2-idl,cpu2-wai,cpu2-hiq,cpu2-siq,cpu2-stl,cpu3-usr,cpu3-sys,cpu3-idl,cpu3-wai,cpu3-hiq,cpu3-siq,cpu3-stl,cpu4-usr,cpu4-sys,cpu4-idl,cpu4-wai,cpu4-hiq,cpu4-siq,cpu4-stl,cpu5-usr,cpu5-sys,cpu5-idl,cpu5-wai,cpu5-hiq,cpu5-siq,cpu5-stl,cpu6-usr,cpu6-sys,cpu6-idl,cpu6-wai,cpu6-hiq,cpu6-siq,cpu6-stl,cpu7-usr,cpu7-sys,cpu7-idl,cpu7-wai,cpu7-hiq,cpu7-siq,cpu7-stl,cpu8-usr,cpu8-sys,cpu8-idl,cpu8-wai,cpu8-hiq,cpu8-siq,cpu8-stl,cpu9-usr,cpu9-sys,cpu9-idl,cpu9-wai,cpu9-hiq,cpu9-siq,cpu9-stl,cpu10-usr,cpu10-sys,cpu10-idl,cpu10-wai,cpu10-hiq,cpu10-siq,cpu10-stl,cpu11-usr,cpu11-sys,cpu11-idl,cpu11-wai,cpu11-hiq,cpu11-siq,cpu11-stl,cpu12-usr,cpu12-sys,cpu12-idl,cpu12-wai,cpu12-hiq,cpu12-siq,cpu12-stl,cpu13-usr,cpu13-sys,cpu13-idl,cpu13-wai,cpu13-hiq,cpu13-siq,cpu13-stl,cpu14-usr,cpu14-sys,cpu14-idl,cpu14-wai,cpu14-hiq,cpu14-siq,cpu14-stl,cpu15-usr,cpu15-sys,cpu15-idl,cpu15-wai,cpu15-hiq,cpu15-siq,cpu15-stl,cpu16-usr,cpu16-sys,cpu16-idl,cpu16-wai,cpu16-hiq,cpu16-siq,cpu16-stl,cpu17-usr,cpu17-sys,cpu17-idl,cpu17-wai,cpu17-hiq,cpu17-siq,cpu17-stl,cpu18-usr,cpu18-sys,cpu18-idl,cpu18-wai,cpu18-hiq,cpu18-siq,cpu18-stl,cpu19-usr,cpu19-sys,cpu19-idl,cpu19-wai,cpu19-hiq,cpu19-siq,cpu19-stl,cpu20-usr,cpu20-sys,cpu20-idl,cpu20-wai,cpu20-hiq,cpu20-siq,cpu20-stl,cpu21-usr,cpu21-sys,cpu21-idl,cpu21-wai,cpu21-hiq,cpu21-siq,cpu21-stl,cpu22-usr,cpu22-sys,cpu22-idl,cpu22-wai,cpu22-hiq,cpu22-siq,cpu22-stl,cpu23-usr,cpu23-sys,cpu23-idl,cpu23-wai,cpu23-hiq,cpu23-siq,cpu23-stl,cpu24-usr,cpu24-sys,cpu24-idl,cpu24-wai,cpu24-hiq,cpu24-siq,cpu24-stl,cpu25-usr,cpu25-sys,cpu25-idl,cpu25-wai,cpu25-hiq,cpu25-siq,cpu25-stl,cpu26-usr,cpu26-sys,cpu26-idl,cpu26-wai,cpu26-hiq,cpu26-siq,cpu26-stl,cpu27-usr,cpu27-sys,cpu27-idl,cpu27-wai,cpu27-hiq,cpu27-siq,cpu27-stl,cpu28-usr,cpu28-sys,cpu28-idl,cpu28-wai,cpu28-hiq,cpu28-siq,cpu28-stl,cpu29-usr,cpu29-sys,cpu29-idl,cpu29-wai,cpu29-hiq,cpu29-siq,cpu29-stl,cpu30-usr,cpu30-sys,cpu30-idl,cpu30-wai,cpu30-hiq,cpu30-siq,cpu30-stl,cpu31-usr,cpu31-sys,cpu31-idl,cpu31-wai,cpu31-hiq,cpu31-siq,cpu31-stl,cpu32-usr,cpu32-sys,cpu32-idl,cpu32-wai,cpu32-hiq,cpu32-siq,cpu32-stl,cpu33-usr,cpu33-sys,cpu33-idl,cpu33-wai,cpu33-hiq,cpu33-siq,cpu33-stl,cpu34-usr,cpu34-sys,cpu34-idl,cpu34-wai,cpu34-hiq,cpu34-siq,cpu34-stl,cpu35-usr,cpu35-sys,cpu35-idl,cpu35-wai,cpu35-hiq,cpu35-siq,cpu35-stl,cpu36-usr,cpu36-sys,cpu36-idl,cpu36-wai,cpu36-hiq,cpu36-siq,cpu36-stl,cpu37-usr,cpu37-sys,cpu37-idl,cpu37-wai,cpu37-hiq,cpu37-siq,cpu37-stl,cpu38-usr,cpu38-sys,cpu38-idl,cpu38-wai,cpu38-hiq,cpu38-siq,cpu38-stl,cpu39-usr,cpu39-sys,cpu39-idl,cpu39-wai,cpu39-hiq,cpu39-siq,cpu39-stl,cpu40-usr,cpu40-sys,cpu40-idl,cpu40-wai,cpu40-hiq,cpu40-siq,cpu40-stl,cpu41-usr,cpu41-sys,cpu41-idl,cpu41-wai,cpu41-hiq,cpu41-siq,cpu41-stl,cpu42-usr,cpu42-sys,cpu42-idl,cpu42-wai,cpu42-hiq,cpu42-siq,cpu42-stl,cpu43-usr,cpu43-sys,cpu43-idl,cpu43-wai,cpu43-hiq,cpu43-siq,cpu43-stl,cpu44-usr,cpu44-sys,cpu44-idl,cpu44-wai,cpu44-hiq,cpu44-siq,cpu44-stl,cpu45-usr,cpu45-sys,cpu45-idl,cpu45-wai,cpu45-hiq,cpu45-siq,cpu45-stl,cpu46-usr,cpu46-sys,cpu46-idl,cpu46-wai,cpu46-hiq,cpu46-siq,cpu46-stl,cpu47-usr,cpu47-sys,cpu47-idl,cpu47-wai,cpu47-hiq,cpu47-siq,cpu47-stl,cpu48-usr,cpu48-sys,cpu48-idl,cpu48-wai,cpu48-hiq,cpu48-siq,cpu48-stl,cpu49-usr,cpu49-sys,cpu49-idl,cpu49-wai,cpu49-hiq,cpu49-siq,cpu49-stl,cpu50-usr,cpu50-sys,cpu50-idl,cpu50-wai,cpu50-hiq,cpu50-siq,cpu50-stl,cpu51-usr,cpu51-sys,cpu51-idl,cpu51-wai,cpu51-hiq,cpu51-siq,cpu51-stl,cpu52-usr,cpu52-sys,cpu52-idl,cpu52-wai,cpu52-hiq,cpu52-siq,cpu52-stl,cpu53-usr,cpu53-sys,cpu53-idl,cpu53-wai,cpu53-hiq,cpu53-siq,cpu53-stl,cpu54-usr,cpu54-sys,cpu54-idl,cpu54-wai,cpu54-hiq,cpu54-siq,cpu54-stl,cpu55-usr,cpu55-sys,cpu55-idl,cpu55-wai,cpu55-hiq,cpu55-siq,cpu55-stl,cpu56-usr,cpu56-sys,cpu56-idl,cpu56-wai,cpu56-hiq,cpu56-siq,cpu56-stl,cpu57-usr,cpu57-sys,cpu57-idl,cpu57-wai,cpu57-hiq,cpu57-siq,cpu57-stl,cpu58-usr,cpu58-sys,cpu58-idl,cpu58-wai,cpu58-hiq,cpu58-siq,cpu58-stl,cpu59-usr,cpu59-sys,cpu59-idl,cpu59-wai,cpu59-hiq,cpu59-siq,cpu59-stl,cpu60-usr,cpu60-sys,cpu60-idl,cpu60-wai,cpu60-hiq,cpu60-siq,cpu60-stl,cpu61-usr,cpu61-sys,cpu61-idl,cpu61-wai,cpu61-hiq,cpu61-siq,cpu61-stl,cpu62-usr,cpu62-sys,cpu62-idl,cpu62-wai,cpu62-hiq,cpu62-siq,cpu62-stl,cpu63-usr,cpu63-sys,cpu63-idl,cpu63-wai,cpu63-hiq,cpu63-siq,cpu63-stl,files,inodes,msg,sem,shm,socket-tot,tcp,udp,raw,frg,tcp-lis,tcp-act,tcp-syn,tcp-tim,tcp-clo,udp-lis,udp-act,unix-dgm,unix-str,unix-lis,unix-act,majpf,minpf,alloc,vm-free,steal,scank,scand,pgoru,astll" > dstat.header

cat /tmp/dstat.header /tmp/dstat.csv > /tmp/dstat-run.csv

dwhite@quark:~$ iostat -x
Linux 4.9.0-2-amd64 (quark)     06/14/2017      _x86_64_        (2 CPU)

avg-cpu:  %user   %nice %system %iowait  %steal   %idle
          11.04    0.03    4.24    0.80    0.31   83.59

Device:         rrqm/s   wrqm/s     r/s     w/s    rkB/s    wkB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.16     7.58    1.31    4.69    12.41    93.83    35.42     0.07   11.74    6.31   13.26   2.62   1.57
dm-0              0.00     0.00    1.47   12.21    12.33    93.83    15.52     0.34   24.87   10.70   26.58   1.18   1.61





netstat -i

Ip:
    Forwarding: 1
    4104045 total packets received
    0 forwarded
    0 incoming packets discarded
    3928651 incoming packets delivered
    2427879 requests sent out
    963 outgoing packets dropped
    679 dropped because of missing route
Icmp:
    1734 ICMP messages received
    108 input ICMP message failed
    ICMP input histogram:
        destination unreachable: 1512
        echo replies: 222
    3562 ICMP messages sent
    0 ICMP messages failed
    ICMP output histogram:
        destination unreachable: 2029
        echo requests: 1533
IcmpMsg:
        InType0: 222
        InType3: 1512
        OutType3: 2029
        OutType8: 1533
Tcp:
    23231 active connection openings
    0 passive connection openings
    1466 failed connection attempts
    3634 connection resets received
    35 connections established
    2356356 segments received
    1619454 segments sent out
    5068 segments retransmitted
    301 bad segments received
    14660 resets sent
Udp:
    1586859 packets received
    1515 packets to unknown port received
    711 packet receive errors
    824023 packets sent
    711 receive buffer errors
    963 send buffer errors
    IgnoredMulti: 5759
UdpLite:
TcpExt:
    59 packets pruned from receive queue because of socket buffer overrun
    44 ICMP packets dropped because they were out-of-window
    4890 TCP sockets finished time wait in fast timer
    4 packetes rejected in established connections because of timestamp
    48140 delayed acks sent
    33 delayed acks further delayed because of locked socket
    Quick ack mode was activated 3512 times
    644186 packets directly queued to recvmsg prequeue
    2942089 bytes directly in process context from backlog
    TCPDirectCopyFromPrequeue: 1001298202
    TCPPrequeueDropped: 32
    974303 packet headers predicted
    798911 packet headers predicted and directly queued to user
    108622 acknowledgments not containing data payload received
    73476 predicted acknowledgments
    28 times recovered from packet loss due to fast retransmit
    TCPSackRecovery: 8
    24 congestion windows fully recovered without slow start
    TCPDSACKUndo: 47
    453 congestion windows recovered without slow start after partial ack
    TCPLostRetransmit: 3
    5 timeouts after reno fast retransmit
    TCPSackFailures: 1
    9 timeouts in loss state
    47 fast retransmits
    1 forward retransmits
    82 retransmits in slow start
    TCPTimeouts: 1092
    TCPLossProbes: 971
    TCPLossProbeRecovery: 48
    TCPRenoRecoveryFail: 2
    TCPSackRecoveryFail: 1
    106 times receiver scheduled too late for direct processing
    639 packets collapsed in receive queue due to low socket buffer
    TCPDSACKOldSent: 3165
    TCPDSACKOfoSent: 33
    TCPDSACKRecv: 647
    2745 connections reset due to unexpected data
    2835 connections reset due to early user close
    271 connections aborted due to timeout
    TCPDSACKIgnoredOld: 4
    TCPDSACKIgnoredNoUndo: 133
    TCPSpuriousRTOs: 75
    TCPSackShiftFallback: 61
    TCPRetransFail: 24
    TCPRcvCoalesce: 830816
    TCPOFOQueue: 82722
    TCPOFOMerge: 33
    TCPChallengeACK: 296
    TCPSYNChallenge: 301
    TCPSpuriousRtxHostQueues: 265
    TCPAutoCorking: 7929
    TCPWantZeroWindowAdv: 175
    TCPSynRetrans: 918
    TCPOrigDataSent: 193534
    TCPHystartDelayDetect: 8
    TCPHystartDelayCwnd: 225
    TCPACKSkippedPAWS: 1
    TCPACKSkippedChallenge: 6
    TCPKeepAlive: 46269
IpExt:
    InNoRoutes: 10
    InMcastPkts: 25840
    OutMcastPkts: 26575
    InBcastPkts: 7144
    OutBcastPkts: 3073
    InOctets: 4536773400
    OutOctets: 277016668
    InMcastOctets: 1821609
    OutMcastOctets: 3745711
    InBcastOctets: 588240
    OutBcastOctets: 390390
    InNoECTPkts: 4104041
    InECT0Pkts: 4
Sctp:
    0 Current Associations
    0 Active Associations
    0 Passive Associations
    0 Number of Aborteds 
    0 Number of Graceful Terminations
    0 Number of Out of Blue packets
    0 Number of Packets with invalid Checksum
    0 Number of control chunks sent
    0 Number of ordered chunks sent
    0 Number of Unordered chunks sent
    0 Number of control chunks received
    0 Number of ordered chunks received
    0 Number of Unordered chunks received
    0 Number of messages fragmented
    0 Number of messages reassembled 
    0 Number of SCTP packets sent
    0 Number of SCTP packets received




dwhite@quark:~$ sar -n TCP 1
Linux 4.9.0-2-amd64 (quark)     06/14/2017      _x86_64_        (2 CPU)

03:16:52 PM  active/s passive/s    iseg/s    oseg/s
03:16:53 PM      0.00      0.00      0.00      0.00
03:16:54 PM      0.00      0.00      1.00      1.00
03:16:55 PM      0.00      0.00      0.00      0.00
03:16:56 PM      0.00      0.00      0.00      0.00



dwhite@quark:~$ sar -n ETCP 1
Linux 4.9.0-2-amd64 (quark)     06/14/2017      _x86_64_        (2 CPU)

03:17:08 PM  atmptf/s  estres/s retrans/s isegerr/s   orsts/s
03:17:09 PM      0.00      0.00      0.00      0.00      0.00
03:17:10 PM      0.00      0.00      0.00      0.00      0.00
03:17:11 PM      0.00      0.00      0.00      0.00      0.00





dwhite@quark:~$ vmstat 1
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 2  0      0 1613996 898792 2711036    0    0    17   129  241  220 11  4 84  1  0
 0  0      0 1609904 898792 2715140    0    0     0     0 1042 1251  1  2 97  0  1
 2  0      0 1610160 898796 2715144    0    0     0   304 1379 1632  1  2 90  7  0
 0  0      0 1614244 898796 2711048    0    0     0     0 1136 1279  2  1 98  0  0



http://www.brendangregg.com/USEmethod/use-linux.html:

