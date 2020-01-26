mkdir /tmp/downloads
cd /tmp/downloads
wget https://storage.googleapis.com/flutter_infra/releases/stable/linux/flutter_linux_v1.12.13+hotfix.5-stable.tar.xz
wget https://dl.google.com/dl/android/studio/ide-zips/3.5.3.0/android-studio-ide-191.6010548-linux.tar.gz
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip

apt-get update && apt-get install curl wget unzip xz-utils git x11-utils

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

