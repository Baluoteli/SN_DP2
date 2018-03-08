#include "Utils.h"
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include <TlHelp32.h>

#pragma warning (disable : 4311)

CFileIO gFileLog;
bool isDebugMode;
bool isSaveDumpPcm;

tstring ExtractFileName( const tstring& strFilePath )
{
	tstring strFileName;
	tstring::size_type nDotPos;
	nDotPos = strFilePath.rfind(_T("\\"));
	if (nDotPos != tstring::npos)
	{
		strFileName = strFilePath.substr(nDotPos + 1, strFilePath.size() - nDotPos - 1);
	}

	return strFileName;
}

BOOL StartupProcess( const TCHAR* pProcessName )
{
	BOOL bWorked;
	STARTUPINFO suInfo;
	PROCESS_INFORMATION procInfo;

	memset (&suInfo, 0, sizeof(suInfo));
	suInfo.cb = sizeof(suInfo);

	bWorked = ::CreateProcess(pProcessName, NULL, NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &suInfo, &procInfo);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	return bWorked;
}

BOOL IsProcessRunning( const TCHAR* pProcessName )
{
	HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD dwsma = GetLastError();
	DWORD dwExitCode = 0;


	PROCESSENTRY32 procEntry = {0};
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hndl, &procEntry);
	do
	{
		HANDLE hHandle;
		hHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procEntry.th32ProcessID);
		if (hHandle == NULL)
		{
			continue;
		}

		TCHAR processFileName[MAX_PATH];
		GetModuleFileNameEx(hHandle, NULL, processFileName, sizeof(processFileName));
		CloseHandle(hHandle);
		if (_tcsicmp(processFileName, pProcessName) == 0)
		{
			return TRUE;
		}
	}
	while (Process32Next(hndl, &procEntry));

	if (hndl != INVALID_HANDLE_VALUE)
	{
		 CloseHandle(hndl);
	}

	return FALSE;
}

BOOL KillProcess( LPTSTR pProcessName )
{
	HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD dwsma = GetLastError();
	DWORD dwExitCode = 0;
	vector<DWORD> terminateProcessIDs;

	PROCESSENTRY32 procEntry = {0};
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hndl, &procEntry);
	do
	{
		HANDLE hHandle;
		hHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procEntry.th32ProcessID);
		if (hHandle == NULL)
		{
			continue;
		}

		TCHAR processFileName[MAX_PATH];
		GetModuleFileNameEx(hHandle, NULL, processFileName, sizeof(processFileName));
		CloseHandle(hHandle);
		if (_tcsicmp(processFileName, pProcessName) == 0)
		{
			terminateProcessIDs.push_back(procEntry.th32ProcessID);
		}
	}
	while (Process32Next(hndl, &procEntry));

	vector<DWORD>::iterator itr = terminateProcessIDs.begin();
	for (; itr != terminateProcessIDs.end(); ++itr)
	{
		HANDLE hHandle = ::OpenProcess(PROCESS_TERMINATE, 0, *itr); 
		if (hHandle != NULL)
		{
			::GetExitCodeProcess(hHandle, &dwExitCode);
			::TerminateProcess(hHandle, dwExitCode);
			CloseHandle(hHandle);
		}
	}
	
	if (hndl != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hndl);
	}

	Sleep(100);
	return !terminateProcessIDs.empty();
}

CString s2cs(const std::string &str)
{
	return CString(str.c_str());
}

std::string cs2s(const CString &str)
{
	CString sTemp(str);
	return CStringA(sTemp.GetBuffer()).GetBuffer();
}

int str2int(const std::string &str)
{
	return atoi(str.data());
}

int CS2int(const CString &csStr)
{
	std::string str = cs2s(csStr);
	return str2int(str);
}

