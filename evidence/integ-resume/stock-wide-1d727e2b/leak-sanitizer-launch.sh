#!/bin/bash
set -eu
r=$(dirname "$(realpath "$0")")
shares=()
fs() { shares+=(-virtfs "local,path=$2,mount_tag=$1,security_model=none,multidevs=remap,readonly=on"); }
fs ubin /usr/bin
fs uinclude /usr/include
fs usyslib /usr/lib/x86_64-linux-gnu
fs ugcc /usr/lib/gcc
fs ulibgcc /usr/libexec/gcc
fs py /usr/lib/python3.13
fs source "$r/code"
runtime="$r/qhost/usr/lib/x86_64-linux-gnu"
exec "$runtime/ld-linux-x86-64.so.2" --library-path "$runtime" "$r/qhost/usr/bin/qemu-system-x86_64" -accel tcg -cpu max -machine pc -smp 1 -m 512 -nodefaults -nic none -display none -monitor none -serial stdio -no-reboot -L "$r/qhost/usr/share/qemu" -bios "$r/qhost/usr/share/seabios/bios-256k.bin" -kernel "$r"/kernel/boot/vmlinuz-* -initrd "$r/initramfs.gz" -append 'console=ttyS0 rdinit=/init panic=-1 quiet' "${shares[@]}"
