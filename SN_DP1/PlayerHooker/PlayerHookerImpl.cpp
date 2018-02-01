#include "PlayerHookerImpl.h"
#include "Utils.h"
#include "HookAudioDll.h"
#include "HookAudioInput.h"
CPlayerHookerV6::CPlayerHookerV6()
	: mHaveHook(false)
	, mpAudioInput(NULL)
{
	//gFileLog.openLog("D:\\V6room\\NativeHook.log", OPEN_ALWAYS);
	//gFileLog.write("===========LocalBuild==========\r\n");
}

CPlayerHookerV6::~CPlayerHookerV6()
{
	stopAudioCapture();
	stopHook();
	gFileLog.close();
}

int CPlayerHookerV6::startHook(TCHAR* playerPath)
{
	hookexepath = playerPath;
	gFileLog.write(__FUNCTION__);
	if (IsProcessRunning(playerPath))
	{
		if (!isHooking())
		{
			Hook(playerPath);
			//KillProcess(playerPath);
			StartupProcess(playerPath);
		}
	}
	else
	{
		Hook(playerPath);
		StartupProcess(playerPath);
	}

	hookexepath = playerPath;
	return 0;
}

void CPlayerHookerV6::stopHook()
{
	gFileLog.write(__FUNCTION__);
	//KillProcess(hookexepath);
	RemoveHookAudio();
}


bool CPlayerHookerV6::isHooking()
{
	return HaveHookAudioRunning();
}

int CPlayerHookerV6::startAudioCapture(IAudioCaptureCallback* callback)
{
	gFileLog.write(__FUNCTION__);
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
	gFileLog.write(__FUNCTION__);
	if (mpAudioInput != NULL)
	{
		KillProcess(hookexepath);
		mpAudioInput->Stop();
		delete mpAudioInput;
		mpAudioInput = NULL;
	}

}

void CPlayerHookerV6::Hook(TCHAR* playerPath)
{
	gFileLog.write(__FUNCTION__);
	InstallHookAudio(playerPath);
}


HOOKER_PLAYER_API IPlayerHooker* createPlayerHookerInstance() {
	return new CPlayerHookerV6();
}

HOOKER_PLAYER_API void destoryPlayerHookerInstance(IPlayerHooker* hooker) {
	delete hooker;
}