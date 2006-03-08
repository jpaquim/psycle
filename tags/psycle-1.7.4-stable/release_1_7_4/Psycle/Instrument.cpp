////////////////////////////////////
//
////////////////////////////////////

#include "stdafx.h"


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
	
	sprintf(_sName,"empty");
}

// delete layer

void Instrument::DeleteLayer(int c)
{
	sprintf(waveName[c],"empty");
	
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

void Instrument::LoadFileChunk(RiffFile* pFile,int version,bool fullopen)
{
	Delete();
	// assume version 0 for now
	pFile->Read(&_loop,sizeof(_loop));
	pFile->Read(&_lines,sizeof(_lines));
	pFile->Read(&_NNA,sizeof(_NNA));

	pFile->Read(&ENV_AT,sizeof(ENV_AT));
	pFile->Read(&ENV_DT,sizeof(ENV_DT));
	pFile->Read(&ENV_SL,sizeof(ENV_SL));
	pFile->Read(&ENV_RT,sizeof(ENV_RT));
	
	pFile->Read(&ENV_F_AT,sizeof(ENV_F_AT));
	pFile->Read(&ENV_F_DT,sizeof(ENV_F_DT));
	pFile->Read(&ENV_F_SL,sizeof(ENV_F_SL));
	pFile->Read(&ENV_F_RT,sizeof(ENV_F_RT));

	pFile->Read(&ENV_F_CO,sizeof(ENV_F_CO));
	pFile->Read(&ENV_F_RQ,sizeof(ENV_F_RQ));
	pFile->Read(&ENV_F_EA,sizeof(ENV_F_EA));
	pFile->Read(&ENV_F_TP,sizeof(ENV_F_TP));

	pFile->Read(&_pan,sizeof(_pan));
	pFile->Read(&_RPAN,sizeof(_RPAN));
	pFile->Read(&_RCUT,sizeof(_RCUT));
	pFile->Read(&_RRES,sizeof(_RRES));

	pFile->ReadString(_sName,sizeof(_sName));

	// now we have to write out the waves, but only the valid ones

	int numwaves;
	pFile->Read(&numwaves, sizeof(numwaves));
	for (int i = 0; i < numwaves; i++)
	{
		char Header[5];

		pFile->Read(&Header,4);
		Header[4] = 0;

		if (strcmp(Header,"WAVE")==0)
		{
			UINT version;
			UINT size;

			pFile->Read(&version,sizeof(version));
			pFile->Read(&size,sizeof(size));
			if (version > CURRENT_FILE_VERSION_WAVE)
			{
				// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
				pFile->Skip(size);
			}
			else
			{
				UINT index;
				pFile->Read(&index,sizeof(index));

				pFile->Read(&waveLength[index],sizeof(waveLength[index]));
				pFile->Read(&waveVolume[index],sizeof(waveVolume[index]));
				pFile->Read(&waveLoopStart[index],sizeof(waveLoopStart[index]));
				pFile->Read(&waveLoopEnd[index],sizeof(waveLoopEnd[index]));
				
				pFile->Read(&waveTune[index],sizeof(waveTune[index]));
				pFile->Read(&waveFinetune[index],sizeof(waveFinetune[index]));
				pFile->Read(&waveLoopType[index],sizeof(waveLoopType[index]));
				pFile->Read(&waveStereo[index],sizeof(waveStereo[index]));
				
				pFile->ReadString(waveName[index],sizeof(waveName[index]));
				
				pFile->Read(&size,sizeof(size));
				byte* pData;
				
				if ( !fullopen )
				{
					pFile->Skip(size);
					waveDataL[index]=new signed short[2];
				}
				else
				{
					pData = new byte[size];
					pFile->Read(pData,size);
					SoundDesquash(pData,&waveDataL[index]);
					delete pData;
				}

				if (waveStereo[index])
				{
					pFile->Read(&size,sizeof(size));
					if ( !fullopen )
					{
						pFile->Skip(size);
						waveDataL[index]=new signed short[2];
					}
					else
					{
						pData = new byte[size];
						pFile->Read(pData,size);
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

void Instrument::SaveFileChunk(RiffFile* pFile)
{
	pFile->Write(&_loop,sizeof(_loop));
	pFile->Write(&_lines,sizeof(_lines));
	pFile->Write(&_NNA,sizeof(_NNA));

	pFile->Write(&ENV_AT,sizeof(ENV_AT));
	pFile->Write(&ENV_DT,sizeof(ENV_DT));
	pFile->Write(&ENV_SL,sizeof(ENV_SL));
	pFile->Write(&ENV_RT,sizeof(ENV_RT));
	
	pFile->Write(&ENV_F_AT,sizeof(ENV_F_AT));
	pFile->Write(&ENV_F_DT,sizeof(ENV_F_DT));
	pFile->Write(&ENV_F_SL,sizeof(ENV_F_SL));
	pFile->Write(&ENV_F_RT,sizeof(ENV_F_RT));

	pFile->Write(&ENV_F_CO,sizeof(ENV_F_CO));
	pFile->Write(&ENV_F_RQ,sizeof(ENV_F_RQ));
	pFile->Write(&ENV_F_EA,sizeof(ENV_F_EA));
	pFile->Write(&ENV_F_TP,sizeof(ENV_F_TP));

	pFile->Write(&_pan,sizeof(_pan));
	pFile->Write(&_RPAN,sizeof(_RPAN));
	pFile->Write(&_RCUT,sizeof(_RCUT));
	pFile->Write(&_RRES,sizeof(_RRES));

	pFile->Write(_sName,strlen(_sName)+1);

	// now we have to write out the waves, but only the valid ones

	int numwaves = 0;
	for (int i = 0; i < MAX_WAVES; i++)
	{
		if (waveLength[i] > 0)
		{
			numwaves++;
		}
	}

	pFile->Write(&numwaves, sizeof(numwaves));
	for (i = 0; i < MAX_WAVES; i++)
	{
		if (waveLength[i] > 0)
		{
			byte * pData1;
			byte * pData2;
			UINT size1,size2;
			size1 = SoundSquash(waveDataL[i],&pData1,waveLength[i]);

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
			pFile->Write("WAVE",4);
			UINT version = CURRENT_FILE_VERSION_PATD;
			UINT size = sizeof(index)
						+sizeof(waveLength[i])
						+sizeof(waveVolume[i])
						+sizeof(waveLoopStart[i])
						+sizeof(waveLoopEnd[i])
						+sizeof(waveTune[i])
						+sizeof(waveFinetune[i])
						+sizeof(waveStereo[i])
						+strlen(waveName[i])+1
						+size1
						+size2;

			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));
			pFile->Write(&index,sizeof(index));

			pFile->Write(&waveLength[i],sizeof(waveLength[i]));
			pFile->Write(&waveVolume[i],sizeof(waveVolume[i]));
			pFile->Write(&waveLoopStart[i],sizeof(waveLoopStart[i]));
			pFile->Write(&waveLoopEnd[i],sizeof(waveLoopEnd[i]));

			pFile->Write(&waveTune[i],sizeof(waveTune[i]));
			pFile->Write(&waveFinetune[i],sizeof(waveFinetune[i]));
			pFile->Write(&waveLoopType[i],sizeof(waveLoopType[i]));
			pFile->Write(&waveStereo[i],sizeof(waveStereo[i]));

			pFile->Write(waveName[i],strlen(waveName[i])+1);

			pFile->Write(&size1,sizeof(size1));
			pFile->Write(pData1,size1);
			delete pData1;
			if (waveStereo[i])
			{
				pFile->Write(&size2,sizeof(size2));
				pFile->Write(pData2,size2);
				delete pData2;
			}
		}
	}
}