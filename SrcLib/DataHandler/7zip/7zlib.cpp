/************************************************************************/

/************************************************************************/  

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif


extern "C"{
#include "7z.h"
#include "7zAlloc.h"
#include "7zCrc.h"
#include "7zFile.h"
#include "7zInBuf.h"
#include "CpuArch.h"
#include "7zlib.h"
};



namespace _7zlib
{
	static WRes MyCreateDir(const WCHAR *name)
	{
		return CreateDirectoryW(name, NULL) ? 0 : GetLastError();
	}

#ifdef UNDER_CE
#define kBufferSize (1 << 13)
#else
#define kBufferSize (1 << 15)
#endif

#define kSignatureSearchLimit (1 << 22)

	static Bool FindSignature(CSzFile *stream, UInt64 *resPos)
	{
		Byte buf[kBufferSize];
		size_t numPrevBytes = 0;
		*resPos = 0;
		for (;;)
		{
			size_t numTests, pos;
			if (*resPos > kSignatureSearchLimit)
				return False;

			do
			{
				size_t processed = kBufferSize - numPrevBytes;
				if (File_Read(stream, buf + numPrevBytes, &processed) != 0)
					return False;
				if (processed == 0)
					return False;
				numPrevBytes += processed;
			}
			while (numPrevBytes <= k7zStartHeaderSize);

			numTests = numPrevBytes - k7zStartHeaderSize;
			for (pos = 0; pos < numTests; pos++)
			{
				for (; buf[pos] != '7' && pos < numTests; pos++);
				if (pos == numTests)
					break;
				if (memcmp(buf + pos, k7zSignature, k7zSignatureSize) == 0)
					if (CrcCalc(buf + pos + 12, 20) == GetUi32(buf + pos + 8))
					{
						*resPos += pos;
						return True;
					}
			}
			*resPos += numTests;
			numPrevBytes -= numTests;
			memmove(buf, buf + numTests, numPrevBytes);
		}
	}

	static Bool FindSignature(CInBuf *stream, UInt64 *resPos)
	{
		Byte buf[kBufferSize];
		size_t numPrevBytes = 0;
		*resPos = 0;
		for (;;)
		{
			size_t numTests, pos;
			if (*resPos > kSignatureSearchLimit)
				return False;

			do
			{
				size_t processed = kBufferSize - numPrevBytes;
				if (InBuf_Read(stream, buf + numPrevBytes, &processed) != 0)
					return False;
				if (processed == 0)
					return False;
				numPrevBytes += processed;
			}
			while (numPrevBytes <= k7zStartHeaderSize);

			numTests = numPrevBytes - k7zStartHeaderSize;
			for (pos = 0; pos < numTests; pos++)
			{
				for (; buf[pos] != '7' && pos < numTests; pos++);
				if (pos == numTests)
					break;
				if (memcmp(buf + pos, k7zSignature, k7zSignatureSize) == 0)
					if (CrcCalc(buf + pos + 12, 20) == GetUi32(buf + pos + 8))
					{
						*resPos += pos;
						return True;
					}
			}
			*resPos += numTests;
			numPrevBytes -= numTests;
			memmove(buf, buf + numTests, numPrevBytes);
		}
	}

// 	static Bool DoesFileOrDirExist(const WCHAR *path)
// 	{
// 		WIN32_FIND_DATAW fd;
// 		HANDLE handle;
// 		handle = FindFirstFileW(path, &fd);
// 		if (handle == INVALID_HANDLE_VALUE)
// 			return False;
// 		FindClose(handle);
// 		return True;
// 	}

	static Bool DoesFileOrDirExist(const WCHAR *path)
	{
		if (path == NULL)
		{
			return False;
		}

		DWORD dwAttr = ::GetFileAttributes(path);
		if (INVALID_FILE_ATTRIBUTES == dwAttr)
		{
			return False;
		}

		return True;
	}

