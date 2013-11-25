/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "ChainWalkContext.h"
#include <string>
#include <ctype.h>
#include <openssl/rand.h>
#ifdef _WIN32
	#pragma comment(lib, "libeay32.lib")
#endif

//////////////////////////////////////////////////////////////////////

string CChainWalkContext::m_sHashRoutineName;
HASHROUTINE CChainWalkContext::m_pHashRoutine;
int CChainWalkContext::m_nHashLen;
unsigned char CChainWalkContext::m_BIN[6];

unsigned char CChainWalkContext::m_PlainCharset[256];
string CChainWalkContext::m_sPlainCharsetContent;
uint64 CChainWalkContext::m_nPlainSpaceUpToX[MAX_PLAIN_LEN + 1];
uint64 CChainWalkContext::m_nPlainSpaceTotal;

int CChainWalkContext::m_nRainbowTableIndex;
uint64 CChainWalkContext::m_nReduceOffset;

//////////////////////////////////////////////////////////////////////

CChainWalkContext::CChainWalkContext() { }
CChainWalkContext::~CChainWalkContext() { }

bool CChainWalkContext::LoadCharset()
{
  m_sPlainCharsetContent = "0123456789";
  memcpy(m_PlainCharset, m_sPlainCharsetContent.c_str(), m_nPlainCharsetLen);
  return true;
}

//////////////////////////////////////////////////////////////////////

bool CChainWalkContext::SetBIN(string sBIN)
{
	if (!sBIN.empty())
	{
    memcpy(m_BIN, sBIN.c_str(), sBIN.size());
		return true;
	}
	else
		return false;
}
bool CChainWalkContext::SetHashRoutine(string sHashRoutineName)
{
	CHashRoutine hr;
	hr.GetHashRoutine(sHashRoutineName, m_pHashRoutine, m_nHashLen);
	if (m_pHashRoutine != NULL)
	{
		m_sHashRoutineName = sHashRoutineName;
		return true;
	}
	else
		return false;
}

bool CChainWalkContext::SetPlainCharset()
{
	// m_PlainCharset, m_nPlainCharsetLen, m_sPlainCharsetName, m_sPlainCharsetContent
	if (!LoadCharset())
		return false;

	// m_nPlainSpaceUpToX
	m_nPlainSpaceUpToX[0] = 0;
	uint64 nTemp = 1;
	int i;
	for (i = 1; i <= m_nPlainLenMax; i++)
	{
		nTemp *= m_nPlainCharsetLen;
		if (i < m_nPlainLenMin)
			m_nPlainSpaceUpToX[i] = 0;
		else
			m_nPlainSpaceUpToX[i] = m_nPlainSpaceUpToX[i - 1] + nTemp;
	}

	// m_nPlainSpaceTotal
	m_nPlainSpaceTotal = m_nPlainSpaceUpToX[m_nPlainLenMax];

	return true;
}

bool CChainWalkContext::SetRainbowTableIndex(int nRainbowTableIndex)
{
	if (nRainbowTableIndex < 0)
		return false;
	m_nRainbowTableIndex = nRainbowTableIndex;
	m_nReduceOffset = 65536 * nRainbowTableIndex;

	return true;
}

bool CChainWalkContext::SetupWithPathName(string sPathName, int& nRainbowChainLen, int& nRainbowChainCount)
{
	// something like lm_alpha#1-7_0_100x16_test.rt

#ifdef _WIN32
	int nIndex = sPathName.find_last_of('\\');
#else
	int nIndex = sPathName.find_last_of('/');
#endif
	if (nIndex != -1)
		sPathName = sPathName.substr(nIndex + 1);

	if (sPathName.size() < 3)
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}
	if (sPathName.substr(sPathName.size() - 3) != ".rt")
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}

	// Parse
	vector<string> vPart;
	if (!SeperateString(sPathName, "__x_.", vPart))
	{
		printf("filename %s not identified\n", sPathName.c_str());
		return false;
	}

	string sHashRoutineName   = vPart[0];
	int nRainbowTableIndex    = stoi(vPart[1].c_str());

	nRainbowChainLen          = stoi(vPart[2].c_str());
	nRainbowChainCount        = stoi(vPart[3].c_str());
	string sBINNumber = vPart[4];

	// Setup
	if (!SetHashRoutine(sHashRoutineName))
	{
		printf("hash routine %s not supported\n", sHashRoutineName.c_str());
		return false;
	}

	if (!SetBIN(sBINNumber))
		return false;
	if (!SetPlainCharset())
		return false;
	if (!SetRainbowTableIndex(nRainbowTableIndex))
	{
		printf("invalid rainbow table index %d\n", nRainbowTableIndex);
		return false;
	}

	return true;
}

