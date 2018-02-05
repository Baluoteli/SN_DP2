#ifndef __PLAYER_HOOKER_IMPL_H__
#define __PLAYER_HOOKER_IMPL_H__
#include "PlayerHooker.h"
#include "Thread.h"
#include "SleepEvent.h"

class CHookAudioInput;
class CPlayerHookerV6 : public IPlayerHooker, public CThread
{
public:
	CPlayerHookerV6();
	virtual ~CPlayerHookerV6();

	virtual int startHook(TCHAR* playerPath);
	virtual bool isHooking();
	virtual void stopHook();
	virtual int startAudioCapture(IAudioCaptureCallback* callback);
	virtual void stopAudioCapture();

protected:
	void Execute();

private:
	void Hook(TCHAR* playerPath);

	static const int kCAPTURE_SAMPLE_RATE = 44100;
	static const int kCAPTURE_CHANNEL = 2;
	static const int kCAPTURE_FRAME_SIZE_IN_MS = 20;
	static const int kCAPTURE_FRAME_SIZE_IN_BYTE = kCAPTURE_SAMPLE_RATE * kCAPTURE_CHANNEL * 2 * 10 / 1000;

	bool m_HaveHook;
	CHookAudioInput* mpAudioInput;
	TCHAR m_HookExePath[256];
};

#endif