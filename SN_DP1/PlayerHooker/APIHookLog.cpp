#include "APIHookLog.h"
#include "Utils.h"

CAPIHookLog::CAPIHookLog(TCHAR* pLogFileName)
{
	if (pLogFileName != NULL)
	{
		m_logFileName = pLogFileName;
	}
	m_logMutex = NULL;
	m_selfCloseMutex = false;
}

CAPIHookLog::~CAPIHookLog()
{
	if (m_logMutex != NULL)
	{
		ReleaseMutex(m_logMutex);
		if (m_selfCloseMutex)
		{
			CloseHandle(m_logMutex);
		}
		m_logMutex = NULL;
	}
}

#ifdef _HOOK_LOG_

void CAPIHookLog::Lock()
{
	if ((m_logMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("CAPIHookLog_Mutex_YDQ"))) == NULL)
	{
		m_logMutex = CreateMutex(NULL, FALSE, _T("CAPIHookLog_Mutex_YDQ"));
		m_selfCloseMutex = true;
	}
	if (m_logMutex != NULL)
	{
		WaitForSingleObject(m_logMutex, INFINITE);
	}
}

void CAPIHookLog::UnLock()
{
	if (m_logMutex != NULL)
	{
		ReleaseMutex(m_logMutex);
	}
}

#endif

void CAPIHookLog::SetLogPath(TCHAR* pLogPath)
{
	m_logFileName = pLogPath;
}

void CAPIHookLog::Trace(TCHAR* pLog, ...)
{
#ifdef _HOOK_LOG_
    Lock();
	if (m_logFileName.size() > 0 && isDebugMode)
	{
		FILE* log;
		log = _tfopen(m_logFileName.c_str(), _T("a+"));
		if (log != NULL)
		{
			#define MAX_DBG_MSG_LEN (1024)
			TCHAR buf[MAX_DBG_MSG_LEN];
			va_list ap;
			va_start(ap, pLog);
			_vsntprintf(buf, sizeof(buf), pLog, ap);
			SYSTEMTIME st;
			GetLocalTime(&st);
			_ftprintf(log, _T("%d%02d%02d-%02d%02d%02d%03d:  %s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds, buf);

			fclose(log);
		}
	}
	UnLock();
#endif
}