/* 7zInBuf.h -- Buf IO
2015-01-29 : flp */

#include <stdio.h>
#include "7zInBuf.h"

void Buf_Construct(CInBuf *p)
{
	p->buf=NULL;
	p->pos=0;
	p->size=0;
}

WRes InBuf_Init(CInBuf *p, const void* pBuf, size_t nBufLength)
{
	if(p==NULL || pBuf==NULL || nBufLength==0)
		return SZ_ERROR_PARAM;

	Buf_Construct(p);

	p->buf=pBuf;
	p->size=nBufLength;

	return SZ_OK;
}

WRes InBuf_Read(CInBuf *p, void *data, size_t *size)
{
	if(p->pos>p->size)
		return SZ_ERROR_READ;
	else if(p->pos==p->size)
	{
		*size=0;
		return SZ_OK;
	}

	memmove(data,(unsigned char*)p->buf+p->pos,min(*size,p->size-p->pos));
	p->pos+=min(*size,p->size-p->pos);
	
	return SZ_OK;
}

WRes InBuf_Seek(CInBuf *p, Int64 *pos, ESzSeek origin)
{
	if(origin==SZ_SEEK_SET)
	{
		if(*pos>p->size)
			p->pos=p->size;
		else
			p->pos=(size_t)*pos;
	}
	else if(origin==SZ_SEEK_CUR)
	{
		if(p->pos+*pos>p->size)
			p->pos=p->size;
		else
			p->pos+=(size_t)*pos;
	}
	else if(origin==SZ_SEEK_END)
	{
		if(*pos>0)
			p->pos=p->size;
		else if(*pos+p->size<0)
			p->pos=0;
		else
			p->pos=p->size-(size_t)*pos;
	}

	*pos=p->pos;

	return SZ_OK;
}

/* ---------- BufSeqInStream ---------- */

static SRes BufSeqInStream_Read(void *pp, void *buf, size_t *size)
{
  CBufSeqInStream *p = (CBufSeqInStream *)pp;
  return InBuf_Read(&p->buf, buf, size) == 0 ? SZ_OK : SZ_ERROR_READ;
}

void BufSeqInStream_CreateVTable(CBufSeqInStream *p)
{
  p->s.Read=BufSeqInStream_Read;
}

/* ---------- BufInStream ---------- */

static SRes BufInStream_Read(void *pp, void *buf, size_t *size)
{
  CBufInStream *p = (CBufInStream *)pp;
  return InBuf_Read(&p->buf, buf, size) == 0 ? SZ_OK : SZ_ERROR_READ;
}

static SRes BufInStream_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
  CBufInStream *p = (CBufInStream *)pp;
  return InBuf_Seek(&p->buf, pos, origin);
}

void BufInStream_CreateVTable(CBufInStream *p)
{
  p->s.Read = BufInStream_Read;
  p->s.Seek = BufInStream_Seek;
}
