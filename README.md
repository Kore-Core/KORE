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

### Installating dependencies

```bash
sudo apt-get update
sudo apt-get install -y git curl jq
sudo apt-get install -y software-properties-common
sudo apt-get install -y autotools-dev autoconf automake build-essential bsdmainutils
sudo apt-get install -y qttools5-dev-tools qttools5-dev libprotobuf-dev libqrencode-dev
sudo apt-get install -y libtool pkg-config protobuf-compiler python3


```

### Building KORE dependencies

```bash
cd depends
make
```

### Building KORE source

```bash
cd ..
./autogen.sh
./configure --with-gui=qt5 --prefix=$(pwd)/depends/x86_64-pc-linux-gnu

make
```


