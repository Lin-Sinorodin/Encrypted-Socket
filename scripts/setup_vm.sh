#!/bin/bash

alias sudo_apt="sudo apt -o Apt::Get::Assume-Yes=true"

# Install prerequisites
sudo_apt update
sudo_apt install build-essential libncurses5-dev bison flex libssl-dev libelf-dev libudev-dev libpci-dev libiberty-dev fakeroot git strace tcpdump

# Get the kernel source
sudo_apt install linux-source
cd /usr/src
tar -xvf linux-source-*.tar.xz
rm -f linux-source-*.tar.xz
cd linux-source-*

# Create config
cp /boot/config-$(uname -r) .config
make olddefconfig
sed -i s/CONFIG_MODULE_SIG=y/CONFIG_MODULE_SIG=n/g .config
sed -i s/CONFIG_DEBUG_INFO_BTF=y/CONFIG_DEBUG_INFO_BTF=n/g .config