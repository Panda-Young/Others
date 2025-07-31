
#include <assert.h>
#include "WaveControl.h"


/*
	----------------- WavInput Methods: -----------------
*/

WavInput::WavInput()
	:AudioInput(),
	pshInterleave(NULL),
	psInterleave(NULL),
	iShortBlockSize(0),
	iPACK24BlockSize(0),
	psFilePtr(NULL),
	poChunkManager(NULL),
	poCueManager(NULL),
	uiFileSampleStart(0),
	uiSamplesRemaining(0)
{
	iAudioIOType = AUDIO_TYPE_WAV;
}

WavInput::WavInput(const char *pchFileName)
	:AudioInput(),
	pshInterleave(NULL),
	psInterleave(NULL),
	iShortBlockSize(0),
	iPACK24BlockSize(0),
	psFilePtr(NULL),
	poChunkManager(NULL),
	poCueManager(NULL),
	uiFileSampleStart(0),
	uiSamplesRemaining(0)
{
	
	unsigned int uiPosition;


	iAudioIOType = AUDIO_TYPE_WAV;

#if (_MSC_VER >= 1400)	// VC8 2005
	fopen_s(&psFilePtr,pchFileName,"rb");
#else
	psFilePtr = fopen(pchFileName,"rb");
#endif
	if(!psFilePtr){
		iLastError = AUDIO_ERROR_INIT_FAIL;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - File Not Found",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - File Not Found",iLastError);
#endif
		return;
	}

	poChunkManager = new ChunkManager();

	if(poChunkManager->ScanWAVFile(psFilePtr) == -1){
		iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - File Does Not Contain WAVE Header",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - File Does Not Contain WAVE Header",iLastError);
#endif
		return;
	}

	/*Read Format Chunk...*/
	uiPosition = poChunkManager->GetChunk(idFMT);
	if(uiPosition == (unsigned int)(-1)){
		iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - File Does Not Contain FMT Header",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - File Does Not Contain FMT Header",iLastError);
#endif
		return;
	}
	fseek(psFilePtr,uiPosition,SEEK_SET);
	fread(&sFormatChunk,sizeof(FORMAT_CHUNK),1,psFilePtr);

	/*Check for supported formats...*/
	iSampleRate = (int)sFormatChunk.dwSampleRate;
	iChannels = (int)sFormatChunk.wChannels;
	iBits = (int)sFormatChunk.wBitsPerSample;
	
	switch(iBits){
		case 8:
			iBytes = 1;
			iFormat = AUDIO_FORMAT_BYTE;
		break;
		case 16:
			iBytes = 2;
			iFormat = AUDIO_FORMAT_SHORT;
		break;
		case 24:
			iBytes = 3;
			iFormat = AUDIO_FORMAT_24BIT;
		break;
		case 32:
			iBytes = 4;
			iFormat = AUDIO_FORMAT_FLOAT;
		break;
		default:
			iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
#if (_MSC_VER >= 1400)	// VC8 2005
			sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - Unsupported Bits Per Sample",iLastError);
#else
			sprintf(achErrorString,"ERROR %d - Unsupported Bits Per Sample",iLastError);
#endif
			return;
	}

	if(sFormatChunk.wChannels > WAVE_MAX_CHANNELS){ 
		iLastError=AUDIO_ERROR_UNIMPLEMENTED;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - Channel Count Not Supported",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - Channel Count Not Supported",iLastError);
#endif
		return;
	}

	if(sFormatChunk.wFormatTag != 1 && sFormatChunk.wFormatTag != 65534){ //PCM or Extensible
		iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - File is Not Linear PCM",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - File is Not Linear PCM",iLastError);
#endif
		return;
	}

	/*Read Data Chunk...*/
	uiPosition = poChunkManager->GetChunk(idDATA);
	if(uiPosition == (unsigned int)(-1)){
		iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - File Does Not Contain DATA Header",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - File Does Not Contain DATA Header",iLastError);
#endif
		return;
	}
	fseek(psFilePtr,uiPosition,SEEK_SET);
	fread(&sDataChunk,sizeof(DATA_CHUNK),1,psFilePtr);

	uiPosition = ftell(psFilePtr);

	uiSampleTotal = (unsigned int)(sDataChunk.dwChunkSize / sFormatChunk.wBlockAlign);
	uiSampleCurrent = 0;
	uiSamplesRemaining = uiSampleTotal;

	uiFileSampleStart = uiPosition;
	
	FlushError();
}


WavInput::~WavInput()
{
	delete[] pshInterleave;
	delete[] psInterleave;
	
	delete poChunkManager;
	delete poCueManager;	
}