bool findPlayerPath(char* exename, int namelen, TCHAR* playerPath)
{
#define MY_BUFSIZE 256
	HKEY hKey;
	TCHAR szFindPath[MY_BUFSIZE] = { 0 };
	char szPlayer[MY_BUFSIZE] = { 0 };

	int i = 0;
	for (; i < namelen; i++)
	{
		if (exename[i] == '.')
			break;
		szPlayer[i] = exename[i];
	}
	if (i == namelen)
		return FALSE;

	TCHAR path_temp[MY_BUFSIZE] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, szPlayer, -1, path_temp, i);

	wsprintf(szFindPath, L"software\\%s", path_temp);
	TCHAR szProductType[MY_BUFSIZE];
	memset(szProductType, 0, sizeof(szProductType));
	DWORD dwBufLen = MY_BUFSIZE;
	LONG lRet;

	// �����Ǵ�ע���, ֻ�д򿪺��������������
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER,  // Ҫ�򿪵ĸ��� 
		szFindPath, // Ҫ�򿪵����Ӽ� 
		0,        // ���һ��ҪΪ0 
		KEY_QUERY_VALUE,  //  ָ���򿪷�ʽ,��Ϊ�� 
		&hKey);    // �������ؾ�� 

	if (lRet != ERROR_SUCCESS)   // �ж��Ƿ�򿪳ɹ� 
		return FALSE;
	//���濪ʼ��ѯ 
	lRet = RegQueryValueEx(hKey,  // ��ע���ʱ���صľ�� 
		TEXT("AppPath"),  //Ҫ��ѯ������,qq��װĿ¼��¼��������� 
		NULL,   // һ��ΪNULL����0 
		NULL,
		(LPBYTE)szProductType, // ����Ҫ�Ķ����������� 
		&dwBufLen);
	if (lRet != ERROR_SUCCESS)  // �ж��Ƿ��ѯ�ɹ� 
		return FALSE;
	RegCloseKey(hKey);

	//memcpy(exePath, szProductType, dwBufLen);
	wsprintf(playerPath, L"%s\\%s", szProductType, L"KuGou.exe");

	return TRUE;
}

bool IsDebugMode(HINSTANCE HModule)
{
	TCHAR szFilePath[MAX_PATH];
	CString DebugMode(_T(""));

	::GetModuleFileName(HModule, szFilePath, MAX_PATH);
	LPTSTR lpLastSlash = _tcsrchr(szFilePath, _T('\\'));

	if (lpLastSlash == NULL)
		return FALSE;

	SIZE_T nNameLen = MAX_PATH - (lpLastSlash - szFilePath + 1);
	_tcscpy_s(lpLastSlash + 1, nNameLen, _T("DebugMode.ini"));

	if (::GetFileAttributes(szFilePath) == INVALID_FILE_ATTRIBUTES) {
		CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		::WritePrivateProfileString(_T("DebugMode"), _T("DebugMode"), _T("1"), szFilePath);
		::WritePrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), _T("0"), szFilePath);

		return FALSE;
	}

	CString strResolution;
	//Default 1
	::GetPrivateProfileString(_T("DebugMode"), _T("DebugMode"), NULL, DebugMode.GetBuffer(MAX_PATH), MAX_PATH, szFilePath);
	if (_T("") == DebugMode){
		DebugMode = _T("1");
		::WritePrivateProfileString(_T("DebugMode"), _T("DebugMode"), DebugMode, szFilePath);
	}

	DebugMode.ReleaseBuffer();

	return CS2int(DebugMode);
}


bool IsSaveDumpPcm(HINSTANCE HModule)
{
	TCHAR szFilePath[MAX_PATH];
	CString DebugMode(_T(""));

	::GetModuleFileName(HModule, szFilePath, MAX_PATH);
	LPTSTR lpLastSlash = _tcsrchr(szFilePath, _T('\\'));

	if (lpLastSlash == NULL)
		return FALSE;

	SIZE_T nNameLen = MAX_PATH - (lpLastSlash - szFilePath + 1);
	_tcscpy_s(lpLastSlash + 1, nNameLen, _T("DebugMode.ini"));

	if (::GetFileAttributes(szFilePath) == INVALID_FILE_ATTRIBUTES) {
		CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		::WritePrivateProfileString(_T("DebugMode"), _T("DebugMode"), _T("1"), szFilePath);
		::WritePrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), _T("0"), szFilePath);

		return FALSE;
	}

	CString strResolution;
	//Default 0
	::GetPrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), NULL, DebugMode.GetBuffer(MAX_PATH), MAX_PATH, szFilePath);
	if (_T("") == DebugMode){
		DebugMode = _T("0");
		::WritePrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), DebugMode, szFilePath);
	}

	DebugMode.ReleaseBuffer();

	return CS2int(DebugMode);
}

