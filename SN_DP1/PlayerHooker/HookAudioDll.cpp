// HookAudioDll.cpp : 定义 DLL 应用程序的入口点。
//

#include <shlwapi.h>
#include <MMSystem.h>
#include "dsound.h"
#include "HookAudioDll.h"
#include "SyncObjs.h"
#include "APIHookLog.h"
#include "AudioDataHooker.h"
#include "SharedMem.h"
#include "Utils.h"

#pragma data_seg(".shared")
HHOOK	g_hook = NULL;
#pragma data_seg()

#pragma comment(linker,"/SECTION:.shared,RWS")

HINSTANCE hDll = NULL;

LRESULT CALLBACK HookProc(int ncode, WPARAM wparam, LPARAM lparam)
{
	//pass control to next hook in the hook chain.	
	return (CallNextHookEx(g_hook, ncode, wparam, lparam));
} 

BOOL APIENTRY DllMain( HINSTANCE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved)
{
	DWORD err =0;
	//CAudioDataHooker::ms_log.Trace(_T("This process need to hook: %d\n"), GetTickCount());
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			TCHAR buffer[256];
			memset(buffer, 0, sizeof(buffer));
			hDll = hModule;
			GetModuleFileName(GetModuleHandle(NULL), buffer, sizeof(buffer));	
			CAudioDataHooker::ms_log.Trace(_T("[Attach Process]: %s %d\n"),buffer,GetTickCount());
			return CAudioDataHooker::Instance(buffer)->StartWork(buffer, hModule);
		}
		break;
	case DLL_THREAD_ATTACH:
		{
#if 0
			TCHAR buffer[256];
			memset(buffer, 0, sizeof(buffer));
			GetModuleFileName(GetModuleHandle(NULL), buffer, sizeof(buffer));
			CAudioDataHooker::ms_log.Trace(_T("Attach Thread %s\n"), buffer);
#endif
		}
		break;
	case DLL_THREAD_DETACH:
		{
#if 0
			TCHAR buffer[256];
			memset(buffer, 0, sizeof(buffer));
			GetModuleFileName(GetModuleHandle(NULL), buffer, sizeof(buffer));
		CAudioDataHooker::ms_log.Trace(_T("Detach Thread %s\n"), buffer);
#endif
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			TCHAR buffer[256];
			memset(buffer, 0, sizeof(buffer));
			GetModuleFileName(GetModuleHandle(NULL), buffer, sizeof(buffer));
			CAudioDataHooker::ms_log.Trace(_T("[Detach process] %s\n"),buffer);
		}
		break;
	}
	return TRUE;
}

HOOK_AUDIO_API bool InstallHookAudio(const TCHAR* pHookPlayerFilePath)
{
	CAudioDataHooker::ms_log.Trace(_T("InstallHookAudio: %s\n"),pHookPlayerFilePath);
	CAudioDataHooker::Instance()->SetHookCertainProcess(pHookPlayerFilePath);
	CSharedMem sharedMem(pszSHARE_MAP_FILE_NAME, dwSHARE_MEM_SIZE);
	DWORD hookRef = sharedMem.GetDwordValue(pszHOOK_PROCESS_START_SECTION_NAME, 0);
	if (hookRef % 2 == 0)
	{
		DWORD installCount = sharedMem.GetDwordValue(pszHOOK_PROCESS_INSTALL_COUNT_SECTION_NAME, 0);
		installCount = 0;
		sharedMem.SetDwordValue(pszHOOK_PROCESS_INSTALL_COUNT_SECTION_NAME, installCount);
	}
	
	CAudioDataHooker::ms_log.Trace(_T("InstallHookAudio g_Hook: %x \n"),g_hook);
	if (g_hook)
	{
		CAudioDataHooker::Instance()->StopWork();
		UnhookWindowsHookEx(g_hook);
		g_hook = NULL;
		CAudioDataHooker::ms_log.Trace(_T("UnhookWindowsHookEx g_Hook: %x \n"), g_hook);
	}
	if (g_hook == NULL)
	{
		g_hook = SetWindowsHookEx(WH_CBT, HookProc, hDll, NULL);
		CAudioDataHooker::ms_log.Trace(_T("SetWindowsHookEx g_Hook: %x \n"), g_hook);
	}
	if (g_hook == NULL)
	{
		return false;
	}

	if (hookRef % 2 == 0)
	{
		hookRef += 1;
	}
	else
	{
		hookRef += 2;
	}
	sharedMem.SetDwordValue(pszHOOK_PROCESS_START_SECTION_NAME, hookRef);
	sharedMem.SetDwordValue(pszHOOK_PROCESS_COMMAND_SECTION_NAME, dwHOOK_AUDIO_DATA_EMPTY);
	CAudioDataHooker::ms_log.Trace(_T("InstallHookAudio: START_SECTION_NAME [%d, %s]\n"),hookRef,pHookPlayerFilePath);
	OutputDebugStringA("pszHOOK_PROCESS_COMMAND_SECTION_NAME dwHOOK_AUDIO_DATA_EMPTY\r\n");
	return true;
}

HOOK_AUDIO_API void RemoveHookAudio()
{
	if (g_hook != NULL)
	{
		CSharedMem sharedMem(pszSHARE_MAP_FILE_NAME, dwSHARE_MEM_SIZE);
		//if (sharedMem.GetDwordValue(pszHOOK_PROCESS_INSTALL_COUNT_SECTION_NAME, 0) == 0)
		//{
			DWORD hookRef = sharedMem.GetDwordValue(pszHOOK_PROCESS_START_SECTION_NAME, 0);
			if (hookRef % 2 == 1)
			{
				hookRef += 1;
			}
			else
			{
				hookRef += 2;
			}
			sharedMem.SetDwordValue(pszHOOK_PROCESS_START_SECTION_NAME, hookRef);
			CAudioDataHooker::ms_log.Trace(_T("RemoveHookAudio:START_SECTION_NAME [%d]\n"), hookRef);
		//}
	}
}

void UninstallHook()
{
	CAudioDataHooker::ms_log.Trace(_T("UninstallHook.\n"));
	if (g_hook != NULL)
	{
		RemoveHookAudio();
		UnhookWindowsHookEx(g_hook);
		g_hook = NULL;
	}
}

