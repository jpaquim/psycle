/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#include "stdafx.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "Instrument.h"
#include "DataCompression.h"


// constructor

Instrument::Instrument()
{
	// clear everythingout
	for (int c=0; c<MAX_WAVES; c++)
	{
		waveDataL[c] = NULL;
		waveDataR[c] = NULL;
		waveLength[c] = 0;
	}
	Delete();
}

// destructor

Instrument::~Instrument()
{
	for (int c=0; c<MAX_WAVES; c++)
	{
		if(waveLength[c]>0)
		{
			delete waveDataL[c];
			if(waveStereo[c])
			{
				delete waveDataR[c];
			}
			waveLength[c] = 0;
		}
	}
}

// other functions

// delete instrument

void Instrument::Delete()
{
	// Reset envelope
	ENV_AT = 1; // 16
	ENV_DT = 1; // 16386
	ENV_SL = 100; // 64
	ENV_RT = 16; // OVERLAPTIME
	
	ENV_F_AT = 16;
	ENV_F_DT = 16384;
	ENV_F_SL = 64;
	ENV_F_RT = 16384;
	
	ENV_F_CO = 64;
	ENV_F_RQ = 64;
	ENV_F_EA = 128;
	ENV_F_TP = 4;
	
	_loop = false;
	_lines = 16;
	
	_NNA = 0; // NNA set to Note Cut [Fast Release]
	
	_pan = 128;
	_RPAN = false;
	_RCUT = false;
	_RRES = false;
	
	for (int c=0; c<MAX_WAVES; c++)
	{
		DeleteLayer(c);
	}
	
	_stprintf(_sName,SF::CResourceString(IDS_MSG0004));
}

// delete layer

void Instrument::DeleteLayer(int c)
{
	_stprintf(waveName[c],SF::CResourceString(IDS_MSG0004));
	
	if(waveLength[c]>0)
	{
		delete waveDataL[c];
		if(waveStereo[c])
		{
			delete waveDataR[c];
		}
		waveLength[c] = 0;
	}

	waveDataL[c] = NULL;
	waveDataR[c] = NULL;
	
	waveStereo[c]=false;
	waveLoopStart[c]=0;
	waveLoopEnd[c]=0;
	waveLoopType[c]=0;
	waveVolume[c]=100;
	waveFinetune[c]=0;
	waveTune[c]=0;
}

// check for empty instrument

