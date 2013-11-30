/*
 * freerainbowtables is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010 Yngve AAdlandsvik
 * Copyright 2008, 2009, 2010, 2011 Steve Thomas (Sc00bz)
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ChainWalkContext.h"

#include <ctype.h>

//////////////////////////////////////////////////////////////////////

std::string CChainWalkContext::m_sHashRoutineName;
HASHROUTINE CChainWalkContext::m_pHashRoutine;
int CChainWalkContext::m_nHashLen;
uint8_t CChainWalkContext::RTfileFormatId;

unsigned char CChainWalkContext::m_BIN[6];
unsigned char CChainWalkContext::m_PlainCharset[256];
std::string CChainWalkContext::m_sPlainCharsetContent;

uint64_t CChainWalkContext::m_nPlainSpaceUpToX[MAX_PLAIN_LEN];
uint64_t CChainWalkContext::m_nPlainSpaceTotal;
unsigned char CChainWalkContext::m_Salt[MAX_SALT_LEN];

int CChainWalkContext::m_nSaltLen = 0;
int CChainWalkContext::m_nRainbowTableIndex;
uint64_t CChainWalkContext::m_nReduceOffset;
KeySpace *CChainWalkContext::m_ks = NULL;

//////////////////////////////////////////////////////////////////////

CChainWalkContext::CChainWalkContext()
{
}

CChainWalkContext::~CChainWalkContext()
{
}

bool CChainWalkContext::LoadCharset()
{
  m_sPlainCharsetContent = std::string("0123456789");
  memcpy(m_PlainCharset, m_sPlainCharsetContent.c_str(), 10);
  return true;
}

//////////////////////////////////////////////////////////////////////

bool CChainWalkContext::SetBIN(std::string sBIN)
{
  if (!sBIN.empty())
  {
    memcpy(m_BIN, sBIN.c_str(), sBIN.size());
    return true;
  }
  else
    return false;
}
bool CChainWalkContext::SetHashRoutine( std::string sHashRoutineName )
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
  if(!LoadCharset())
    return false;

	m_nPlainSpaceUpToX[0] = 0;
  uint64_t nTemp = 1;
  for (int i = 1; i <= m_nPlainLenMax; i++)
  {			
    nTemp *= m_nPlainCharsetLen;

    if (i < m_nPlainLenMin)
    {
      m_nPlainSpaceUpToX[i] = 0;
    }
    else
    {
      m_nPlainSpaceUpToX[i] = m_nPlainSpaceUpToX[i - 1] + nTemp;
    }
  }
  m_nPlainSpaceTotal = 1e9;//m_nPlainSpaceUpToX[m_nPlainLenMax];


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

bool CChainWalkContext::SetSalt(unsigned char *Salt, int nSaltLength)
{
	memcpy(&m_Salt[0], Salt, nSaltLength);
	
	m_nSaltLen = nSaltLength;
//	m_sSalt = sSalt;
	return true;
}

bool CChainWalkContext::SetupWithPathName( std::string sPathName, int& nRainbowChainLen, int& nRainbowChainCount)
{
#ifdef _WIN32
	std::string::size_type nIndex = sPathName.find_last_of('\\');
#else
	std::string::size_type nIndex = sPathName.find_last_of('/');
#endif
	if (nIndex != std::string::npos)
		sPathName = sPathName.substr(nIndex + 1);

	if (sPathName.size() < 3)
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}
	if (sPathName.substr(sPathName.size() - 5) == ".rti2")
	{
		RTfileFormatId = getRTfileFormatId( "RTI2" );
	}
	else if (sPathName.substr(sPathName.size() - 4) == ".rti")
	{
		RTfileFormatId = getRTfileFormatId( "RTI" );
	}
	else if (sPathName.substr(sPathName.size() - 3) == ".rt")
	{
		RTfileFormatId = getRTfileFormatId( "RT" );
	}
	else
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}

	// Parse
	std::vector<std::string> vPart;
	if (!SeperateString(sPathName, "__x_.", vPart))
	{
		printf("filename %s not identified\n", sPathName.c_str());
		return false;
	}

	std::string sHashRoutineName   = vPart[0];
	int nRainbowTableIndex    = atoi(vPart[1].c_str());
	nRainbowChainLen          = atoi(vPart[2].c_str());
	nRainbowChainCount        = atoi(vPart[3].c_str());
	std::string sBIN = vPart[4];
	
	// Setup
	if (!SetHashRoutine(sHashRoutineName))
	{
		printf("hash routine %s not supported\n", sHashRoutineName.c_str());
		return false;
	}
	if (!SetBIN(sBIN))
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

std::string CChainWalkContext::GetHashRoutineName()
{
	return m_sHashRoutineName;
}

std::string CChainWalkContext::GetBIN()
{
  return std::string((char *) m_BIN);
}

int CChainWalkContext::GetHashLen()
{
	return m_nHashLen;
}

std::string CChainWalkContext::GetPlainCharsetContent()
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

uint64_t CChainWalkContext::GetPlainSpaceTotal()
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

  printf( "m_nPlainCharSetLen: %d\n", m_nPlainCharsetLen );

  printf("plain charset: ");

  for ( uint32_t j = 0; j < m_nPlainCharsetLen; j++ )
  {
    if (isprint(m_PlainCharset[j]))
      printf("%c", m_PlainCharset[j]);
    else
      printf("?");
  }
  printf("\n");

  for ( int j = 0; j <= m_nPlainLenMax; j++ )
  {
    printf( "m_vCharset.m_nPlainSpaceUpToX[%d]: %"PRIu64"\n" , j, m_nPlainSpaceUpToX[j] );
  }

  printf("plain charset in hex: ");

  for ( uint32_t j = 0; j < m_nPlainCharsetLen; j++ )
    printf("%02x ", m_PlainCharset[j]);
  printf("\n");

  printf("plain subkey space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());
		
	for ( int i = 0; i <= m_nPlainLenMax; i++ )
	{
		printf( "m_nPlainSpaceUpToX[%d]: %"PRIu64"\n" , i, m_nPlainSpaceUpToX[i] );
	}

	//printf("plain charset content: %s\n", m_sPlainCharsetContent.c_str());
	//for (i = 0; i <= m_nPlainLenMax; i++)
	//	printf("plain space up to %d: %s\n", i, uint64tostr(m_nPlainSpaceUpToX[i]).c_str());
	printf("plain space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());

	printf("rainbow table index: %d\n", m_nRainbowTableIndex);
	printf("reduce offset: %s\n", uint64tostr(m_nReduceOffset).c_str());
	printf("\n");
}

void CChainWalkContext::SetIndex(uint64_t nIndex)
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
  const int d[] = {0,2,4,6,8,1,3,5,7,9}; // mapping for multiplying by 2
  int val;
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
int CChainWalkContext::normalIndexToPlain(uint64_t index, uint64_t *plainSpaceUpToX, unsigned char *charSet, int charSetLen, int min, int max, unsigned char *plain)
{

	uint32_t plainLen = 16;
  uint64_t nIndexOfX = m_nIndex - m_nPlainSpaceUpToX[15];
  int a = plainLen - 1;
	index -= plainSpaceUpToX[a]; // plainLen - 1 == a

  //Copies the BIN into the first 6 digits
  for(int k = 0 ; k < 6; k++) {
    m_Plain[k] = m_BIN[k];
  }
  for (int a = plainLen - 2; a >= 6; a-- ) //start from -2 because len-1 will store the luhn
  {
    // XXX this is optimized for 32-bit platforms
#if defined(_WIN32) && !defined(__GNUC__)
    if (index < 0x100000000I64)
      break;
#else
		if (index < 0x100000000llu)
			break;
#endif
		plain[a] = charSet[index % charSetLen];
		index /= charSetLen;
	}

	unsigned int index32 = (unsigned int) index;
	for ( ; a >= 6; a-- )
	{
		// remarks from Sc00bz
		// Note the lack of assembly code.
		// Assembly code is not needed since all the variables are in the stack.
		// If you add in assembly code it will be slower than the compiler's code.

		plain[a] = charSet[index32 % charSetLen];
		index32 /= charSetLen;
	}
  Luhn();
	return plainLen;
}

void CChainWalkContext::IndexToPlain()
{
		m_nPlainLen = 0;
		uint64_t indexTmp = m_nIndex;

/*
		for ( uint32_t a = 0; a < numKeySpaces - 1; a++ )
		{
			m_vCharset[a].m_nIndexX = indexTmp % m_vCharset[a].m_nPlainSpaceTotal;
			indexTmp /= m_vCharset[a].m_nPlainSpaceTotal;
			m_nPlainLen += normalIndexToPlain(m_vCharset[a].m_nIndexX, m_vCharset[a].m_nPlainSpaceUpToX, m_vCharset[a].m_PlainCharset, m_vCharset[a].m_nPlainCharsetLen, m_vCharset[a].m_nPlainLenMin, m_vCharset[a].m_nPlainLenMax, m_Plain + m_nPlainLen);
		}
    */
		m_nIndexX = indexTmp;
		m_nPlainLen += normalIndexToPlain(m_nIndexX, m_nPlainSpaceUpToX, m_PlainCharset, m_nPlainCharsetLen, m_nPlainLenMin, m_nPlainLenMax, m_Plain + m_nPlainLen);
}