string CChainWalkContext::GetHashRoutineName()
{
	return m_sHashRoutineName;
}

string CChainWalkContext::GetBIN()
{
	return std::string((char*) m_BIN);
}

int CChainWalkContext::GetHashLen()
{
	return m_nHashLen;
}

string CChainWalkContext::GetPlainCharsetContent()
{
	return m_sPlainCharsetContent;
}

int CChainWalkContext::GetPlainLenMin()
{
	return m_nPlainLenMin;
}

int CChainWalkContext::GetPlainLenMax()
{
	return m_nPlainLenMax;
}

uint64 CChainWalkContext::GetPlainSpaceTotal()
{
	return m_nPlainSpaceTotal;
}

int CChainWalkContext::GetRainbowTableIndex()
{
	return m_nRainbowTableIndex;
}

void CChainWalkContext::Dump()
{
	printf("hash routine: %s\n", m_sHashRoutineName.c_str());
	printf("hash length: %d\n", m_nHashLen);

	printf("plain charset: ");
	int i;
	for (i = 0; i < m_nPlainCharsetLen; i++)
	{
		if (isprint(m_PlainCharset[i]))
			printf("%c", m_PlainCharset[i]);
		else
			printf("?");
	}
	printf("\n");

	printf("plain charset in hex: ");
	for (i = 0; i < m_nPlainCharsetLen; i++)
		printf("%02x ", m_PlainCharset[i]);
	printf("\n");

	printf("plain length range: %d - %d\n", m_nPlainLenMin, m_nPlainLenMax);
	//printf("plain charset content: %s\n", m_sPlainCharsetContent.c_str());
	//for (i = 0; i <= m_nPlainLenMax; i++)
	//	printf("plain space up to %d: %s\n", i, uint64tostr(m_nPlainSpaceUpToX[i]).c_str());
	printf("plain space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());

	printf("rainbow table index: %d\n", m_nRainbowTableIndex);
	printf("reduce offset: %s\n", uint64tostr(m_nReduceOffset).c_str());
	printf("\n");
}

void CChainWalkContext::GenerateRandomIndex()
{
	RAND_bytes((unsigned char*)&m_nIndex, 8);
	m_nIndex = m_nIndex % m_nPlainSpaceTotal;
}

void CChainWalkContext::SetIndex(uint64 nIndex)
{
	m_nIndex = nIndex;
}

void CChainWalkContext::SetHash(unsigned char* pHash)
{
	memcpy(m_Hash, pHash, m_nHashLen);
}
void CChainWalkContext::Luhn() {
  int check = 0;
  /* Luhn Check works this way:
   *       7 9 9 2 7 3 9 8 7 1 
   * times 1 2 1 2 1 2 1 2 1 2
   * equals7 9 9 4 7 6 9 7 7 2
   * sum = 67 mod 10 = 7
   */
  const int d[] = {0,2,4,6,8,1,3,5,7,9}; // mapping for rule 3
  int val = m_Plain[0] - '0';
  m_Plain[15] = '0';
  for (int i = 0; i < 16; i++) {
  val = m_Plain[i] - '0';
    if (i % 2 == 0)
      check += d[val];
    else
      check += val;
  }
  m_Plain[15] = '0' + ((9 * check) % 10); //Convert our int to ASCII char, store it properly
  /*
  for(int i = 0; i < 16; i++) {
    printf("%c", m_Plain[i]);
  }
  printf("\n");
  */
}
void CChainWalkContext::IndexToPlain()
{
  int i;
	uint64 nIndexOfX = m_nIndex - m_nPlainSpaceUpToX[15];

  //Copies the BIN into the first 6 digits
  for(int k = 0 ; k < 6; k++) {
    m_Plain[k] = m_BIN[k];
  }

  //At the bottom, after everything else is generated, I added the Luhn check.
	// Fast copy
	for (i = 14; i >= 6; i--) // Start at -2, go until 6
	{
		if (nIndexOfX < 0x100000000llu)
			break;

		m_Plain[i] = m_PlainCharset[nIndexOfX % m_nPlainCharsetLen];
		nIndexOfX /= m_nPlainCharsetLen;
	}
	unsigned int nIndexOfX32 = (unsigned int)nIndexOfX;
	for (; i >= 6; i--)
	{
	//	m_Plain[i] = m_PlainCharset[nIndexOfX32 % m_nPlainCharsetLen];
	//	nIndexOfX32 /= m_nPlainCharsetLen;

		unsigned int nPlainCharsetLen = m_nPlainCharsetLen;
		unsigned int nTemp;
#ifdef _WIN32 || defined (__i386__) 
		__asm
		{
			mov eax, nIndexOfX32
			xor edx, edx
			div nPlainCharsetLen
			mov nIndexOfX32, eax
			mov nTemp, edx
		}

#elif defined __APPLE__ && (defined (__ppc__) || defined (__ppc64__))

                __asm__ {
			mr r2, nIndexOfX32
			xor r3, r3, r3
			divwu r3, r2, nPlainCharsetLen
                        mr nIndexOfX32, r3
			mullw r3, r3, nPlainCharsetLen 
                        subf r3, r3, r2 
                        mr nTemp, r3
                        }

#else
                __asm__ __volatile__ (  "mov %2, %%eax;"
                                                                "xor %%edx, %%edx;"
                                                                "divl %3;"
                                                                "mov %%eax, %0;"
                                                                "mov %%edx, %1;"
                                                                : "=m"(nIndexOfX32), "=m"(nTemp)
                                                                : "m"(nIndexOfX32), "m"(nPlainCharsetLen)
                                                                : "%eax", "%edx"
                                                         );

#endif

		m_Plain[i] = m_PlainCharset[nTemp];
	}
  Luhn();
}

void CChainWalkContext::PlainToHash()
{
	m_pHashRoutine(m_Plain, 16, m_Hash);
}

void CChainWalkContext::HashToIndex(int nPos)
{
	m_nIndex = (*(uint64*)m_Hash + m_nReduceOffset + nPos) % m_nPlainSpaceTotal;
}

uint64 CChainWalkContext::GetIndex()
{
	return m_nIndex;
}

string CChainWalkContext::GetPlain()
{
	string sRet;
	int i;
	for (i = 0; i < 16; i++)
	{
		char c = m_Plain[i];
		if (c >= 32 && c <= 126)
			sRet += c;
		else
			sRet += '?';
	}
	
	return sRet;
}

string CChainWalkContext::GetBinary()
{
	return HexToStr(m_Plain, 16);
}

string CChainWalkContext::GetPlainBinary()
{
	string sRet;
	sRet += GetPlain();
	int i;
	for (i = 0; i < m_nPlainLenMax - 16; i++)
		sRet += ' ';

	sRet += "|";

	sRet += GetBinary();
	for (i = 0; i < m_nPlainLenMax - 16; i++)
		sRet += "  ";

	return sRet;
}

string CChainWalkContext::GetHash()
{
	return HexToStr(m_Hash, m_nHashLen);
}

bool CChainWalkContext::CheckHash(unsigned char* pHash)
{
	if (memcmp(m_Hash, pHash, m_nHashLen) == 0)
		return true;

	return false;
}
