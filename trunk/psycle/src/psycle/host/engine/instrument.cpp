///\file
///\brief interface file for psycle::host::Filter.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/engine/instrument.hpp>
#include <psycle/host/engine/DataCompression.hpp>
#include <psycle/host/engine/filter.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		Instrument::Instrument()
		:
			waveDataL(),
			waveDataR(),
			waveLength()
		{
			// clear everythingout
			Delete();
		}

		Instrument::~Instrument()
		{
			delete[] waveDataL;
			delete[] waveDataR;
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
			ENV_F_TP = dsp::F_NONE;
			
			_loop = false;
			_lines = 16;
			
			_NNA = 0; // NNA set to Note Cut [Fast Release]
			
			_pan = 128;
			_RPAN = false;
			_RCUT = false;
			_RRES = false;
			
			DeleteLayer();
			
			std::sprintf(_sName, "empty");
		}

		void Instrument::DeleteLayer(void)
		{
			std::sprintf(waveName, "empty");
			
			delete[] waveDataL; waveDataL = 0;
			delete[] waveDataR; waveDataR = 0;
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
			// assume version 0 for now
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
			pFile->Read(ENV_F_TP);

			pFile->Read(_pan);
			pFile->Read(_RPAN);
			pFile->Read(_RCUT);
			pFile->Read(_RRES);

			pFile->ReadString(_sName, sizeof _sName);

			// now we have to read waves

			int numwaves;
			pFile->Read(numwaves);
			for (int i = 0; i < numwaves; i++)
			{
				char Header[5];

				pFile->Read(&Header,4);
				Header[4] = 0;
				std::uint32_t version;
				std::uint32_t size;

				if (strcmp(Header,"WAVE")==0)
				{

					pFile->Read(version);
					pFile->Read(size);
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
						pFile->Read(index);

						pFile->Read(waveLength);
						pFile->Read(waveVolume);
						pFile->Read(waveLoopStart);
						pFile->Read(waveLoopEnd);
						
						pFile->Read(waveTune);
						pFile->Read(waveFinetune);
						pFile->Read(waveLoopType);
						pFile->Read(waveStereo);
						
						pFile->ReadString(waveName, sizeof waveName);
						
						pFile->Read(size);
						byte* pData;
						
						if ( !fullopen )
						{
							pFile->Skip(size);
							waveDataL=new std::int16_t[2];
						}
						else
						{
							pData = new std::uint8_t[size+4];// +4 to avoid any attempt at buffer overflow by the code
							pFile->Read(pData,size);
							SoundDesquash(pData,&waveDataL);
							delete[] pData;
						}

						if (waveStereo)
						{
							pFile->Read(size);
							if ( !fullopen )
							{
								pFile->Skip(size);
								delete[] waveDataR;
								waveDataR = new std::int16_t[2];
							}
							else
							{
								pData = new std::uint8_t[size+4]; // +4 to avoid any attempt at buffer overflow by the code
								pFile->Read(pData,size);
								SoundDesquash(pData,&waveDataR);
								delete[] pData;
							}
						}
					}
				}
				else
				{
					pFile->Read(version);
					pFile->Read(size);
					// there is an error, this file is newer than this build of psycle
					//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
			}
		}

		void Instrument::SaveFileChunk(RiffFile* pFile)
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

			pFile->Write(_pan);
			pFile->Write(_RPAN);
			pFile->Write(_RCUT);
			pFile->Write(_RRES);

			pFile->Write(_sName, std::strlen(_sName) + 1);

			// now we have to write out the waves, but only if valid

			int numwaves = (waveLength > 0) ? 1 : 0; // The sampler has never supported more than one sample per instrument, even when the GUI did.

			pFile->Write(numwaves);
			if (waveLength > 0)
			{
				std::uint8_t * pData1(0);
				std::uint8_t * pData2(0);
				std::uint32_t size1=0,size2=0;
				size1 = SoundSquash(waveDataL,&pData1,waveLength);
				if (waveStereo)
				{
					size2 = SoundSquash(waveDataR,&pData2,waveLength);
				}

				std::uint32_t index = 0;
				pFile->Write("WAVE",4);
				std::uint32_t version = CURRENT_FILE_VERSION_PATD;
				std::uint32_t size =
					sizeof index +
					sizeof waveLength +
					sizeof waveVolume +
					sizeof waveLoopStart +
					sizeof waveLoopEnd +
					sizeof waveTune +
					sizeof waveFinetune +
					sizeof waveStereo +
					std::strlen(waveName) + 1 +
					size1 +
					size2;

				pFile->Write(version);
				pFile->Write(size);
				pFile->Write(index);

				pFile->Write(waveLength);
				pFile->Write(waveVolume);
				pFile->Write(waveLoopStart);
				pFile->Write(waveLoopEnd);

				pFile->Write(waveTune);
				pFile->Write(waveFinetune);
				pFile->Write(waveLoopType);
				pFile->Write(waveStereo);

				pFile->Write(waveName, std::strlen(waveName) + 1);

				pFile->Write(size1);
				pFile->Write(pData1,size1);
				delete[] pData1;
				if (waveStereo)
				{
					pFile->Write(size2);
					pFile->Write(pData2,size2);
				}
				delete[] pData2;
			}
		}
	}
}
