
all: pangen rtdump rtsort rcrack pangen

pangen:
	g++ Public.cpp ChainWalkContext.cpp HashAlgorithm.cpp HashRoutine.cpp RainbowTableGenerate.cpp -lssl -lcrypto -O3 -arch i386 -fstrict-aliasing -funroll-loops -ffast-math -mdynamic-no-pic -mtune=pentium4 -msse2 -fasm-blocks -o pangen

rtdump:
	g++ Public.cpp ChainWalkContext.cpp HashAlgorithm.cpp HashRoutine.cpp RainbowTableDump.cpp -lssl -lcrypto -O3 -arch i386 -fstrict-aliasing -funroll-loops -ffast-math -mdynamic-no-pic -mtune=pentium4 -msse2 -fasm-blocks -o rtdump

rtsort:
	g++ -g Public.cpp RainbowTableSort.cpp -arch i386 -fstrict-aliasing -funroll-loops -ffast-math -mdynamic-no-pic -mtune=pentium4 -msse2 -o rtsort

rcrack:
	g++ Public.cpp ChainWalkContext.cpp HashAlgorithm.cpp HashRoutine.cpp HashSet.cpp MemoryPool.cpp ChainWalkSet.cpp CrackEngine.cpp RainbowCrack.cpp -lssl -lcrypto -O3 -arch i386 -fstrict-aliasing -funroll-loops -ffast-math -mdynamic-no-pic -mtune=pentium4 -msse2 -fasm-blocks -o rcrack

clean: 
	rm -f pangen 
	rm -f rtdump 
	rm -f rtsort 
	rm -f rcrack 

