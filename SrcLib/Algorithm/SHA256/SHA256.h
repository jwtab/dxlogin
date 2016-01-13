/* Sha256.h -- SHA-256 Hash
2009-02-07 : Igor Pavlov : Public domain */

#ifndef __CRYPTO_SHA256_H
#define __CRYPTO_SHA256_H

#include <string.h>
#include <stdio.h>

#ifdef _WIN32

#include <stdlib.h>
#define rotlFixed(x, n) _rotl((x), (n))
#define rotrFixed(x, n) _rotr((x), (n))

#else

#define rotlFixed(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define rotrFixed(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#endif

typedef int Int32;
typedef unsigned int UInt32;
typedef __int64 Int64;
typedef unsigned __int64 UInt64;
typedef unsigned char Byte;

#define SHA256_DIGEST_SIZE 32

typedef struct
{
	UInt32 state[8];
	UInt64 count;
	Byte buffer[64];
} CSha256;

#ifdef __cplusplus
extern "C" {
#endif
	
void Sha256_Init(CSha256 *p);
void Sha256_Update(CSha256 *p, const Byte *data, size_t size);
void Sha256_Final(CSha256 *p, Byte *digest);

void SHA256_String(const Byte * string,size_t len,char szDigest[128],bool bLowerCase = false);

#ifdef __cplusplus
}
#endif

#endif
