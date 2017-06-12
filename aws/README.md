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
MY_HOME_IP="67.217.144.188"
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


