
#ifndef BASE_64_H_
#define BASE_64_H_

#include <string.h>

int encode_base64(const char* aIn, size_t aInLen, char* aOut,
						 size_t aOutSize, size_t* aOutLen);

int decode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen);

#endif