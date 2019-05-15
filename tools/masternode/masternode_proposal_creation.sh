#!/bin/sh

if [ $# -lt 5 ]
then
echo "#############################################################################"
echo "#############################################################################"
echo "#############################################################################"
echo "## "
echo "## This is a Kore script to help create a proposal "
echo "## masternode_proposal_creation.sh <network> <proposal-name> <proposal-link> <amount> <payments>"
echo "## Parameters"
echo "##   network: mainnet or testnet"
echo "##   name: a name for your proposal - without spaces"
echo "##   link: link to your proposal - without spaces"
echo "##   amount: one payment amount"
echo "##   payments: how many payments of amount"
echo "##"
echo "##   example: ./masternode_proposal_creation.sh testnet Primeira-Proposta Link-da-Proposta 555 2"
exit 1
fi

dir=`pwd`/../../src
network=$1
proposal_name="\"$2\""
proposal_link="\"$3\""
proposal_one_payment=$4
proposal_how_many_payments=$5
control_wallet_user=kore
control_wallet_password=kore
# 50COINS submition fee
# 50COINS activation fee
masternode_proposal_fee=100 
# This parameter should match 
# chainparams nBudgetFeeConfirmations
proposal_fee_confirmations=6
nBudgetFeeConfirmations=6


cli_args="-$network -debug -rpcuser=$control_wallet_user -rpcpassword=$control_wallet_password"

echo "## "
echo "## Parameters used"
echo "##   network       : $network"
echo "##   proposal name : $proposal_name"
echo "##   proposal link : $proposal_link"
echo "##   proposal fee  : $masternode_proposal_fee"
echo "##   cli args      : $cli_args"
echo "##   collateral conf: $collateral_confirmations"
echo "##   "
echo "##   proposal $proposal_how_many_payments parcel of $proposal_one_payment, total amount  `expr $proposal_one_payment \* $proposal_how_many_payments` "
echo "##########################################################################"

echo "Executing from $dir"

echo ""
echo "##########################################################################"
echo "## 1 Step - Create the proposal address"
echo "## Creating Proposal address"
command="$dir/kore-cli $cli_args getaccountaddress $proposal_name"
echo "  command: $command"
proposal_account=`$command`

echo ""
echo ""
echo "##########################################################################"
echo "## 2 Step - Send $masternode_proposal_fee to $proposal_account"
command="$dir/kore-cli $cli_args sendtoaddress $proposal_account $masternode_proposal_fee"
echo "  command: $command"
proposal_fee_tx=`$command`

echo "##########################################################################"
echo "## Let's wait for the Proposal Fee Confirmations"
echo "##########################################################################"
command="$dir/kore-cli $cli_args gettransaction $proposal_fee_tx"
echo "command: $command"
confirmations=`$command | jq .confirmations`
while [ $confirmations -lt $proposal_fee_confirmations ]
do
  echo " Waiting for $proposal_fee_confirmations confirmations, so far we have $confirmations"
  sleep 10
  confirmations=`$command | jq .confirmations`
done


command="$dir/kore-cli $cli_args mnbudget nextblock"
proposal_start_at_block=`$command`
block_count=`$dir/kore-cli $cli_args getblockcount`

echo "##########################################################################"
echo "## Get the next Superblock "
echo "next super block: $proposal_start_at_block"
echo "current block   : $block_count"
while [ $(expr $proposal_start_at_block - $block_count) -lt 5 ]
do
  echo "Next Super Block Too Close."
  echo " Waiting the distance to be greather then 5: now: `expr $proposal_start_at_block - $block_count`"
  sleep 1
  proposal_start_at_block=`$command`
  block_count=`$dir/kore-cli $cli_args getblockcount`
done


echo ""
echo ""
echo "##########################################################################"
echo "## 3 Step - Prepare Proposal"
command="$dir/kore-cli $cli_args mnbudget prepare $proposal_name $proposal_link $proposal_how_many_payments $proposal_start_at_block $proposal_account $proposal_one_payment"
echo "  command: $command"
proposal_preparation_hash=`$command`



echo "##########################################################################"
echo "## it is necessary to wait for the collateral confirmations"
echo "##########################################################################"
command="$dir/kore-cli $cli_args gettransaction $proposal_preparation_hash"
echo "$command | jq .confirmations"
confirmations=`$command | jq .confirmations`
while [ $confirmations -lt $nBudgetFeeConfirmations ]
do
  echo " Waiting for $nBudgetFeeConfirmations confirmations, so far we have $confirmations"
  sleep 1
  confirmations=`$command | jq .confirmations`
done
echo " COOL ! We got at least $nBudgetFeeConfirmations confirmations"
echo ""
echo ""
echo ""
echo "##########################################################################"
echo "## 4 Step - Submit Proposal"
command="$dir/kore-cli $cli_args mnbudget submit $proposal_name $proposal_link $proposal_how_many_payments $proposal_start_at_block $proposal_account $proposal_one_payment $proposal_preparation_hash"
echo "  command: $command"
voting_hash=`$command`
echo ""
echo ""
echo "##########################################################################"
echo "## Proposal Details"
command="$dir/kore-cli $cli_args mnbudget getinfo $proposal_name"
echo "  command: $command"
echo `$command`

echo ""
echo ""
echo "##########################################################################"
echo "## Now you can publish the hash for voting"
echo "## "
echo "## VOTING FROM CONTROL WALLET"
echo "##     To vote YES: $dir/kore-cli $cli_args mnbudget vote-many $voting_hash yes"
echo "##     To vote NO : $dir/kore-cli $cli_args mnbudget vote-many $voting_hash no"
echo "##"
echo "## VOTING FROM MASTERNODE"
echo "##     To vote YES: $dir/kore-cli $cli_args mnbudget vote-many $voting_hash yes"
echo "##     To vote NO : $dir/kore-cli $cli_args mnbudget vote-many $voting_hash no"