int WavInput::GetAudio(float **ppfData,int iSamples)
{
	if(iLastError){
		return iLastError;
	}

	switch(iFormat){
		case AUDIO_FORMAT_SHORT:
			if(iSamples * iChannels > iShortBlockSize){
				delete[] pshInterleave;
				iShortBlockSize = iSamples * iChannels;
				pshInterleave = new short[iShortBlockSize];
				memset(pshInterleave,0,sizeof(short)*iShortBlockSize);
			}

			{
				int		n,k,j;
				int		iMaxToRead;

				iMaxToRead = (int)uiSamplesRemaining;
				iMaxToRead = (iMaxToRead < iSamples) ? iMaxToRead : iSamples;
				
				fread(pshInterleave, iBytes, iMaxToRead*iChannels, psFilePtr);	
				
				j = 0;
				for(n = 0; n < iMaxToRead; n ++){
					for(k = 0; k < iChannels; k ++, j ++){
						ppfData[k][n] = (float)pshInterleave[j];
						ppfData[k][n] /= afFloatScale[iFormat];
					}
				}
				uiSamplesRemaining -= iMaxToRead;
				uiSampleCurrent += iMaxToRead;
				iSamples -= iMaxToRead;

				if(iSamples){
					for( n = 0;n < iSamples;n ++){
						for(k = 0;k < iChannels;k ++){
							ppfData[k][iMaxToRead + n] = 0.0f;
						}
					}
					iLastError = AUDIO_ERROR_END;
				}

			}
		break;
		case AUDIO_FORMAT_24BIT:
			if(iSamples * iChannels > iPACK24BlockSize){
				delete[] psInterleave;
				iPACK24BlockSize = iSamples*iChannels;
				psInterleave = new PACK24[iPACK24BlockSize];
			}
			{
				int		n,k;
				int		iMaxToRead;
				PACK24	*psTemp;

				psTemp = psInterleave;
				iMaxToRead = (int)uiSamplesRemaining;
				iMaxToRead = (iMaxToRead < iSamples) ? iMaxToRead:iSamples;
				
				fread(psInterleave, iBytes, iMaxToRead * iChannels, psFilePtr);	
				
				for(n = 0; n < iMaxToRead; n ++){
					for(k = 0; k < iChannels; k ++){
						ppfData[k][n] = (float)*psTemp++;
						ppfData[k][n] /= afFloatScale[iFormat];
					}
				}
				uiSamplesRemaining -= iMaxToRead;
				uiSampleCurrent += iMaxToRead;
				iSamples -= iMaxToRead;

				if(iSamples){
					for(n = 0; n < iSamples; n ++){
						for(k = 0; k < iChannels; k ++){
							ppfData[k][iMaxToRead + n] = 0.0f;
						}
					}
					iLastError = AUDIO_ERROR_END;
				}

			}
		break;
		default:
			iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
	}

	return iLastError;
}

int WavInput::GetAudio(float **ppfData, int iSamples, int *piNumChannels)
{
	return GetAudio(ppfData, iSamples);
}


int WavInput::GetAudio(double **ppfData,int iSamples)
{
	if(iLastError){
		return iLastError;
	}

	switch(iFormat){
		case AUDIO_FORMAT_SHORT:
			if(iSamples * iChannels > iShortBlockSize){
				delete[] pshInterleave;
				iShortBlockSize = iSamples * iChannels;
				pshInterleave = new short[iShortBlockSize];
				memset(pshInterleave,0,sizeof(short)*iShortBlockSize);
			}

			{
				int		n,k,j;
				int		iMaxToRead;

				iMaxToRead = (int)uiSamplesRemaining;
				iMaxToRead = (iMaxToRead < iSamples) ? iMaxToRead : iSamples;
				
				fread(pshInterleave, iBytes, iMaxToRead*iChannels, psFilePtr);	
				
				j = 0;
				for(n = 0; n < iMaxToRead; n ++){
					for(k = 0; k < iChannels; k ++, j ++){
						ppfData[k][n] = (double)pshInterleave[j];
						ppfData[k][n] /= afDoubleScale[iFormat];
					}
				}
				uiSamplesRemaining -= iMaxToRead;
				uiSampleCurrent += iMaxToRead;
				iSamples -= iMaxToRead;

				if(iSamples){
					for( n = 0;n < iSamples;n ++){
						for(k = 0;k < iChannels;k ++){
							ppfData[k][iMaxToRead + n] = 0.0;
						}
					}
					iLastError = AUDIO_ERROR_END;
				}

			}
		break;
		case AUDIO_FORMAT_24BIT:
			if(iSamples * iChannels > iPACK24BlockSize){
				delete[] psInterleave;
				iPACK24BlockSize = iSamples*iChannels;
				psInterleave = new PACK24[iPACK24BlockSize];
			}
			{
				int		n,k,j;
				int		iMaxToRead;

				iMaxToRead = (int)uiSamplesRemaining;
				iMaxToRead = (iMaxToRead < iSamples) ? iMaxToRead:iSamples;
				
				fread(psInterleave, iBytes, iMaxToRead * iChannels, psFilePtr);	
				
				j = 0;
				for(n = 0; n < iMaxToRead; n ++){
					for(k = 0; k < iChannels; k ++, j ++){
						ppfData[k][n] = (double)psInterleave[j];
						ppfData[k][n] /= afDoubleScale[iFormat];
					}
				}
				uiSamplesRemaining -= iMaxToRead;
				uiSampleCurrent += iMaxToRead;
				iSamples -= iMaxToRead;

				if(iSamples){
					for(n = 0; n < iSamples; n ++){
						for(k = 0; k < iChannels; k ++){
							ppfData[k][iMaxToRead + n] = 0.0;
						}
					}
					iLastError = AUDIO_ERROR_END;
				}

			}
		break;
		default:
			iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
	}

	return iLastError;
}

