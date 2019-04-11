#!/bin/sh
set -e

masternode_coins_amount=500
coin=kore
echo " how many parameters $#"
if [ $# -lt 3 ]
then
echo "#############################################################################"
echo "#############################################################################"
echo "#############################################################################"
echo "## "
echo "## This is a Kore script to help create a masternode configuration file"
echo "## masternode_creation.sh <network> <name> <onion-address> <user> <password>"
echo "## Parameters"
echo "##   network: mainnet or testnet"
echo "##   name: a name for your masternode"
echo "##   onion-address: your masternode onion address"
echo "##   user: control wallet rpc-user (optional)"
echo "##   password: control wallet rpc-password (option)"
echo "## example:"
echo "##   masternode_creation.sh testnet MasternodeI 443xtqzgm2vxi7fy.onion  kore 12345"
echo "## "
echo "## Requirements"
echo "##   1) run this script in your control wallet"
echo "##   2) have jq installed in the systems"
echo "##   3) you should have $masternode_coins_amount COINS in this wallet"
echo "##   4) Make sure the kored is running at your masternode, you need the masternode"
echo "##       onion address and it needs to be up to lock the coins."
echo "##"
echo "## PLEASE install jq from here https://stedolan.github.io/jq/", in this folder
echo "##        make sure it is named as jq. Ubuntu you can install like this:"
echo "##        sudo apt-get install jq -y"
echo "#############################################################################"
echo "#############################################################################"

exit 1
fi

if [ $# -eq 4 ]
then
$echo "## missing authentication user and password"
exit 1
fi


if [ $# -eq 5 ]
then
control_wallet_user=$4
control_wallet_password=$5
fi

echo "##########################################################################"
echo "####################                          ############################"
echo "#################### MASTERNODE CONFIGURATION ############################"
echo "####################                          ############################"

dir=`pwd`/../../src
user_dir=$HOME
network=$1
masternode_name=$2
masternode_onion_address=$3
masternode_user=$control_wallet_user
masternode_password=$control_wallet_password
masternode_conf_file=`pwd`/$masternode_name.conf
readme=`pwd`/$masternode_name.readme
activation_file=`pwd`/$masternode_name.activation
my_conf_file=$user_dir/.$coin/kore.conf
if [ $# -eq 5 ]
then
cli_args="-$network -debug -rpcuser=$control_wallet_user -rpcpassword=$control_wallet_password"
else 
cli_args="-$network -debug"
fi

if [ "$network" = "testnet" ] || [ "$network" = "TESTNET" ]
then
  masternode_port=11743
  control_wallet="$user_dir/.$coin/testnet3/masternode.conf"
  control_wallet_onion=`cat $user_dir/.$coin/testnet3/onion/hostname`
else
  masternode_port=10743
  control_wallet="$user_dir/.$coin/masternode.conf"
  control_wallet_onion=`cat $user_dir/.$coin/onion/hostname`
fi
# needs to be the same as nMasternodeMinConfirmations
txConfirmations=15

echo "## "
echo "## Parameters used"
echo "##   network: $network"
echo "##   masternode-name   : $masternode_name"
echo "##   masternode-address: $masternode_onion_address"
echo "##   masternode-port   : $masternode_port"
if [ $# -eq 5 ]
then
echo "##   user    : $masternode_user"
echo "##   password: $masternode_password"
fi
echo "##   cli args: $cli_args"
echo "##########################################################################"

echo "Executing from $dir"
echo "Creating masternode account"
command="$dir/kore-cli $cli_args getaccountaddress $masternode_name"
echo "  command: $command"
masternode_account=`$command`
echo "  account created: $masternode_account"
echo "Sending $masternode_coins_amount to $masternode_account"
command="$dir/kore-cli $cli_args sendtoaddress $masternode_account $masternode_coins_amount"
echo "  command: $command"
masternode_tx=`$command`
echo "send result: $masternode_tx"

echo "Generating masternode Private Key"
command="$dir/kore-cli $cli_args masternode genkey"
echo "  command: $command"
masternode_private_key=`$command`

echo "Generating $masternode_conf_file file"
if [ -f "$masternode_conf_file" ]
then
    rm $masternode_conf_file
fi

echo "server=1" > $masternode_conf_file
echo "daemon=1" >> $masternode_conf_file
echo "addnode=$control_wallet_onion" >> $masternode_conf_file
if [ $# -eq 5 ]
then
echo "rpcuser=$masternode_user"  >> $masternode_conf_file
echo "rpcpassword=$masternode_password"  >> $masternode_conf_file
fi
echo "listen=1"  >> $masternode_conf_file
echo "staking=0"  >> $masternode_conf_file

echo "masternode=1"  >> $masternode_conf_file
echo "masternodeprivkey=$masternode_private_key"  >> $masternode_conf_file
echo "masternodeaddr=$masternode_onion_address"   >> $masternode_conf_file
echo "masternode account= $masternode_account"    >> $masternode_conf_file

echo "Generating $control_wallet file"
command="$dir/kore-cli $cli_args gettransaction $masternode_tx"
echo "  command: $command"
hex_raw_transaction=`$command | jq .hex | tr -d \"`

echo "$hex_raw_transaction"
command="$dir/kore-cli $cli_args decoderawtransaction $hex_raw_transaction"
echo "  command: $command"
nValue=`$command | jq .vout[] | jq select\(.value==$masternode_coins_amount\) | jq .n`

echo "#######################################################################"
echo "##  Updating this wallet masternode.conf file: $control_wallet #"
new_masternode="$masternode_name $masternode_onion_address:$masternode_port $masternode_private_key $masternode_tx $nValue"
echo "## $new_masternode"
echo "$new_masternode" >> $control_wallet
masternode_activation_command="`pwd`/masternode_activation.sh $dir/kore-cli \"$cli_args\" $masternode_name $masternode_tx $masternode_onion_address"
echo "## command to activate this masternode: $masternode_activation_command" >> $masternode_conf_file
echo "## The following is the masternode entry at masternode.conf "  >> $masternode_conf_file
echo "## $new_masternode"   >> $masternode_conf_file

echo "##########################################################################"
echo "## "
echo "addnode=$masternode_onion_address" >> $my_conf_file

echo "##########################################################################"
echo "## Let's wait for the Confirmations"
echo "##########################################################################"
command="$dir/kore-cli $cli_args gettransaction $masternode_tx"
echo " Sending command: $command"
confirmations=`$command | jq .confirmations`
echo "Confirmations $confirmations"
while [ $confirmations -lt $txConfirmations ]
do
  echo " Waiting for $txConfirmations confirmations, so far we have $confirmations"
  sleep 10
  confirmations=`$command | jq .confirmations`
done
echo ""
echo " COOL ! We got at least $txConfirmations confirmations"
echo ""

echo "Generating $activation_file file"
if [ -f "$activation_file" ]
then
    rm $activation_file
fi
echo "#!/bin/sh" >> $activation_file
echo "set -e" >> $activation_file
echo "" >> $activation_file
echo $masternode_activation_command >> $activation_file
chmod +x $activation_file


echo "Congratulations !!! Your Masternode is ready to be started."  >> $readme
echo "Please, now follow instruction at $readme"
if [ -f "$readme" ] 
then
    rm $readme
fi

echo "##########################################################################" >> $readme
echo "## Congratulations !!!"  >> $readme
echo "## your Masternode is ready to be started !!!             "  >> $readme
echo "## " >> $readme
echo "## Now you need to perform the following steps"  >> $readme
echo "##   1. Change your masternode $coin.conf with the parameters found here:"  >> $readme
echo "##      $masternode_conf_file"  >> $readme
echo "##   2. Restart your masternode"  >> $readme
echo "##   3. Restart this control Wallet, so the local masternode.conf will take effect."  >> $readme
echo "##   4. Activate your masternode. Make sure your mastenode is in sync."  >> $readme
echo "##      execute this command: $activation_file"  >> $readme
echo "##   5. Check if the masternode are enabled"   >> $readme
echo "##        kore-cli -testnet masternode status"   >> $readme
echo "##        ** The message has to be: \"Masternode successfully started\""   >> $readme
echo "##"
echo "##   Some importante commands command you may use from Control Wallet" >> $readme
echo "##     - kore-cli -testnet listreceivedbyaccount" >> $readme
echo "##     - kore-cli -testnet mnsync status" >> $readme
echo "##     - kore-cli -testnet mnsync reset" >> $readme
echo "##" >> $readme
echo "##   Some importante commands command you may use from Masternode" >> $readme
echo "##     - kore-cli -testnet masternode status" >> $readme
echo "##     - kore-cli -testnet masternode count" >> $readme
echo "##     - kore-cli -testnet masternode list" >> $readme
echo "##########################################################################"  >> $readme