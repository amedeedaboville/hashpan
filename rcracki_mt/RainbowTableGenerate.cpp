/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <time.h>

#include "ChainWalkContext.h"

void Usage()
{
	Logo();

	printf("usage: pangen hash_algorithm \\\n");
	printf("             bin \\\n");
	printf("             rainbow_table_index \\\n");
	printf("             rainbow_chain_length rainbow_chain_count \\\n");
	printf("             file_title_suffix\n");
	printf("       pangen hash_algorithm \\\n");
	printf("             bin \\\n");
	printf("             rainbow_table_index \\\n");
	printf("             -bench\n");
	printf("\n");

	CHashRoutine hr;
	printf("hash_algorithm:       available: %s\n", hr.GetAllHashRoutineName().c_str());
	printf("bin:                  the 6 digit Bank Identification Number\n");
	printf("                      use \"byte\" to specify all 256 characters as the charset of the plaintext\n");
	printf("rainbow_table_index:  index of the rainbow table\n");
	printf("rainbow_chain_length: length of the rainbow chain\n");
	printf("rainbow_chain_count:  count of the rainbow chain to generate\n");
	printf("file_title_suffix:    the string appended to the file title\n");
	printf("                      add your comment of the generated rainbow table here\n");
	printf("-bench:               do some benchmark\n");

	printf("\n");
	printf("example: rpangen sha1 530957 1 7 0 100 16 test\n");
	printf("         rpangen md5  123456 4 4 0 100 16 test\n");
	printf("         rpangen sha2 430513 1 -bench\n");
}

void Bench(string sHashRoutineName, string sBINNumber, int nRainbowTableIndex)
{
	// Setup CChainWalkContext
	if (!CChainWalkContext::SetHashRoutine(sHashRoutineName))
	{
		printf("hash routine %s not supported\n", sHashRoutineName.c_str());
		return;
	}
	if (!CChainWalkContext::SetPlainCharset())
		return;
	if (!CChainWalkContext::SetBIN(sBINNumber))
		return;
	if (!CChainWalkContext::SetRainbowTableIndex(nRainbowTableIndex))
	{
		printf("invalid rainbow table index %d\n", nRainbowTableIndex);
		return;
	}

	// Bench hash
	{
	CChainWalkContext cwc;
	cwc.GenerateRandomIndex();
	cwc.IndexToPlain();

	clock_t t1 = clock();
	int nLoop = 2500000;
	int i;
	for (i = 0; i < nLoop; i++)
		cwc.PlainToHash();
	clock_t t2 = clock();
	float fTime = 1.0f * (t2 - t1) / CLOCKS_PER_SEC;

	printf("%s hash speed: %d / s\n", sHashRoutineName.c_str(), int(nLoop / fTime));
	}

	// Bench step
	{
	CChainWalkContext cwc;
	cwc.GenerateRandomIndex();

	clock_t t1 = clock();
	int nLoop = 2500000;
	int i;
	for (i = 0; i < nLoop; i++)
	{
		cwc.IndexToPlain();
		cwc.PlainToHash();
		cwc.HashToIndex(i);
	}
	clock_t t2 = clock();
	float fTime = 1.0f * (t2 - t1) / CLOCKS_PER_SEC;

	printf("%s step speed: %d / s\n", sHashRoutineName.c_str(), int(nLoop / fTime));
	}
}

int main(int argc, char* argv[])
{
	if (argc == 5)
	{
		if (strcmp(argv[5], "-bench") == 0)
		{
			Bench(argv[1], argv[2], stoi(argv[3]));
			return 0;
		}
	}

	if (argc != 7)
	{
		Usage();
		return 0;
	}

	string sHashRoutineName  = argv[1];
	string sBINNumber        = argv[2];
	int nRainbowTableIndex   = stoi(argv[3]);

	int nRainbowChainLen     = stoi(argv[4]);
	int nRainbowChainCount   = stoi(argv[5]);
	string sFileTitleSuffix  = argv[6];

	// nRainbowChainCount check
	if (nRainbowChainCount >= 134217728)
	{
		printf("this will generate a table larger than 2GB, which is not supported\n");
		printf("please use a smaller rainbow_chain_count(less than 134217728)\n");
		return 0;
	}

	// Setup CChainWalkContext
	if (!CChainWalkContext::SetHashRoutine(sHashRoutineName))
	{
		printf("hash routine %s not supported\n", sHashRoutineName.c_str());
		return 0;
	}
	if (!CChainWalkContext::SetBIN(sBINNumber))
		return false;
	if (!CChainWalkContext::SetPlainCharset())
		return 0;
	if (!CChainWalkContext::SetRainbowTableIndex(nRainbowTableIndex))
	{
		printf("invalid rainbow table index %d\n", nRainbowTableIndex);
		return 0;
	}
	CChainWalkContext::Dump();

	// Low priority
#ifdef _WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
#else
	nice(19);
#endif

	// FileName
	char szFileName[256];
	sprintf(szFileName, "%s_%d_%dx%d_%s.rt", sHashRoutineName.c_str(),
													  nRainbowTableIndex,
													  nRainbowChainLen,
													  nRainbowChainCount,
													  sFileTitleSuffix.c_str());

	// Open file
	fclose(fopen(szFileName, "a"));
	FILE* file = fopen(szFileName, "r+b");
	if (file == NULL)
	{
		printf("failed to create %s\n", szFileName);
		return 0;
	}

	// Check existing chains
	unsigned int nDataLen = GetFileLen(file);
	nDataLen = nDataLen / 16 * 16;
	if (nDataLen == nRainbowChainCount * 16)
	{
		printf("precomputation of this rainbow table already finished\n");
		fclose(file);
		return 0;
	}
	if (nDataLen > 0)
		printf("continuing from interrupted precomputation...\n");
	fseek(file, nDataLen, SEEK_SET);

	// Generate rainbow table
	printf("generating...\n");
	CChainWalkContext cwc;
	clock_t t1 = clock();
	int i;
	for (i = nDataLen / 16; i < nRainbowChainCount; i++)
	{
		cwc.GenerateRandomIndex();
		uint64 nIndex = cwc.GetIndex();
		if (fwrite(&nIndex, 1, 8, file) != 8)
		{
			printf("disk write fail\n");
			break;
		}

		int nPos;
		for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
		{
			cwc.IndexToPlain();
			cwc.PlainToHash();
			cwc.HashToIndex(nPos);
		}

		nIndex = cwc.GetIndex();
		if (fwrite(&nIndex, 1, 8, file) != 8)
		{
			printf("disk write fail\n");
			break;
		}

		if ((i + 1) % 100000 == 0 || i + 1 == nRainbowChainCount)
		{
			clock_t t2 = clock();
			int nSecond = (t2 - t1) / CLOCKS_PER_SEC;
			printf("%d of %d rainbow chains generated (%d m %d s)\n", i + 1,
																	  nRainbowChainCount,
																	  nSecond / 60,
																	  nSecond % 60);
			t1 = clock();
		}
	}

	// Close
	fclose(file);

	return 0;
}