int WavInput::SeekPosition(unsigned int uiSample)
{
	if(iLastError){
		return iLastError;
	}

	if(uiSample < uiSampleTotal){
		unsigned int uiPosition;

		uiPosition = uiSample * (unsigned int)iBytes * (unsigned int)iChannels;
		uiPosition += uiFileSampleStart;

		uiSampleCurrent = uiSample;
		uiSamplesRemaining = uiSampleTotal - uiSample;
	
		fseek(psFilePtr,uiPosition,SEEK_SET);
	}

	return iLastError;
}

int WavInput::CloseAudio()
{	
	if (psFilePtr) {
		fclose(psFilePtr);
	}

	return iLastError;
}


/*
	----------------- WavOutput Methods: -----------------
*/


WavOutput::WavOutput()
	:AudioOutput(),
	pshInterleave(NULL),
	psInterleave(NULL),
	iShortBlockSize(0),
	iPACK24BlockSize(0),
	psFilePtr(NULL),
	poChunkManager(NULL),
	poCueManager(NULL)
{
	iAudioIOType = AUDIO_TYPE_WAV;
}


WavOutput::WavOutput(	const char	*pchFileName,
						int		_iSampleRate,
						int		_iChannels,
						int		_iFormat,
                        int     _iStartRecChannel)
	:AudioOutput(),
	pshInterleave(NULL),
	psInterleave(NULL),
	iShortBlockSize(0),
	iPACK24BlockSize(0),
	psFilePtr(NULL),
	poChunkManager(NULL),
	poCueManager(NULL)
{

	unsigned int uiPosition;
	
	iAudioIOType = AUDIO_TYPE_WAV;

	iSampleRate = _iSampleRate;
	if(iSampleRate < 8000 || iSampleRate > 192000){
		iLastError = AUDIO_ERROR_UNSUPORTED_RATE;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - Sample Rate Not Supported",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - Sample Rate Not Supported",iLastError);
#endif
		return;
	}
	
	iChannels = _iChannels;
	if(iChannels > WAVE_MAX_CHANNELS){
		iLastError = AUDIO_ERROR_UNSUPORTED_CHANNELS;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - Channel Count Not Supported",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - Channel Count Not Supported",iLastError);
#endif
		return;
	}

	iFormat = _iFormat;
	if(iFormat < 0 || iFormat > 3){
		iLastError = AUDIO_ERROR_UNSUPORTED_FORMAT;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - Format Not Supported",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - Format Not Supported",iLastError);
#endif
		return;
	}

	iBits = aiBitLUT[iFormat];
	iBytes = aiByteLUT[iFormat];

	/*Open Output file...*/
#if (_MSC_VER >= 1400)	// VC8 2005
	fopen_s(&psFilePtr,pchFileName,"wb");
#else
	psFilePtr = fopen(pchFileName,"wb");
#endif
	if(!psFilePtr){
		iLastError = AUDIO_ERROR_INIT_FAIL;
#if (_MSC_VER >= 1400)	// VC8 2005
		sprintf_s(achErrorString,AUDIO_MAX_ERROR_STRING,"ERROR %d - File Could Not be Opened",iLastError);
#else
		sprintf(achErrorString,"ERROR %d - File Could Not be Opened",iLastError);
#endif
		return;
	}

	poChunkManager = new ChunkManager;

	uiPosition = ftell(psFilePtr);
	poChunkManager->AddChunk(idRIFF,uiPosition);
	fwrite(&sRiffChunk,sizeof(RIFF_CHUNK),1,psFilePtr);

	uiPosition = ftell(psFilePtr);
	poChunkManager->AddChunk(idFMT,uiPosition);
	fwrite(&sFormatChunk,sizeof(FORMAT_CHUNK),1,psFilePtr);

	uiPosition = ftell(psFilePtr);
	poChunkManager->AddChunk(idDATA,uiPosition);
	fwrite(&sDataChunk,sizeof(DATA_CHUNK),1,psFilePtr);

	poCueManager = new CueManager;

	uiSampleTotal = 0;
	uiSampleTotal = 0;

    iStartRecChannel = _iStartRecChannel;

	FlushError();
}

WavOutput::~WavOutput()
{
	delete[] pshInterleave;
	delete[] psInterleave;
	
	delete poChunkManager;
	delete poCueManager;	
}

