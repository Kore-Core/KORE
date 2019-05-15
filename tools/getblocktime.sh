#!/bin/bash
block=$1
targettime=$2
blocktime=$3
timespan=$4
targettime=$((targettime+timespan))
cli="/home/kore/projects/expresskore/src/kore-cli -datadir=/home/kore/.kore-fast"
lasttime=0
lastblock=$1
foundtime=0
foundblock=0
targetlimit=$blocktime
try=0

echo "Starting in block $block for time $targettime"

checktime() {
    echo -ne "\r""."

    hash=$($cli getblockhash $block)
    retval=$?
        
    if [ $retval -ne 0 ]; then
        echo -ne "\r""No block found! Wait to try again!"
        echo ""
        exit 1
    fi

    time=$($cli getblock $hash | grep -Po "(?<=\"time\": )\d*")

    echo -ne "\r"".."
    if [[ "$time" -eq "$targettime" ]]; then
        foundblock=$block
        foundtime=$time
    else
        thistimelambda=$((targettime-time))
        thistimelambda=$(echo $thistimelambda | sed 's/-//g')

        echo -ne "\r""..."
        if [[ "$lasttime" -ne "0" ]]; then
            lasttimelambda=$((targettime-lasttime))
            lasttimelambda=$(echo $lasttimelambda | sed 's/-//g')
            
            echo -ne "\r""...."
            if [[ "$lasttimelambda" -lt "$targetlimit" || "$thistimelambda" -lt "$targetlimit" ]]; then
                if [[ "$lasttimelambda" -ge "$thistimelambda" ]]; then
                    foundblock=$lastblock
                    foundtime=$lasttime
                else
                    foundblock=$block
                    foundtime=$time
                fi
            elif [[ "$try" -ge "10" ]]; then
                echo -ne "\r""....."
                targetlimit=$((targetlimit+1))
            fi

            try=$((try+1))
        fi

        if [[ "$time" -lt "$targettime" ]]; then
            block=$((block+1))
        else
            block=$((block-1))
        fi

        lastblock=$block
        lasttime=$time
    fi
}

while [[ 1 == 1 ]]; do
    while [[ "$foundblock" -eq "0" ]]; do
        checktime
    done

    echo -ne "\r"
    echo "$foundblock $foundtime"
    try=0
    targetlimit=$blocktime

    block=$foundblock
    targettime=$foundtime
    targettime=$((targettime+timespan))
    foundblock=0
    foundtime=0
done