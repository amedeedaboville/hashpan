#!/bin/bash

for bin in `cat $1`
do
  echo "Making tables for BIN $bin"
  for i in {0..4};
  do
    ./pangen sha1 $bin $i 500 2000000 $bin
    filename="sha1_"$i"_500x2000000_$bin.rt"
    #./rtsort $filename
  done
done
