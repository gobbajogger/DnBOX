#!/bin/sh
/bin/busybox --install -s /bin
export PATH=/bin:/usr/bin
set -e
mount -t devtmpfs none /dev
mount -t proc none /proc
mount -t sysfs none /sys
mkdir -p /dev/shm /dev/pts /tmp /home/tester /etc /mnt/source
mount -t tmpfs none /dev/shm
chmod 1777 /tmp /dev/shm
mount -t devpts devpts /dev/pts
echo 'root:x:0:0:root:/root:/bin/sh' >/etc/passwd
echo 'tester:x:1000:1000:tester:/home/tester:/bin/sh' >>/etc/passwd
echo 'tester:x:1000:' >/etc/group
chown 1000:1000 /home/tester
for m in netfs 9pnet 9pnet_virtio 9p; do insmod /modules/$m.ko; done
fs() { mkdir -p "$2"; mount -t 9p -o ro,trans=virtio,version=9p2000.L,msize=262144,cache=loose "$1" "$2"; }
fs ubin /usr/bin
fs uinclude /usr/include
fs usyslib /usr/lib/x86_64-linux-gnu
fs ugcc /usr/lib/gcc
fs ulibgcc /usr/libexec/gcc
fs py /usr/lib/python3.13
fs source /mnt/source
ln -s /usr/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu
ln -s /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
ln -s /usr/lib/gcc /lib/gcc
mkdir -p /etc/alternatives
ln -s /usr/bin/gcc /etc/alternatives/cc
uname -a
set +e
su tester -s /bin/sh -c '/bin/sh /mnt/source/perform.sh'
echo I4_GUEST_EXIT=$?
sync
poweroff -f
