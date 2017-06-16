~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SECURITY_GROUP_DESCRIPTION="Allow http/https, and ICMP, from same security group. Allow SSH from home."
MY_HOME_IP="203.0.113.234"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
INSTANCE_OUTPUT=`mktemp`
MY_STRETCH_AMI="ami-40bb9d25"
INSTANCE_TYPE="r4.16xlarge"
VOLUME_OUTPUT=`mktemp`
SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
aws ec2 run-instances --image-id $MY_STRETCH_AMI --count 1 --instance-type $INSTANCE_TYPE --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
IPV4_ADDR=`aws ec2 describe-instances | grep ASSOCIATION | head -1 | awk '{print $4}'`
~~~~

~~~~
aws ec2 create-volume --region us-east-2 --availability-zone $AVAILABILITY_ZONE --size 500 --volume-type io1 --iops 20000 > $VOLUME_OUTPUT
VOLUME_ID=`cat "$VOLUME_OUTPUT" | awk '{print $7}'`
sleep 5
aws ec2 attach-volume --volume-id $VOLUME_ID --instance-id $INSTANCE_ID --device /dev/xvdb
~~~~

~~~~
ssh -i $SSH_PRIV_FILE admin@${IPV4_ADDR}
~~~~

~~~~
sudo mkfs -t ext4 /dev/xvdb
sudo mount /dev/xvdb -o noatime,defaults /mnt
sudo chown -R admin:admin /mnt
cd ~/src
git clone https://github.com/whitehse/libwebsockets-benchmarks.git
dstat --nocolor --output ~/cmd3-24-dstat.csv \
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
cd ~/src/leveldb/
sh ~/src/libwebsockets-benchmarks/lmdb/cmd3-24 > ~/cmd3-24.output
sed -i '1,6d' ~/cmd3-24-dstat.csv
cat ~/src/libwebsockets-benchmarks/aws/dstat-header ~/cmd3-24-dstat.csv > ~/dstat-cmd3-24-run.csv
~~~~
