#!/bin/bash
set -e

array_value=(   80  90	110	120	130	140	150	160	170	180	190	200	210	220	230	240	250	263	267	278	291	302	330	342	355	360	375	388	394	404	420	432	440	455	487	491	502	531	545	568	630	651	680	699	707	713	720	746	764	777	789	800)
array_address=(	31	15	10	12	14	10	16	3	2	2	1	1	2	3	4	2	2	3	1	4	2	1	1	2	3	1	3	4	5	6	7	3	4	5	6	3	7	3	5	2	4	1	2	3	4	5	3	2	3	2	5	6)
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
       echo "$k Should send ${array_value[$i]} to ${addressess[$k]}, on error call: $0 $1 $k $i $j"
       k=$[k+1]
       j=$(( $j+1 ))   
    done
    i=$(( $i+1 ))
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
       echo "$k Sending ${array_value[$i]} to ${addressess[$k]}, on error call: $0 $1 $k $i $j"
       k=$[k+1]
       j=$(( $j+1 ))   
    done
    i=$(( $i+1 ))
done
echo "Congratulations you have sent all the money!!!"
exit 0
