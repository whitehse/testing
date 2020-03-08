# As of Sat 07 Mar 2020 11:57:28 AM CST
sudo apt-get install docker.io
sudo usermod -aG docker $USER
newgrp docker
docker pull debian

docker image list

mkdir /tmp/downloads
cd /tmp/downloads
wget https://storage.googleapis.com/flutter_infra/releases/stable/linux/flutter_linux_v1.12.13+hotfix.8-stable.tar.xz
wget https://dl.google.com/dl/android/studio/ide-zips/3.6.1.0/android-studio-ide-192.6241897-linux.tar.gz
wget https://dl.google.com/android/repository/platform-tools-latest-linux.zip
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip
wget https://dl.google.com/go/go1.14.linux-amd64.tar.gz 
# Download java 8 from https://java.com/en/download/linux_manual.jsp (Linux x86) and place the download in /tmp/downloads
# Download vscode (.deb) from https://code.visualstudio.com/download. Place in /tmp/downloads

xhost +

docker run \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /tmp/downloads:/tmp/downloads \
  --privileged -v /dev/bus/usb:/dev/bus/usb \
  -i -t debian /bin/bash
#  --device=/dev/ttyACM0 \

apt-get update
apt-get install procps git sudo vim wget curl unzip xz-utils git x11-utils default-jdk
adduser --disabled-password --gecos "" zero_cool
usermod -aG src zero_cool
usermod -aG sudo zero_cool
chown root:src /usr/src
chmod 1775 /usr/src
mkdir -m 1775 /usr/src/git
chown root:src /usr/src/git
visudo
# Set: %sudo	ALL=(ALL:ALL) NOPASSWD:ALL
su - zero_cool

#apt-get install libasound2 libasound2-data
#tar -xvzf /tmp/downloads/jdk-8u45-linux-x64.tar.gz
#export PATH="$PATH:`pwd`/jdk1.8.0_45/bin"

cd

tar xf /tmp/downloads/flutter_linux_*.xz
#cd flutter
#flutter precache
tar -xvzf /tmp/downloads/android-studio-ide-*.gz
cat >> ~/.bash_profile << EOF
export PATH="$PATH:`pwd`/flutter/bin:`pwd`/android-studio/bin"
export VISUAL=vi
export EDITOR=vi
export DISPLAY=:0
EOF
source ~/.bash_profile
studio.sh
# Select "Do not import settings", and press "OK"
# Click "Don't Send". Click Next
# Select "Standard". Click Next
# Select "Dracula". Click Next
# Click Next
# Click Finish
# After all downloads, click Finish again.
# After restart, click "Configure" and "Plugins"
# Search for "Flutter". Click "Install" next to "Flutter". Click "Accept"
# When prompted if you want to install the "Dart" plugin, press "Yes"
# Click "Restart IDE". Click "Shutdown"
#
unzip /tmp/downloads/platform-tools-latest-linux.zip
#unzip /tmp/downloads/sdk-tools-linux*zip

unzip /tmp/downloads/sdk-tools-linux-4333796.zip
mv tools/ Android/Sdk
tar -xvzf /tmp/downloads/jre-8u241-linux-x64.tar.gz
touch ~/.android/repositories.cfg
JAVA_HOME=~/jre1.8.0_241/ Android/Sdk/tools/bin/sdkmanager --update
flutter doctor --android-licenses
#export PATH="$PATH:`pwd`/Android/tools/bin"

#vs code

sudo dpkg -i /tmp/downloads/code*deb
sudo apt --fix-broken install
code
# Select File -> Preferences -> Extentions
# Search for "Flutter". click install
# Search for "Material Icon Theme". click install
# Select File -> Preferences -> Settings
# Search for "editor.formatOnSave". Click the "Format a file on Save" checkbox

#ctrl-s to save an reformat (setting editor.formatOnSave must be true)
#ctrl-space get a list of named arguments
#ctrl-k-c comment block
#ctrl-k-u uncomment block
#ctrl-hover-over-link, then click. View source code
#ctrl-shift-r Refactor (after clicking on the function/word)
#ctrl-shift-p, type 'dart devtools' to open dev tools

# Select File -> Preferences -> Extentions
# Search for "Go". click install

----

# go

cd
cd /usr/local
sudo tar -xvzf /tmp/downloads/go1.14.linux-amd64.tar.gz

cat >> ~/.bash_profile << EOF
export PATH="$PATH:/usr/local/go/bin"
EOF
source ~/.bash_profile
go get github.com/bmatsuo/lmdb-go/lmdb
go get github.com/kevburnsjr/lmdb-go/lmdb
cd ~/go/src/github.com/
rm -rf bmatsuo/
mv kevburnsjr/ bmatsuo

----

Freeze image as a new docker image:

Outside of the docker instance:

docker ps
# Note the container ID and use it in this command:
docker commit -p -a "Dan White" -m "Initial creation of developer image" 1fcd4c8af924 zero_cool

# Exit the instance, and re-enter under the new one:

docker run \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --privileged -v /dev/bus/usb:/dev/bus/usb \
  -i -t zero_cool /bin/bash
  #-v /tmp/downloads:/tmp/downloads \
#  --device=/dev/ttyACM0 \

su - zero_cool

go get -u github.com/golang/protobuf/protoc-gen-go
cd /tmp/
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.7.1/protoc-3.7.1-linux-x86_64.zip
sudo mkdir /usr/local/protoc
cd /usr/local/protoc
sudo unzip /tmp/protoc-3.7.1-linux-x86_64.zip

cat >> ~/.bash_profile << EOF
export PATH="$PATH:/usr/local/protoc/bin"
EOF
source ~/.bash_profile

# dart

sudo apt-get install make apt-transport-https
sudo sh -c 'wget -qO- https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -'
sudo sh -c 'wget -qO- https://storage.googleapis.com/download.dartlang.org/linux/debian/dart_stable.list > /etc/apt/sources.list.d/dart_stable.list'
sudo apt-get update
sudo apt-get install dart
mkdir /usr/src/git/dart-lang
cd /usr/src/git/dart-lang
git clone https://github.com/dart-lang/protobuf
cd protobuf/protoc_plugin
cat >> ~/.bash_profile << EOF
export PATH="$PATH:/usr/lib/dart/bin/"
EOF
source ~/.bash_profile
pub install
cat >> ~/.bash_profile << EOF
export PATH="$PATH:/usr/src/git/dart-lang/protobuf/protoc_plugin/bin"
EOF
source ~/.bash_profile
git config --global user.email "whitehse@gmail.com"
git config --global user.name "Dan White"

Freeze image, again, as a new docker image:

Outside of the docker instance:

docker ps
# Note the container ID and use it in this command:
docker commit -p -a "Dan White" -m "Add go and dart tools" 5956637f12ee zero_cool_v2

----
flutter create first_app 

#apt-get install vim-pathogen
#vim-addons install pathogen
#
#echo "execute pathogen#infect()" >> ~/.vimrc
