#!/bin/sh
set -e

if [ $# -lt 2 ]
then
  echo "parameters <build><commit_message>"
  exit 1
fi

build=$1
commit_message=$2
tag="v0.13.0.$build"
build_message="build #$build"

echo "Before running this script, are you sure you have created: tag $tag ?"
echo "Please verify that now with the command: git tag | grep $tag"
echo "If you have not created, just type <ctrl><c>, otherwiser press <enter>"
read _


echo "Retrieving the tag"
cd expresskore
git checkout dev
git pull
git checkout $tag

echo "Updating github Repository"
cd ..
rm -rf ./KORE/*
cp -rf ./expresskore/* ./KORE
cd ./KORE
git status .

echo 'If the changes are there, please press [ENTER] to continue.'
read _

git add .
git commit -m \"$commit_message\"

Creating the tag $tag
git push
git tag -a $tag -m \"$build_message\"
git push --force origin $tag