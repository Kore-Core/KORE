#!/bin/sh
set -e

if [ $# -lt 3 ]
then
  echo "parameters <address> <amount> <times> <datadir>"
  exit 1
fi

cli="../src/kore-cli"

if [ ! -z "$4" ]; then
  cli="$cli -testnet -datadir=$4"
fi

address=$1
amount=$2
times=$3
i=0
while [ $i -lt $times ]
do
    txid=`$cli sendtoaddress $address $amount`
    echo "<$i, $txid> sent $amount to $address"
    i=$((i + 1))
done