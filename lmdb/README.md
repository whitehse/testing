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
aws ec2 create-volume --region us-east-2 --availability-zone $AVAILABILITY_ZONE --size 500 --volume-type io1 --iops 100 > $VOLUME_OUTPUT
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
--vm --vm-adv \
-c -C 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 > /dev/null &
cd ~/src/leveldb/
sh cmd3-24 > ~/cmd3-24.output
sed -i '1,6d' ~/cmd3-24-dstat.csv
echo "timestamp,xvdb-read,xvdb-write,paging-in,paging-out,interrupt-48,interrupt-49,interrupt-50,total,used,free,buff,cach,dirty,shmem,recl,swap-used,swap-free,eth0-recv,eth0-send,run,blk,new,io-total-read,io-total-write,interrupts,context-switches,aio,cpu0-usr,cpu0-sys,cpu0-idl,cpu0-wai,cpu0-hiq,cpu0-siq,cpu0-stl,cpu1-usr,cpu1-sys,cpu1-idl,cpu1-wai,cpu1-hiq,cpu1-siq,cpu1-stl,cpu2-usr,cpu2-sys,cpu2-idl,cpu2-wai,cpu2-hiq,cpu2-siq,cpu2-stl,cpu3-usr,cpu3-sys,cpu3-idl,cpu3-wai,cpu3-hiq,cpu3-siq,cpu3-stl,cpu4-usr,cpu4-sys,cpu4-idl,cpu4-wai,cpu4-hiq,cpu4-siq,cpu4-stl,cpu5-usr,cpu5-sys,cpu5-idl,cpu5-wai,cpu5-hiq,cpu5-siq,cpu5-stl,cpu6-usr,cpu6-sys,cpu6-idl,cpu6-wai,cpu6-hiq,cpu6-siq,cpu6-stl,cpu7-usr,cpu7-sys,cpu7-idl,cpu7-wai,cpu7-hiq,cpu7-siq,cpu7-stl,cpu8-usr,cpu8-sys,cpu8-idl,cpu8-wai,cpu8-hiq,cpu8-siq,cpu8-stl,cpu9-usr,cpu9-sys,cpu9-idl,cpu9-wai,cpu9-hiq,cpu9-siq,cpu9-stl,cpu10-usr,cpu10-sys,cpu10-idl,cpu10-wai,cpu10-hiq,cpu10-siq,cpu10-stl,cpu11-usr,cpu11-sys,cpu11-idl,cpu11-wai,cpu11-hiq,cpu11-siq,cpu11-stl,cpu12-usr,cpu12-sys,cpu12-idl,cpu12-wai,cpu12-hiq,cpu12-siq,cpu12-stl,cpu13-usr,cpu13-sys,cpu13-idl,cpu13-wai,cpu13-hiq,cpu13-siq,cpu13-stl,cpu14-usr,cpu14-sys,cpu14-idl,cpu14-wai,cpu14-hiq,cpu14-siq,cpu14-stl,cpu15-usr,cpu15-sys,cpu15-idl,cpu15-wai,cpu15-hiq,cpu15-siq,cpu15-stl,cpu16-usr,cpu16-sys,cpu16-idl,cpu16-wai,cpu16-hiq,cpu16-siq,cpu16-stl,cpu17-usr,cpu17-sys,cpu17-idl,cpu17-wai,cpu17-hiq,cpu17-siq,cpu17-stl,cpu18-usr,cpu18-sys,cpu18-idl,cpu18-wai,cpu18-hiq,cpu18-siq,cpu18-stl,cpu19-usr,cpu19-sys,cpu19-idl,cpu19-wai,cpu19-hiq,cpu19-siq,cpu19-stl,cpu20-usr,cpu20-sys,cpu20-idl,cpu20-wai,cpu20-hiq,cpu20-siq,cpu20-stl,cpu21-usr,cpu21-sys,cpu21-idl,cpu21-wai,cpu21-hiq,cpu21-siq,cpu21-stl,cpu22-usr,cpu22-sys,cpu22-idl,cpu22-wai,cpu22-hiq,cpu22-siq,cpu22-stl,cpu23-usr,cpu23-sys,cpu23-idl,cpu23-wai,cpu23-hiq,cpu23-siq,cpu23-stl,cpu24-usr,cpu24-sys,cpu24-idl,cpu24-wai,cpu24-hiq,cpu24-siq,cpu24-stl,cpu25-usr,cpu25-sys,cpu25-idl,cpu25-wai,cpu25-hiq,cpu25-siq,cpu25-stl,cpu26-usr,cpu26-sys,cpu26-idl,cpu26-wai,cpu26-hiq,cpu26-siq,cpu26-stl,cpu27-usr,cpu27-sys,cpu27-idl,cpu27-wai,cpu27-hiq,cpu27-siq,cpu27-stl,cpu28-usr,cpu28-sys,cpu28-idl,cpu28-wai,cpu28-hiq,cpu28-siq,cpu28-stl,cpu29-usr,cpu29-sys,cpu29-idl,cpu29-wai,cpu29-hiq,cpu29-siq,cpu29-stl,cpu30-usr,cpu30-sys,cpu30-idl,cpu30-wai,cpu30-hiq,cpu30-siq,cpu30-stl,cpu31-usr,cpu31-sys,cpu31-idl,cpu31-wai,cpu31-hiq,cpu31-siq,cpu31-stl,cpu32-usr,cpu32-sys,cpu32-idl,cpu32-wai,cpu32-hiq,cpu32-siq,cpu32-stl,cpu33-usr,cpu33-sys,cpu33-idl,cpu33-wai,cpu33-hiq,cpu33-siq,cpu33-stl,cpu34-usr,cpu34-sys,cpu34-idl,cpu34-wai,cpu34-hiq,cpu34-siq,cpu34-stl,cpu35-usr,cpu35-sys,cpu35-idl,cpu35-wai,cpu35-hiq,cpu35-siq,cpu35-stl,cpu36-usr,cpu36-sys,cpu36-idl,cpu36-wai,cpu36-hiq,cpu36-siq,cpu36-stl,cpu37-usr,cpu37-sys,cpu37-idl,cpu37-wai,cpu37-hiq,cpu37-siq,cpu37-stl,cpu38-usr,cpu38-sys,cpu38-idl,cpu38-wai,cpu38-hiq,cpu38-siq,cpu38-stl,cpu39-usr,cpu39-sys,cpu39-idl,cpu39-wai,cpu39-hiq,cpu39-siq,cpu39-stl,cpu40-usr,cpu40-sys,cpu40-idl,cpu40-wai,cpu40-hiq,cpu40-siq,cpu40-stl,cpu41-usr,cpu41-sys,cpu41-idl,cpu41-wai,cpu41-hiq,cpu41-siq,cpu41-stl,cpu42-usr,cpu42-sys,cpu42-idl,cpu42-wai,cpu42-hiq,cpu42-siq,cpu42-stl,cpu43-usr,cpu43-sys,cpu43-idl,cpu43-wai,cpu43-hiq,cpu43-siq,cpu43-stl,cpu44-usr,cpu44-sys,cpu44-idl,cpu44-wai,cpu44-hiq,cpu44-siq,cpu44-stl,cpu45-usr,cpu45-sys,cpu45-idl,cpu45-wai,cpu45-hiq,cpu45-siq,cpu45-stl,cpu46-usr,cpu46-sys,cpu46-idl,cpu46-wai,cpu46-hiq,cpu46-siq,cpu46-stl,cpu47-usr,cpu47-sys,cpu47-idl,cpu47-wai,cpu47-hiq,cpu47-siq,cpu47-stl,cpu48-usr,cpu48-sys,cpu48-idl,cpu48-wai,cpu48-hiq,cpu48-siq,cpu48-stl,cpu49-usr,cpu49-sys,cpu49-idl,cpu49-wai,cpu49-hiq,cpu49-siq,cpu49-stl,cpu50-usr,cpu50-sys,cpu50-idl,cpu50-wai,cpu50-hiq,cpu50-siq,cpu50-stl,cpu51-usr,cpu51-sys,cpu51-idl,cpu51-wai,cpu51-hiq,cpu51-siq,cpu51-stl,cpu52-usr,cpu52-sys,cpu52-idl,cpu52-wai,cpu52-hiq,cpu52-siq,cpu52-stl,cpu53-usr,cpu53-sys,cpu53-idl,cpu53-wai,cpu53-hiq,cpu53-siq,cpu53-stl,cpu54-usr,cpu54-sys,cpu54-idl,cpu54-wai,cpu54-hiq,cpu54-siq,cpu54-stl,cpu55-usr,cpu55-sys,cpu55-idl,cpu55-wai,cpu55-hiq,cpu55-siq,cpu55-stl,cpu56-usr,cpu56-sys,cpu56-idl,cpu56-wai,cpu56-hiq,cpu56-siq,cpu56-stl,cpu57-usr,cpu57-sys,cpu57-idl,cpu57-wai,cpu57-hiq,cpu57-siq,cpu57-stl,cpu58-usr,cpu58-sys,cpu58-idl,cpu58-wai,cpu58-hiq,cpu58-siq,cpu58-stl,cpu59-usr,cpu59-sys,cpu59-idl,cpu59-wai,cpu59-hiq,cpu59-siq,cpu59-stl,cpu60-usr,cpu60-sys,cpu60-idl,cpu60-wai,cpu60-hiq,cpu60-siq,cpu60-stl,cpu61-usr,cpu61-sys,cpu61-idl,cpu61-wai,cpu61-hiq,cpu61-siq,cpu61-stl,cpu62-usr,cpu62-sys,cpu62-idl,cpu62-wai,cpu62-hiq,cpu62-siq,cpu62-stl,cpu63-usr,cpu63-sys,cpu63-idl,cpu63-wai,cpu63-hiq,cpu63-siq,cpu63-stl,files,inodes,msg,sem,shm,socket-tot,tcp,udp,raw,frg,tcp-lis,tcp-act,tcp-syn,tcp-tim,tcp-clo,udp-lis,udp-act,unix-dgm,unix-str,unix-lis,unix-act,majpf,minpf,alloc,vm-free,steal,scank,scand,pgoru,astll
cat ~/dstat-csv.header ~/cmd3-24-dstat.csv` > ~/dstat-cmd3-24-run.csv
~~~~
