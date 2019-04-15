#!/usr/bin/env bash

# supondo que KORE dir esteja /home/$USER/

/home/$USER/KORE/src/kore-cli -testnet getinfo

split_amount=500
boo=0
amount_of_addresses=50

while [ $boo -lt $amount_of_addresses ]
do
    /home/$USER/KORE/src/kore-cli -testnet getnewaddress >> addr.list
    boo=$((boo + 1))
done

echo Addresses generated: $boo
sendAddress=""
while read address;
do
	echo $address
	sendAddress="$sendAddress\\\"$address\\\":$split_amount,"
done < <(cat addr.list)
sendmany="\"\" \"{$sendAddress"
sendmany=${sendmany:0:${#sendmany}-1}
sendmany="$sendmany}\""
echo /home/$USER/KORE/src/kore-cli -testnet sendmany $sendmany

# kore-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}"
