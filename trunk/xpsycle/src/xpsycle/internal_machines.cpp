#include "internal_machines.h"

///\todo: These three includes need to be replaced by a "host" callback which gives such information.
#include "song.h"
#include "player.h"
#include "configuration.h"

///\todo : The information required from this include should go to constants.hpp
#include "inputhandler.h"
///\todo : this is crap
#include "global.h"

namespace psycle {
	namespace host {

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy

		std::string Dummy::_psName = "DummyPlug";

		Dummy::Dummy(int id, Song* song)
			:
		Machine(MACH_DUMMY, MACHMODE_FX, id, song)
		{
			_audiorange = 32768.0f;
			_editName = "Dummy";
		}

        Dummy* Dummy::clone() const {
          return new Dummy(*this);
        }

		Dummy::~Dummy() 
		{
		}

		int Dummy::GenerateAudio(int numSamples)
		{
			Machine::SetVolumeCounter(numSamples);
			_worked = true;
			return numSamples;
		}

		bool Dummy::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Skip(size);
			return true;
		};


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// NoteDuplicator

		std::string DuplicatorMac::_psName = "Dupe it!";

		DuplicatorMac::DuplicatorMac(int id, Song* song)
			:
		Machine(MACH_DUPLICATOR, MACHMODE_GENERATOR, id, song)
		{
			_numPars = 16;
			_nCols = 2;
			bisTicking = false;
			_editName = "Dupe it!";
			for (int i=0;i<8;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
			}
		}

		DuplicatorMac::~DuplicatorMac() 
		{
		}

        DuplicatorMac* DuplicatorMac::clone() const {
          return new DuplicatorMac(*this);
        }

		void DuplicatorMac::Init()
		{
			Machine::Init();
			for (int i=0;i<8;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
			}
		}
		void DuplicatorMac::PreWork(int numSamples)
		{
			Machine::PreWork(numSamples);
			for(; !workEvents.empty(); workEvents.pop_front()) {
			
				WorkEvent & workEvent = workEvents.front();
				if ( !_mute && !bisTicking)
				{
					bisTicking=true;
					for (int i=0;i<8;i++)
					{
						
						PatternEvent temp = workEvent.event();
						if ( temp.note() < 120 )
						{
							temp.setNote(temp.note() +noteOffset[i]);
						}
						if (macOutput[i] != -1 && song()->_pMachine[macOutput[i]] != NULL 
							&& song()->_pMachine[macOutput[i]] != this) song()->_pMachine[macOutput[i]]->AddEvent(workEvent.beatOffset(),workEvent.track(),temp);
					}
				}
				bisTicking=false;
			}
		}
		void DuplicatorMac::Tick( int channel, const PatternEvent & pData )
		{
		}

		void DuplicatorMac::GetParamName(int numparam,char *name)
		{
			if (numparam >=0 && numparam<8)
			{
				sprintf(name,"Output Machine %d",numparam);
			} else if (numparam >=8 && numparam<16) {
				sprintf(name,"Note Offset %d",numparam-8);
			}
			else name[0] = '\0';
		}

		void DuplicatorMac::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if ( numparam < 8) { minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if ( numparam < 16) { minval = -48; maxval = 48; }
		}

		int DuplicatorMac::GetParamValue(int numparam)
		{
			if (numparam >=0 && numparam<8)
			{
				return macOutput[numparam];
			} else if (numparam >=8 && numparam <16) {
				return noteOffset[numparam-8];
			}
			else return 0;
		}

		void DuplicatorMac::GetParamValue(int numparam, char *parVal)
		{
			if (numparam >=0 && numparam <8)
			{
				if ((macOutput[numparam] != -1 ) &&( song()->_pMachine[macOutput[numparam]] != NULL))
				{
					sprintf(parVal,"%X -%s", macOutput[numparam], song()->_pMachine[macOutput[numparam]]->GetEditName().c_str());
				}
				else if (macOutput[numparam] != -1) sprintf(parVal,"%X (none)",macOutput[numparam]);
				else sprintf(parVal,"(disabled)");

			} else if (numparam >=8 && numparam <16) {
				char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
				sprintf(parVal,"%s%d",notes[(noteOffset[numparam-8]+60)%12],(noteOffset[numparam-8]+60)/12);
			}
			else parVal[0] = '\0';
		}

