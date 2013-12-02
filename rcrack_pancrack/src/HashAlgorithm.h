/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _HASHALGORITHM_H
#define _HASHALGORITHM_H

void HashLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);
void HashNTLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);

void HashMD2(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);
void HashMD4(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);
void HashMD5(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);

void HashSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);
void HashRIPEMD160(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);


//****************************************************************************
// MySQL Password Hashing
//****************************************************************************
void HashMySQL323(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);
void HashMySQLSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);

//****************************************************************************
// Cisco PIX Password Hashing
//****************************************************************************
void HashPIX(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);

#endif
