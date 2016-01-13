
#include "stdafx.h"

#include "Log.h"

CXLog::CXLog()
{
	m_lLogMaxSize = LOG_MAX_SIZE;
	m_pFileLog    = NULL;

	m_iLogAddIndex = 0;
}

CXLog::~CXLog()
{
	UnInitLog();
}

void CXLog::InitLog(const char * pszLogPath,LOG_LEVEL iType)
{
	UnInitLog();

	fopen_s(&m_pFileLog,pszLogPath,"a+");
	if (NULL != m_pFileLog)
	{
		WriteLogHead();
	}

	strcpy_s(m_szLogPath,pszLogPath);

	m_iMiniRank = iType;
}

void CXLog::UnInitLog()
{
	if (NULL != m_pFileLog)
	{
		fclose(m_pFileLog);
		m_pFileLog = NULL;
	}

	ZeroMemory(m_szLogPath,MAX_PATH);
}

void CXLog::SetLogMaxSize(long lSize)
{
	m_lLogMaxSize = lSize;
}

void CXLog::WriteLogHead()
{
	char szHeadText[2048] = {0};

	SYSTEMTIME localTime = {0};
	char szComputerName[512] = {0};
	unsigned long lLen = 512;

	GetLocalTime(&localTime);
	GetComputerNameA(szComputerName,&lLen);

	int iHeadLen = sprintf_s(szHeadText,2048,
		"\nLog created at(local) : %d-%02d-%02d %02d:%02d:%02d\nRunning on machine: %s(%d)\nLog line format: [IWEF][yy-mm-dd hh:mm:ss] msg\n\n",
		localTime.wYear,localTime.wMonth,localTime.wDay,localTime.wHour,localTime.wMinute,localTime.wSecond,
		szComputerName,GetCurrentProcessId());

	fwrite(szHeadText,1,iHeadLen,m_pFileLog);
}

void CXLog::WriteLine(LOG_LEVEL iType,const char *szMsg,va_list args)
{
	if ((iType >= m_iMiniRank) &&
		(iType < LOG_MAX))
	{
		char szLineHead[2048] = {0};
		SYSTEMTIME sysTime = {0};
		GetLocalTime(&sysTime);

		sprintf_s(szLineHead,2048,"[%s][%d-%02d-%02d %02d:%02d:%02d] ",
			LOG_RANK_TEXT[iType],
			sysTime.wYear,sysTime.wMonth,sysTime.wDay,
			sysTime.wHour,sysTime.wMinute,sysTime.wSecond);

		fprintf(m_pFileLog,szLineHead);
		vfprintf(m_pFileLog,szMsg,args);

		fprintf(m_pFileLog,"\n");		

		long lFileLen = ftell(m_pFileLog);
		if (lFileLen > m_lLogMaxSize)
		{
			m_iLogAddIndex++;

			char szNewLogPath[MAX_PATH] = {0};
			sprintf_s(szNewLogPath,MAX_PATH,"%s_%d",m_szLogPath,m_iLogAddIndex);
			UnInitLog();

			InitLog(szNewLogPath,m_iMiniRank);			
		}
	}	
}

void CXLog::LogLine(LOG_LEVEL iType,const char *szMsg, ...)
{	
	va_list args;

	va_start(args,szMsg);
	WriteLine(iType,szMsg,args);

	va_end(args);
}

void CXLog::DebugLogLine(LOG_LEVEL iType,const char *szMsg, ...)
{
#ifdef _DEBUG
	{
		va_list args;

		va_start(args,szMsg);
		WriteLine(iType,szMsg,args);

		va_end(args);
	}
#endif
}

void CXLog::LogLine_Check(LOG_LEVEL iType,BOOL bCanLog,const char *szMsg, ...)
{
	if (bCanLog)
	{
		va_list args;

		va_start(args,szMsg);
		WriteLine(iType,szMsg,args);

		va_end(args);
	}
}