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
#include <openssl/rand.h> //What's wrong with a little openssl?-Amedee

//////////////////////////////////////////////////////////////////////

std::string CChainWalkContext::m_sHashRoutineName;
HASHROUTINE CChainWalkContext::m_pHashRoutine;
int CChainWalkContext::m_nHashLen;
uint8_t CChainWalkContext::RTfileFormatId;

unsigned char CChainWalkContext::m_BIN[6];
unsigned char CChainWalkContext::m_PlainCharset[256];
std::string CChainWalkContext::m_sPlainCharsetContent;

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
    memcpy(m_BIN, sBIN.c_str(), 6);
    return true;
  }
  else
    return false;
}
bool CChainWalkContext::SetHashRoutine( std::string sHashRoutineName )
{
  std::cout <<"setting hash routine to "<< sHashRoutineName<< std::endl;
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

  m_nPlainSpaceTotal = 1e9;//removed a lot of stuff here because we only have one plain length.
  printf("plain subkey space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());

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


  printf("plain charset in hex: ");

  for ( uint32_t j = 0; j < m_nPlainCharsetLen; j++ )
    printf("%02x ", m_PlainCharset[j]);
  printf("\n");

  printf("plain subkey space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());
		

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
void CChainWalkContext::IndexToPlain()
{

	uint32_t plainLen = 16; //m_nPlainSpace up to X, in our case is 15 0's, then 1 1billion. Not that useful
	uint32_t index = m_nIndex;// //eh, assume index< 1e9 because we use CCs.
  int a;

  for (a = plainLen - 2; a >= 6; a-- ) //start from -2 because len-1 will store the luhn
  {
		m_Plain[a] = m_PlainCharset[index % m_nPlainCharsetLen];
		index /= m_nPlainCharsetLen;
	}

  //Copies the BIN into the first 6 digits
  for(a = 5 ; a >= 0; a--) {
    m_Plain[a] = m_BIN[a];
  }
    
  // remarks from Sc00bz
  // Note the lack of assembly code.
  // Assembly code is not needed since all the variables are in the stack.
  // If you add in assembly code it will be slower than the compiler's code.
    
  Luhn();
}
  /*std::cout << "index " << m_nIndex << ": ";
  for(int i = 0; i < 16; i++) {
    printf("%c", m_Plain[i]);
  }
  printf("\n");
  */
void CChainWalkContext::GenerateRandomIndex()
{
  RAND_bytes((unsigned char*)&m_nIndex, 8);
  m_nIndex = m_nIndex % m_nPlainSpaceTotal;
}

void CChainWalkContext::PlainToHash()
{	
//  for(int i = 0; i < 6; i++) {
//    printf("%c", m_BIN[i]);
//  }
//  printf("   ");
//  for(int i = 0; i < 16; i++) {
//    printf("%c", m_Plain[i]);
//  }
//  printf("   ");
//  for (int i = 0; i < 20; i++)
//  {
//      if (i > 0) printf(":");
//          printf("%02X", m_Hash[i]);
//  }
//  printf("\n");
	m_pHashRoutine(m_Plain, 16, m_Hash);
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
