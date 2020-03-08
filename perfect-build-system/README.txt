# As of Sat 07 Mar 2020 11:57:28 AM CST
sudo apt-get install docker.io
sudo usermod -aG docker $USER
newgrp docker
docker pull debian
sudo chmod 666 /dev/kvm
# This is obviously insecure, but is an easy way
# to allow the docker user to have the ability to
# run Android instances

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

Exit and then restart the docker instance:

docker run \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --privileged -v /dev/bus/usb:/dev/bus/usb \
  --device=/dev/kvm \
  -i -t zero_cool_v2 /bin/bash

# More

sudo bash -c 'cat >> /etc/vim/vimrc.local << EOF
set paste
set tabstop=4 shiftwidth=4 expandtab
EOF'
sed -i 's/^#force_color.*/force_color_prompt=yes/' ~/.bashrc

sudo apt-get install gcc
cd /tmp/
git clone https://github.com/golang/protobuf.git && (cd protobuf && git checkout v1.2.0 && go build -o /tmp/protoc-gen-go ./protoc-gen-go) && rm -r protobuf
mv /tmp/protoc-gen-go ~/go/bin/

----

exit
chmod 666 /dev/kvm
apt-get install qemu-kvm
su - zero_cool
bash
sed -i 's/: Big enough.*/ \*\//' ~/go/src/github.com/bmatsuo/lmdb-go/lmdb/mdb.c


studio.io
# Select Configurage -> AVD Manager
# Download the top recommended system image by clicking the download link
flutter create flutter_client
cd flutter_client

studio.sh
# Select "Configure" and then "AVD Manager"
# Click "Create Virtual Device"
# Select "Pixel 2". Clicke Next
# 

Outside of the docker instance:

docker ps
# Note the container ID and use it in this command:
docker commit -p -a "Dan White" -m "Add go and dart tools" 5956637f12ee zero_cool_v2

Exit and then restart the docker instance:

docker run \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --privileged -v /dev/bus/usb:/dev/bus/usb \
  --device=/dev/kvm \
  -i -t zero_cool_v3 /bin/bash

su - zero_cool
bash

cat >> ~/.bash_profile << EOF
export PATH="$PATH:`pwd`/Android/Sdk/emulator/"
EOF
source ~/.bash_profile

emulator -list-avds
# e.g.: Pixel_2_API_R
# This may be necessary
rm .android/avd/Pixel_2_API_R.avd/*lock
emulator -avd Pixel_2_API_R 2>&1 > /dev/null &

# I experience a seg fault here after a couple of minutes.
# I'd recommend letting the instance boot up for a few
# minutes and monitor load. Proceed when load has dropped.
#
# btrfs workers seem to be busy here. I'm not sure if btrfs
# is a poor choice for the host system or not. Perhaps there's
# just a lot of disk activity. I haven't had issues with
# btrfs otherwise.

# To shut down, press the power button on the side
# panel. A shutdown button will then be displayed on screen.
# Click it to cleanly shut down. 
#
# Sometimes when pressing the power button the side panel,
# the android screen goes black and it's not clear how to
# proceed. Then clicking the Close 'X' on the side panel
# may be necessary to close. This doesn't appear to leave
# any hanging locks around. When closing this way, you'll
# probably need to press the side panel power button to
# turn the device back on. Perhaps the poer button in the
# side panel is more like a sleep button, and to actually
# turn the device off, you need to do it within the
# emulator.

# verify the flutter system can see the emulator, when
# it's running:
flutter doctor

# Run a project to test:
cd /tmp/
flutter create example_test
cd example_test
flutter run

# This takes several minutes to build and load, on my
# development system. The "Taking a long time" messages
# can be ignored.

# Press "Q", in the console, to end the demo app in the
# emulator.

# I can't figure out the proper way to shutdown the
# emulator.