/*
void CChainWalkContext::IndexToPlain()
{
	int i;
	m_nPlainLen = 0;
	for (i = m_nPlainLenMaxTotal - 1; i >= m_nPlainLenMinTotal - 1; i--)
	{
		if (m_nIndex >= m_nPlainSpaceUpToX[i])
		{
			m_nPlainLen = i + 1;
			break;
		}
	}
	if(m_nPlainLen == 0)
		m_nPlainLen = m_nPlainLenMinTotal;
	uint64_t nIndexOfX = m_nIndex - m_nPlainSpaceUpToX[m_nPlainLen - 1];

// this is the generic code for non x86/x86_64 platforms
#if !defined(_M_X64) && !defined(_M_IX86) && !defined(__i386__) && !defined(__x86_64__)
	
	// generic version (slow for non 64-bit platforms and gcc < 4.5.x)
	for (i = m_nPlainLen - 1; i >= 0; i--)
	{
		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{
				m_Plain[i] = m_PlainCharset[nIndexOfX % m_nPlainCharsetLen];
				nIndexOfX /= m_nPlainCharsetLen;
				break;
			}
		}
	}

#elif defined(_M_X64) || defined(_M_IX86) || defined(__i386__) || defined(__x86_64__)

	// Fast ia32 version
	for (i = m_nPlainLen - 1; i >= 0; i--)
	{
		// 0x100000000 = 2^32
#ifdef _M_IX86
		if (nIndexOfX < 0x100000000I64)
			break;
#else
		if (nIndexOfX < 0x100000000llu)
			break;
#endif

		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{
				m_Plain[i] = m_PlainCharset[nIndexOfX % m_nPlainCharsetLen];
				nIndexOfX /= m_nPlainCharsetLen;
				break;
			}
		}
	}

	uint32_t nIndexOfX32 = (uint32_t)nIndexOfX;
	for (; i >= 0; i--)
	{
		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{

				m_Plain[i] = m_PlainCharset[nIndexOfX % m_nPlainCharsetLen];
				nIndexOfX /= m_nPlainCharsetLen;
				break;
			}
		}
	}

	uint32_t nIndexOfX32 = (uint32_t)nIndexOfX;
	for (; i >= 0; i--)
	{
		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{

//		m_Plain[i] = m_PlainCharset[nIndexOfX32 % m_nPlainCharsetLen];
//		nIndexOfX32 /= m_nPlainCharsetLen;

//	moving nPlainCharsetLen into the asm body and avoiding the extra temp
//	variable results in a performance gain
//				unsigned int nPlainCharsetLen = m_nPlainCharsetLen;
				unsigned int nTemp;

#if defined(_WIN32) && !defined(__GNUC__)
		// VC++ still needs this
		unsigned int nPlainCharsetLen = m_nPlainCharsetLen;

		__asm
		{
			mov eax, nIndexOfX32
			xor edx, edx
			div nPlainCharsetLen
			mov nIndexOfX32, eax
			mov nTemp, edx
		}
		m_Plain[i] = m_PlainCharset[nTemp];
#else
		__asm__ __volatile__ ("xor %%edx, %%edx;"
								"divl %3;"
								: "=a"(nIndexOfX32), "=d"(nTemp)
								: "a"(nIndexOfX32), "rm"(m_nPlainCharsetLen)
								: );
		m_Plain[i] = m_PlainCharset[nTemp];
#endif
		break;
			}
		}
	}
#endif
}
*/

