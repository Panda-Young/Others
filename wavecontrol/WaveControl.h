#ifndef _WAVE_CONTROL_H_
#define _WAVE_CONTROL_H_

#include <stdio.h>
#include <string.h>
#include "AudioControl.h"

typedef unsigned int       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

/*Defines					*/
#define MAX_LABEL			(1024)
#define WAVE_MAX_CHANNELS	(24)
#define BUFFER_LENGTH		(1.0)	// WavIn thread buffer length in seconds
#define LOAD_THRESHOLD		(1024)	// Buffer empty space must be above this to trigger a file load

union IDNAME{
	public:
		IDNAME(){dwIdName=0;}
        IDNAME(const char achName[4]) {	abyIdName[0]=achName[0];
									abyIdName[1]=achName[1];
									abyIdName[2]=achName[2];
									abyIdName[3]=achName[3];}
		
		bool operator == (IDNAME	oIdName)	{return (dwIdName == oIdName.dwIdName);}
        bool operator != (IDNAME	oIdName)
        {
            return (dwIdName != oIdName.dwIdName);
        }
		bool operator <  (IDNAME	oIdName)	{return (dwIdName < oIdName.dwIdName);}
		bool operator >  (IDNAME	oIdName)	{return (dwIdName > oIdName.dwIdName);}
		bool operator == (DWORD		dwId)		{return (dwIdName == dwId);}
		bool operator != (DWORD		dwId)		{return (dwIdName != dwId);}

		operator DWORD()						{return dwIdName;}
	public:
		DWORD	dwIdName;
		BYTE	abyIdName[4];	
};

/*List of known chunks:					*/
struct GENERAL_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
};

struct RIFF_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
	IDNAME			idTypeID;	
};

struct FORMAT_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
	WORD			wFormatTag;
	WORD			wChannels;
	DWORD			dwSampleRate;
	DWORD			dwBytesPerSecond;
	WORD			wBlockAlign;
	WORD			wBitsPerSample;
};

struct DATA_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
};

struct	CUE_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
	DWORD			dwCueCount;
};

struct	CUE_DATA{
	DWORD			dwCueID;
	DWORD			dwPosition;
	IDNAME			idChunkID;
	DWORD			dwChunkStart;
	DWORD			dwBlockStart;
	DWORD			dwSampleOffset;
};

struct	LIST_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
	IDNAME			idListType;
};

struct	LABL_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
	DWORD			dwCueID;
};

struct	LTXT_CHUNK{
	IDNAME			idChunkID;
	DWORD			dwChunkSize;
	DWORD			dwCueID;
	DWORD			dwLength;
	IDNAME			idPurpose;
	WORD			wCountry;
	WORD			wLanguage;
	WORD			wDialect;
	WORD			wCodePage;
};

/*List of known chunk names:			*/
const	IDNAME		idRIFF	=	"RIFF";
const	IDNAME		idWAVE	=	"WAVE";
const	IDNAME		idFMT	=	"fmt ";
const	IDNAME		idDATA	=	"data";
const	IDNAME		idCUE	=	"cue ";
const	IDNAME		idLIST	=	"LIST";
const	IDNAME		idADTL	=	"adtl";
const	IDNAME		idLABL	=	"labl";
const	IDNAME		idLTXT	=	"ltxt";
const	IDNAME		idRGN	=	"rgn ";

/*
	------- ChunkNode Class -------
*/
class ChunkManager;
class ChunkNode{
	friend ChunkManager;
	public:
		ChunkNode();
		ChunkNode(	IDNAME			_idName,
					unsigned int	_uiPosition);

		bool operator ==	(IDNAME _idName) {return (idName == _idName);}
		bool operator <		(IDNAME _idName) {return (idName < _idName);}
		bool operator >		(IDNAME _idName) {return (idName > _idName);}

		bool operator ==	(ChunkNode &oComp) {return (idName == oComp.idName);}
		bool operator <		(ChunkNode &oComp) {return (idName < oComp.idName);}
		bool operator >		(ChunkNode &oComp) {return (idName > oComp.idName);}
	private:
		ChunkNode		*poLeft;
		ChunkNode		*poRight;
		IDNAME			idName;
		unsigned int	uiPosition;
};

/*
	------- ChunkManager Class -------
*/
class ChunkManager{
	public:
		ChunkManager();
		~ChunkManager();

		void AddChunk(	IDNAME			idName,
						unsigned int	uiPosition);
	
		unsigned int GetChunk(IDNAME	idName);

		int ScanWAVFile(FILE *psFilePtr);
	
		void PrintNodes();

	private:
		void AddNode(	ChunkNode *poPresentNode,
						ChunkNode *poNodeToAdd);
		void DeleteNode(ChunkNode *poNode);
		void PrintNode(	ChunkNode *poNode);

		ChunkNode* Search(	ChunkNode	*poNode,
							IDNAME		idName);
	private:
		ChunkNode		*poTop;
};

/*
	------- CueInformation Class -------
*/
class CueManager;
class CueInformation{
	friend CueManager;
	public:
		CueInformation();
		~CueInformation();

