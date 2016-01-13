
#include "stdafx.h"

#include "LZMA_Fun.h"


unsigned long CompressBuffer(unsigned char *inBuffer, unsigned long inSize,
							 unsigned char *outBuffer, unsigned long outSize, size_t *outSizeProcessed)
{
	if (inBuffer == NULL || outBuffer == NULL || inSize == 0)
	{
		return 0;
	}

	unsigned char* pOutBufferTemp = NULL;
	unsigned long ulOutSize = 0;
	UInt32 dict = (UInt32)-1;
	bool dictDefined = false;
	
	ulOutSize = (unsigned long)(inSize / 20 * 21 + (1 << 16)); 
	if (ulOutSize != 0)
	{
		pOutBufferTemp = (Byte *)MyAlloc((unsigned long)ulOutSize);
		if (pOutBufferTemp == 0) 
			return 0;//throw "Can not allocate memory";
	}

	if (outSizeProcessed != NULL)
	{
		*outSizeProcessed = ulOutSize;
	}

	if (!dictDefined)
	{
	   dict = 1 << 23;
	}

	int res = Lzma86_Encode(pOutBufferTemp, outSizeProcessed, inBuffer, inSize,5, dict, SZ_FILTER_AUTO);

	if (res != 0)
	{
		// Encoder error = (int)res
		return 0;
	}

	if (pOutBufferTemp)
	{
		memcpy(outBuffer, pOutBufferTemp, *outSizeProcessed);

		MyFree(pOutBufferTemp);

		pOutBufferTemp = NULL;
	}

   return *outSizeProcessed;
}

unsigned long UncompressBuffer(unsigned char *inBuffer, unsigned long inSize,
							   unsigned char *outBuffer, unsigned long outSize, size_t *outSizeProcessed)
{
	if (inBuffer == NULL || outBuffer == NULL || inSize == 0)
	{
		return 0;
	}

	unsigned char* pOutBufferTemp = NULL;
	unsigned long ulOutSize = 0;
	UInt32 dict = (UInt32)-1;
	bool dictDefined = false;

	UInt64 outSize64;

	if (Lzma86_GetUnpackSize(inBuffer, inSize, &outSize64) != 0)
	{
		return 1;//throw "data error";
	}
	ulOutSize = (unsigned long)outSize64;

	if (ulOutSize != 0)
	{
		pOutBufferTemp = (unsigned char*)MyAlloc(ulOutSize);

		 if (pOutBufferTemp == 0)
		 {
			 return 0;
		 }
	}

	*outSizeProcessed = ulOutSize;

	size_t inSize2 = inSize;

	int res = Lzma86_Decode(pOutBufferTemp, outSizeProcessed, inBuffer, &inSize2);

	if (inSize2 != (size_t)inSize)
	{
		return 0;
	}

	if (res != 0)
	{
		return 1;//throw "LzmaDecoder error";
	}

	if (pOutBufferTemp)
	{
		memcpy(outBuffer, pOutBufferTemp, *outSizeProcessed);

		MyFree(pOutBufferTemp);

		pOutBufferTemp = NULL;

	}

   return *outSizeProcessed;
}

unsigned long GetUncompressSize(unsigned char *inBuffer, unsigned long inSize,  
								UInt64* poutSize64)
{
	if (inBuffer == NULL || poutSize64 == NULL || inSize == 0)
	{
		return 0;
	}

	if (Lzma86_GetUnpackSize(inBuffer, inSize, poutSize64) != 0)
	{
		return 0;
	}

	return (unsigned long)*poutSize64;
}

unsigned long GetCompressSize(unsigned char *inBuffer, unsigned long inSize, 
							  UInt64* poutSize64)
{
	if (inBuffer == NULL || poutSize64 == NULL || inSize == 0)
	{
		return 0;
	}

	*poutSize64 = (UInt64)(inSize / 20 * 21 + (1 << 16));

	return (unsigned long)*poutSize64;
}