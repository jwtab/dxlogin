
#ifndef URL_DECODER_H_
#define URL_DECODER_H_

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

BOOL UrlDecode(const char* szSrc, char* pBuf, int cbBufLen);
BOOL UrlEncode(const char* szSrc, char* pBuf, int cbBufLen, BOOL bUpperCase);

#endif