void CChainWalkContext::PlainToHash()
{	
	m_pHashRoutine(m_Plain, m_nPlainLen, m_Hash);
}

void CChainWalkContext::HashToIndex(int nPos)
{
	// breaks strict aliasing
	//m_nIndex = (*(uint64_t*)m_Hash + m_nReduceOffset + nPos) % m_nPlainSpaceTotal;

	//printf("plain space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());
	memcpy( m_Hash_alias.m_Hash, m_Hash, 8 );

	m_nIndex = (m_Hash_alias.alias + m_nReduceOffset + nPos) % m_nPlainSpaceTotal;
}

uint64_t CChainWalkContext::GetIndex()
{
	return m_nIndex;
}
const uint64_t *CChainWalkContext::GetIndexPtr()
{
	return &m_nIndex;
}

std::string CChainWalkContext::GetPlain()
{
	std::string sRet;
	int i;
	for (i = 0; i < m_nPlainLen; i++)
	{
		char c = m_Plain[i];
		if (c >= 32 && c <= 126)
			sRet += c;
		else
			sRet += '?';
	}
	
	return sRet;
}

std::string CChainWalkContext::GetBinary()
{
	return HexToStr(m_Plain, m_nPlainLen);
}

std::string CChainWalkContext::GetHash()
{
	return HexToStr(m_Hash, m_nHashLen);
}

bool CChainWalkContext::CheckHash(unsigned char* pHash)
{
	if (memcmp(m_Hash, pHash, m_nHashLen) == 0)
		return true;

	return false;
}

uint8_t CChainWalkContext::getRTfileFormat()
{
	return RTfileFormatId;
}
