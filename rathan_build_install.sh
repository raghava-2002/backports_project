#!/bin/bash

#echo 'cleaning the backport project now......'
# Clean any previous build artifacts
# make clean

#echo 'configuring the mac80211_hwsim now.....'
# Configure the build
#make defconfig-hwsim

echo 'Building the hwsim now.......'
# Build the project
make -j8

echo 'Installing the backports now'
# installing the project
sudo make install

