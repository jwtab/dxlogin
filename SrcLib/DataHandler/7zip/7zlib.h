/************************************************************************/

/************************************************************************/  

extern "C"{
	namespace _7zlib
	{

		int SzExtract(const wchar_t* cSzPackFile, const wchar_t* cUnPackPath);
		int SzExtractFromBuf(const void* pBuf, unsigned int nBufLength, wchar_t* cUnPackPath, DWORD dwPathLength);
	}
}
