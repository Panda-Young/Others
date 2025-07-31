#ifndef _AUDIO_CONTROL_H_
#define _AUDIO_CONTROL_H_

//#include <windows.h>
#include <string.h>
#include <math.h>
typedef unsigned char       BYTE;

/*Defines:										*/
#define AUDIO_MAX_ERROR_STRING					(1024)
#define AUDIO_BYTE_MAX							(255)
#define AUDIO_BYTE_MIN							(0)
#define AUDIO_SHORT_MAX							(32767)
#define AUDIO_SHORT_MIN							(-32768)
#define AUDIO_24BIT_MAX							(8388607)
#define AUDIO_24BIT_MIN							(-8388608)

/*Data Formats:									*/
#define AUDIO_FORMAT_BYTE						(0)
#define AUDIO_FORMAT_SHORT						(1)
#define AUDIO_FORMAT_24BIT						(2)
#define AUDIO_FORMAT_FLOAT						(3)

/*Audio Device/File Types:						*/
#define AUDIO_TYPE_PCM							(0)
#define AUDIO_TYPE_WAV							(1)
#define AUDIO_TYPE_MCI							(2)
#define AUDIO_TYPE_CD							(3)
#define AUDIO_TYPE_ASIO							(4)
#ifdef ENABLE_REMOTE_RENDER
#define AUDIO_TYPE_REMOTE						(5)
#endif

#define AUDIO_TYPE_AC3							(10)
#define AUDIO_TYPE_AAC							(11)
#define AUDIO_TYPE_MP3							(12)

	
#define AUDIO_TYPE_UNKNOWN						(99)

/*Error Codes:									*/
#define AUDIO_ERROR_NONE						(0)
#define AUDIO_ERROR_END							(1)
#define AUDIO_ERROR_INIT_FAIL					(2)
#define AUDIO_ERROR_UNSUPORTED_CHANNELS			(3)
#define AUDIO_ERROR_UNSUPORTED_RATE				(4)
#define AUDIO_ERROR_UNSUPORTED_FORMAT			(5)
#define AUDIO_ERROR_UNIMPLEMENTED				(6)
#define AUDIO_ERROR_ASIO_IN_OUT_DEVICE_NOT_SAME	(7)
#define AUDIO_ERROR_ASIO_NO_DEVICES				(8)
#define AUDIO_ERROR_GET_BUF_TOO_LARGE			(11)

#define AUDIO_ERROR_CORRUPT_AC3_FRAME			(98)
#define AUDIO_ERROR_UNKNOWN						(99)
#define AUDIO_ERROR_DEV_SPECIFIC				(100)

/*LUTS:											*/
extern	int		aiByteLUT[];
extern	int		aiBitLUT[];
extern	float	afFloatScale[];		
extern	double	afDoubleScale[];

/* 24 Bit Packaging Structure					*/
struct PACK24{
	public:
		PACK24()			{	
								abyValue[0] = 0;
								abyValue[1] = 0;
								abyValue[2] = 0;	
							}
		PACK24(int iValue)	{	abyValue[0] = (BYTE)(iValue&0xff);
								abyValue[1] = (BYTE)((iValue>>8)&0xff);
								abyValue[2] = (BYTE)((iValue>>16)&0xff);
							}
		PACK24& operator = (int iValue) {		
								abyValue[0] = (BYTE)(iValue&0xff);
								abyValue[1] = (BYTE)((iValue>>8)&0xff);
								abyValue[2] = (BYTE)((iValue>>16)&0xff);
								return *this;
							}
		operator	int()	{	int iValue;
								iValue=((int)abyValue[0]+((int)abyValue[1]<<8)+((int)abyValue[2]<<16));
								if(abyValue[2] & 0x80){
									iValue |= 0xFF000000;
								}
								return iValue;
							}
		operator	float()	{
								int iValue;
								iValue=((int)abyValue[0]+((int)abyValue[1]<<8)+((int)abyValue[2]<<16));
								if(abyValue[2] & 0x80){
									iValue |= 0xFF000000;
								}
								return (float)iValue;
							}
		operator	double(){
								int iValue;
								iValue=((int)abyValue[0]+((int)abyValue[1]<<8)+((int)abyValue[2]<<16));
								if(abyValue[2] & 0x80){
									iValue |= 0xFF000000;
								}
								return (double)iValue;
							}

