~~~~
sudo apt-get install awscli
~~~~

~~~~
aws configure
AWS Access Key ID [None]: <removed>
AWS Secret Access Key [None]: <removed>
Default region name [None]: us-east-2
Default output format [None]: text
~~~~

~~~~
aws s3 mb s3://cafedemocracy-benchmarks --region us-east-2
~~~~

~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="global-http-restricted-ssh"
SECURITY_GROUP_DESCRIPTION="Allow global access to http/https. Limit ssh"
MY_HOME_IP="203.0.113.234"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
VOLUME_OUTPUT=`mktemp`
INSTANCE_OUTPUT=`mktemp`
~~~~

~~~~
aws ec2 create-key-pair --key-name $KEY_NAME | sed 's/.*-----BEGIN/-----BEGIN/' | sed 's/KEY-----.*/KEY-----/' > $SSH_PRIV_FILE
chmod 600 $SSH_PRIV_FILE
~~~~

~~~~
aws ec2 create-security-group --group-name $SECURITY_GROUP_NAME --description "$SECURITY_GROUP_DESCRIPTION"
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol tcp --port 80 --cidr 0.0.0.0/0
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol tcp --port 443 --cidr 0.0.0.0/0
aws ec2 authorize-security-group-ingress --group-name $SECURITY_GROUP_NAME --protocol tcp --port 22 --cidr ${MY_HOME_IP}/32
SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
# debian-jessie-amd64-hvm-2017-01-15-1221-ebs: ami-b2795cd7
aws ec2 run-instances --image-id ami-b2795cd7 --count 1 --instance-type r4.large --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
IPV4_ADDR=`aws ec2 describe-instances | grep ASSOCIATION | head -1 | awk '{print $4}'`
# aws ec2 terminate-instances --instance-ids $INSTANCE_ID
~~~~

~~~~
aws ec2 create-volume --region us-east-2 --availability-zone $AVAILABILITY_ZONE --size 500 --volume-type io1 --iops 100 > $VOLUME_OUTPUT
VOLUME_ID=`cat "$VOLUME_OUTPUT" | awk '{print $7}'`
~~~~

~~~~
aws ec2 attach-volume --volume-id $VOLUME_ID --instance-id $INSTANCE_ID --device /dev/xvdb
~~~~

~~~~
ssh -i $SSH_PRIV_FILE admin@${IPV4_ADDR}
~~~~

~~~~
sudo apt-get update && sudo apt-get dist-upgrade
sudo apt-get install vim git automake libtool numactl time bc
sudo sed -i 's/^.syntax on/syntax on/' /etc/vim/vimrc
sudo sed -i 's/^.set background=dark/set background=dark/' /etc/vim/vimrc
sudo apt-get build-dep liblmdb-dev
~~~~

~~~~
mkdir ~/src
cd ~/src
wget https://github.com/LMDB/lmdb/archive/LMDB_0.9.21.tar.gz
tar -xvzf LMDB_0.9.21.tar.gz
cd lmdb-LMDB_0.9.21/libraries/liblmdb/
make
sudo make install
cd ~/src
wget https://github.com/google/snappy/releases/download/1.1.4/snappy-1.1.4.tar.gz
tar -xvzf snappy-1.1.4.tar.gz
cd snappy-1.1.4
./configure && make
sudo make install
cd ~/src
git clone https://github.com/hyc/leveldb.git
cd leveldb
git checkout benches
sed -i 's/liblmdb.a/liblmdb.a \/usr\/local\/lib\/libsnappy.a/' Makefile
sed -i '58,60d' Makefile
sed -i 's/BENCHMARKS = db.*/BENCHMARKS = db_bench db_bench_mdb/' Makefile
sed -i 's/cstdatomic/atomic/g' build_detect_platform
sed -i 's/cstdatomic/atomic/g' port/atomic_pointer.h
make bench
cd ~/src/
wget http://www.lmdb.tech/bench/ondisk/dorww.c
gcc -o dorww dorww.c
wget http://www.lmdb.tech/bench/ondisk/cmd3-24
sed -i 's/DUR=1200/DUR=300/' cmd3-24
sed -i '/ENGINES="$ENGINES/d' cmd3-24
sed -i 's/ENGINES=""/ENGINES="LMDB"/' cmd3-24
sudo mkfs -t ext4 /dev/xvdb
sudo mount /dev/xvdb -o noatime,defaults /mnt
sudo chown -R admin:admin /mnt
# Make this 8000000000 entries when running on r4.16xlarge to fit within ram.
sh cmd3-24 > ~/cmd3-24.output
~~~~
