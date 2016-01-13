
#ifndef AR_SHA1_H_
#define AR_SHA1_H_

#include <stdio.h> 
#include <memory.h>
#include <string.h>

#define  SHA1_DIGEST_SIZE 20

#ifdef __cplusplus
extern "C" {
#endif

int SHA1_String(const unsigned char* inputString, unsigned long len, char szSha1Digest[80],bool bLowerCase = false);

#ifdef __cplusplus
}
#endif

#endif