	public:
		BYTE	abyValue[3];
};

struct AUDIOINFORMATIONEX{
	unsigned int uiSize;
	int			 iInformationFlag;
	int			 iArrayLength;
	char*		 pchTextInfo;
	short*		 pshShortInfo;

};

/* Audio Input					*/
class AudioInput{
	public:
		/*
			------ Contructors: ------
		*/

		AudioInput();
		
		/*
			------ Destructor: ------
		*/

		virtual ~AudioInput(){};
		

		/*
			int CloseAudio();
			DESCRIPTION:
				Closes audio file/device
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int CloseAudio() = 0;
		
		
		/*
			------- IO Calls: -------
		*/
		
		/*
			int GetAudio(float **ppfData, int iSamples)
			DESCRIPTION:
				Provides FLOAT input
			INPUTS:
				ppfData			-		FLOAT data to read or written
				iSamples		-		Number of samples int each channel
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int GetAudio(float **ppfData, int iSamples) = 0;

		/*
			int GetAudio(float **ppfData, int iSamples, int *piNumChannels)
			DESCRIPTION:
				Provides FLOAT input
			INPUTS:
				ppfData			-		FLOAT data to read or written
				iSamples		-		Number of samples int each channel
				piNumChannels	-		Number of channels in ppfData array
			RETURNS:
				iLastError		-		Last error encountered
				piNumChannels	-		Number of channels available for input
		*/
//		virtual int GetAudio(float **ppfData, int iSamples, int *piNumChannels) = 0;

		/*
			int GetAudio(double **ppfData, int iSamples)
			DESCRIPTION:
				Provides DOUBLE input
			INPUTS:
				ppfData			-		DOUBLE data to read or written
				iSamples		-		Number of samples int each channel
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int GetAudio(double **ppfData, int iSamples) = 0;

		/*
			------- Seek Calls: -------
		*/

		/*
			int SeekPosition(int iSample)
			DESCRIPTION:
				Seeks to specified sample
			INPUTS:
				uiSample		-		Desired Sample Location
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int SeekPosition(unsigned int uiSample) = 0;

		/*
			------- Information Calls Calls: -------
		*/
		
		int GetAudioIOType()			const				{return iAudioIOType;}


		/*
			int GetSampleRate()
			DESCRIPTION:
				Returns sample rate.
			RETURNS
				iSampleRate		-		Sample rate
		*/
		int GetSampleRate()				const				{return iSampleRate;}

		/*
			int GetSampleTotal()
			DESCRIPTION:
				Returns Total Samples.
			RETURNS
				uiSampleCount	-		Total Sample Count
		*/
		unsigned int GetSampleTotal()	const			{return uiSampleTotal;}

		/*
			int GetSampleCurrent()
			DESCRIPTION:
				Returns Total Samples.
			RETURNS
				uiSampleCount	-		Total Sample Count
		*/
		unsigned int GetSampleCurrent()	const			{return uiSampleCurrent;}

		/*
			int GetChannels()
			DESCRIPTION:
				Returns Number of Channels.
			RETURNS
				iChannels		-		Number of Channels	
		*/
		int GetChannels()				const				{return iChannels;}

		/*
			int GetFormat()
			DESCRIPTION:
				Returns data format ID.
			RETURNS
				iFormat			-		Data format ID	
		*/
		int GetFormat()					const				{return iFormat;}

		/*
			int GetBytes()
			DESCRIPTION:
				Returns number of bytes for data format.
			RETURNS
				iBytes			-		Number of bytes
		*/
		int GetBytes()					const				{return iBytes;}

		/*
			int GetBits()
			DESCRIPTION:
				Returns number of bits for data format.
			RETURNS
				iBits			-		Number of bits	
		*/
		int GetBits()					const				{return iBits;}

