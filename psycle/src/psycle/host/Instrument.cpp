/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psycle::host::Filter.
#include <psycle/project.private.hpp>
#include "Instrument.hpp"
#include "DataCompression.hpp"
#include "filter.hpp"
#include "zap.hpp"
namespace psycle
{
	namespace host
	{
		Instrument::Instrument()
		{
			// clear everythingout
			waveDataL = NULL;
			waveDataR = NULL;
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
			_lock_instrument_to_machine = -1;
			_LOCKINST = false;

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
			ENV_F_TP = dsp::F_NONE;
			
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
			return !waveLength;
		}

		void Instrument::LoadFileChunk(RiffFile* pFile,int version,bool fullopen)
		{
			Delete();
			
			if ((version & 0xFF00) == 0)
			{ // Version 0

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

				// now we have to read waves

				int numwaves;
				pFile->Read(&numwaves, sizeof(numwaves));
				for (int i = 0; i < numwaves; i++)
				{
					char Header[5];

					pFile->Read(&Header,4);
					Header[4] = 0;
					UINT version;
					UINT size;

					if (strcmp(Header,"WAVE")==0)
					{

						pFile->Read(&version,sizeof(version));
						pFile->Read(&size,sizeof(size));
						//fileformat supports several waves, but sampler only supports one.
						if (version > CURRENT_FILE_VERSION_WAVE || i > 0)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
							pFile->Skip(size);
						}
						else
						{
							UINT index;
							pFile->Read(&index,sizeof(index));

							pFile->Read(&waveLength,sizeof(waveLength));
							pFile->Read(&waveVolume,sizeof(waveVolume));
							pFile->Read(&waveLoopStart,sizeof(waveLoopStart));
							pFile->Read(&waveLoopEnd,sizeof(waveLoopEnd));
							
							pFile->Read(&waveTune,sizeof(waveTune));
							pFile->Read(&waveFinetune,sizeof(waveFinetune));
							pFile->Read(&waveLoopType,sizeof(waveLoopType));
							pFile->Read(&waveStereo,sizeof(waveStereo));
							
							pFile->ReadString(waveName,sizeof(waveName));
							
							pFile->Read(&size,sizeof(size));
							byte* pData;
							
							if ( fullopen )
							{
								pData = new byte[size+4];// +4 to avoid any attempt at buffer overflow by the code
								pFile->Read(pData,size);
								SoundDesquash(pData,&waveDataL);
								zapArray(pData);
							}
							else
							{
								pFile->Skip(size);
								waveDataL=new signed short[2];
							}

							if (waveStereo)
							{
								pFile->Read(&size,sizeof(size));
								if ( fullopen )
								{
									pData = new byte[size+4]; // +4 to avoid any attempt at buffer overflow by the code
									pFile->Read(pData,size);
									SoundDesquash(pData,&waveDataR);
									zapArray(pData);
								}
								else
								{
									pFile->Skip(size);
									waveDataR =new signed short[2];
								}
							}
						}
					}
					else
					{
						pFile->Read(&version,sizeof(version));
						pFile->Read(&size,sizeof(size));
						// there is an error, this file is newer than this build of psycle
						//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size);
					}
				}

				if ((version & 0xFF) >= 1) 
				{ //revision 1 or greater
					pFile->Read(&_lock_instrument_to_machine,sizeof(_lock_instrument_to_machine));
					pFile->Read(&_LOCKINST,sizeof(_LOCKINST));
				}
			}
		}

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
				UINT version = CURRENT_FILE_VERSION_WAVE;
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

				pFile->Write(&_lock_instrument_to_machine,sizeof(_lock_instrument_to_machine));
				pFile->Write(&_LOCKINST,sizeof(_LOCKINST));
			}
		}
	}
}
