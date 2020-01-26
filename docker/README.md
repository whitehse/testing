apt-get install docker.io

docker pull debian

docker image list

#docker start debian
docker run -i -t debian /bin/bash

apt-get update
apt-get install procps git sudo vim
mkdir -p /usr/src/git
cd /usr/src/git
git clone https://github.com/whitehse/lab-oss.git
cd lab-oss
apt-get install slapd bind9-dyndb-ldap ldap-utils python-ldap dnsutils
cd ldap
./build-ldap.sh
sed -i 's/^#BASE.*/BASE     dc=example,dc=net/' /etc/ldap/ldap.conf
sed -i 's/^#URI.*/URI     ldapi:\/\//' /etc/ldap/ldap.conf
cat > ~/.ldaprc << EOF
SASL_MECH EXTERNAL
EOF
cd ..
cd nslcd
apt-get install libpam-ldapd
./build-nslcd.sh
cd ..
cd freeradius
apt-get install freeradius freeradius-ldap
./build-freeradius.sh
cd ..
cd bind-dyndb-ldap
./build-bind-ldap.sh
cd
apt-get install libpyside2-py3-5.11 pyside2-tools python3-pyside2.qtwidgets python3-pyside2.qtgui python3-pyside2.qtcore



###

mkdir /tmp/downloads
cd /tmp/downloads
wget https://storage.googleapis.com/flutter_infra/releases/stable/linux/flutter_linux_v1.12.13+hotfix.5-stable.tar.xz
wget https://dl.google.com/dl/android/studio/ide-zips/3.5.3.0/android-studio-ide-191.6010548-linux.tar.gz
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip

xhost +

# As root
docker run \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /tmp/downloads:/tmp/downloads \
  --privileged -v /dev/bus/usb:/dev/bus/usb \
  -i -t debian /bin/bash
#  --device=/dev/ttyACM0 \


apt-get update && apt-get install curl wget unzip xz-utils git x11-utils

#apt-get install libasound2 libasound2-data
#tar -xvzf /tmp/downloads/jdk-8u45-linux-x64.tar.gz
#export PATH="$PATH:`pwd`/jdk1.8.0_45/bin"

cd

tar xf /tmp/downloads/flutter_linux_v1.12.13+hotfix.5-stable.tar.xz
#cd flutter
export PATH="$PATH:`pwd`/flutter/bin"
#flutter precache
tar -xvzf /tmp/downloads/android-studio-ide-191.6010548-linux.tar.gz
export PATH="$PATH:`pwd`/android-studio/bin"
studio.sh
#unzip /tmp/downloads/sdk-tools-linux-4333796.zip
#mkdir Android
#mv tools/ Android/
#export PATH="$PATH:`pwd`/Android/tools/bin"

flutter doctor --android-licenses
flutter doctor
flutter create first_app 

apt-get install vim-pathogen
vim-addons install pathogen

echo "execute pathogen#infect()" >> ~/.vimrc
