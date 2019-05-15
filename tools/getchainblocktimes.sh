#!/bin/sh
set -e

if [ $# -lt 3 ]; then
echo "#############################################################################"
echo "#############################################################################"
echo "#############################################################################"
echo "## getchainblocktimes.sh <startblock> <finalblock> <lastBlockTime> <file>"
echo "## Parameters"
echo "##   startblock: block to start"
echo "##   finalblock: block to finish"
echo "##   file : csv file name"
echo "##   datadir : optional"
exit 1
fi

cli="../src/kore-cli"

if [ ! -z "$4" ]; then
  cli="$cli -datadir=$4"    
fi

startBlock=$1
finalBlock=$(($2 + 1))
fileName=$3

blockHeight=514


if [ ! -f "$fileName" ]; then
  # only add the header it the file doesn exists
  echo "BlockHeight BlockTime TimeDiffPrev" >> $fileName    
fi

# lets get the first block time, to start our calculations
command="$cli getblockhash $(($startBlock-1))"
echo "$command"
firstblockhash=`$command`
command="$cli getblock $firstblockhash"
timeBlockPrev=`$command | jq .time`

i=$startBlock
while [ $i -lt $finalBlock ]
do
    echo "Getting Information from Block: $i"
    command="$cli getblockhash $i"
    blockhash=`$command`
    command="$cli getblock $blockhash"
    blockTime=`$command | jq .time`
    timeDiff=$(($blockTime - $timeBlockPrev))
    echo "TimeDiff $timeDiff"
    echo "$i $blockTime $timeDiff" >> $fileName
    timeBlockPrev=$blockTime
    i=$((i + 1))
done

echo "Results at file: $fileName"