bool Instrument::Empty()
{
	for (int i = 0; i < MAX_WAVES; i++)
	{
		if (waveLength[i] > 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

// load instrument

void Instrument::LoadFileChunk(RiffFile& riffFile,int version,bool fullopen)
{
	Delete();
	// assume version 0 for now
	riffFile.Read(&_loop,sizeof(_loop));
	riffFile.Read(&_lines,sizeof(_lines));
	riffFile.Read(&_NNA,sizeof(_NNA));

	riffFile.Read(&ENV_AT,sizeof(ENV_AT));
	riffFile.Read(&ENV_DT,sizeof(ENV_DT));
	riffFile.Read(&ENV_SL,sizeof(ENV_SL));
	riffFile.Read(&ENV_RT,sizeof(ENV_RT));
	
	riffFile.Read(&ENV_F_AT,sizeof(ENV_F_AT));
	riffFile.Read(&ENV_F_DT,sizeof(ENV_F_DT));
	riffFile.Read(&ENV_F_SL,sizeof(ENV_F_SL));
	riffFile.Read(&ENV_F_RT,sizeof(ENV_F_RT));

	riffFile.Read(&ENV_F_CO,sizeof(ENV_F_CO));
	riffFile.Read(&ENV_F_RQ,sizeof(ENV_F_RQ));
	riffFile.Read(&ENV_F_EA,sizeof(ENV_F_EA));
	riffFile.Read(&ENV_F_TP,sizeof(ENV_F_TP));

	riffFile.Read(&_pan,sizeof(_pan));
	riffFile.Read(&_RPAN,sizeof(_RPAN));
	riffFile.Read(&_RCUT,sizeof(_RCUT));
	riffFile.Read(&_RRES,sizeof(_RRES));

	riffFile.ReadStringA2T(_sName,sizeof(_sName));

	// now we have to write out the waves, but only the valid ones

	int numwaves;
	riffFile.Read(&numwaves, sizeof(numwaves));
	for (int i = 0; i < numwaves; i++)
	{
		char Header[5];

		riffFile.Read(&Header,4);
		Header[4] = 0;

		if (strcmp(Header,"WAVE")==0)
		{
			UINT version;
			UINT size;

			riffFile.Read(&version,sizeof(version));
			riffFile.Read(&size,sizeof(size));
			if (version > CURRENT_FILE_VERSION_WAVE)
			{
				// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
				riffFile.Skip(size);
			}
			else
			{
				UINT index;
				riffFile.Read(&index,sizeof(index));

				riffFile.Read(&waveLength[index],sizeof(waveLength[index]));
				riffFile.Read(&waveVolume[index],sizeof(waveVolume[index]));
				riffFile.Read(&waveLoopStart[index],sizeof(waveLoopStart[index]));
				riffFile.Read(&waveLoopEnd[index],sizeof(waveLoopEnd[index]));
				
				riffFile.Read(&waveTune[index],sizeof(waveTune[index]));
				riffFile.Read(&waveFinetune[index],sizeof(waveFinetune[index]));
				riffFile.Read(&waveLoopType[index],sizeof(waveLoopType[index]));
				riffFile.Read(&waveStereo[index],sizeof(waveStereo[index]));
				
				riffFile.ReadStringA2T(waveName[index],sizeof(waveName[index]) / sizeof(TCHAR));
				
				riffFile.Read(&size,sizeof(size));
				byte* pData;
				
				if ( !fullopen )
				{
					riffFile.Skip(size);
					waveDataL[index]=new signed short[2];
				}
				else
				{
					pData = new byte[size];
					riffFile.Read(pData,size);
					SoundDesquash(pData,&waveDataL[index]);
					delete pData;
				}

				if (waveStereo[index])
				{
					riffFile.Read(&size,sizeof(size));
					if ( !fullopen )
					{
						riffFile.Skip(size);
						waveDataL[index]=new signed short[2];
					}
					else
					{
						pData = new byte[size];
						riffFile.Read(pData,size);
						SoundDesquash(pData,&waveDataR[index]);
						delete pData;
					}
				}
			}
		}
		else
		{
			// file error!
		}
	}
}

// save instrument

void Instrument::SaveFileChunk(RiffFile& riffFile)
{
	riffFile.Write(&_loop,sizeof(_loop));
	riffFile.Write(&_lines,sizeof(_lines));
	riffFile.Write(&_NNA,sizeof(_NNA));

	riffFile.Write(&ENV_AT,sizeof(ENV_AT));
	riffFile.Write(&ENV_DT,sizeof(ENV_DT));
	riffFile.Write(&ENV_SL,sizeof(ENV_SL));
	riffFile.Write(&ENV_RT,sizeof(ENV_RT));
	
	riffFile.Write(&ENV_F_AT,sizeof(ENV_F_AT));
	riffFile.Write(&ENV_F_DT,sizeof(ENV_F_DT));
	riffFile.Write(&ENV_F_SL,sizeof(ENV_F_SL));
	riffFile.Write(&ENV_F_RT,sizeof(ENV_F_RT));

	riffFile.Write(&ENV_F_CO,sizeof(ENV_F_CO));
	riffFile.Write(&ENV_F_RQ,sizeof(ENV_F_RQ));
	riffFile.Write(&ENV_F_EA,sizeof(ENV_F_EA));
	riffFile.Write(&ENV_F_TP,sizeof(ENV_F_TP));

	riffFile.Write(&_pan,sizeof(_pan));
	riffFile.Write(&_RPAN,sizeof(_RPAN));
	riffFile.Write(&_RCUT,sizeof(_RCUT));
	riffFile.Write(&_RRES,sizeof(_RRES));

	CT2A _name(_sName);
	riffFile.Write(_name,strlen(_name.m_szBuffer) + 1);

	// now we have to write out the waves, but only the valid ones

	int numwaves = 0;
	for (int i = 0; i < MAX_WAVES; i++)
	{
		if (waveLength[i] > 0)
		{
			numwaves++;
		}
	}

	riffFile.Write(&numwaves, sizeof(numwaves));
	for (i = 0; i < MAX_WAVES; i++)
	{
		if (waveLength[i] > 0)
		{
			byte * pData1;
			byte * pData2;
			UINT size1(SoundSquash(waveDataL[i],&pData1,waveLength[i]));
			UINT size2(0);

			/*  test for accuracy of compress/decompress - it's 100%
			SoundDesquash(pData1,&waveDataR[i]);
			for (int c = 0; c < waveLength[i]; c++)
			{
				if (waveDataL[i][c] != waveDataR[i][c])
				{
					int z = 2000; // error
				}
			}
			*/

			if (waveStereo[i])
			{
				size2 = SoundSquash(waveDataR[i],&pData2,waveLength[i]);
			}


			UINT index = i;
			riffFile.Write("WAVE",4);
			UINT version = CURRENT_FILE_VERSION_PATD;
			CT2A _wavename(waveName[i]);
			UINT size = sizeof(index)
						+sizeof(waveLength[i])
						+sizeof(waveVolume[i])
						+sizeof(waveLoopStart[i])
						+sizeof(waveLoopEnd[i])
						+sizeof(waveTune[i])
						+sizeof(waveFinetune[i])
						+sizeof(waveStereo[i])
						+strlen(_wavename)+1
						+size1
						+size2;

			riffFile.Write(&version,sizeof(version));
			riffFile.Write(&size,sizeof(size));
			riffFile.Write(&index,sizeof(index));

			riffFile.Write(&waveLength[i],sizeof(waveLength[i]));
			riffFile.Write(&waveVolume[i],sizeof(waveVolume[i]));
			riffFile.Write(&waveLoopStart[i],sizeof(waveLoopStart[i]));
			riffFile.Write(&waveLoopEnd[i],sizeof(waveLoopEnd[i]));

			riffFile.Write(&waveTune[i],sizeof(waveTune[i]));
			riffFile.Write(&waveFinetune[i],sizeof(waveFinetune[i]));
			riffFile.Write(&waveLoopType[i],sizeof(waveLoopType[i]));
			riffFile.Write(&waveStereo[i],sizeof(waveStereo[i]));

			riffFile.Write(_wavename,strlen(_wavename) + 1);

			riffFile.Write(&size1,sizeof(size1));
			riffFile.Write(pData1,size1);
			delete pData1;
			if (waveStereo[i])
			{
				riffFile.Write(&size2,sizeof(size2));
				riffFile.Write(pData2,size2);
				delete pData2;
			}
		}
	}
}