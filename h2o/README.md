~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
INSTANCE_OUTPUT=`mktemp`
MY_STRETCH_AMI="ami-40bb9d25"
# ~$4.50 per hour
INSTANCE_TYPE="r4.16xlarge"
VOLUME_OUTPUT=`mktemp`
SERVER_TAGS='ResourceType=instance,Tags=[{Key=role,Value=h2o-server}]'
CLIENT_TAGS='ResourceType=instance,Tags=[{Key=role,Value=h2load}]'
SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
aws ec2 run-instances --image-id $MY_STRETCH_AMI --count 1 --instance-type $INSTANCE_TYPE --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address --tag-specifications $SERVER_TAGS > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
SERVER_IP=`aws ec2 describe-instances --filters "Name=tag:role,Values=h2o-server" | grep ASSOCIATION | head -1 | awk '{print $4}'`
~~~~

~~~~
echo "ssh -i $SSH_PRIV_FILE admin@${SERVER_IP}"
cd ~/src/h2o-2.2.2/
sid -i '^num-threads.*/num-threads: 32/' /usr/local/etc/h2o.conf
sudo /usr/local/bin/h2o -c /usr/local/etc/h2o.conf
~~~~

~~~~
aws ec2 run-instances --image-id $MY_STRETCH_AMI --count 1 --instance-type $INSTANCE_TYPE --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address --tag-specifications $CLIENT_TAGS > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
CLIENT_IP=`aws ec2 describe-instances --filters "Name=tag:role,Values=h2load" | grep ASSOCIATION | head -1 | awk '{print $4}'`
~~~~

~~~~
echo "ssh -i $SSH_PRIV_FILE admin@${CLIENT_IP}"
# -n - The number of total requests. Default: 1
# -c - The number of concurrent clients. Default: 1
# -m - The max concurrent streams to issue per client. Default: 1
# -t - The number of native threads. Default: 1
#h2load -n10000000 -c10 -m250 -t 2 https://<private_ip_of_server>
h2load -n10000000 -c250 -m250 -t 64 https://172.31.32.186

finished in 3.04s, 3288470.92 req/s, 652.33MB/s
requests: 10000000 total, 10000000 started, 10000000 done, 10000000 succeeded, 0 failed, 0 errored, 0 timeout
status codes: 10000000 2xx, 0 3xx, 0 4xx, 0 5xx
traffic: 1.94GB (2080041924) total, 124.01MB (130034424) headers (space savings 92.48%), 1.65GB (1770000000) data
                     min         max         mean         sd        +/- sd
time for request:      353us     46.87ms      5.64ms      3.53ms    78.32%
time for connect:     6.40ms     95.47ms     58.28ms     26.20ms    57.20%
time to 1st byte:     8.29ms    114.14ms     69.34ms     31.76ms    60.00%
req/s           :   12941.67    69773.73    19408.01     9521.71    82.40%

~~~~





libh2o testing
~~~~
KEY_NAME="dan"
SECURITY_GROUP_NAME="http-from-security-group-ssh-from-home"
SSH_PRIV_FILE=~/.ssh/amazon-benchmark-server.pem
INSTANCE_OUTPUT=`mktemp`
MY_STRETCH_AMI="ami-40bb9d25"
# ~$4.50 per hour
INSTANCE_TYPE="r4.large"
VOLUME_OUTPUT=`mktemp`
SERVER_TAGS='ResourceType=instance,Tags=[{Key=role,Value=h2o-server}]'
SECURITY_GROUP_ID=`aws ec2 describe-security-groups | grep SECURITYGROUPS | grep $SECURITY_GROUP_NAME | sed 's/.*\(sg-[0-9a-f]\+\).*/\1/'`
~~~~

~~~~
aws ec2 run-instances --image-id $MY_STRETCH_AMI --count 1 --instance-type $INSTANCE_TYPE --key-name $KEY_NAME --security-group-ids $SECURITY_GROUP_ID --associate-public-ip-address --tag-specifications $SERVER_TAGS > $INSTANCE_OUTPUT
INSTANCE_ID=`cat "$INSTANCE_OUTPUT" | grep INSTANCES | awk '{print $7}'`
AVAILABILITY_ZONE=`cat "$INSTANCE_OUTPUT" | grep PLACEMENT | awk '{print $2}'`
sleep 10
SERVER_IP=`aws ec2 describe-instances --filters "Name=tag:role,Values=h2o-server" | grep ASSOCIATION | head -1 | awk '{print $4}'`
~~~~

~~~~
echo "ssh -i $SSH_PRIV_FILE admin@${SERVER_IP}"
~~~~