		/*
			int GetDelay()
			DESCRIPTION:
				Returns dealy in samples.
			RETURNS
				iDelay			-		Delay in samples
		*/
		int GetDelay()					const				{return iDelay;}

		/*
			int GetLastError()
			DESCRIPTION:
				Returns error code of last encountered error.
			RETURNS:
				iLastError		-		Last error encountered
		*/
		int GetLastError()				const				{return iLastError;}
		
		/*
			char*	GetErrorString()
			DESCRIPTION:
				Returns error string of last encountered error.
			RETURNS:
				achErrorString	-		Error string for last encountered error
		*/
		const char*	GetErrorString()	const				{return achErrorString;}

		
		/*
			void FlushError()
			DESCRIPTION:
				Resets error state to no error.
		*/
		void FlushError()									{iLastError=0; achErrorString[0]='\0';}

		/*
			int GetInfoEx(LPAUDIOINFORMATIONEX psAudioInformationEx)
			DESCRIPTION:
				Provides access to extra information
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int GetInfoEx(AUDIOINFORMATIONEX *psAudioInformationEx) const {return iLastError;}

	protected: 
		
		/*	
			------- Common Data: ------- 
		*/
		
		/*Audio Device/File Type ID							*/
		int		iAudioIOType;			
		/*Sample Rate										*/
		int		iSampleRate;		
		/* Total Sample Count								*/
		unsigned int uiSampleTotal;
		/* Current Sample Count								*/
		unsigned int uiSampleCurrent;
		/*Number of channels								*/
		int		iChannels;			
		/*Data format type BYTE, SHORT, 24BIT, and FLOAT	*/
		int		iFormat;			
		/*Byte count for data format						*/
		int		iBytes;				
		/*Bit count for data format							*/
		int		iBits;				
		/*Delay												*/
		int		iDelay;
		
		
		/* 
			------- Error Information: -------
		*/

		int		iLastError;			
		char	achErrorString[AUDIO_MAX_ERROR_STRING];
};

/* Audio Output					*/
class AudioOutput{
	public:
		/*
			------ Contructors: ------
		*/

		AudioOutput();
		
		/*
			------ Destructor: ------
		*/

		virtual ~AudioOutput(){};
		

		/*
			int CloseAudio();
			DESCRIPTION:
				Closes audio file/device
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int CloseAudio() = 0;
		
		
		/*
			------- IO Calls: -------
		*/
		
		/*
			int GetAudio(float **ppfData, int iSamples)
			DESCRIPTION:
				Provides FLOAT input
			INPUTS:
				ppfData			-		FLOAT data to read or written
				iSamples		-		Number of samples int each channel
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int PutAudio(float **ppfData, int iSamples) = 0;

		//virtual int PutAudio(float *ppfData, int iSamples) = 0;

		/*
			int PutAudio(double **ppfData, int iSamples)
			DESCRIPTION:
				Provides DOUBLE input
			INPUTS:
				ppfData			-		DOUBLE data to read or written
				iSamples		-		Number of samples int each channel
			RETURNS:
				iLastError		-		Last error encountered
		*/
		virtual int PutAudio(double **ppfData, int iSamples) = 0;

		/*
			------- Information Calls Calls: -------
		*/
		
		int GetAudioIOType()			const				{return iAudioIOType;}


		/*
			int GetSampleRate()
			DESCRIPTION:
				Returns sample rate.
			RETURNS
				iSampleRate		-		Sample rate
		*/
		int GetSampleRate()				const				{return iSampleRate;}

		/*
			int GetSampleTotal()
			DESCRIPTION:
				Returns Total Samples.
			RETURNS
				uiSampleCount	-		Total Sample Count
		*/
		unsigned int GetSampleTotal()	const			{return uiSampleTotal;}

		/*
			int GetSampleCurrent()
			DESCRIPTION:
				Returns Total Samples.
			RETURNS
				uiSampleCount	-		Total Sample Count
		*/
		unsigned int GetSampleCurrent()	const			{return uiSampleCurrent;}