		bool DuplicatorMac::SetParameter(int numparam, int value)
		{
			if (numparam >=0 && numparam<8)
			{
				macOutput[numparam]=value;
				return true;
			} else if (numparam >=8 && numparam<16) {
				noteOffset[numparam-8]=value;
				return true;
			}
			else return false;
		}

		int DuplicatorMac::GenerateAudio( int numSamples )
		{
			_worked = true;
			return numSamples;
		}

		bool DuplicatorMac::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(macOutput);
			pFile->Read(noteOffset);
			return true;
		}

		void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof macOutput + sizeof noteOffset);
			pFile->Write(size);
			pFile->Write(macOutput);
			pFile->Write(noteOffset);
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Master

		std::string Master::_psName = "Master";

		float * Master::_pMasterSamples = 0;

		Master::Master(int id, Song* song)
			:
		Machine(0, MACHMODE_MASTER, id, song),
			sampleCount(0),
			decreaseOnClip(false)
		{
    		  _outDry = 256;
			_audiorange = 32768.0f;
			_editName = "Master";
		}

        Master* Master::clone() const {
          return new Master(*this);
        }

		Master::~Master() 
		{
		}

		void Master::Stop()
		{
			_clip = false;
			sampleCount = 0;
		}

		void Master::Init()
		{
			Machine::Init();
			sampleCount = 0;
			currentpeak=0.0f;
			peaktime=1;
			_lMax = 1;
			_rMax = 1;
			vuupdated = false;
			_clip = false;
		}

		void Master::Tick(int channel, const PatternEvent & data )
		{
			if ( data.note() == PatternCmd::SET_VOLUME )
			{
				_outDry = data.parameter();
			}
		}

		int Master::GenerateAudio( int numSamples )
		{
			float mv = CValueMapper::Map_255_1(_outDry);
		
			float *pSamples = _pMasterSamples;
			float *pSamplesL = _pSamplesL;
			float *pSamplesR = _pSamplesR;

			if(vuupdated) 
			{ 
				_lMax *= 0.5; 
				_rMax *= 0.5; 
			}
			int i = numSamples;
			if(decreaseOnClip)
			{
				do
				{
					// Left channel
					if(std::fabs(*pSamples = *pSamplesL = *pSamplesL * mv) > _lMax)
					{
						_lMax = fabsf(*pSamplesL);
					}
					if(*pSamples > 32767.0f)
					{
						_outDry = f2i((float)_outDry * 32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesL = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = f2i((float)_outDry * -32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesL = -32767.0f; 
					}
					pSamples++;
					pSamplesL++;
					// Right channel
					if(std::fabs(*pSamples = *pSamplesR = *pSamplesR * mv) > _rMax)
					{
						_rMax = fabsf(*pSamplesR);
					}
					if(*pSamples > 32767.0f)
					{
						_outDry = f2i((float)_outDry * 32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesR = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = f2i((float)_outDry * -32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesR = -32767.0f; 
					}
					pSamples++;
					pSamplesR++;
				}
				while (--i);
			}
			else
			{
				do
				{
					// Left channel
					if(std::fabs( *pSamples++ = *pSamplesL = *pSamplesL * mv) > _lMax)
					{
						_lMax = fabsf(*pSamplesL);
					}
					pSamplesL++;
					// Right channel
					if(std::fabs(*pSamples++ = *pSamplesR = *pSamplesR * mv) > _rMax)
					{
						_rMax = fabsf(*pSamplesR);
					}
					pSamplesR++;
				}
				while (--i);
			}
			if(_lMax > 32767.0f)
			{
				_clip=true;
				_lMax = 32767.0f; //_LMAX = 32768;
			}
			else if (_lMax < 1.0f) { _lMax = 1.0f; /*_LMAX = 1;*/ }
			//else _LMAX = Dsp::F2I(_lMax);
			if(_rMax > 32767.0f)
			{
				_clip=true;
				_rMax = 32767.0f; //_RMAX = 32768;
			}
			else if(_rMax < 1.0f) { _rMax = 1.0f; /*_RMAX = 1;*/ }

			if( _lMax > currentpeak ) currentpeak = _lMax;
			if( _rMax > currentpeak ) currentpeak = _rMax;

			sampleCount+=numSamples;
			_worked = true;
			return numSamples;
		}

		bool Master::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(_outDry);
			pFile->Read(decreaseOnClip);
			return true;
		};

		void Master::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof _outDry + sizeof decreaseOnClip);
			pFile->Write(size);
			pFile->Write(_outDry);
			pFile->Write(decreaseOnClip);
		};


	}
}
