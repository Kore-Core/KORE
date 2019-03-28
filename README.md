KORE Core integration/staging repository
=====================================

[![Build Status](https://travis-ci.org/KORE-Project/KORE.svg?branch=master)](https://travis-ci.org/KORE-Project/KORE) [![GitHub version](https://badge.fury.io/gh/KORE-Project%2FKORE.svg)](https://badge.fury.io/gh/KORE-Project%2FKORE)

KORE is an open source crypto-currency focused on fast private transactions with low transaction fees & environmental footprint.  It utilizes a custom Proof of Stake protocol for securing its network and uses an innovative variable seesaw reward mechanism that dynamically balances 90% of its block reward size between masternodes and staking nodes and 10% dedicated for budget proposals. The goal of KORE is to achieve a decentralized sustainable crypto currency with near instant full-time private transactions, fair governance and community intelligence.

- Anonymized transactions using the [_Zerocoin Protocol_](http://www.kore.org/zpiv).
- Fast transactions featuring guaranteed zero confirmation transactions, we call it _SwiftX_.
- Decentralized blockchain voting utilizing Masternode technology to form a DAO. The blockchain will distribute monthly treasury funds based on successful proposals submitted by the community and voted on by the DAO.

More information at [kore.org](http://www.kore.org) Visit our ANN thread at [BitcoinTalk](http://www.bitcointalk.org/index.php?topic=1262920)

### Coin Specs

<table>
<tr><td>Algo</td><td>Quark</td></tr>
<tr><td>Block Time</td><td>60 Seconds</td></tr>
<tr><td>Difficulty Retargeting</td><td>Every Block</td></tr>
<tr><td>Max Coin Supply (PoW Phase)</td><td>43,199,500 KORE</td></tr>
<tr><td>Max Coin Supply (PoS Phase)</td><td>Infinite</td></tr>
<tr><td>Premine</td><td>60,000 KORE*</td></tr>
</table>

*60,000 KORE Premine was burned in block [279917](http://www.presstab.pw/phpexplorer/KORE/block.php?blockhash=206d9cfe859798a0b0898ab00d7300be94de0f5469bb446cecb41c3e173a57e0)

### Reward Distribution

<table>
<th colspan=4>Genesis Block</th>
<tr><th>Block Height</th><th>Reward Amount</th><th>Notes</th></tr>
<tr><td>1</td><td>60,000 KORE</td><td>Initial Pre-mine, burnt in block <a href="http://www.presstab.pw/phpexplorer/KORE/block.php?blockhash=206d9cfe859798a0b0898ab00d7300be94de0f5469bb446cecb41c3e173a57e0">279917</a></td></tr>
</table>

### PoW Rewards Breakdown

<table>
<th>Block Height</th><th>Masternodes</th><th>Miner</th><th>Budget</th>
<tr><td>2-43200</td><td>20% (50 KORE)</td><td>80% (200 KORE)</td><td>N/A</td></tr>
<tr><td>43201-151200</td><td>20% (50 KORE)</td><td>70% (200 KORE)</td><td>10% (25 KORE)</td></tr>
<tr><td>151201-259200</td><td>45% (22.5 KORE)</td><td>45% (22.5 KORE)</td><td>10% (5 KORE)</td></tr>
</table>

### PoS Rewards Breakdown

To be Updated!!!

### Installation Steps

Note1: that you can speed up the compilation using the option -j when using make, for example: make -j3

Note2: If you machine has less than 3G memory, you should use a swapfile.

a) enabling swap

```bash
    sudo dd if=/dev/zero of=/swapfile bs,=4096 count=1048576
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
sudo apt-get install -y software-properties-common
sudo add-apt-repository -y ppa:bitcoin/bitcoin
sudo apt-get update
sudo apt-get install -y autotools-dev autoconf automake build-essential bsdmainutils 
sudo apt-get install -y libssl-dev libevent-dev libboost-all-dev libcurl4-openssl-dev sudo apt-get install -y libdb4.8-dev libdb4.8++-dev libzmq3-dev 
sudo apt-get install -y libtool pkg-config protobuf-compiler python3 qttools5-dev
sudo apt-get install -y qttools5-dev-tools libprotobuf-dev libqrencode-dev git curl jq

sudo apt-get update && sudo apt-get upgrade -y
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

### Testnet Onion Address

```bash
Here are some testnet onion address.
you can use the console from kore-qt and give the command to add
addnode a3y4tqttfcj7dvf3.onion onetry
addnode mnzwvlk7lme5yuht.onion onetry
```