#!/bin/bash

for bin in `cat $1`
do
  echo "Making tables for BIN $bin"
  for i in {0..3};
  do
    ./pangen sha1 $bin $i 1000 1000000 $bin 
    filename="sha1_"$i"_1000x1000000_$bin.rt"
    ./rtsort $filename
  done
done
