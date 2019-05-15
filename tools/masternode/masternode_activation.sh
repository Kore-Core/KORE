#!/bin/sh
set -e

if [ $# -lt 3 ]
then
echo "#############################################################################"
echo "#############################################################################"
echo "#############################################################################"
echo "## "
echo "## This is a Kore script and it activates a masternode."
echo "## masternode_activation.sh <cli> <cli-args> <masternode-name> <masternode_tx> <masternode_onion_address>"
echo "## Parameters"
echo "##   cli: kore-cli location"
echo "##   cli-args: kore-cli arguments"
echo "##   masternode-name: masternode name, as it is in the masternode.conf"
echo "##   masternode_tx: masternode transaction"
echo "##   masternode_onion_address: masternode onion"

exit 1
fi

cli="$1"
cli_args="$2"
masternode_name="$3"
masternode_tx=$4
masternode_onion_address=$5

echo "## Parameters"
echo "##   cli: $cli"
echo "##   cli-args: $cli_args"
echo "##   masternode-name: $masternode_name"
echo "##   masternode_tx: $masternode_tx"


echo "#############################################################################"
echo "## Let's make sure we have a direct connection to the masternode "
command="$cli $cli_args addnode $masternode_onion_address onetry "
`$command`
echo "Waiting the masternode onion address ($masternode_onion_address) to be in the peerlist"
command="$cli $cli_args getpeerinfo"
amIinthePeerList=`$command | jq [.[].addr] | jq map\(test\(\"$masternode_onion_address\"\)\) | jq any`
while [ $amIinthePeerList != true ]
do
  echo "Waiting the masternode onion address ($masternode_onion_address) to be in the peerlist"
  sleep 5
  amIinthePeerList=`$command | jq [.[].addr] | jq map\(test\(\"$masternode_onion_address\"\)\) | jq any`
done

echo "#############################################################################"
echo "## Let's make sure the blockchain is in SYNC"
echo ""
command="$cli $cli_args mnsync status"
IsBlockChainSynced=`$command | jq .IsBlockchainSynced`

while [ $IsBlockChainSynced != true ]
do
  echo "Waiting Blockchain to be in sync"
  sleep 5
  IsBlockChainSynced=`$command | jq .IsBlockchainSynced`
done

echo "#############################################################################"
echo "## COOL, Block Chain is in SYNC"

echo "##########################################################################"
echo "## Let's Make sure the coins are locked"
echo "##########################################################################"
command="$cli $cli_args listlockunspent"
echo "Command: $command"
tx_locked=`$command | jq .[].txid | jq scan\(\"$masternode_tx\"\)`
echo "Check: $tx_locked = \"$masternode_tx\""
if [ "$tx_locked" = "\"$masternode_tx\"" ]
then
echo "## GOOD the transaction $masternode_tx locked $masternode_coins_amount $coin"
else
echo "## BAD NEWS !!!"
echo "## Something Went Wrong, the coins are not locked !"
echo "## Please take a look at the command: $command"
exit 0
fi


echo ""
echo "## Now Let's start the masternode"

command="$cli $cli_args startmasternode alias false $masternode_name"
echo "Command: $command"
masternodeStarted=`$command | jq .detail[].result`

echo ""
echo ""
if [ "$masternodeStarted" = "\"successful\"" ]
then
  echo "##########################################################################"
  echo "## Congratulations !!!"
  echo "## YOUR MASTERNODE WAS ACTIVATED SUCCESSFULLY"
  echo "##########################################################################"
  echo ""
  echo ""
  echo "## Now you just need to wait for the Masternode Synchronization"
else
  echo "##########################################################################"
  echo "## Some Problem Happened when ACTIVATING THE masternode"
  echo "## result: $masternodeStarted"
  echo "## if you wanna try the command here it is:"
  echo "## $command"
  echo "##########################################################################"
fi


