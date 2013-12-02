#!/bin/bash

#Here we list the tables in order of popularity of their BINS. This should significantly
#speed up the process as more hashes are found earlier.
tablelist=""
counter=0
for i in {0..4};
do
  for bin in `cat bins.txt`
  do
    filename="sha1_"$i"_250x4000000_$bin.rt"
    if [ -e "$filename" ]
    then
      tablelist="$tablelist sha1_"$i"_250x4000000_$bin.rt"
      let counter=$counter+1
    fi
  done
done

echo "Rainbow Table Cracking with $counter tables:"
sudo nice -n -20 ./pancrack $tablelist -l hexhashes.txt -t $1 