		/*
			int GetChannels()
			DESCRIPTION:
				Returns Number of Channels.
			RETURNS
				iChannels		-		Number of Channels	
		*/
		int GetChannels()				const				{return iChannels;}

		/*
			int GetFormat()
			DESCRIPTION:
				Returns data format ID.
			RETURNS
				iFormat			-		Data format ID	
		*/
		int GetFormat()					const				{return iFormat;}

		/*
			int GetBytes()
			DESCRIPTION:
				Returns number of bytes for data format.
			RETURNS
				iBytes			-		Number of bytes
		*/
		int GetBytes()					const				{return iBytes;}

		/*
			int GetBits()
			DESCRIPTION:
				Returns number of bits for data format.
			RETURNS
				iBits			-		Number of bits	
		*/
		int GetBits()					const				{return iBits;}

		/*
			int GetDelay()
			DESCRIPTION:
				Returns dealy in samples.
			RETURNS
				iDelay			-		Delay in samples
		*/
		int GetDelay()					const				{return iDelay;}

		/*
			int GetLastError()
			DESCRIPTION:
				Returns error code of last encountered error.
			RETURNS:
				iLastError		-		Last error encountered
		*/
		int GetLastError()				const				{return iLastError;}
		
		/*
			char*	GetErrorString()
			DESCRIPTION:
				Returns error string of last encountered error.
			RETURNS:
				achErrorString	-		Error string for last encountered error
		*/
		const char*	GetErrorString()	const				{return achErrorString;}

		
		/*
			void FlushError()
			DESCRIPTION:
				Resets error state to no error.
		*/
		void FlushError()									{iLastError=0; achErrorString[0]='\0';}

	protected: 
		
		/*	
			------- Common Data: ------- 
		*/
		
		/*Audio Device/File Type ID							*/
		int		iAudioIOType;			
		/*Sample Rate										*/
		int		iSampleRate;		
		/* Total Sample Count								*/
		unsigned int uiSampleTotal;
		/* Current Sample Count								*/
		unsigned int uiSampleCurrent;
		/*Number of channels								*/
		int		iChannels;			
		/*Data format type BYTE, SHORT, 24BIT, and FLOAT	*/
		int		iFormat;			
		/*Byte count for data format						*/
		int		iBytes;				
		/*Bit count for data format							*/
		int		iBits;				
		/*Delay												*/
		int		iDelay;
		
		
		/* 
			------- Error Information: -------
		*/

		int		iLastError;			
		char	achErrorString[AUDIO_MAX_ERROR_STRING];
};

/* 

float representation:

 BYTE 1    BYTE 2	 BYTE 3    BYTE 4
SXXX XXXX XMMM MMMM MMMM MMMM MMMM MMMM

*/

#define FLOATCAST(x) (*(unsigned int*)&x) 
#define CASTFLOAT(x) (*(float*)&x)

/* Support Functions */
inline int AudioRound(float fVal)
{
	unsigned int iRetValue;
	int iTemp1;
	unsigned int iTemp2,iTemp3;

	iTemp1 = FLOATCAST(fVal);
	iTemp1 &= 0x7FFFFFFF;
	iTemp1 >>= 23;
	iTemp1 = 150 - iTemp1;


	/* This is here as MSVC insisted an unsigned int >> 33 was not zero... */
	if(iTemp1 > 24 || iTemp1 < -8){
		return 0;
	}

	iTemp2 = FLOATCAST(fVal);
	iTemp2 &= 0x007FFFFF;
	iTemp2 |= 0x00800000;

	if(iTemp1 > 0){
		iTemp3 = iTemp2 >> iTemp1;
		iTemp1 --;
		iTemp3 += ((iTemp2 >> iTemp1) & 0x1);
	}
	else{
		-- iTemp1 ^= 0xFFFFFFFF;
		iTemp3 = iTemp2 << iTemp1;
	} 
	
	iTemp1 = FLOATCAST(fVal);	 
	if(iTemp1 & 0x80000000){
		iRetValue = iTemp3;
		--iRetValue ^= 0xFFFFFFFF; 
	}
	else{
		iRetValue = iTemp3;
	}
	
	return iRetValue;
}


#endif