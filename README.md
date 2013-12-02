Stats
----
A whole run takes ~17 minutes on a 2012 macbook pro, gives ~99% accuracy.

Scripts
-----
make the tables with

./generateRainbows.sh bins.txt

crack the hashes with
./crackTables $NUMTHREADS

NB pangen takes the list of hashes in hex.

Clover HashPAN Challenge
========================
Problem: Crack a list of SHA1-encrypted credit card numbers.


#####Come up an algorithmically efficient implementation that minimizes work assuming a single core. Bonus points for efficiently using any/all cores you have at your disposal.

Credit Card numbers are usually 16 digits long and consist of
* a 6 digit Bank Id Number
* a 9 digit Account number
* a 1 digit Checksum

It's not unreasonable to assume that the hashes's BINS only come from the the given PANs, which makes the keyspace 73 billion keys.

Rainbow Tables
-------------
The gist is that they allow you to pick a point between Pure Brute Force and Pure Precomputation of each hash.
######Pure Precomputation
Calculate every of 73 billion hashes and store them. Cracking is just a lookup. Takes 1.8 Tb of space for this contest, (20 bytes hash, 5 bytes PAN * 73E9), which kinda sucks.

######Pure Brute Force
What everyone else here is doing. Run HashCat on a supercomputing multi-GPU machine, get bajjillions of hashes per second and steamroll the hashes. 

Rainbow Tables balance are well suited for this example because there is a limited keyspace, and they definitely "minimize work for a single core".
Also, I'm sure they could scale well to other hashes like sha256 or sha512, where brute force is much slower.

My computer has an SSD in it, making lookups pretty fast, too.
The first run of my untuned single-threaded program took 2 hours to crack 98%.

Technical Info
-----
I've modified the original GPL implementation of rainbow tables, called rainbowcrack. It now takes a BIN number and generates table for all valid PANS with that BIN.
As mentioned above, a run using that (in the folder old) takes about 2 hours.

To add to that, I changed an extension to rcrack, called rcracki\_mt to also work with BINs. It's multithreaded, and has CUDA extensions (which I haven't touched, so there's still a lot of potential for speedup.)

In that rewrite I changed the index size from 64 bit to 32, thereby cutting the size of the tables in half. The actual parameters to the tables can be tweaked, but for this I used 5 tables with 4 million chains of length 250. By halfing the table size I was able to also half the chain length. 
Cracking time is O(n^2) with chain length, which affords a 4x speedup.

The time to go through one table (at the beginning) is ~8seconds and goes down as hashes get cracked. The whole thing takes ~17 minutes.

The tables take ~11Gb of space.