int	WavOutput::PutAudio(float	**ppfData,	int iSamples)
{
	if(iLastError){
		return iLastError;
	}

	switch(iFormat){
		case AUDIO_FORMAT_SHORT:
			if(iSamples * iChannels > iShortBlockSize){
				delete[] pshInterleave;
				iShortBlockSize = iSamples * iChannels;
				pshInterleave = new short[iShortBlockSize];
			}
			{
				int		n,k;
				short	*pshTemp;
				
				pshTemp = pshInterleave;
				for(n = 0; n < iSamples; n ++){
					for(k = iStartRecChannel; k < iStartRecChannel + iChannels; k ++){
						float fTemp;
						fTemp = ppfData[k][n];
						fTemp *= afFloatScale[iFormat];
						if (fTemp >= 0.0)
							fTemp += 0.5f;
						else
							fTemp -= 0.5f;
						fTemp = (fTemp > AUDIO_SHORT_MIN) ? fTemp : AUDIO_SHORT_MIN;
						fTemp = (fTemp < AUDIO_SHORT_MAX) ? fTemp : AUDIO_SHORT_MAX;
						*pshTemp++ = (int)fTemp;
					}
					uiSampleTotal ++;
					uiSampleCurrent ++;
				}
				fwrite(pshInterleave, iBytes, iSamples * iChannels, psFilePtr);
			}
		
		break;
		case AUDIO_FORMAT_24BIT:
			if(iSamples * iChannels > iPACK24BlockSize){
				delete[] psInterleave;
				iPACK24BlockSize = iSamples * iChannels;
				psInterleave = new PACK24[iPACK24BlockSize];
			}

			{
				int		n,k;
				PACK24	*psTemp;
				
				psTemp = psInterleave;
				for(n = 0;n < iSamples; n ++){
					for(k = iStartRecChannel; k < iStartRecChannel + iChannels; k ++){
						float fTemp;
						fTemp = ppfData[k][n];
						fTemp *= afFloatScale[iFormat];
						if (fTemp >= 0.0)
							fTemp += 0.5f;
						else
							fTemp -= 0.5f;
						fTemp = (fTemp > AUDIO_24BIT_MIN) ? fTemp : AUDIO_24BIT_MIN;
						fTemp = (fTemp < AUDIO_24BIT_MAX) ? fTemp : AUDIO_24BIT_MAX;
						*psTemp++ = (int)fTemp;
					}
					uiSampleTotal ++;
					uiSampleCurrent ++;
				}
				fwrite(psInterleave, iBytes, iSamples * iChannels, psFilePtr);
			}
		break;
	}

	return iLastError;
}
/*
int	WavOutput::PutAudio(float	*ppfData, int iSamples)
{
	if (iLastError){
		return iLastError;
	}

	switch (iFormat){
	case AUDIO_FORMAT_SHORT:
		if (iSamples * iChannels > iShortBlockSize){
			delete[] pshInterleave;
			iShortBlockSize = iSamples * iChannels;
			pshInterleave = new short[iShortBlockSize];
		}
		{
			int		n, k;
			short	*pshTemp;

			pshTemp = pshInterleave;
			for (n = 0; n < iSamples; n++){
					float fTemp;
					fTemp = ppfData[n];
					fTemp *= afFloatScale[iFormat];
					if (fTemp >= 0.0)
						fTemp += 0.5f;
					else
						fTemp -= 0.5f;
					fTemp = (fTemp > AUDIO_SHORT_MIN) ? fTemp : AUDIO_SHORT_MIN;
					fTemp = (fTemp < AUDIO_SHORT_MAX) ? fTemp : AUDIO_SHORT_MAX;
					*pshTemp++ = (int)fTemp;
				uiSampleTotal++;
				uiSampleCurrent++;
			}
			
			fwrite(pshInterleave, iBytes, iSamples * iChannels, psFilePtr);
		}

		break;
	case AUDIO_FORMAT_24BIT:
		if (iSamples * iChannels > iPACK24BlockSize){
			delete[] psInterleave;
			iPACK24BlockSize = iSamples * iChannels;
			psInterleave = new PACK24[iPACK24BlockSize];
		}

		{
			int		n, k;
			PACK24	*psTemp;

			psTemp = psInterleave;
			for (n = 0; n < iSamples; n++){
					float fTemp;
					fTemp = ppfData[n];
					fTemp *= afFloatScale[iFormat];
					if (fTemp >= 0.0)
						fTemp += 0.5f;
					else
						fTemp -= 0.5f;
					fTemp = (fTemp > AUDIO_24BIT_MIN) ? fTemp : AUDIO_24BIT_MIN;
					fTemp = (fTemp < AUDIO_24BIT_MAX) ? fTemp : AUDIO_24BIT_MAX;
					*psTemp++ = (int)fTemp;
				uiSampleTotal++;
				uiSampleCurrent++;
			}
			fwrite(psInterleave, iBytes, iSamples * iChannels, psFilePtr);
		}
		break;
	}

	return iLastError;
}
*/

