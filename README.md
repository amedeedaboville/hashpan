Clover HashPAN Challenge
========================

Winner gets a Mac Pro, solid entries get an interview at Clover.
Problem: Crack a list of SHA1-encrypted credit card numbers.

Credit Card numbers are usually 16 digits long and consist of
  a 6 digit Bank Id Number
  a 9 digit Account number
  a 1 digit Checksum

I'll be covering two cases: This contest, with a small number of hashes, and a real scenario where a DB gets dumped and there are millions of hashes.
The difference between the two is that the contest has a much smaller number of hashes (1022 vs millions)  and likely fewer BINs (73 vs I'd guess ~1000).

The given PANs (Primary Account Numbers) are all fake, though they reflect a real distribution.  There are 73 different BINs, which means our keyspace is 73e9. Piece of cake.

"Come up an algorithmically efficient implementation that minimizes work assuming a single core. Bonus points for efficiently using any/all cores you have at your disposal."

My solution: Rainbow Tables.
The gist is that they allow you to pick a point between Pure Brute Force and Pure Precomputation of each hash.
Pure Precomputation: Calculate every of 73e9 (or 1e10) hashes and store them. Cracking is just a lookup. Takes 1.8 Tb of space for this contest, (20 bytes hash, 5 bytes PAN * 73e9), which kinda sucks.
A database would be able to handle this fairly well. I think this might actually be valid mysql code, if you implement the PAN function.
CREATE PROCEDURE insert_range()
BEGIN
  DECLARE i INT DEFAULT 0;
    WHILE i <= 1000000000 DO
        INSERT my_table (bin,hash) VALUES (PAN(i), SHA1(PAN(i));
        SET i = i + 1;
    END WHILE;
END
Once you have an index lookups could be really fast, even on HDDs.

Pure Brute Force: What everyone else here is doing. Run HashCat on a supercomputing multi-GPU machine, get bajjillions of hashes per second and steamroll these hashes. Takes no space, suckas.
The time will depends on hardware. To make it go faster, throw teraflops at it. My computer does about 2.3MH/s. So in extremely, extremely crude terms this is a bit under 9 hours.
Obviously, there are cache lookups and branch misdirects and architectures, but realistically this will take underr a day. Also, this is really good because it also scales horizontally: You can split up
the keyspace to 10 computers and have it solved 10x as fast.

Rainbow tables are dying for this reason, mainly. They have a 'brute force point', after which if the list is too large, it's faster to brute force it.
But they're good for this example, and they definitely "minimize work for a single core". My computer has an SSD in it which means they're actually competitive: The first run of my untuned single-threaded
program took ~10 hours to generate the tables and < 2 hours to crack. Theoretically only the < 2 hrs counts, so that is pretty fast.

Rainbow Tables
-----
I'm going to write a blog post about this, but basically they allow you to precompute all of the hashes, and only store a small fraction of the computation.
How? (Slightly longer) They chain the computations up and only store the first and last piece of the chain. Then for a given hash they check if it's in the chain.

I've modified rcrack1.2 to take a BIN number and generate a rainbow table for valid PANs with that BIN (with the mod10 check). It's in the /pangen folder.
I create tables with chain length 500 and 2 million chains, and make 4. This gives each PAN 99.9% accuracy, but 99.9%^73 is 96.4% (which is what I saw in my first run). So I turned it to 5.
It takes ~9Gb of space.
