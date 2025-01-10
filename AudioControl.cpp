#include "AudioControl.h"

int		aiByteLUT[] =		{1,2,3,4};
int		aiBitLUT[]	=		{8,16,24,32};
float	afFloatScale[] =	{128.0f,32768.0f,8388608.0f,1.0f};		
double	afDoubleScale[]	=	{128.0,32768.0,8388608.0,1.0};

AudioInput::AudioInput()
	:iAudioIOType(-1),
	iSampleRate(-1),
	uiSampleTotal(-1),
	uiSampleCurrent(-1),
	iChannels(-1),
	iFormat(-1),
	iBytes(-1),
	iBits(-1),
	iDelay(0),
	iLastError(AUDIO_ERROR_INIT_FAIL)
{
}

AudioOutput::AudioOutput()
	:iAudioIOType(-1),
	iSampleRate(-1),
	uiSampleTotal(-1),
	uiSampleCurrent(-1),
	iChannels(-1),
	iFormat(-1),
	iBytes(-1),
	iBits(-1),
	iDelay(0),
	iLastError(AUDIO_ERROR_INIT_FAIL)
{
}
