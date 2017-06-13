Server:
~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
VOLUME_OUTPUT=`mktemp`
INSTANCE_OUTPUT=`mktemp`
SERVER_TAGS='ResourceType=instance,Tags=[{Key=role,Value=h2o-server}]'
CLIENT_TAGS='ResourceType=instance,Tags=[{Key=role,Value=h2load}]'
#SERVER_TAGS="--tag-specifications 'ResourceType=instance,Tags=[{Key=role,Value=h2o-server}]'
~~~~

SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
# debian-jessie-amd64-hvm-2017-01-15-1221-ebs: ami-b2795cd7
aws ec2 run-instances --image-id ami-b2795cd7 --count 1 --instance-type r4.large --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address --tag-specifications $SERVER_TAGS > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
SERVER_IP=`aws ec2 describe-instances --filters "Name=tag:role,Values=h2o-server" | grep ASSOCIATION | head -1 | awk '{print $4}'`
# aws ec2 terminate-instances --instance-ids $INSTANCE_ID
~~~~

~~~~
echo "ssh -i $SSH_PRIV_FILE admin@${SERVER_IP}"
~~~~

~~~~
sudo apt-get update && sudo apt-get dist-upgrade
sudo apt-get install vim git build-essential automake libtool cmake libssl-dev time bc
sudo sed -i 's/^.syntax on/syntax on/' /etc/vim/vimrc
sudo sed -i 's/^.set background=dark/set background=dark/' /etc/vim/vimrc

~~~~

~~~~
mkdir ~/src
cd ~/src
wget https://github.com/h2o/h2o/archive/v2.2.2.tar.gz
tar -xvzf v2.2.2.tar.gz
cd h2o-2.2.2
cmake -DWITH_BUNDLED_SSL=on .
make
sudo make install
#sudo cp examples/h2o/* /usr/local/etc/
#modify /usr/local/etc/h2o.conf
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
sudo /usr/local/bin/h2o -c /usr/local/etc/h2o.conf
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
cd /usr/src
wget https://github.com/h2o/h2o/archive/v2.2.2.tar.gz
tar -xvzf v2.2.2.tar.gz h2o-2.2.1/examples/h2o/server.crt
sudo cp h2o-2.2.2/examples/h2o/server.crt /etc/ssl/certs/
# -n - The number of total requests. Default: 1
# -c - The number of concurrent clients. Default: 1
# -m - The max concurrent streams to issue per client. Default: 1
# -t - The number of native threads. Default: 1
h2load -n10000000 -c10 -m250 -t 2 https://172.31.4.253
~~~~