	int SzExtract(const wchar_t* cSzPackFile, const wchar_t* cUnPackPath)
	{
		SRes res = SZ_OK;
		CFileInStream archiveStream;
		CLookToRead lookStream;
		CSzArEx db;
		ISzAlloc allocImp;
		ISzAlloc allocTempImp;
		const char *errorMessage = NULL;

		wchar_t path[MAX_PATH * 3 + 2] = {0};
		size_t pathLen = wcslen(cUnPackPath);
		wcscpy_s(path,MAX_PATH * 3 + 2, cUnPackPath);

		if (cUnPackPath[pathLen - 1] != '\\') 
		{ 
			wcscat_s(path, L"\\");
			pathLen = wcslen(path);
		}


		CrcGenerateTable();

		allocImp.Alloc = SzAlloc;
		allocImp.Free = SzFree;

		allocTempImp.Alloc = SzAllocTemp;
		allocTempImp.Free = SzFreeTemp;

		FileInStream_CreateVTable(&archiveStream);
		LookToRead_CreateVTable(&lookStream, False);

		
		if (InFile_OpenW(&archiveStream.file, cSzPackFile) != 0)
		{
			errorMessage = "can not open input file";
			res = SZ_ERROR_FAIL;
		}
		else
		{
			UInt64 pos = 0;
			if (!FindSignature(&archiveStream.file, &pos))
				res = SZ_ERROR_FAIL;
			else if (File_Seek(&archiveStream.file, (Int64 *)&pos, SZ_SEEK_SET) != 0)
				res = SZ_ERROR_FAIL;
			if (res != 0)
				errorMessage = "Can't find 7z archive";
		}

		if (res == SZ_OK)
		{
			lookStream.realStream = &archiveStream.s;
			LookToRead_Init(&lookStream);
		}

		SzArEx_Init(&db);
		if (res == SZ_OK)
		{
			res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
		}
		if (res == SZ_OK)
		{
			UInt32 executeFileIndex = (UInt32)(Int32)-1;
			UInt32 minPrice = 1 << 30;
			UInt32 i;
			UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
			Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
			size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

			for (i = 0; i < db.db.NumFiles; i++)
			{
				size_t offset = 0;
				size_t outSizeProcessed = 0;
				const CSzFileItem *f = db.db.Files + i;
				size_t len;
				wchar_t *temp;
				len = SzArEx_GetFileNameUtf16(&db, i, NULL);

				if (len >= MAX_PATH)
				{
					res = SZ_ERROR_FAIL;
					break;
				}

				temp = path + pathLen;

				SzArEx_GetFileNameUtf16(&db, i, reinterpret_cast<UInt16*>(temp));
				{
					res = SzArEx_Extract(&db, &lookStream.s, i,
						&blockIndex, &outBuffer, &outBufferSize,
						&offset, &outSizeProcessed,
						&allocImp, &allocTempImp);
					if (res != SZ_OK)
						break;
				}
				{
					CSzFile outFile;
					size_t processedSize;
					size_t j;
					size_t nameStartPos = 0;
					for (j = 0; temp[j] != 0; j++)
					{
						if (temp[j] == '/')
						{
							temp[j] = 0;
							MyCreateDir(path);
							temp[j] = CHAR_PATH_SEPARATOR;
							nameStartPos = j + 1;
						}
					}

					if (f->IsDir)
					{
						MyCreateDir(path);
						continue;
					}
					else
					{

						if (DoesFileOrDirExist(path))
						{
							errorMessage = "Duplicate file";
							res = SZ_ERROR_FAIL;
							break;
						}
						if (OutFile_OpenW(&outFile, path))
						{
							errorMessage = "Can't open output file";
							res = SZ_ERROR_FAIL;
							break;
						}
					}
					processedSize = outSizeProcessed;
					if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
					{
						errorMessage = "Can't write output file";
						res = SZ_ERROR_FAIL;
					}

#ifdef USE_WINDOWS_FILE
					if (f->MTimeDefined)
					{
						FILETIME mTime;
						mTime.dwLowDateTime = f->MTime.Low;
						mTime.dwHighDateTime = f->MTime.High;
						SetFileTime(outFile.handle, NULL, NULL, &mTime);
					}
#endif

					{
						SRes res2 = File_Close(&outFile);
						if (res != SZ_OK)
							break;
						if (res2 != SZ_OK)
						{
							res = res2;
							break;
						}
					}
#ifdef USE_WINDOWS_FILE
					if (f->AttribDefined)
						SetFileAttributesW(path, f->Attrib);
#endif
				}
			}
			IAlloc_Free(&allocImp, outBuffer);
		}
		SzArEx_Free(&db, &allocImp);

		File_Close(&archiveStream.file);

		if (res == SZ_OK)
			return 0;
		else
			return 1;
	}

#include <tchar.h>
	int SzExtractFromBuf(const void* pBuf, unsigned int nBufLength, wchar_t* cUnPackPath, DWORD dwPathLength)
	{
		if(pBuf==NULL || nBufLength<=0)
			return 1;

		SRes res = SZ_OK;
		CBufInStream archiveStream;
		CLookToRead lookStream;
		CSzArEx db;
		ISzAlloc allocImp;
		ISzAlloc allocTempImp;
		const char *errorMessage = NULL;

		wchar_t path[MAX_PATH * 3 + 2] = {0};
		size_t pathLen = wcslen(cUnPackPath);
		wcscpy_s(path,MAX_PATH * 3 + 2, cUnPackPath);

		if (cUnPackPath[pathLen - 1] != '\\') 
		{ 
			wcscat_s(path, L"\\");
			pathLen = wcslen(path);
		}


		CrcGenerateTable();

		allocImp.Alloc = SzAlloc;
		allocImp.Free = SzFree;

		allocTempImp.Alloc = SzAllocTemp;
		allocTempImp.Free = SzFreeTemp;

		BufInStream_CreateVTable(&archiveStream);
		LookToRead_CreateVTable(&lookStream, False);

		
		if (InBuf_Init(&archiveStream.buf, pBuf, nBufLength) != 0)
		{
			errorMessage = "can not open input file";
			res = SZ_ERROR_FAIL;
		}
		else
		{
			UInt64 pos = 0;
			if (!FindSignature(&archiveStream.buf, &pos))
				res = SZ_ERROR_FAIL;
			else if (InBuf_Seek(&archiveStream.buf, (Int64 *)&pos, SZ_SEEK_SET) != 0)
				res = SZ_ERROR_FAIL;
			if (res != 0)
				errorMessage = "Can't find 7z archive";
		}

		if (res == SZ_OK)
		{
			lookStream.realStream = &archiveStream.s;
			LookToRead_Init(&lookStream);
		}

		SzArEx_Init(&db);
		if (res == SZ_OK)
		{
			res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
		}
		if (res == SZ_OK)
		{
			UInt32 executeFileIndex = (UInt32)(Int32)-1;
			UInt32 minPrice = 1 << 30;
			UInt32 i;
			UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
			Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
			size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

			for (i = 0; i < db.db.NumFiles; i++)
			{
				size_t offset = 0;
				size_t outSizeProcessed = 0;
				const CSzFileItem *f = db.db.Files + i;
				size_t len;
				wchar_t *temp;
				len = SzArEx_GetFileNameUtf16(&db, i, NULL);

				if (len >= MAX_PATH)
				{
					res = SZ_ERROR_FAIL;
					break;
				}

				temp = path + pathLen;

				SzArEx_GetFileNameUtf16(&db, i, reinterpret_cast<UInt16*>(temp));
				{
					res = SzArEx_Extract(&db, &lookStream.s, i,
						&blockIndex, &outBuffer, &outBufferSize,
						&offset, &outSizeProcessed,
						&allocImp, &allocTempImp);
					if (res != SZ_OK)
						break;
				}
				{
					CSzFile outFile;
					size_t processedSize;
					size_t j;
					size_t nameStartPos = 0;
					for (j = 0; temp[j] != 0; j++)
					{
						if (temp[j] == '/')
						{
							temp[j] = 0;
							MyCreateDir(path);
							temp[j] = CHAR_PATH_SEPARATOR;
							nameStartPos = j + 1;
						}
					}

					if (f->IsDir)
					{
						MyCreateDir(path);
						continue;
					}
					else
					{

						if (DoesFileOrDirExist(path))
						{
							errorMessage = "Duplicate file";
							res = SZ_ERROR_FAIL;
							break;
						}
						if (OutFile_OpenW(&outFile, path))
						{
							errorMessage = "Can't open output file";
							res = SZ_ERROR_FAIL;
							break;
						}
					}
					processedSize = outSizeProcessed;
					if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
					{
						errorMessage = "Can't write output file";
						res = SZ_ERROR_FAIL;
					}

#ifdef USE_WINDOWS_FILE
					if (f->MTimeDefined)
					{
						FILETIME mTime;
						mTime.dwLowDateTime = f->MTime.Low;
						mTime.dwHighDateTime = f->MTime.High;
						SetFileTime(outFile.handle, NULL, NULL, &mTime);
					}
#endif

					{
						SRes res2 = File_Close(&outFile);
						if (res != SZ_OK)
							break;
						if (res2 != SZ_OK)
						{
							res = res2;
							break;
						}
					}
#ifdef USE_WINDOWS_FILE
					if (f->AttribDefined)
						SetFileAttributesW(path, f->Attrib);
#endif
				}
			}
			IAlloc_Free(&allocImp, outBuffer);
		}
		SzArEx_Free(&db, &allocImp);

		if (res == SZ_OK)
		{
			_tcscpy_s(cUnPackPath, dwPathLength, path);
			return 0;
		}
		else
			return 1;
	}
}