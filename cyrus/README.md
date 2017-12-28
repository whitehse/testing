```
wget https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-9.3.0-amd64-netinst.iso
#wget https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-9.3.0-amd64-xfce-CD-1.iso
#apt-get install syslinux-utils dosfstools
apt-get install dosfstools
dd if=/dev/zero of=debian.img bs=1M count=1024
echo 'start=1, type=6, bootable' | /sbin/sfdisk debian.img
sudo adduser dwhite disk
newgrp disk
export PATH=$PATH:/sbin/:/usr/sbin/
losetup -o 512 /dev/loop0 debian.img
mkdosfs /dev/loop0
strace -f syslinux /dev/loop0
mkdir mnt
sudo mount /dev/loop0 -o uid=dwhite,gid=src mnt
mkdir mnt-install
sudo mount debian-9.3.0-amd64-netinst.iso mnt-install/
cp -a mnt-install/* mnt/
sudo umount mnt-install/
cat - <<EOF > mnt/syslinux.cfg
default install.amd/vmlinuz initrd=install.amd/initrd.gz
EOF
sudo umount mnt
losetup -d /dev/loop0

https://wiki.n0r1sk.com/index.php/Debian_Remaster_Netinstaller_-_Integrate_Firmware_bnx2x_and_Preseed

*/

mkdir -p netinstaller/cd
cd netinstaller && wget https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-9.3.0-amd64-netinst.iso
sudo apt-get install -y p7zip
7z x -ocd debian-9.3.0-amd64-netinst.iso
mkdir initrd
cd initrd && zcat ../cd/install.amd/initrd.gz | cpio -iv
cp -v <yourpreseedfile> reseed.cfg
find . -print0 | cpio -0 -H newc -ov | gzip -c > ../cd/install.amd/initrd.gz
cd ../cd
md5sum `find ! -name "md5sum.txt" ! -path "./isolinux/*" -follow -type f` > md5sum.txt;cd -
mkisofs -o debian-9.3.0-amd64-netinst-preseed.iso -r -J -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/boot.cat cd/
```
