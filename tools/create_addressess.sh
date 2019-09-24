#!/bin/bash
set -e

if [ $# -lt 3 ]
then
  echo "parameters <network> <number_of_addressess> <file>. The file with $number_of_addressess addressess to send money"
  exit 1
fi

network=$1
quantity=$2
file=$3

cli="../src/kore-cli $network"

for i in $(eval echo {1..$quantity}); do    
    address=`$cli getnewaddress`
    echo "$address" >> $file
done