int	WavOutput::PutAudio(double	**ppfData,	int iSamples)
{
	if(iLastError){
		return iLastError;
	}

	switch(iFormat){
		case AUDIO_FORMAT_SHORT:
			if(iSamples * iChannels > iShortBlockSize){
				delete[] pshInterleave;
				iShortBlockSize = iSamples * iChannels;
				pshInterleave = new short[iShortBlockSize];
			}
			{
				int		n,k;
				short	*pshTemp;
				
				pshTemp = pshInterleave;
				for(n = 0; n < iSamples; n ++){
					for(k = 0; k < iChannels; k ++){
						int		iTemp;
						double	fTemp;
						
						fTemp = ppfData[k][n];
						fTemp *= afDoubleScale[iFormat];
						fTemp = (fTemp > AUDIO_SHORT_MIN) ? fTemp : AUDIO_SHORT_MIN;
						fTemp = (fTemp < AUDIO_SHORT_MAX) ? fTemp : AUDIO_SHORT_MAX; 
						
						iTemp = (int)(floor(fTemp));
						iTemp += ((fTemp >= ((double)iTemp + 0.5)) ? 1 : 0);
						
						*pshTemp++ = iTemp;
					}
					uiSampleTotal ++;
					uiSampleCurrent ++;
				}
				fwrite(pshInterleave, iBytes, iSamples * iChannels, psFilePtr);
			}
		
		break;
		case AUDIO_FORMAT_24BIT:
			if(iSamples * iChannels > iPACK24BlockSize){
				delete[] psInterleave;
				iPACK24BlockSize = iSamples * iChannels;
				psInterleave = new PACK24[iPACK24BlockSize];
			}

			{
				int		n,k;
				PACK24	*psTemp;
				
				psTemp = psInterleave;
				for(n = 0;n < iSamples; n ++){
					for(k = 0;k < iChannels; k ++){
						int		iTemp;
						double	fTemp;
						
						fTemp = ppfData[k][n];
						fTemp *= afDoubleScale[iFormat];
						fTemp = (fTemp > AUDIO_24BIT_MIN) ? fTemp : AUDIO_24BIT_MIN;
						fTemp = (fTemp < AUDIO_24BIT_MAX) ? fTemp : AUDIO_24BIT_MAX; 
						
						iTemp = (int)(floor(fTemp));
						iTemp += ((fTemp >= ((double)iTemp + 0.5)) ? 1 : 0);
						
						*psTemp++ = iTemp;
					}
					uiSampleTotal ++;
					uiSampleCurrent ++;
				}
				fwrite(psInterleave, iBytes, iSamples * iChannels, psFilePtr);
			}
		break;
	}

	return iLastError;
}

int WavOutput::AddMarker(char*pchLabel,int iDelta)
{
	if(iLastError){
		return iLastError;
	}
	
	poCueManager->AddCue(pchLabel, uiSampleCurrent + iDelta);

	return iLastError;
}

int WavOutput::CloseAudio()
{
	
	unsigned int uiPosition;
	/*
		First if it is 24Bit format and iChannels is odd then we may have to add a byte 
		to the data section...
	*/
	if(iFormat == AUDIO_FORMAT_24BIT && ((uiSampleTotal * iBytes * iChannels) & 0x1)){
		BYTE byDummy = 0;
		fwrite(&byDummy,iBytes,1,psFilePtr);
	} 

	/*
		Now Add the Markers...
	*/
	if(poCueManager->GetCueCount() > 0){
		poCueManager->FillWAVFile(psFilePtr);
	}

	/*
		Fill in and Add the Format Section...
	*/
	sFormatChunk.idChunkID = idFMT;
	sFormatChunk.dwChunkSize = sizeof(FORMAT_CHUNK) - 8;
	sFormatChunk.wFormatTag = 1;
	sFormatChunk.wChannels = (WORD)iChannels;
	sFormatChunk.dwSampleRate = (DWORD)iSampleRate;
	sFormatChunk.dwBytesPerSecond = (DWORD)(iSampleRate * iBytes * iChannels);
	sFormatChunk.wBlockAlign = (WORD)(iBytes * iChannels);
	sFormatChunk.wBitsPerSample = (WORD)(iBits);

	uiPosition = poChunkManager->GetChunk(idFMT);
	fseek(psFilePtr,uiPosition,SEEK_SET);
	fwrite(&sFormatChunk,sizeof(FORMAT_CHUNK),1,psFilePtr);


	/*
		Fill in and Add Data Chunk Header...
	*/

	sDataChunk.idChunkID = idDATA;
	sDataChunk.dwChunkSize = (unsigned int)iChannels * (unsigned int)iBytes * uiSampleTotal;
	sDataChunk.dwChunkSize += (sDataChunk.dwChunkSize&1);

	uiPosition=poChunkManager->GetChunk(idDATA);
	fseek(psFilePtr,uiPosition,SEEK_SET);
	fwrite(&sDataChunk,sizeof(DATA_CHUNK),1,psFilePtr);

	/*
		Fill in and add Riff Chunk...
	*/
	fseek(psFilePtr,0,SEEK_END);
	uiPosition=ftell(psFilePtr);

	sRiffChunk.idChunkID = idRIFF;
	sRiffChunk.idTypeID = idWAVE;
	sRiffChunk.dwChunkSize = uiPosition-8;

	fseek(psFilePtr,0,SEEK_SET);
	fwrite(&sRiffChunk,sizeof(RIFF_CHUNK),1,psFilePtr);
	fclose(psFilePtr);

	return iLastError;
}


