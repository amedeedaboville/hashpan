#!/usr/bin/python

#This shows a bit of preliminary info about how common each BIN is.
from collections import defaultdict 

f = open('pans.txt', 'r')
raw_pans = [line.strip() for line in f]
f.close()

#Split into parts
pans = [{'bin':p[0:6], 'uid':p[6:15], 'check':p[15]} for p in raw_pans]

counts = defaultdict(int)
for card in pans:
  counts[card['bin']] += 1

#There are only 73 unique BINs in this dataset, out of 983 PANS given.
print "The number of unique pans is %d, out of %d" % (len(counts), len(pans))

#The BIN '503957' is the most common, being used 84 times. 
l_counts = [(k,v) for k,v in counts.items()] #convert to list of pairs, hacks
l_counts.sort(key= lambda x: x[1], reverse=True) #Sort by count
print "The most common BIN is %s, used %d times" % (l_counts[0][0],l_counts[0][1])
#print l_counts
