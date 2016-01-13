
#ifndef LOG_H_H_
#define LOG_H_H_

/*
	日志类.
*/

/************************************************************/
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
/************************************************************/

/*****************		日志级别		********************/
typedef enum
{
	LOG_TEMP,  //[X]

	LOG_INFO, //[I]
	LOG_WARN, //[W]
	LOG_EROR, //[E]
	LOG_FATL, //[F]

	LOG_MAX,  //[M]
}LOG_RANK;

typedef int LOG_LEVEL;

const long LOG_MAX_SIZE = 1024*1024*10;

const char LOG_RANK_TEXT[LOG_MAX][3] = {
		"X",
		"I",
		"W",
		"E",
		"F"
	};

/************************************************************/

class CXLog
{
public:
	CXLog();
	~CXLog();

public:
	void InitLog(const char * pszLogPath,LOG_LEVEL iType = LOG_TEMP);

	void UnInitLog();

	void SetLogMaxSize(long lSize);

	void LogLine(LOG_LEVEL iType,const char *szMsg, ...);

	void DebugLogLine(LOG_LEVEL iType,const char *szMsg, ...);

	void LogLine_Check(LOG_LEVEL iType,BOOL bCanLog,const char *szMsg, ...);

private:
	void WriteLogHead();

	void WriteLine(LOG_LEVEL iType,const char *szMsg,va_list args);

private:
	char  m_szLogPath[MAX_PATH];
	FILE * m_pFileLog;

	LOG_LEVEL m_iMiniRank;

	long  m_lLogMaxSize;
	int   m_iLogAddIndex;
};

#endif