/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#include "HashAlgorithm.h"

#include "Public.h"

#include <openssl/des.h>
#include <openssl/md2.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#ifdef _WIN32
	#pragma comment(lib, "libeay32.lib")
#endif

void setup_des_key(unsigned char key_56[], des_key_schedule &ks)
{
	des_cblock key;

	key[0] = key_56[0];
	key[1] = (key_56[0] << 7) | (key_56[1] >> 1);
	key[2] = (key_56[1] << 6) | (key_56[2] >> 2);
	key[3] = (key_56[2] << 5) | (key_56[3] >> 3);
	key[4] = (key_56[3] << 4) | (key_56[4] >> 4);
	key[5] = (key_56[4] << 3) | (key_56[5] >> 5);
	key[6] = (key_56[5] << 2) | (key_56[6] >> 6);
	key[7] = (key_56[6] << 1);

	//des_set_odd_parity(&key);
	des_set_key(&key, ks);
}

void HashLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	/*
	unsigned char data[7] = {0};
	memcpy(data, pPlain, nPlainLen > 7 ? 7 : nPlainLen);
	*/

	int i;
	for (i = nPlainLen; i < 7; i++)
		pPlain[i] = 0;

	static unsigned char magic[] = {0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25};
	des_key_schedule ks;
	//setup_des_key(data, ks);
	setup_des_key(pPlain, ks);
	des_ecb_encrypt((des_cblock*)magic, (des_cblock*)pHash, ks, DES_ENCRYPT);
}

void HashNTLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char UnicodePlain[MAX_PLAIN_LEN * 2];
	int i;
	for (i = 0; i < nPlainLen; i++)
	{
		UnicodePlain[i * 2] = pPlain[i];
		UnicodePlain[i * 2 + 1] = 0x00;
	}

        MD4_CTX ctx;
        MD4_Init(&ctx);
        MD4_Update(&ctx, UnicodePlain, nPlainLen * 2);
        MD4_Final((unsigned char *) pHash, &ctx);

}

void HashMD2(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
        MD2_CTX ctx;
        MD2_Init(&ctx);
        MD2_Update(&ctx, pPlain, nPlainLen);
        MD2_Final((unsigned char *) pHash, &ctx);

}

void HashMD4(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
        MD4_CTX ctx;
        MD4_Init(&ctx);
        MD4_Update(&ctx, pPlain, nPlainLen);
        MD4_Final((unsigned char *) pHash, &ctx);

}

void HashMD5(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
          MD5_CTX ctx;
          MD5_Init(&ctx);
          MD5_Update(&ctx, pPlain, nPlainLen);
          MD5_Final((unsigned char *) pHash, &ctx);

}

void HashSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
                SHA_CTX ctx;
                SHA1_Init(&ctx);
                SHA1_Update(&ctx, (unsigned char *) pPlain, nPlainLen);
                SHA1_Final(pHash, &ctx);

}

void HashRIPEMD160(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
                RIPEMD160_CTX ctx;
                RIPEMD160_Init(&ctx);
                RIPEMD160_Update(&ctx, (unsigned char *) pPlain, nPlainLen);
                RIPEMD160_Final(pHash, &ctx);

}

//Required for big endian systems

long convert_long(long in)
{
  long out;
  char *p_in = (char *) &in;
  char *p_out = (char *) &out;
  p_out[0] = p_in[3];
  p_out[1] = p_in[2];
  p_out[2] = p_in[1];
  p_out[3] = p_in[0];
  return out;
}

//*********************************************************************************
// Code for MySQL password hashing
//*********************************************************************************

inline void mysql_hash_password_323(unsigned long *result, const char *password)
{
  register unsigned long nr=1345345333L, add=7, nr2=0x12345671L;
  register unsigned long tmp, ec;
  for (; *password ; password++)
  {
    if (*password == ' ' || *password == '\t') continue;
        tmp= (unsigned long) (unsigned char) *password;
        nr^= (((nr & 63)+add)*tmp)+ (nr << 8);
        nr2+=(nr2 << 8) ^ nr;
        add+=tmp;
  }
  #ifdef BIG_ENDIAN
  {
  	result[0]=convert_long(nr & (((unsigned long) 1L << 31) -1L)); /* Don't use sign bit (str2int) */;
  	result[1]=convert_long(nr2 & (((unsigned long) 1L << 31) -1L));
  }
  #else
  {
  	result[0]=nr & (((unsigned long) 1L << 31) -1L); /* Don't use sign bit (str2int) */;
  	result[1]=nr2 & (((unsigned long) 1L << 31) -1L);
  }
  #endif

  return;
}

void HashMySQL323(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
        unsigned long hash_pass[2];
        unsigned char* f = (unsigned char*) hash_pass;

        unsigned char* pass = (unsigned char*) calloc (nPlainLen+4,sizeof(unsigned char));
        memcpy(pass,pPlain,nPlainLen);

        mysql_hash_password_323(hash_pass, (char*) pass);
        pHash[0]=*(f+3); pHash[1]=*(f+2); pHash[2]=*(f+1); pHash[3]=*(f+0);
        pHash[4]=*(f+7); pHash[5]=*(f+6); pHash[6]=*(f+5); pHash[7]=*(f+4);
        free (pass);
}

void HashMySQLSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
        unsigned char hash_stage1[SHA_DIGEST_LENGTH];
        SHA_CTX ctx;

        SHA1_Init(&ctx);
        SHA1_Update(&ctx, (unsigned char *) pPlain, nPlainLen);
        SHA1_Final(hash_stage1, &ctx);
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, hash_stage1, SHA_DIGEST_LENGTH);
        SHA1_Final(pHash, &ctx);
}

//*********************************************************************************
// Code for PIX password hashing
//*********************************************************************************

static char itoa64[] =          /* 0 ... 63 => ascii - 64 */
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void _crypt_to64(char *s, unsigned long v, int n)
{
	#ifdef BIG_ENDIAN
	{
		v = convert_long(v);
        	while (--n >= 0) {
                	*s++ = itoa64[v&0x3f];
                	v >>= 6;
        	}
	}
	#else
	{	
        	while (--n >= 0) {
                	*s++ = itoa64[v&0x3f];
                	v >>= 6;
        	}
	} 
	#endif
}

void HashPIX(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
        char temp[MD5_DIGEST_LENGTH+1];
        unsigned char final[MD5_DIGEST_LENGTH];
        char* pass = (char*) calloc (nPlainLen+MD5_DIGEST_LENGTH,sizeof(char));

        memcpy (pass,pPlain,nPlainLen);

        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, (unsigned char *) pass, MD5_DIGEST_LENGTH);
        MD5_Final(final, &ctx);

        char* p = (char*) temp;
      	_crypt_to64(p,*(unsigned long*) (final+0),4); p += 4;
        _crypt_to64(p,*(unsigned long*) (final+4),4); p += 4;
        _crypt_to64(p,*(unsigned long*) (final+8),4); p += 4;
        _crypt_to64(p,*(unsigned long*) (final+12),4); p += 4;
        *p=0;

        memcpy(pHash,temp,MD5_DIGEST_LENGTH);
        free (pass);
}

