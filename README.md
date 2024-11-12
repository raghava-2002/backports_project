# Backports Project: Enhancing Privacy in Wireless Communications: AP-Initiated Dynamic MAC Re-Randomization

## Introduction

This project is part of my MSc thesis work at KTH Royal Institute of Technology, implements a MAC address re-randomization scheme within a wireless network to enhance security and privacy. By dynamically managing and translating MAC addresses, this project reduces tracking risks and improves overall privacy in wireless communications.

## Problem Statement

In modern wireless networks, static MAC addresses can lead to user tracking and privacy breaches. This project addresses these concerns by developing a dynamic MAC address re-randomization protocol, offering enhanced privacy without significant impact on network performance.

## Solution Overview

This protocol introduces time independent MAC address re-randomization within the mac80211 subsystem, achieved through kernel-level modifications. By integrating wmediumd and Mininet-WiFi, the project simulates real-world scenarios, analyzing how time drift between AP and stations affects privacy and network stability.

## Key Features

- **Dynamic MAC Address Randomization**: A robust kernel-level protocol for periodically changing MAC addresses to hinder tracking.
- **Time-Drift Analysis**: Tests with applied time drift to assess the protocol's effectiveness in linking MAC addresses without compromising privacy.
- **Network Performance Optimization**: Evaluation of throughput, packet loss, and jitter using iperf3 and Wireshark to ensure minimal performance impact.
- **Visualization of MAC Transitions**: Graphical representation of MAC address transitions, highlighting privacy gains and potential vulnerabilities.


## Technologies Used

- **Linux Kernel (mac80211 subsystem modifications)**
- **Programming Languages: C, Python**
- **Simulation and Testing: wmediumd, Mininet-WiFi, iperf3, Wireshark**


## System Requirements

- **Operating System**: Ubuntu 20.04-6 LTS
- **Kernel Version**: 5.4.0-177-generic (x86_64)
- **Compiler**: GCC 9.4.0
- **Make**: GNU Make 4.2.1
- **Backports Version**: backports-5.4.56-1

## Installation Instructions

### Step 1: Clone the Repository

To get started, clone the project repository:

```sh
git clone https://github.com/raghava-2002/backports_project.git
cd backports-project
```

### Step 2: Build and Install

Configure the Backports Project for WiFi:

```sh
make defconfig-wifi
```

Build the Backports:

```sh
make
```

Install the Backports:

```sh
sudo make install
```

### Step 3: Using Support Scripts and Tools

The support folder in the project contains various tools and scripts, including configurations for hostapd and wpa_supplicant to manage wireless authentication and associations. To explore and use these tools, run:

```sh
./full_clean.sh
```

## Dependencies and System Setup

This project has several dependencies that need to be installed and configured. Below are the steps for setting up your system.

### Essential Kernel and Development Tools

Update System:

```sh
sudo apt-get update
```

Install the Required Kernel Version: Install and boot into the specified kernel version (5.4) for compatibility:

```sh
sudo apt-get install linux-image-5.4.0-177-generic linux-headers-5.4.0-177-generic
```

Reboot with the 5.4 Kernel: Ensure the system boots with the correct kernel by updating GRUB if necessary:

```sh
sudo update-grub
```

Install Build Essentials:

```sh
sudo apt-get install build-essential
```

Install Additional Dependencies:

```sh
sudo apt-get install libncurses-dev flex bison
```

Install Networking Libraries:

```sh
sudo apt-get install libnl-3-dev libnl-genl-3-dev
```

Install Hostapd and Net-Tools:

```sh
sudo apt-get install hostapd net-tools
```

### Optional: Mininet-WiFi Installation

For testing and simulation, install Mininet-WiFi by following these steps:

Clone the Mininet-WiFi Repository:

```sh
sudo apt-get install git
git clone https://github.com/intrig-unicamp/mininet-wifi
cd mininet-wifi
```

Install Mininet-WiFi with Dependencies:

```sh
sudo util/install.sh -Wlnfv
```

- `-W`: Wireless dependencies
- `-n`: Mininet-WiFi dependencies
- `-f`: OpenFlow
- `-v`: OpenvSwitch
- `-l`: wmediumd (optional for wireless medium emulation)

### Optional: Wmediumd Installation

To install wmediumd for wireless emulation:

Navigate to the Support Folder:

```sh
cd support/wmediumd
```

Build and Install Wmediumd:

```sh
sudo make install
```

## Results

Checkout the other repository, support folder for this project https://github.com/raghava-2002/Hwsim_test_backport_project


## License 

This project is licensed under Rathan Appana

## Contact

For questions or collaboration inquiries, please reach out via rathanappana@gmail.com