/*
	----------------- ChunkNode Methods: -----------------
*/

ChunkNode::ChunkNode()
	:poLeft(NULL),
	poRight(NULL),
	idName(0),
	uiPosition(0)
{
}
	
ChunkNode::ChunkNode(	IDNAME			_idName,
						unsigned int	_uiPosition)
	:poLeft(NULL),
	poRight(NULL),
	idName(_idName),
	uiPosition(_uiPosition)
{
}

/*
	----------------- ChunkManager Methods: -----------------
*/

ChunkManager::ChunkManager()
	:poTop(NULL)
{
}

ChunkManager::~ChunkManager()
{
	if(poTop){
		DeleteNode(poTop);
	}
	poTop=NULL;
}

void ChunkManager::AddNode(	ChunkNode *poPresentNode,
							ChunkNode *poNodeToAdd)
{
	if(*poNodeToAdd>*poPresentNode){
		if(poPresentNode->poLeft){
			poPresentNode=poPresentNode->poLeft;
			AddNode(poPresentNode,poNodeToAdd);
		}
		else{
			poPresentNode->poLeft=poNodeToAdd;
		}
	}
	else{
		if(poPresentNode->poRight){
			poPresentNode=poPresentNode->poRight;
			AddNode(poPresentNode,poNodeToAdd);
		}
		else{
			poPresentNode->poRight=poNodeToAdd;
		}
	}
}

void ChunkManager::AddChunk(IDNAME			idName,
							unsigned int	uiPosition)
{
	ChunkNode	*poNewNode;
	poNewNode = new ChunkNode(idName,uiPosition);

	if(poTop){
		AddNode(poTop,poNewNode);
	}
	else{
		poTop=poNewNode;
	}
}

void ChunkManager::DeleteNode(ChunkNode *poNode)
{
	if(poNode->poLeft){
		DeleteNode(poNode->poLeft);
	}
	if(poNode->poRight){
		DeleteNode(poNode->poRight);
	}
	delete poNode;
}

void ChunkManager::PrintNode(ChunkNode	*poNode)
{
	if(poNode->poLeft){
		PrintNode(poNode->poLeft);
	}
	
	printf("%c%c%c%c\t%u\n",poNode->idName.abyIdName[0],
							poNode->idName.abyIdName[1],
							poNode->idName.abyIdName[2],
							poNode->idName.abyIdName[3],
							poNode->uiPosition);

	if(poNode->poRight){
		PrintNode(poNode->poRight);
	}
}

void ChunkManager::PrintNodes()
{
	if(poTop){
		printf("Chunks found;");
		PrintNode(poTop);
	}
	else{
		printf("Tree is empty...");
	}
}

ChunkNode* ChunkManager::Search(ChunkNode	*poNode,
								IDNAME		idName)
{
	ChunkNode *poRetNode;
	poRetNode=NULL;
	if(*poNode==idName){
		poRetNode=poNode;
	}
	else{
		if(*poNode < idName && poNode->poLeft){
			poRetNode=Search(poNode->poLeft,idName);
		}
		if(*poNode > idName && poNode->poRight){
			poRetNode=Search(poNode->poRight,idName);
		} 
	}
	
	return poRetNode;	
}

unsigned int ChunkManager::GetChunk(IDNAME	idName)
{
	unsigned int	uiPosition;
	ChunkNode		*poNode;
	
	uiPosition=(unsigned int)(-1);

	if(poTop){
		poNode=Search(poTop,idName);
	}

	if(poNode){
		uiPosition=poNode->uiPosition;
	}

	return uiPosition;
}