		bool operator == (DWORD _dwCueId)		{return (dwCueId==_dwCueId);}

		
		void	SetCueID(DWORD _dwCueId)		{dwCueId = _dwCueId;}
		DWORD	GetCueId()						{return dwCueId;}
		void	SetPosition(DWORD _dwPosition)	{dwPosition = _dwPosition;}
		DWORD	GetPosition()					{return dwPosition;}
		DWORD	GetLabelLength()				{return dwLabelLength;}	
		void	SetLabel(char *_pchLabel);
		char*	GetLabel()						{return pchLabel;}
		void	SetIsRegion(bool _bIsRegion)	{bIsRegion = _bIsRegion;}
		bool	GetIsRegion()					{return bIsRegion;}	
		CueInformation* GetNext()				{return poNext;}	
	private:
		DWORD			dwCueId;
		DWORD			dwPosition;
		DWORD			dwLabelLength;
		char*			pchLabel;
		bool			bIsRegion;
		
		CueInformation	*poNext;			
};

/*
	------- CueManager Class -------
*/
class CueManager{
	public:
		CueManager();
		~CueManager();
		
		void			AddCue(DWORD dwCueId,DWORD dwPosition);
		void			AddCue(char *pchLabel,DWORD dwPosition);
		
		int				ScanWAVFile(ChunkManager	*poChunkManager,
									FILE			*psFilePtr);

		int				FillWAVFile(FILE			*psFilePtr);

		DWORD			GetCueCount()	{return dwCueCount;}
	 
		
		void			PrintCueInfo();
		CueInformation* GetCueInformation()	{return poTop;}
	private:
		CueInformation* Search(	CueInformation *poPresent,
								DWORD			dwCueId);

		void			PrintNode( CueInformation *poPresent);
		void			DeleteNodes(CueInformation *poPresent);
	private:
		CueInformation	*poTop;
		CueInformation	*poBottom;
		DWORD			dwCueCount;
		DWORD			dwListSize;
};

class WavInThreadControl
{
	public:
		bool	bThread;			// Thread running
		int		iBufferLen;			// Buffer length in samples
		int		iNumSampsInBuffer;	// How full the buffer is
		int		iFileSampsLen;		// Length of the audio file in samples
		int		iFileSampsRead;		// File position in samples
		int		iBufWriteIndex;		// Buffer write index
		int		iBufReadIndex;		// Buffer read index
		short	*pshLongBuffer;		// Raw file sample buffer

};

/*
	------- Wave Input Class -------
*/
class WavInput : public AudioInput{
	public:
		WavInput();
        WavInput(const char	*pchFileName);
		
		virtual ~WavInput();

		virtual int CloseAudio();
		
		virtual int GetAudio(float **ppfData, int iSamples);

		virtual int GetAudio(float **ppfData, int iSamples, int *piNumChannels);

		virtual int GetAudio(double **ppfData, int iSamples);

		virtual int SeekPosition(unsigned int uiSample);
		
		/* Infomation Access */
		const RIFF_CHUNK*		GetRiffChunk()		const	{return &sRiffChunk;}
		const FORMAT_CHUNK*		GetFmtChunk()		const	{return &sFormatChunk;}
		const DATA_CHUNK*		GetDataChunk()		const	{return &sDataChunk;}
			
		ChunkManager*	GetChunkManager()			const	{return poChunkManager;}
		CueManager*		GetCueManager()				const	{return poCueManager;}

	private:
		short				*pshInterleave;
		PACK24				*psInterleave;
		
		int					iShortBlockSize;
		int					iPACK24BlockSize;

		FILE				*psFilePtr;
		ChunkManager		*poChunkManager;
		CueManager			*poCueManager;
		
		RIFF_CHUNK			sRiffChunk;
		FORMAT_CHUNK		sFormatChunk;
		DATA_CHUNK			sDataChunk;	

		unsigned int		uiFileSampleStart;
		unsigned int		uiSamplesRemaining;
};

/*
	------- Wave Input Class -------
*/
class WavOutput : public AudioOutput{
	public:
		WavOutput();
        WavOutput(	const char	*pchFileName,
					int		_iSampleRate,
					int		_iChannels,
					int		_iFormat,
                    int     _iStartRecChannel = 0);
		
		virtual ~WavOutput();

		virtual int CloseAudio();
		
		virtual int PutAudio(float **ppfData, int iSamples);

		//virtual int PutAudio(float *ppfData, int iSamples);

		virtual int PutAudio(double **ppfData, int iSamples);
		
		/* Infomation Access */
		const RIFF_CHUNK*		GetRiffChunk()		const	{return &sRiffChunk;}
		const FORMAT_CHUNK*		GetFmtChunk()		const	{return &sFormatChunk;}
		const DATA_CHUNK*		GetDataChunk()		const	{return &sDataChunk;}
			
		ChunkManager*	GetChunkManager()			const	{return poChunkManager;}
		CueManager*		GetCueManager()				const	{return poCueManager;}


		int AddMarker(char*pchLabel,int iDelta);

	private:
		short				*pshInterleave;
		PACK24				*psInterleave;
		
		int					iShortBlockSize;
		int					iPACK24BlockSize;

		FILE				*psFilePtr;
		ChunkManager		*poChunkManager;
		CueManager			*poCueManager;
		
		RIFF_CHUNK			sRiffChunk;
		FORMAT_CHUNK		sFormatChunk;
		DATA_CHUNK			sDataChunk;

        //starting channel id for recording only a selective number of channels
        int                 iStartRecChannel;
};

#endif
