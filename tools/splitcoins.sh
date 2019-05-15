#!/bin/sh
set -e

if [ $# -lt 3 ]
then
  echo "parameters <amount_per_address> <number_of_addresses> <datadir>"
  exit 1
fi

cli="../src/kore-cli"

if [ ! -z "$3" ]; then
  cli="$cli -datadir=$3"    
fi

amount_per_address=$1
number_of_addresses=$2
i=0
while [ $i -lt $number_of_addresses ]
do
    address=`$cli getaccountaddress Stake$i`
    txid=`$cli sendtoaddress $address $amount_per_address`
    echo "<$i, $txid> sent $amount_per_address to $address"
    i=$((i + 1))
done