int ChunkManager::ScanWAVFile(FILE *psFilePtr)
{	
	unsigned int	uiPosition;
	unsigned int	uiEndPosition;
	RIFF_CHUNK		sRiffChunk;
	GENERAL_CHUNK	sGeneralChunk;

	uiPosition=0;
	fseek(psFilePtr,0,SEEK_END);
	uiEndPosition=ftell(psFilePtr);
	fseek(psFilePtr,0,SEEK_SET);

	/*Read RIFF Header Chunk*/
	fread(&sRiffChunk,sizeof(RIFF_CHUNK),1,psFilePtr);
	if(sRiffChunk.idChunkID!=idRIFF){
		return -1;
	}
	if(sRiffChunk.idTypeID!=idWAVE){
		return -1;
	}

	AddChunk(sRiffChunk.idChunkID,uiPosition);

	uiPosition=ftell(psFilePtr);
	while(uiPosition<uiEndPosition){
		fread(&sGeneralChunk,sizeof(GENERAL_CHUNK),1,psFilePtr);
		AddChunk(sGeneralChunk.idChunkID,uiPosition);
		sGeneralChunk.dwChunkSize+=(sGeneralChunk.dwChunkSize&1);
		if(uiPosition+sGeneralChunk.dwChunkSize>uiEndPosition){
            break;
		}

		fseek(psFilePtr,sGeneralChunk.dwChunkSize,SEEK_CUR);
		uiPosition=ftell(psFilePtr);
	}
	

	return 0;
}

/*
	----------------- CueInformation Methods: -----------------
*/

CueInformation::CueInformation()
	:pchLabel(NULL),
	poNext(NULL),
	dwCueId(0),
	dwPosition(0),
	dwLabelLength(0),
	bIsRegion(false)
{
}

CueInformation::~CueInformation()
{
	delete[] pchLabel;
}

void	CueInformation::SetLabel(char *_pchLabel)
{
	delete[] pchLabel;
	dwLabelLength=strlen(_pchLabel)+1;
	dwLabelLength+=(dwLabelLength&0x1);
	pchLabel=new char[dwLabelLength];
	pchLabel[dwLabelLength-1]='\0';
#if (_MSC_VER >= 1400)	// VC8 2005
	strcpy_s(pchLabel,dwLabelLength,_pchLabel);
#else
	strcpy(pchLabel,_pchLabel);
#endif
}


/*
	----------------- CueManager Methods: -----------------
*/

CueManager::CueManager()
	:poTop(NULL),
	poBottom(NULL),
	dwCueCount(0),
	dwListSize(4)
{
}

void CueManager::DeleteNodes(CueInformation *poPresent)
{
	if(poPresent->poNext){
		DeleteNodes(poPresent->poNext);
	}
	delete poPresent;
}

CueManager::~CueManager()
{
	if(poTop){
		DeleteNodes(poTop);
	}
	poTop=NULL;
	poBottom=NULL;
}

void CueManager::AddCue(DWORD dwCueId,DWORD dwPosition)
{
	CueInformation* poNewNode;
	poNewNode=new CueInformation;
	poNewNode->SetCueID(dwCueId);
	poNewNode->SetPosition(dwPosition);

	if(poBottom){
		poBottom->poNext=poNewNode;	
		poBottom=poNewNode;
	}
	else{
		poBottom=poNewNode;
		poTop=poBottom;
	}
}

void CueManager::AddCue(char *pchLabel,DWORD dwPosition)
{
	CueInformation* poNewNode;
	poNewNode=new CueInformation;
	poNewNode->SetCueID(dwCueCount+1);
	poNewNode->SetLabel(pchLabel);
	poNewNode->SetPosition(dwPosition);

	if(poBottom){
		poBottom->poNext=poNewNode;	
		poBottom=poNewNode;
	}
	else{
		poBottom=poNewNode;
		poTop=poBottom;
	}

	dwListSize+=12+poNewNode->GetLabelLength();
	dwCueCount++;
}

CueInformation* CueManager::Search(	CueInformation *poPresent,
									DWORD			dwCueId)
{
	CueInformation *poRetNode;
	poRetNode=NULL;
	if(*poPresent==dwCueId){
		poRetNode=poPresent;
	}
	else if(poPresent->poNext){
		poRetNode=Search(poPresent->poNext,dwCueId);
	}

	return poRetNode;
}

