#!/bin/bash

#Here we list the tables in order of popularity of their BINS. This should significantly
#speed up the process as more hashes are found earlier.
tablelist=""
counter=0
for bin in `cat bins.txt`
do
  for i in {0..3};
  do
    filename="sha1_"$i"_1000x1000000_$bin.rt"
    if [ -e "$filename" ]
    then
      tablelist="$tablelist sha1_"$i"_1000x1000000_$bin.rt"
      let counter=$counter+1
    fi
  done
done

#echo $tablelist
echo "Rainbow Table Cracking with $counter tables:"
sudo nice -n -20 ../rcrack $tablelist -l hexhashes.txt
