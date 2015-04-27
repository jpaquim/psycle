///\file
///\brief implementation file for psycle::host::Instrument.

#include <psycle/host/detail/project.private.hpp>
#include "Instrument.hpp"
#include "XMInstrument.hpp"
#include <psycle/helpers/datacompression.hpp>
#include <psycle/helpers/value_mapper.hpp>
#include "Zap.hpp"
namespace psycle
{
	namespace host
	{
		Instrument::Instrument()
		{
			// clear everythingout
			Init();
		}

		Instrument::~Instrument()
		{
		}

		void Instrument::Init()
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
			
			_RPAN = false;
			_RCUT = false;
			_RRES = false;
		}

		void Instrument::LoadFileChunk(RiffFile* pFile,int version,SampleList& samples, int sampleIdx, bool fullopen)
		{
			Init();
			
			if ((version & 0xFF00) == 0)
			{ // Major Version 0

				pFile->Read(_loop);
				pFile->Read(_lines);
				pFile->Read(_NNA);

				pFile->Read(ENV_AT);
				pFile->Read(ENV_DT);
				pFile->Read(ENV_SL);
				pFile->Read(ENV_RT);
				
				pFile->Read(ENV_F_AT);
				pFile->Read(ENV_F_DT);
				pFile->Read(ENV_F_SL);
				pFile->Read(ENV_F_RT);

				pFile->Read(ENV_F_CO);
				pFile->Read(ENV_F_RQ);
				pFile->Read(ENV_F_EA);
				int val;
				pFile->Read(val);
				ENV_F_TP = static_cast<helpers::dsp::FilterType>(val);

				int pan=128;
				pFile->Read(pan);
				pFile->Read(_RPAN);
				pFile->Read(_RCUT);
				pFile->Read(_RRES);
				char instrum_name[32];
				pFile->ReadString(instrum_name,sizeof(instrum_name));

				// now we have to read waves

				int numwaves;
				pFile->Read(&numwaves, sizeof(numwaves));
				for (int i = 0; i < numwaves; i++)
				{
					char Header[8];

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
							XMInstrument::WaveData<> wave;
							wave.PanFactor(value_mapper::map_256_1(pan));
							wave.WaveSampleRate(44100);
							UINT index;
							pFile->Read(&index,sizeof(index));
							pFile->Read(wave.m_WaveLength);
							unsigned short waveVolume = 0;
							pFile->Read(waveVolume);
							wave.WaveGlobVolume(waveVolume*0.01f);
							pFile->Read(wave.m_WaveLoopStart);
							pFile->Read(wave.m_WaveLoopEnd);
							
							int tmp = 0;
							pFile->Read(tmp);
							wave.WaveTune(tmp);
							pFile->Read(tmp);
							//Current sample uses 100 cents. Older used +-256
							tmp = static_cast<int>((float)tmp/2.56f);
							wave.WaveFineTune(tmp);
							bool doloop = false;;
							pFile->Read(doloop);
							wave.WaveLoopType(doloop?XMInstrument::WaveData<>::LoopType::NORMAL:XMInstrument::WaveData<>::LoopType::DO_NOT);
							pFile->Read(wave.m_WaveStereo);
							char dummy[32];
							//Old sample name, never used.
							pFile->ReadString(dummy,sizeof(dummy));
							wave.WaveName(instrum_name);
							
							pFile->Read(&size,sizeof(size));
							byte* pData;
							
							if ( fullopen )
							{
								pData = new byte[size+4];// +4 to avoid any attempt at buffer overflow by the code
								pFile->Read(pData,size);
								DataCompression::SoundDesquash(pData,&wave.m_pWaveDataL);
								zapArray(pData);
							}
							else
							{
								pFile->Skip(size);
								wave.m_pWaveDataL=new signed short[2];
							}

							if (wave.m_WaveStereo)
							{
								pFile->Read(&size,sizeof(size));
								if ( fullopen )
								{
									pData = new byte[size+4]; // +4 to avoid any attempt at buffer overflow by the code
									pFile->Read(pData,size);
									DataCompression::SoundDesquash(pData,&wave.m_pWaveDataR);
									zapArray(pData);
								}
								else
								{
									pFile->Skip(size);
									wave.m_pWaveDataR =new signed short[2];
								}
							}
							samples.SetSample(wave, sampleIdx);
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

		void Instrument::SaveFileChunk(RiffFile* pFile,const SampleList& samples, int sampleIdx, bool savesamples)
		{
			pFile->Write(_loop);
			pFile->Write(_lines);
			pFile->Write(_NNA);

			pFile->Write(ENV_AT);
			pFile->Write(ENV_DT);
			pFile->Write(ENV_SL);
			pFile->Write(ENV_RT);
			
			pFile->Write(ENV_F_AT);
			pFile->Write(ENV_F_DT);
			pFile->Write(ENV_F_SL);
			pFile->Write(ENV_F_RT);

			pFile->Write(ENV_F_CO);
			pFile->Write(ENV_F_RQ);
			pFile->Write(ENV_F_EA);
			pFile->Write(ENV_F_TP);

			 // The sampler has never supported more than one sample per instrument, even when the GUI did.
			int numwaves = savesamples?((samples.IsEnabled(sampleIdx))?1:0):0;
			int pan = 128;
			if (numwaves > 0 ) {
				pan = samples[sampleIdx].PanFactor()*256;
			}
			pFile->Write(pan);

			pFile->Write(_RPAN);
			pFile->Write(_RCUT);
			pFile->Write(_RRES);

			char instrum_name[32]={0};
			if (samples.Exists(sampleIdx)) {
				strcpy(instrum_name,samples[sampleIdx].WaveName().substr(0,31).c_str());
				instrum_name[31]='\0';
			}
			pFile->Write(instrum_name,strlen(instrum_name)+1);

			// now we have to write out the waves, but only if valid

			pFile->Write(numwaves);
			if (numwaves > 0)
			{
				const XMInstrument::WaveData<>& wave = samples[sampleIdx];
				byte * pData1(0);
				byte * pData2(0);
				uint32_t size1=0,size2=0;
				size1 = DataCompression::SoundSquash(wave.m_pWaveDataL,&pData1,wave.m_WaveLength);
				if (wave.m_WaveStereo)
				{
					size2 = DataCompression::SoundSquash(wave.m_pWaveDataR,&pData2,wave.m_WaveLength);
				}

				UINT index = 0;
				pFile->Write("WAVE",4);
				UINT version = CURRENT_FILE_VERSION_WAVE;
				UINT size = sizeof(index)
							+sizeof(wave.m_WaveLength)
							+sizeof(unsigned short)
							+sizeof(wave.m_WaveLoopStart)
							+sizeof(wave.m_WaveLoopEnd)
							+sizeof(int)
							+sizeof(int)
							+sizeof(wave.m_WaveStereo)
							+wave.m_WaveName.length()+1
							+size1
							+size2;

				pFile->Write(&version,sizeof(version));
				pFile->Write(&size,sizeof(size));
				pFile->Write(&index,sizeof(index));

				pFile->Write(wave.m_WaveLength);
				unsigned short waveVolume = (unsigned short)(wave.WaveGlobVolume()*100.f);

				pFile->Write(waveVolume);
				pFile->Write(wave.m_WaveLoopStart);
				pFile->Write(wave.m_WaveLoopEnd);

				int tmp = wave.WaveTune();
				pFile->Write(tmp);
				//Current sample uses 100 cents. Older used +-256
				tmp = static_cast<int>((float)wave.WaveFineTune()*2.56);
				pFile->Write(tmp);
				bool doloop = wave.m_WaveLoopType == XMInstrument::WaveData<>::LoopType::NORMAL;
				pFile->Write(doloop);
				pFile->Write(wave.m_WaveStereo);

				pFile->WriteString(wave.m_WaveName);

				pFile->Write(&size1,sizeof(size1));
				pFile->Write(pData1,size1);
				zapArray(pData1);
				if (wave.m_WaveStereo)
				{
					pFile->Write(&size2,sizeof(size2));
					pFile->Write(pData2,size2);
				}
				zapArray(pData2);
			}
			pFile->Write(&_lock_instrument_to_machine,sizeof(_lock_instrument_to_machine));
			pFile->Write(&_LOCKINST,sizeof(_LOCKINST));
		}
	}
}