#include "PlayerHookerImpl.h"
#include "Utils.h"
#include "HookAudioDll.h"
#include "HookAudioInput.h"
CPlayerHookerV6::CPlayerHookerV6()
	: m_HaveHook(false)
	, mpAudioInput(NULL)
	, CThread(TRUE)
{
	CAudioDataHooker::ms_log.Trace(_T("CPlayerHookerV6::CPlayerHookerV6() \n"));
	ZeroMemory(m_HookExePath, 256 * sizeof(TCHAR));
}

CPlayerHookerV6::~CPlayerHookerV6()
{
	CAudioDataHooker::ms_log.Trace(_T("CPlayerHookerV6::~CPlayerHookerV6().\n"));
// 	stopAudioCapture();
// 	stopHook();
// 	gFileLog.close();
}

int CPlayerHookerV6::startHook(TCHAR* playerPath)
{
	memcpy(m_HookExePath, (void*)playerPath, _tcsclen(playerPath) * sizeof(TCHAR));
	CAudioDataHooker::ms_log.Trace(_T("startHook: %s\n"), m_HookExePath);
	if (IsProcessRunning(playerPath))
	{
		if (!isHooking())
		{
			Hook(playerPath);
			KillProcess(playerPath);
			StartupProcess(playerPath);
		}
		else{

		}
	}
	else
	{
		Hook(playerPath);
		StartupProcess(playerPath);
	}

	m_HaveHook = TRUE;
	Resume();
	return 0;
}

void CPlayerHookerV6::stopHook()
{
	m_HaveHook = FALSE;
	CAudioDataHooker::ms_log.Trace(_T("stopHook : %s\n"), m_HookExePath);
	//KillProcess(hookexepath);
	RemoveHookAudio();
	CThread::Terminate();
}


bool CPlayerHookerV6::isHooking()
{
	return HaveHookAudioRunning();
}

int CPlayerHookerV6::startAudioCapture(IAudioCaptureCallback* callback)
{
	CAudioDataHooker::ms_log.Trace(_T("startAudioCapture : %s\n"), m_HookExePath);
	if (mpAudioInput == NULL)
	{
		mpAudioInput = new CHookAudioInput();
		mpAudioInput->Open(kCAPTURE_SAMPLE_RATE, kCAPTURE_CHANNEL, 16, kCAPTURE_FRAME_SIZE_IN_BYTE * 5, kCAPTURE_FRAME_SIZE_IN_BYTE);
	}
	if (mpAudioInput != NULL)
	{
		mpAudioInput->Start(callback);
	}
	return 0;
}

void CPlayerHookerV6::stopAudioCapture()
{
	CAudioDataHooker::ms_log.Trace(_T("stopAudioCapture : %s\n"), m_HookExePath);
	if (mpAudioInput != NULL)
	{
		//KillProcess(m_HookExePath);
		mpAudioInput->Stop();
		delete mpAudioInput;
		mpAudioInput = NULL;
	}

}

void CPlayerHookerV6::Hook(TCHAR* playerPath)
{
	CAudioDataHooker::ms_log.Trace(_T("Hook : %s\n"),playerPath);
	InstallHookAudio(playerPath);
}


HOOKER_PLAYER_API IPlayerHooker* createPlayerHookerInstance() {
	return new CPlayerHookerV6();
}

HOOKER_PLAYER_API void destoryPlayerHookerInstance(IPlayerHooker* hooker) {
	delete hooker;
}

void CPlayerHookerV6::Execute()
{
	while (!Terminated){

		if (m_HaveHook){
			if (!IsProcessRunning(m_HookExePath)){
				StartupProcess(m_HookExePath);
				CAudioDataHooker::ms_log.Trace(_T("Monitor Kugou.exe had closed ..reopen it [ %s ,%d]\n"), m_HookExePath, GetTickCount());
				ShowDebugInfo(_T("Reopen Kugou.exe  stamp: %d\n"),GetTickCount());
			}
			Sleep(40);
			continue;
		}

		ShowDebugInfo(_T("NoNeedHook Check HookStatus  : terminated: %d stamp:  %d\n"),Terminated, GetTickCount());
		Sleep(40);
	}

	ShowDebugInfo(_T("MonitorHook Exit  : terminated: %d stamp:  %d\n"), Terminated, GetTickCount());
}