~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SECURITY_GROUP_DESCRIPTION="Allow http/https, and ICMP, from same security group. Allow SSH from home."
MY_HOME_IP="203.0.113.234"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
INSTANCE_OUTPUT=`mktemp`
JESSIE_AMI="ami-b2795cd7"
INSTANCE_TYPE="r4.large"
~~~~

~~~~
aws ec2 create-key-pair --key-name $KEY_NAME | sed 's/.*-----BEGIN/-----BEGIN/' | sed 's/KEY-----.*/KEY-----/' > $SSH_PRIV_FILE
chmod 600 $SSH_PRIV_FILE
~~~~

~~~~
SECURITY_GROUP_ID=`aws ec2 create-security-group --group-name $SECURITY_GROUP_NAME --description "$SECURITY_GROUP_DESCRIPTION"`
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol icmp --port -1 --source-group $SECURITY_GROUP_NAME
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol tcp --port 80 --source-group $SECURITY_GROUP_NAME
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol tcp --port 443 --source-group $SECURITY_GROUP_NAME
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol tcp --port 22 --cidr ${MY_HOME_IP}/32
#SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
aws ec2 run-instances --image-id $JESSIE_AMI --count 1 --instance-type $INSTANCE_TYPE --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
# Figure this out to resize
VOLUME_ID=`aws ec2 describe-instances --instance-id=$INSTANCE_ID | grep vol- | awk '{print $5}'`
aws ec2 modify-volume --volume-id $VOLUME_ID --size 16
sleep 10
IPV4_ADDR=`aws ec2 describe-instances | grep ASSOCIATION | head -1 | awk '{print $4}'`
# aws ec2 terminate-instances --instance-ids $INSTANCE_ID
~~~~

~~~~
ssh -i $SSH_PRIV_FILE admin@${IPV4_ADDR}
~~~~

~~~~
Run sudo parted and figure out how to extend /xvda1 to fill up 16GBs
OLDIFS="$IFS"
IFS='
'
new_sources=`mktemp`
for foo in `cat /etc/apt/sources.list`
do
  echo "$foo" | sed 's/jessie/stretch/g' >> $new_sources
  echo "$foo" | grep -q "deb-src"
  if [ $? -ne 0 ]; then
    echo "$foo" | sed 's/^deb /deb-src /' | sed 's/jessie/stretch/g' >> $new_sources
  fi
done
IFS="$OLDIFS"
sudo mv $new_sources /etc/apt/sources.list
sudo apt-get update && sudo apt-get dist-upgrade
  Configuration file '/etc/systemd/logind.conf': N
  What do you want to do about modified configuration file grub? keep the local version currently installed
  What do you want to do about modified configuration file sshd_config? keep the local version currently installed
sudo apt-get dist-upgrade
  Configuration file '/etc/cloud/cloud.cfg': N
sudo reboot
ssh -i $SSH_PRIV_FILE admin@${IPV4_ADDR}
sudo apt-get install vim time bc git build-essential cmake iotop sysstat dstat htop tcpdump nicstat lsof iptraf-ng golang perf-tools-unstable lmbench fio pchar trace-cmd systemtap sysdig cscope libllvm3.8 llvm-3.8-dev libclang-3.8-dev libelf-dev bison flex libedit-dev clang-format-3.8 python python-netaddr python-pyroute2 luajit libluajit-5.1-dev arping iperf netperf ethtool devscripts linux-source-4.9 libssl-dev g++ make binutils autoconf automake autotools-dev libtool pkg-config zlib1g-dev libcunit1-dev libxml2-dev libev-dev libevent-dev libjansson-dev libc-ares-dev libjemalloc-dev libsystemd-dev cython python3-dev python-setuptools numactl libuv1-dev msr-tools asciidoc xmlto
sudo apt-get build-dep liblmdb-dev

# unzip linux source
cd /usr/src
sudo tar -xf linux-source-4.9.tar.xz
cd linux-source-4.9
cscope -b -R
~~~~

trace tools:
~~~~
sudo GOPATH=/usr/local go get golang.org/x/sys/unix
sudo GOPATH=/usr/local go get github.com/tobert/pcstat/pcstat
mkdir ~/src
cd ~/src
git clone https://github.com/brendangregg/msr-cloud-tools.git
git clone --depth 1 https://github.com/brendangregg/FlameGraph
git clone https://github.com/iovisor/bcc.git
cd bcc
cscope -b -R
#comment out dh_auto_test in debian/rules
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
cd ~/src/
wget http://http.debian.net/debian/pool/main/d/dstat/dstat_0.7.3.orig.tar.gz
tar -xvzf dstat_0.7.3.orig.tar.gz
cd dstat-0.7.3
wget wget http://http.debian.net/debian/pool/main/d/dstat/dstat_0.7.3-1.debian.tar.xz
tar -xf dstat_0.7.3-1.debian.tar.xz
sudo debuild -b -uc -us
cd ..
sudo dpkg -i *dstat*deb
~~~~

