#!/bin/sh
set -e

if [ $# -lt 2 ]
then
  echo "parameters <amount_per_address> <number_of_addresses>"
  exit 1
fi

amount_per_address=$1
number_of_addresses=$2
i=0
while [ $i -lt $number_of_addresses ]
do
    address=`src/kore-cli -datadir=/home/kore/.kore-fast getaccountaddress Stake$i`
    txid=`src/kore-cli -datadir=/home/kore/.kore-fast sendtoaddress $address $amount_per_address`
    echo "<$i, $txid> sent $amount_per_address to $address"
    i=$((i + 1))
done

