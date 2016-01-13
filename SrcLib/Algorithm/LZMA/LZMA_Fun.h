#pragma once


#include "Src/Alloc.h"
#include "Src/Lzma86.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
*函数名称 : CompressBuffer
*参数1: 传入需要压缩的buffer 
*参数2: 需要压缩的buffer大小
*参数3: 压缩后传出的buffer,预给定buffer
*参数4: 压缩后传出buffer的大小,预给定大小
*参数5: 实际压缩后buffer大小,如果参数3的长度大于了实际压缩的大小
*返回值: 不为零视为成功,返回参数5
*/
unsigned long CompressBuffer(unsigned char *inBuffer, unsigned long inSize,
							 unsigned char *outBuffer, unsigned long outSize, size_t *outSizeProcessed);    

/*
*函数名称 : UncompressBuffer
*参数1: 传入需要解压缩的buffer 
*参数2: 需要解压缩的buffer大小
*参数3: 解压缩后传出的buffer,预给定buffer
*参数4: 解压缩后传出buffer的大小,预给定大小
*参数5: 实际解压缩后buffer大小,如果参数3的长度大于了实际解压缩的大小
*返回值: 不为零视为成功,返回参数5
*/
unsigned long UncompressBuffer(unsigned char *inBuffer, unsigned long inSize,
							   unsigned char *outBuffer, unsigned long outSize, size_t *outSizeProcessed);  

/*
*函数名称 : GetUncompressSize
*参数1: 传入需要解压缩的buffer 
*参数2: 需要解压缩的buffer大小
*参数3: 传出解压后需要的buffer大小
*返回值: 直接返回参数3
*/
unsigned long GetUncompressSize(unsigned char *inBuffer, unsigned long inSize,  
								UInt64* poutSize64); 

/*
*函数名称 : GetCompressSize
*参数1: 传入需要压缩的buffer 
*参数2: 需要压缩的buffer大小
*参数3: 传出压缩后需要的buffer大小,给出提前new的大小
*返回值: 直接返回参数3
*/
unsigned long GetCompressSize(unsigned char *inBuffer, unsigned long inSize, 
							  UInt64* poutSize64);  


#ifdef __cplusplus
}
#endif