int	CueManager::ScanWAVFile(ChunkManager	*poChunkManager,
							FILE			*psFilePtr)
{
	DWORD			dwCount;
	unsigned int	uiPosition;
	unsigned int	uiEndPosition;
	CUE_CHUNK		sCueChunk;
	CUE_DATA		sCueData;
	LIST_CHUNK		sListChunk;
	LABL_CHUNK		sLabelChunk;
	LTXT_CHUNK		sLTxtChunk;
	IDNAME			idTemp;
	char			achText[MAX_LABEL];

	uiPosition=poChunkManager->GetChunk(idCUE);
	if(uiPosition==(unsigned int)-1){
		return -1;
	}
	/*Position the file location:*/
	fseek(psFilePtr,uiPosition,SEEK_SET);

	/*Read the cue header:*/
	fread(&sCueChunk,sizeof(CUE_CHUNK),1,psFilePtr);
	dwCueCount=sCueChunk.dwCueCount;

	/*Read the cue data:*/
	for(dwCount=0;dwCount<dwCueCount;dwCount++){
		fread(&sCueData,sizeof(CUE_DATA),1,psFilePtr);
		AddCue(sCueData.dwCueID,sCueData.dwPosition);
	}

	/*Now add the labels to the cues...*/
	uiPosition=poChunkManager->GetChunk(idLIST);
	if(uiPosition==(unsigned int)-1){
		return -1;
	}
	fseek(psFilePtr,uiPosition,SEEK_SET);

	/*Read the LIST chunk*/
	fread(&sListChunk,sizeof(LIST_CHUNK),1,psFilePtr);
	uiEndPosition=uiPosition+7+sListChunk.dwChunkSize;
	if(sListChunk.idListType!=idADTL){
		return -1;
	}

	if(!poTop){
		return -1;
	}

	uiPosition=ftell(psFilePtr);
	while(uiPosition<uiEndPosition){
		CueInformation *poCue;
		fread(&idTemp,sizeof(IDNAME),1,psFilePtr);
		fseek(psFilePtr,uiPosition,SEEK_SET);
		if(idTemp==idLTXT){
			DWORD dwLength;
			fread(&sLTxtChunk,sizeof(LTXT_CHUNK),1,psFilePtr);
			dwLength=sLTxtChunk.dwChunkSize-20;
			dwLength+=(dwLength&1);
			if(dwLength>MAX_LABEL){
				return -1;
			}
			if(sLTxtChunk.idPurpose==idRGN){
				poCue=Search(poTop,sLTxtChunk.dwCueID);
				if(poCue){
					poCue->SetIsRegion(true);
				}
			}
			if(dwLength){
				fread(achText,dwLength,1,psFilePtr);
			}
		}
		else if(idTemp==idLABL){
			DWORD dwLength;
			fread(&sLabelChunk,sizeof(LABL_CHUNK),1,psFilePtr);
			dwLength=sLabelChunk.dwChunkSize-4;
			dwLength+=(dwLength&1);
			if(dwLength>MAX_LABEL){
				return -1;
			}
			if(dwLength){
				fread(achText,dwLength,1,psFilePtr);
				poCue=Search(poTop,sLabelChunk.dwCueID);
				if(poCue){
					poCue->SetLabel(achText);
				}
			}
		}
		else{
			return -1;
		}

		uiPosition=ftell(psFilePtr);
	}
		
	return 0;
}

int	CueManager::FillWAVFile(FILE *psFilePtr)
{

	if(!poTop){
		return -1;
	}

	DWORD			dwCount;
	CUE_CHUNK		sCueChunk;
	CUE_DATA		sCueData;
	LIST_CHUNK		sListChunk;
	LABL_CHUNK		sLabelChunk;
	CueInformation	*poPresent;

	fseek(psFilePtr,0,SEEK_END);

	sCueChunk.idChunkID=idCUE;
	sCueChunk.dwChunkSize=4+24*dwCueCount;
	sCueChunk.dwCueCount=dwCueCount;

	fwrite(&sCueChunk,sizeof(CUE_CHUNK),1,psFilePtr);

	poPresent=poTop;
	for(dwCount=0;dwCount<dwCueCount;dwCount++){
		sCueData.dwCueID=poPresent->dwCueId;
		sCueData.dwPosition=poPresent->dwPosition;
		sCueData.idChunkID=idDATA;
		sCueData.dwChunkStart=0;
		sCueData.dwBlockStart=0;
		sCueData.dwSampleOffset=poPresent->dwPosition;

		fwrite(&sCueData,sizeof(CUE_DATA),1,psFilePtr);
		
		poPresent=poPresent->poNext;
		if(!poPresent && dwCount!=(dwCueCount-1)){
			return -1;
		}
	}

	sListChunk.idChunkID=idLIST;
	sListChunk.dwChunkSize=dwListSize;
	sListChunk.idListType=idADTL;
	
	fwrite(&sListChunk,sizeof(LIST_CHUNK),1,psFilePtr);

	poPresent=poTop;
	for(dwCount=0;dwCount<dwCueCount;dwCount++){
		sLabelChunk.idChunkID=idLABL;
		sLabelChunk.dwChunkSize=(4+poPresent->GetLabelLength());
		sLabelChunk.dwCueID=poPresent->GetCueId();

		fwrite(&sLabelChunk,sizeof(LABL_CHUNK),1,psFilePtr);
		fwrite(poPresent->GetLabel(),poPresent->GetLabelLength(),1,psFilePtr);
		
		poPresent=poPresent->poNext;
		if(!poPresent && dwCount!=(dwCueCount-1)){
			return -1;
		}
	}

	return 0;
}

void CueManager::PrintNode( CueInformation *poPresent)
{
	printf("CueId:\t%u\n",poPresent->dwCueId);
	printf("Position:\t%u\n",poPresent->dwPosition);
	if(poPresent->pchLabel){
		printf("Label:\t%s\n\n",poPresent->pchLabel);
	}
	else{
		printf("No label\n\n");
	}
	if(poPresent->poNext){
		PrintNode(poPresent->poNext);
	}
}

void CueManager::PrintCueInfo()
{
	if(poTop){
		PrintNode(poTop);
	}
	else{
		printf("No cues in file...");
	}
}


