KORE Core integration/staging repository
=====================================

KORE is an open source crypto-currency focused on security and privacy with fast private transactions consisting of low transaction fees & environmental footprint.  It utilizes a custom Proof of Stake protocol for securing its network.


### Coin Specs

<table>
<tr><td>Pre Fork Algo</td><td>Momentum</td></tr>
<tr><td>Post Fork Algo</td><td>Yescrypt R32</td></tr>
<tr><td>Block Time</td><td>60 Seconds</td></tr>
<tr><td>Difficulty Retargeting</td><td>Every Block</td></tr>
<tr><td>Max Coin Supply</td><td>12,000,000 KORE</td></tr>
</table>



### PoS Rewards Breakdown

To be Added!!!

### Installation Steps

Note1: that you can speed up the compilation using the option -j when using make, for example: make -j3

Note2: If you machine has less than 3G memory, you should use a swapfile.

a) enabling swap

```bash
    sudo dd if=/dev/zero of=/swapfile bs=4096 count=1048576
    sudo chmod 600 /swapfile
    sudo mkswap /swapfile
    sudo swapon /swapfile
```

b) disabling swap

```bash
    sudo swapoff -v /swapfile
    sudo rm /swapfile
```

### 1. Installating dependencies

```bash
sudo apt-get update
sudo apt-get install -y git curl jq
sudo apt-get install -y software-properties-common
sudo apt-get install -y autotools-dev autoconf automake build-essential
sudo apt-get install -y qttools5-dev-tools qttools5-dev libprotobuf-dev libqrencode-dev
sudo apt-get install -y libtool pkg-config protobuf-compiler python3
sudo apt-get install -y devscripts debhelper


```

### 2. Building KORE dependencies

```bash
cd depends
make
```

### 3. Building KORE source

```bash
cd ..
./autogen.sh
./configure --with-gui=qt5 --prefix=`pwd`/depends/x86_64-pc-linux-gnu --disable-tests  --enable-tor-browser

make
```

### 4. Generating the installer (.deb)
#### First, Download Go
```bash
From a web browser open and save the following link: 
  https://golang.org/doc/install?download=go1.12.7.linux-amd64.tar.gz
```

#### Second, Install Go
```bash
cd ~/Downloads
sudo tar -C /usr/local -xzf go1.12.7.linux-amd64.tar.gz
```

#### Third, make Go available 
```bash
in a terminal make go available, with the following command:
export PATH=$PATH:/usr/local/go/bin
```

#### Fourth, generate the (.deb)
```bash
from the kore root directory, give the command:
make deploy
```

### 5. Installing kore (.deb)
```bash
  The installer is generated in the share folder, so in order to install it, give the following command: 
  cd <kore-dir>/share
  sudo apt install ./kore_<version>_amd64.deb
  * <kore-dir> is the directory where you download the kore git repository
  * <version> is this source code version
  ** if you get problems with old version, you can remove with the command:
    sudo apt-get remove kore --purge -y
```

