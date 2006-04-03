/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "instrument.h"
#include "filter.h"
#include <cstdio>
#include <Constants.h>
#include <datacompression.h>


template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
{
   if(pointer) delete [] pointer;
   return pointer = new_value;
}


Instrument::Instrument()
{
  // clear everythingout
  waveDataL = 0;
  waveDataR = 0;
  waveLength = 0;
  Delete();
}

Instrument::~Instrument()
{
  zapArray(waveDataL);
  zapArray(waveDataR);
  waveLength = 0;
}

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
  ENV_F_TP = F_NONE;

  _loop = false;
  _lines = 16;

  _NNA = 0; // NNA set to Note Cut [Fast Release]

  _pan = 128;
  _RPAN = false;
  _RCUT = false;
  _RRES = false;

  DeleteLayer();

  sprintf(_sName,"empty");
}

void Instrument::DeleteLayer(void)
{
  sprintf(waveName,"empty");

  zapArray(waveDataL);
  zapArray(waveDataR);
  waveLength = 0;
  waveStereo=false;
  waveLoopStart=0;
  waveLoopEnd=0;
  waveLoopType=0;
  waveVolume=100;
  waveFinetune=0;
  waveTune=0;
}

bool Instrument::Empty()
{
  if (waveLength > 0) return false;
  return true;
}




void Instrument::LoadFileChunk(DeSerializer* pFile,int version,bool fullopen)
		{
			Delete();
			// assume version 0 for now
			pFile->read(&_loop,sizeof(_loop));
			pFile->read(&_lines,sizeof(_lines));
			pFile->read(&_NNA,sizeof(_NNA));

			pFile->read(&ENV_AT,sizeof(ENV_AT));
			pFile->read(&ENV_DT,sizeof(ENV_DT));
			pFile->read(&ENV_SL,sizeof(ENV_SL));
			pFile->read(&ENV_RT,sizeof(ENV_RT));
			
			pFile->read(&ENV_F_AT,sizeof(ENV_F_AT));
			pFile->read(&ENV_F_DT,sizeof(ENV_F_DT));
			pFile->read(&ENV_F_SL,sizeof(ENV_F_SL));
			pFile->read(&ENV_F_RT,sizeof(ENV_F_RT));

			pFile->read(&ENV_F_CO,sizeof(ENV_F_CO));
			pFile->read(&ENV_F_RQ,sizeof(ENV_F_RQ));
			pFile->read(&ENV_F_EA,sizeof(ENV_F_EA));
			pFile->read(&ENV_F_TP,sizeof(ENV_F_TP));

			pFile->read(&_pan,sizeof(_pan));
			pFile->read(&_RPAN,sizeof(_RPAN));
			pFile->read(&_RCUT,sizeof(_RCUT));
			pFile->read(&_RRES,sizeof(_RRES));

			pFile->readString(_sName,sizeof(_sName));

			// now we have to read waves

			int numwaves;
			pFile->read(&numwaves, sizeof(numwaves));
			for (int i = 0; i < numwaves; i++)
			{
				char Header[5];

				pFile->read(Header,4);
				Header[4] = 0;
				int version;
				int size;

				if (strcmp(Header,"WAVE")==0)
				{

					pFile->read(&version,sizeof(version));
					pFile->read(&size,sizeof(size));
					//fileformat supports several waves, but sampler only supports one.
					if (version > CURRENT_FILE_VERSION_WAVE || i > 0)
					{
						// there is an error, this file is newer than this build of psycle
						//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
						pFile->skip(size);
					}
					else
					{
						int index;
						pFile->read(&index,sizeof(index));

						pFile->read((char*)&waveLength,sizeof(waveLength));
						pFile->read((char*)&waveVolume,sizeof(waveVolume));
						pFile->read((char*)&waveLoopStart,sizeof(waveLoopStart));
						pFile->read((char*)&waveLoopEnd,sizeof(waveLoopEnd));
						
						pFile->read(&waveTune,sizeof(waveTune));
						pFile->read(&waveFinetune,sizeof(waveFinetune));
						pFile->read(&waveLoopType,sizeof(waveLoopType));
						pFile->read(&waveStereo,sizeof(waveStereo));
						
						pFile->readString(waveName,sizeof(waveName));
						
						pFile->read(&size,sizeof(size));
						byte* pData;
						
						if ( !fullopen )
						{
							pFile->skip(size);
							waveDataL=new signed short[2];
						}
						else
						{
							pData = new byte[size+4];// +4 to avoid any attempt at buffer overflow by the code
							pFile->read(pData,size);
							DataCompression::SoundDesquash(pData,&waveDataL);
							zapArray(pData);
						}

						if (waveStereo)
						{
							pFile->read(&size,sizeof(size));
							if ( !fullopen )
							{
								pFile->skip(size);
								zapArray(waveDataR,new signed short[2]);
							}
							else
							{
								pData = new byte[size+4]; // +4 to avoid any attempt at buffer overflow by the code
								pFile->read(pData,size);
								DataCompression::SoundDesquash(pData,&waveDataR);
								zapArray(pData);
							}
						}
					}
				}
				else
				{
					pFile->read(&version,sizeof(version));
					pFile->read(&size,sizeof(size));
					// there is an error, this file is newer than this build of psycle
					//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
					pFile->skip(size);
				}
			}
		}

		/*void Instrument::SaveFileChunk(RiffFile* pFile)
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

			// now we have to write out the waves, but only if valid

			int numwaves = (waveLength > 0)?1:0; // The sampler has never supported more than one sample per instrument, even when the GUI did.

			pFile->Write(&numwaves, sizeof(numwaves));
			if (waveLength > 0)
			{
				byte * pData1(0);
				byte * pData2(0);
				UINT size1=0,size2=0;
				size1 = SoundSquash(waveDataL,&pData1,waveLength);
				if (waveStereo)
				{
					size2 = SoundSquash(waveDataR,&pData2,waveLength);
				}

				UINT index = 0;
				pFile->Write("WAVE",4);
				UINT version = CURRENT_FILE_VERSION_PATD;
				UINT size = sizeof(index)
							+sizeof(waveLength)
							+sizeof(waveVolume)
							+sizeof(waveLoopStart)
							+sizeof(waveLoopEnd)
							+sizeof(waveTune)
							+sizeof(waveFinetune)
							+sizeof(waveStereo)
							+strlen(waveName)+1
							+size1
							+size2;

				pFile->Write(&version,sizeof(version));
				pFile->Write(&size,sizeof(size));
				pFile->Write(&index,sizeof(index));

				pFile->Write(&waveLength,sizeof(waveLength));
				pFile->Write(&waveVolume,sizeof(waveVolume));
				pFile->Write(&waveLoopStart,sizeof(waveLoopStart));
				pFile->Write(&waveLoopEnd,sizeof(waveLoopEnd));

				pFile->Write(&waveTune,sizeof(waveTune));
				pFile->Write(&waveFinetune,sizeof(waveFinetune));
				pFile->Write(&waveLoopType,sizeof(waveLoopType));
				pFile->Write(&waveStereo,sizeof(waveStereo));

				pFile->Write(waveName,strlen(waveName)+1);

				pFile->Write(&size1,sizeof(size1));
				pFile->Write(pData1,size1);
				zapArray(pData1);
				if (waveStereo)
				{
					pFile->Write(&size2,sizeof(size2));
					pFile->Write(pData2,size2);
				}
				zapArray(pData2);
			}
		}
	}
}*/
