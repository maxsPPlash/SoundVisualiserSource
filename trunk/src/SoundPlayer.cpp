#include "SoundPlayer.h"
#include <windows.h>
#include "Utils.h"

bool SoundPlayer::Play(ISound *snd) {
	PlaySound(StringToWide(snd->Path()).c_str(), NULL, SND_ASYNC);
	return true;
}