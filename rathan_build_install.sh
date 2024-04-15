#!/bin/bash

# Clean any previous build artifacts
make clean

# Configure the build
make defconfig-hwsim

# Build the project
make -j8

# installing the project
sudo make install
