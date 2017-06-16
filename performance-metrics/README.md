~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SECURITY_GROUP_DESCRIPTION="Allow http/https, and ICMP, from same security group. Allow SSH from home."
MY_HOME_IP="203.0.113.234"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
INSTANCE_OUTPUT=`mktemp`
STRETCH_AMI="ami-0db79168"
INSTANCE_TYPE="r4.large"
~~~~

~~~~
SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
aws ec2 run-instances --image-id $STRETCH_AMI --count 1 --instance-type $INSTANCE_TYPE --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
IPV4_ADDR=`aws ec2 describe-instances | grep ASSOCIATION | head -1 | awk '{print $4}'`
~~~~

~~~~
echo ssh -i $SSH_PRIV_FILE admin@${IPV4_ADDR}
~~~~

~~~~
sudo apt-get update && sudo apt-get dist-upgrade
sudo apt-get install vim time bc git build-essential cmake
sudo sed -i 's/^.syntax on/syntax on/' /etc/vim/vimrc
sudo sed -i 's/^.set background=dark/set background=dark/' /etc/vim/vimrc
~~~~

~~~~
sudo apt-get install iotop sysstat dstat htop tcpdump nicstat lsof iptraf-ng golang perf-tools-unstable msr-tools lmbench fio pchar trace-cmd systemtap sysdig cscope

iostat -x 1
netstat -s
# interface counters
netstat -i
# socket stat
ss
vmstat 1
# reports processor related statistics
mpstat 1
iostat -xmdz 1
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
xvda              0.00     1.00    0.00    2.00     0.00     0.01    12.00     0.00    0.00    0.00    0.00   0.00   0.00
# -x display extended statistics
# -m display statistics in megabytes per second
# -d display the device utilization report
# -z Tell iostat to omit output for any devices for which there was no activity during the sample period.
# rrqm/s - The number of read requests merged per second that were queued to the device.
# wrqm/s - The number of write requests merged per second that were queued to the device.
# r/s    - The number (after merges) of read requests completed per second for the device.
# w/s    - The number (after merges) of write requests completed per second for the device.
# rMB/s  - The number of sectors (kilobytes, megabytes) read from the device per second.
# wMB/s  - The number of sectors (kilobytes, megabytes) written to the device per second.
# avgrq-sz - The average size (in sectors) of the requests that were issued to the device.
# avgqu-sz - The average queue length of the requests that were issued to the device.
# await  The average time (in milliseconds) for I/O requests issued to the device to be served. This includes  the  time spent by the requests in queue and the time spent servicing them.
# r_await  - The average time (in milliseconds) for read requests issued to the device to be served. This includes the time spent by the requests in queue and the time spent servicing them.
# w_await  - The average time (in milliseconds) for write requests issued to the device to be served.
# svctm    - The average service time (in milliseconds) for I/O requests that were issued to the device. Warning! Do not trust this field any more.  This field will be removed in a future sysstat version.
# %util    - Percentage of elapsed time during which I/O requests were issued to the device (bandwidth utilization for the device). Device saturation occurs when this value is close to 100% for devices serving requests serially. But for devices serving requests in parallel, such as RAID arrays and modern SSDs, this number does not reflect their performance limits.

# Display interface rates and errors, as well as tcp/udp info
nicstat -a [-p]
pidstat -t 1
# Show per process disk activity
pidstat -d 1
lsof -iTCP -sTCP:ESTABLISHED 
ss -mop
# -m - Show socket memory usage.
# -o - Show timer information.
# -p - Show process using socket.
ss -i
# -i Show internal TCP information.

iptraf-ng
slabtop

sudo apt-get install golang
mkdir ~/golang
go get golang.org/x/sys/unix
go get github.com/tobert/pcstat/pcstat
PATH="$PATH:~/golang/bin"
pcstat `which ls`
apt-get install msr-tools
mkdir ~/src
cd ~/src
git clone https://github.com/brendangregg/msr-cloud-tools.git
cd msr-cloud-tools
sudo ./showboost
sudo ./cputemp -u 1
sudo ./cpuhot
sudo lmbench-run
sudo perf record -F 99 -ag -- sleep 30
perf report -n --stdio
cd ~/src
git clone --depth 1 https://github.com/brendangregg/FlameGraph
cd FlameGraph
sudo perf record -F 99 -ag -- sleep 30
perf script | ./stackcollapse-perf.pl | ./flamegraph.pl > perf.svg
# display (snoop) system new execs
sudo /usr/sbin/execsnoop-perf

# bcc
sudo apt-get install libllvm3.8 llvm-3.8-dev libclang-3.8-dev \
  libelf-dev bison flex libedit-dev clang-format-3.8 python python-netaddr \
  python-pyroute2 luajit libluajit-5.1-dev arping iperf netperf ethtool \
  devscripts zlib1g-dev

cd ~/src
git clone https://github.com/iovisor/bcc.git
cd bcc
# removed the two python bridge tests, and the smoke test, from tests/python/CMakeLists.txt
sudo debuild -b -uc -us
cd ..
sudo dpkg -i *bcc*.deb
cd /usr/share/bcc/tools/
for foo in `ls | grep -v '\.c\|doc\|lib\|old'`
do
  which $foo
  if [ $? -eq 1 ]; then
    sudo ln -s /usr/share/bcc/tools/$foo /usr/sbin/
  fi
done

sudo iosnoop-perf -ts

# Need systemtap examples

# sysdig
sudo sysdig fd.type=file and evt.failed=true
sudo sysdig evt.type=open and fd.name contains /etc
sudo sysdig -p"%proc.name %fd.name" "evt.type=acceptand proc.name!=httpd"

# cscope
sudo apt-get install linux-source-4.9
cd /usr/src/
sudo tar -xf linux-source-4.9.tar.xz
cd /usr/src/linux-source-4.9
cscope -b -R
cscope -dq

# funccount
sudo funccount -i 1 "*icmp*"

sudo kprobe-perf 'p:icmp_out_count'
# print out the icmp type:
sudo kprobe-perf 'p:icmp_out_count %si'

admin@ip-172-31-45-129:/usr/src/linux-source-4.9$ sudo kprobe-perf 'p:icmp_out_count %si'
Tracing kprobe icmp_out_count. Ctrl-C to end.
            ping-24389 [001] d...  7053.958110: icmp_out_count: (icmp_out_count+0x0/0x30) arg1=0x8
            ping-24389 [001] d...  7054.960090: icmp_out_count: (icmp_out_count+0x0/0x30) arg1=0x8
            ping-24389 [001] d...  7055.962104: icmp_out_count: (icmp_out_count+0x0/0x30) arg1=0x8

# kprobe-perf -s will give stack backtraces

sudo funcgraph-perf icmp_rcv
sudo funcslower-perf icmp_rcv 0 # if it exceeds 0

sudo perf stat ls
sudo perf list

# drop caches
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"

# biolatency
sudo biolatency
# another shell
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
cksum /usr/bin/*

sudo tcptop

sudo ttysnoop /dev/pts/2
