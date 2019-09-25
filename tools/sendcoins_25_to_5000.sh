#!/bin/bash
set -e

cli="../src/kore-cli -testnet"

# This array tells the amount we need to send to the address
array_value=(  5000 2500 2000 1500 1250 1000 800 600 500 400 300 200 100 50 25)
array_address=( 2    5     8    8    6    6   6   6   6   5   4   4   4   3  2)
number_of_addressess=$( IFS="+"; bc <<< "${array_address[*]}" )

# Let's compute the amount to send
x=0
total_to_send=0

while [ $x -le  $[${#array_value[@]}-1] ]; do
    value=${array_value[$x]}
    addressess=${array_address[$x]}
    total_to_send=$(($total_to_send + $value*$addressess ))
    x=$(( $x+1 ))
done

if [ $# -lt 1 ]
then
  echo "This script will send $total_to_send splitted into $number_of_addressess addressess"
  echo "Please make sure that you have this amount in this wallet, prepare a <file> with $number_of_addressess addressess"
  echo "Call $0 <file>"
  exit 1
fi
 
# Read the file into addressess array
mapfile -t addressess < $1

if [ ${#addressess[@]} -ne $number_of_addressess ]
then
  echo "The file should have one address per line, total of $number_of_addressess addressess"
  exit 1
fi

total=$[${#array_address[@]} - 1]
if [ $# -gt 1 ]
then
    # The previous execution was not able to finish and we need to continue sending money from where it stopped
    # the idea here is to finished the last loop, and continue from the next element
    k=$2
    i=$3
    j=$4
    # let's finish the last loop
    times=$[${array_address[$i]} -1]
    while [ $j -le $times ]; do
       echo "$k sendtoaddress ${addressess[$k]} ${array_value[$i]}, on error call: $0 $1 $k $i $j"
       txid=`$cli sendtoaddress ${addressess[$k]} ${array_value[$i]}`
       echo "txid=$txid"
       k=$[k+1]
       j=$(( $j+1 ))   
       sleep 1
    done
    i=$(( $i+1 ))
    sleep 5
else
    # First time executing the script
    # k point to the address we want to send the money
    k=0
    i=0
fi

## main loop to send money
while [ $i -le $total ]; do
   # number of times we need to send the same amount
   j=0
   times=$[${array_address[$i]} -1]
   while [ $j -le $times ]; do
       echo "$k sendtoaddress ${addressess[$k]} ${array_value[$i]}, on error call: $0 $1 $k $i $j"
       txid=`$cli sendtoaddress ${addressess[$k]} ${array_value[$i]}`
       echo "txid=$txid"
       k=$[k+1]
       j=$(( $j+1 ))
       sleep 1
    done
    i=$(( $i+1 ))
    sleep 5
done
echo "Congratulations you have sent all the money!!!"
exit 0
