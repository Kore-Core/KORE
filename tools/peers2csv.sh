#!/bin/sh
set -e

if [ $# -lt 2 ]; then
    echo "#############################################################################"
    echo "#############################################################################"
    echo "#############################################################################"
    echo "## peerstocsv.sh "
    echo "## Parameters"
    echo "##   file : csv file name"
    echo "##   datadir : optional"
    exit 1
fi

cli="../src/kore-cli"
if [ ! -z "$2" ]; then
  cli="$cli -datadir=$2"
fi

fileName=$1

if [ -f "$fileName" ]; then
  echo "File Already exists !!!"
  exit 1
fi

command="$cli getpeerinfo"
csvheader="id addr version subver banscore startingheight synced_headers"
$csvheader >> $fileName
csvlines=`$command | jq .[] | jq "\(.id) \n"`
$csvlines >> $fileName

