#!/bin/sh
set -e

if [ $# -lt 1 ]
then
echo "#############################################################################"
echo "#############################################################################"
echo "#############################################################################"
echo "sendmoney number_of_addressess"
exit 1
fi

mkdir `pwd`/delete-me

send_money_file=`pwd`/delete-me/send_money_script.sh
if [ -f "$send_money_file" ] 
then
    rm $send_money_file
fi
echo "#!/bin/sh" >> $send_money_file
echo "set -e" >> $send_money_file

max_addressess=$1
count=0
while [ $count -lt $max_addressess ]
do
  addr=`../../kore-cli -testnet getnewaddress`
  echo "../../kore-cli -testnet sendtoaddress $addr 1" >> $send_money_file
  count=$((count+1))
done
