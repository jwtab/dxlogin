/* 7zInBuf.h -- Buf IO
2015-01-29 : flp */

#ifndef __7Z_INBUF_H
#define __7Z_INBUF_H

#include "Types.h"

EXTERN_C_BEGIN

/* ---------- InBuf ---------- */

typedef struct
{
	const void* buf;
	size_t size;
	size_t pos;
}CInBuf;

WRes InBuf_Init(CInBuf *p, const void* pBuf, size_t nBufLength);
WRes InBuf_Read(CInBuf *p, void *data, size_t *size);
WRes InBuf_Seek(CInBuf *p, Int64 *pos, ESzSeek origin);

/* ---------- InBuf ---------- */

typedef struct
{
	ISeqInStream s;
	CInBuf buf;
} CBufSeqInStream;

void BufSeqInStream_CreateVTable(CBufSeqInStream *p);


typedef struct
{
	ISeekInStream s;
	CInBuf buf;
} CBufInStream;

void BufInStream_CreateVTable(CBufInStream *p);

EXTERN_C_END

#endif