h2o
~~~~
cd ~/src
wget https://github.com/h2o/h2o/archive/v2.2.2.tar.gz
tar -xvzf v2.2.2.tar.gz
cd h2o-2.2.2
cscope -b -R
mkdir build
cd build
cmake -DWITH_BUNDLED_SSL=on -DBUILD_SHARED_LIBS=on ..
make
sudo make install
cat - > /tmp/h2o.conf << EOF
listen: 80
listen:
  port: 443
  ssl:
    certificate-file: examples/h2o/server.crt
    key-file:         examples/h2o/server.key
hosts:
  "127.0.0.1.xip.io:8080":
    paths:
      /:
        file.dir: examples/doc_root
num-threads: 1
EOF
sudo cp /tmp/h2o.conf /usr/local/etc/
cd ..
# sudo /usr/local/bin/h2o -c /usr/local/etc/h2o.conf
~~~~

nghttp2
~~~~
cd ~/src
wget https://github.com/nghttp2/nghttp2/releases/download/v1.23.1/nghttp2-1.23.1.tar.gz
tar -xvzf nghttp2-1.23.1.tar.gz
cd nghttp2-1.23.1
cscope -b -R
./configure
make
sudo make install
sudo /sbin/ldconfig
~~~~

libwebsockets
~~~~
mkdir ~/src
cd ~/src
wget https://github.com/warmcat/libwebsockets/archive/v2.2.1.tar.gz
tar -xvzf v2.2.1.tar.gz
cd libwebsockets-2.2.1
cscope -b -R
sed -i '/libwebsockets\.h/a #include <string.h>' test-server/test-server-v2.0.c
mkdir build
cd build
cmake .. -DLWS_WITH_LWSWS=1 -DLWS_WITH_HTTP2=1
make
sudo make install
sudo /sbin/ldconfig
sudo mkdir -p /etc/lwsws/conf.d /var/log/lwsws
sudo cp ../lwsws/etc-lwsws-conf-EXAMPLE /etc/lwsws/conf
sudo cp ../lwsws/etc-lwsws-conf.d-localhost-EXAMPLE /etc/lwsws/conf.d/test-server
sudo sed -i 's/7681/80/' /etc/lwsws/conf.d/test-server
sudo sed -i 's/\(.*access-log.*\)/#\1/' /etc/lwsws/conf.d/test-server
sudo sed -i 's/\"lo\"/\"eth0\"/' /etc/lwsws/conf.d/test-server
~~~~

lmdb
~~~~
cd ~/src
wget https://github.com/LMDB/lmdb/archive/LMDB_0.9.21.tar.gz
tar -xvzf LMDB_0.9.21.tar.gz
cd lmdb-LMDB_0.9.21/libraries/liblmdb/
cscope -b -R
make
sudo make install
cd ~/src
wget https://github.com/google/snappy/releases/download/1.1.4/snappy-1.1.4.tar.gz
tar -xvzf snappy-1.1.4.tar.gz
cd snappy-1.1.4
cscope -b -R
./configure && make
sudo make install
cd ~/src
git clone https://github.com/hyc/leveldb.git
cd leveldb
git checkout benches
cscope -b -R
sed -i 's/liblmdb.a/liblmdb.a \/usr\/local\/lib\/libsnappy.a/' Makefile
sed -i '58,60d' Makefile
sed -i 's/BENCHMARKS = db.*/BENCHMARKS = db_bench db_bench_mdb/' Makefile
#sed -i 's/cstdatomic/atomic/g' build_detect_platform
#sed -i 's/cstdatomic/atomic/g' port/atomic_pointer.h
make bench
wget http://www.lmdb.tech/bench/ondisk/cmd3-24
sed -i 's/DUR=1200/DUR=300/' cmd3-24
sed -i '/ENGINES="$ENGINES/d' cmd3-24
sed -i 's/ENGINES=""/ENGINES="LMDB"/' cmd3-24
#sudo mkfs -t ext4 /dev/xvdb
#sudo mount /dev/xvdb -o noatime,defaults /mnt
#sudo chown -R admin:admin /mnt
#sh cmd3-24
~~~~

cleanup
~~~~
sudo sed -i 's/^.syntax on/syntax on/' /etc/vim/vimrc
sudo sed -i 's/^.set background=dark/set background=dark/' /etc/vim/vimrc
sudo apt-get clean
sudo rm -rf /tmp/*
sudo rm -rf /tmp/.*
~~~~

~~~~
STRETCH_AMI=`aws ec2 create-image --instance-id $INSTANCE_ID --name testbed-stretch-freeze-amd64-hvm-2017-06-15-ebs`
~~~~
