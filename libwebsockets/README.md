Server:
~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
VOLUME_OUTPUT=`mktemp`
INSTANCE_OUTPUT=`mktemp`
SERVER_TAGS='ResourceType=instance,Tags=[{Key=role,Value=lwsws}]'
CLIENT_TAGS='ResourceType=instance,Tags=[{Key=role,Value=h2load}]'
~~~~

SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
# debian-jessie-amd64-hvm-2017-01-15-1221-ebs: ami-b2795cd7
aws ec2 run-instances --image-id ami-b2795cd7 --count 1 --instance-type r4.large --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address --tag-specifications $SERVER_TAGS > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
SERVER_IP=`aws ec2 describe-instances --filters "Name=tag:role,Values=lwsws" | grep ASSOCIATION | head -1 | awk '{print $4}'`
# aws ec2 terminate-instances --instance-ids $INSTANCE_ID
~~~~

~~~~
echo "ssh -i $SSH_PRIV_FILE admin@${SERVER_IP}"
~~~~

~~~~
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
sudo reboot
sudo apt-get install vim git build-essential cmake libuv1-dev libssl-dev
sudo apt-get build-dep libwebsockets-dev
sudo sed -i 's/^.syntax on/syntax on/' /etc/vim/vimrc
sudo sed -i 's/^.set background=dark/set background=dark/' /etc/vim/vimrc
~~~~

~~~~
mkdir ~/src
cd ~/src
wget https://github.com/warmcat/libwebsockets/archive/v2.2.1.tar.gz
tar -xvzf v2.2.1.tar.gz
cd libwebsockets-2.2.1
sed -i '/libwebsockets\.h/a #include <string.h>/' test-server/test-server-v2.0.c
#git clone https://github.com/warmcat/libwebsockets.git
#cd libwebsockets
mkdir build
cd build
cmake .. -DLWS_WITH_LWSWS=1 -DLWS_WITH_HTTP2=1
make
sudo make install
sudo /sbin/ldconfig
/usr/local/bin/lwsws --help
sudo mkdir -p /etc/lwsws/conf.d /var/log/lwsws
sudo cp ../lwsws/etc-lwsws-conf-EXAMPLE /etc/lwsws/conf
sudo cp ../lwsws/etc-lwsws-conf.d-localhost-EXAMPLE /etc/lwsws/conf.d/test-server
sudo sed -i 's/7681/80/' /etc/lwsws/conf.d/test-server
sudo sed -i 's/\(.*access-log.*\)/#\1/' /etc/lwsws/conf.d/test-server
sudo sed -i 's/\"lo\"/\"eth0\"/' /etc/lwsws/conf.d/test-server
sudo lwsws
~~~~

Client:

~~~~
# debian-jessie-amd64-hvm-2017-01-15-1221-ebs: ami-b2795cd7
aws ec2 run-instances --image-id ami-b2795cd7 --count 1 --instance-type r4.large --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address --tag-specifications $CLIENT_TAGS > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
CLIENT_IP=`aws ec2 describe-instances --filters "Name=tag:role,Values=h2load" | grep ASSOCIATION | head -1 | awk '{print $4}'`
# aws ec2 terminate-instances --instance-ids $INSTANCE_ID
~~~~

~~~~
echo "ssh -i $SSH_PRIV_FILE admin@${CLIENT_IP}"
~~~~

~~~~
OLDIFS="$IFS"
IFS='
'
new_sources=`mktemp`
for foo in `cat /etc/apt/sources.list`
do
  echo "$foo" >> $new_sources
  echo "$foo" | grep -q "deb-src"
  if [ $? -ne 0 ]; then
    echo "$foo" | sed 's/^deb /deb-src /' >> $new_sources
  fi
done
IFS="$OLDIFS"
sudo mv $new_sources /etc/apt/sources.list
sudo apt-get update && sudo apt-get dist-upgrade
sudo apt-get install vim git g++ make binutils autoconf automake autotools-dev libtool pkg-config \
  zlib1g-dev libcunit1-dev libssl-dev libxml2-dev libev-dev libevent-dev libjansson-dev \
  libc-ares-dev libjemalloc-dev libsystemd-dev \
  cython python3-dev python-setuptools
sudo sed -i 's/^.syntax on/syntax on/' /etc/vim/vimrc
sudo sed -i 's/^.set background=dark/set background=dark/' /etc/vim/vimrc
~~~~

~~~~
mkdir ~/src
cd ~/src
wget https://github.com/nghttp2/nghttp2/releases/download/v1.23.1/nghttp2-1.23.1.tar.gz
tar -xvzf nghttp2-1.23.1.tar.gz
cd nghttp2-1.23.1
./configure
make
sudo make install
sudo /sbin/ldconfig
export PATH=$PATH:/usr/local/bin/
cd ~/src/
wget https://github.com/h2o/h2o/archive/v2.2.2.tar.gz
tar -xvzf v2.2.2.tar.gz h2o-2.2.2/examples/h2o/server.crt
sudo cp h2o-2.2.2/examples/h2o/server.crt /etc/ssl/certs/
# -n - The number of total requests. Default: 1
# -c - The number of concurrent clients. Default: 1
# -m - The max concurrent streams to issue per client. Default: 1
# -t - The number of native threads. Default: 1
h2load -n1000000 -c10 -m250 -t 2 http://172.31.4.253
~~~~
