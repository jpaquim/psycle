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
#include "machine.h"
#include "global.h"
#include "dsp.h"
#include "configuration.h"
#include "deserializer.h"
#include "serializer.h"
#include "plugin.h"


char* Master::_psName = "Master";
char* Dummy::_psName = "DummyPlug";
char* DuplicatorMac::_psName = "Dupe it!";
char* Mixer::_psName = "Mixer";

#define MAX_SCOPE_BANDS 128
#define SCOPE_BUF_SIZE 4096
#define SCOPE_SPEC_SAMPLES	256


using namespace dsp;

Machine::Machine() :/* crashed_(),
			 fpu_exception_mask_(),*/
  _macIndex(0)
  , _type(MACH_UNDEFINED)
  , _mode(MACHMODE_UNDEFINED)
  , _bypass(false)
  , _mute(false)
  , _waitingForSound(false)
  , _stopped(false)
  , _worked(false)
  , _pSamplesL(0)
  , _pSamplesR(0)
  , _lVol(0)
  , _rVol(0)
  , _panning(0)
  , _x(0)
  , _y(0)
  , _numPars(0)
  , _nCols(1)
  , _numInputs(0)
  , _numOutputs(0)
  , TWSSamples(0)
  , TWSActive(false)
  , _volumeCounter(0.0f)
  , _volumeDisplay(0)
  , _volumeMaxDisplay(0)
  , _volumeMaxCounterLife(0)
  , _pScopeBufferL(0)
  , _pScopeBufferR(0)
  , _scopeBufferIndex(0)
  , _scopePrevNumSamples(0)
  {
    _editName[0] = '\0';
    _pSamplesL = new float[STREAM_SIZE];
    _pSamplesR = new float[STREAM_SIZE];
    // Clear machine buffer samples
    for (int c=0; c<STREAM_SIZE; c++)
    {
      _pSamplesL[c] = 0;
      _pSamplesR[c] = 0;
    }
    for (int c = 0; c<MAX_TRACKS; c++)
    {
       TriggerDelay[c]._cmd = 0;
       TriggerDelayCounter[c]=0;
       RetriggerRate[c]=256;
       ArpeggioCount[c]=0;
    }
    for (int c = 0; c<MAX_TWS; c++)
    {
       TWSInst[c] = 0;
       TWSDelta[c] = 0;
       TWSCurrent[c] = 0;
       TWSDestination[c] = 0;
    }

    for (int i = 0; i<MAX_CONNECTIONS; i++)
    {
       _inputMachines[i]=-1;
       _outputMachines[i]=-1;
       _inputConVol[i]=0.0f;
       _wireMultiplier[i]=0.0f;
       _connection[i]=false;
       _inputCon[i]=false;
    }
}


Machine::~Machine()
{
}

void Machine::Init( )
{
 // Standard gear initalization
 _cpuCost = 0;
 _wireCost = 0;
 _mute = false;
 _stopped = false;
 _bypass = false;
 _waitingForSound = false;
 // Centering volume and panning
 SetPan(64);
 // Clearing connections
 for(int i=0; i<MAX_CONNECTIONS; i++)
 {
   _inputConVol[i] = 1.0f;
   _wireMultiplier[i] = 1.0f;
   _connection[i] = false;
   _inputCon[i] = false;
 }
 _numInputs = 0;
 _numOutputs = 0;
}

void Machine::SetPan( int newPan )
{
  if (newPan < 0)
  {
    newPan = 0;
  }
  if (newPan > 128)
  {
    newPan = 128;
  }
  _rVol = newPan * 0.015625f;
  _lVol = 2.0f-_rVol;
  if (_lVol > 1.0f)
  {
    _lVol = 1.0f;
  }
  if (_rVol > 1.0f)
  {
    _rVol = 1.0f;
  }
  _panning = newPan;
}

void Machine::PreWork( int numSamples )
{
  _worked = false;
  _waitingForSound= false;
  //CPUCOST_INIT(cost);
  if (_pScopeBufferL && _pScopeBufferR)
  {
     float *pSamplesL = _pSamplesL;
     float *pSamplesR = _pSamplesR;
     int i = _scopePrevNumSamples;
     while (i > 0) {
        if (i+_scopeBufferIndex >= SCOPE_BUF_SIZE)
        {
           memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1)*sizeof(float));
           memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1)*sizeof(float));
           pSamplesL+=(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
           pSamplesR+=(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
           i -= (SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
           _scopeBufferIndex = 0;
        } else {
          memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,i*sizeof(float));   
          memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,i*sizeof(float));   
          _scopeBufferIndex += i;
          i = 0;
        }
    }
  }
  _scopePrevNumSamples=numSamples;
  dsp::Clear(_pSamplesL, numSamples);
  dsp::Clear(_pSamplesR, numSamples);
  //CPUCOST_CALC(cost, numSamples);
  //_cpuCost = cost;
//  _wireCost+= cost;
}

void Machine::InitWireVolume( MachineType mType, int wireIndex, float value )
{
  if ( mType == MACH_VST || mType == MACH_VSTFX )
  {
    if (_type == MACH_VST || _type == MACH_VSTFX ) // VST to VST, no need to convert.
    {
      _inputConVol[wireIndex] = value;
      _wireMultiplier[wireIndex] = 1.0f;
    }
    else // VST to native, multiply
    {
      _inputConVol[wireIndex] = value*32768.0f;
      _wireMultiplier[wireIndex] = 0.000030517578125f;
    }
  }
  else if ( _type == MACH_VST || _type == MACH_VSTFX ) // native to VST, divide.
  {
    _inputConVol[wireIndex] = value*0.000030517578125f;
    _wireMultiplier[wireIndex] = 32768.0f;
  } else  // native to native, no need to convert.
  {
    _inputConVol[wireIndex] = value;
    _wireMultiplier[wireIndex] = 1.0f;
  }
  // The reason of the conversions in the case of MACH_VST is because VST's output wave data
  // in the range -1.0 to +1.0, while native and internal output at -32768.0 to +32768.0
  // Initially (when the format was made), Psycle did convert this in the "Work" function,
  // but since it already needs to multiply the output by inputConVol, I decided to remove
  // that extra conversion and use directly the volume to do so.

}

bool Machine::SetDestWireVolume(int srcIndex, int WireIndex,float value)
{
  // Get reference to the destination machine
  if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
  Machine *_pDstMachine = Global::pSong()->_pMachine[_outputMachines[WireIndex]];

  if (_pDstMachine)
  {
    //if ( value == 255 ) value =256; // FF = 255
    //const float invol = CValueMapper::Map_255_1(value); // Convert a 0..256 value to a 0..1.0 value

    int c;
    if ( (c = _pDstMachine->FindInputWire(srcIndex)) != -1)
    {
       _pDstMachine->SetWireVolume(c,value);
       return true;
    }
  }
  return false;
}

bool Machine::GetDestWireVolume(int srcIndex, int WireIndex,float &value)
{
  // Get reference to the destination machine
  if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
  Machine *_pDstMachine = Global::pSong()->_pMachine[_outputMachines[WireIndex]];

  if (_pDstMachine)
  {
    int c;
    if ( (c = _pDstMachine->FindInputWire(srcIndex)) != -1)
    {
       //float val;
       _pDstMachine->GetWireVolume(c,value);
       //value = f2i(val*256.0f);
       return true;
    }
  }
  return false;
}


int Machine::FindInputWire(int macIndex)
{
  for (int c=0; c<MAX_CONNECTIONS; c++)
  {
    if (_inputCon[c])
    {
      if (_inputMachines[c] == macIndex)
      {
        return c;
      }
    }
  }
  return -1;
}

int Machine::FindOutputWire(int macIndex)
{
  for (int c=0; c<MAX_CONNECTIONS; c++)
  {
    if (_connection[c])
    {
      if (_outputMachines[c] == macIndex)
      {
        return c;
      }
    }
  }
  return -1;
}

typedef unsigned int UINT;

bool Machine::LoadSpecificChunk(DeSerializer* pFile, int version)
{
  UINT size;
  size = pFile->getInt();
  UINT count;
  count = pFile->getInt();
  for (UINT i = 0; i < count; i++) {
   int temp = pFile->getInt();
   SetParameter(i,temp);
  }
  pFile->skip(size-sizeof(count)-(count*sizeof(int)));
  return true;
};


//// Dummy

Dummy::Dummy(int index)
{
  _macIndex = index;
  _type = MACH_DUMMY;
  _mode = MACHMODE_FX;
  sprintf(_editName, "Dummy");
}

void Dummy::Work(int numSamples)
{
  Machine::Work(numSamples);
  CPUCOST_INIT(cost);
  Machine::SetVolumeCounter(numSamples);
  if ( Global::pConfig()->autoStopMachines )
  {
    //Machine::SetVolumeCounterAccurate(numSamples);
    if (_volumeCounter < 8.0f) {
      _volumeCounter = 0.0f;
      _volumeDisplay = 0;
      _stopped = true;
    }
  }
  //else Machine::SetVolumeCounter(numSamples);
//  CPUCOST_CALC(cost, numSamples);
  _cpuCost += cost;
  _worked = true;
}

bool Dummy::LoadSpecificChunk(DeSerializer* pFile, int version)
{
  UINT size;
  size = pFile->getInt(); // size of this part params to load
  pFile->skip(size);
  return true;
};

void Machine::Work( int numSamples )
{
 _waitingForSound=true;
 for (int i=0; i<MAX_CONNECTIONS; i++)
 {
   if (_inputCon[i])
   {
     Machine* pInMachine = Global::pSong()->_pMachine[_inputMachines[i]];
     if (pInMachine)
     {
        /*
         * Change the sound routing to understand what a feedback loop is,
         * creating a special type of wire that will have a buffer which will give as output,
         * and which will be (internally) connected to master, 
         * to fill again the buffer once all the other machines have done its job.
        */
        if (!pInMachine->_worked && !pInMachine->_waitingForSound)
        {
          {
            #if PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS
             processor::fpu::exception_mask fpu_exception_mask(pInMachine->fpu_exception_mask()); 
            // (un)masks fpu exceptions in the current scope
            #endif
            pInMachine->Work(numSamples);
          }
          /*
          This could be a different Undenormalize funtion, using the already calculated
          "_volumeCounter".Note: It needs that muted&|bypassed machines set the variable correctly.
          if(pInMachine->_volumeCounter*_inputConVol[i] < 0.004f) // this gives for 24bit depth.
          {
            std::memset(pInMachine->_pSamplesL,0,numSamples*sizeof(float));
            std::memset(pInMachine->_pSamplesR,0,numSamples*sizeof(float));
          }
          */
          pInMachine->_waitingForSound = false;
        }
        if(!pInMachine->_stopped) _stopped = false;
        if(!_mute && !_stopped)
        {
          CPUCOST_INIT(wcost);
          dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
          dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
//          CPUCOST_CALC(wcost,numSamples);
          _wireCost+=wcost;
        }
      }
    }
  }
  CPUCOST_INIT(wcost);
  dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
  //CPUCOST_CALC(wcost,numSamples);
  _wireCost+=wcost;
}


//



float * Master::_pMasterSamples = 0;

Master::Master(int index)
{
   _macIndex = index;
   sampleCount = 0;
   _outDry = 256;
   decreaseOnClip=false;
   _type = MACH_MASTER;
   _mode = MACHMODE_MASTER;
   sprintf(_editName, "Master");
}

void Master::Init(void)
{
  Machine::Init();
  sampleCount = 0;
  //_LMAX = 1; // Min value should NOT be zero, because we use a log10() to calculate the vu-meter's value.
  //_RMAX = 1;
  currentpeak=0.0f;
  peaktime=1;
  _lMax = 1;
  _rMax = 1;
  vuupdated = false;
  _clip = false;
}

void Master::Work(int numSamples)
{
  Machine::Work(numSamples);
  CPUCOST_INIT(cost);
  //if(!_mute)
  //{
  float mv = CValueMapper::Map_255_1(_outDry);

  float *pSamples = _pMasterSamples;
  float *pSamplesL = _pSamplesL;
  float *pSamplesR = _pSamplesR;

  //_lMax -= numSamples*8;
  //_rMax -= numSamples*8;
  //_lMax *= 0.5;
  //_rMax *= 0.5;

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
				//else _RMAX = Dsp::F2I(_rMax);
				if( _lMax > currentpeak ) currentpeak = _lMax;
				if( _rMax > currentpeak ) currentpeak = _rMax;
			//}
			sampleCount+=numSamples;
//			CPUCOST_CALC(cost, numSamples);
			_cpuCost += cost;
			_worked = true;
		}

bool Master::LoadSpecificChunk(DeSerializer* pFile, int version) {
   UINT size;
   size    = pFile->getInt(); // size of this part params to load
   _outDry = pFile->getInt();
   decreaseOnClip = pFile->getByte();
   return true;
};
/*
		void Master::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof _outDry + sizeof decreaseOnClip;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&_outDry,sizeof _outDry);
			pFile->Write(&decreaseOnClip, sizeof decreaseOnClip); 
		};*/



/*bool Dummy::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Skip(size);
			return true;
		};
		
*/


		// NoteDuplicator
		DuplicatorMac::DuplicatorMac(int index)
		{
			_macIndex = index;
			_numPars = 16;
			_nCols = 2;
			_type = MACH_DUPLICATOR;
			_mode = MACHMODE_GENERATOR;
			bisTicking = false;
			strcpy(_editName, "Dupe it!");
			for (int i=0;i<8;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
			}
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
		void DuplicatorMac::Tick( int channel,PatternEntry* pData)
		{
			if ( !_mute && !bisTicking)
			{
				bisTicking=true;
				for (int i=0;i<8;i++)
				{
					PatternEntry pTemp = *pData;
					if ( pTemp._note < 120 )
					{
						pTemp._note+=noteOffset[i];
					}
					if (macOutput[i] != -1 && Global::pSong()->_pMachine[macOutput[i]] != NULL 
						&& Global::pSong()->_pMachine[macOutput[i]] != this) Global::pSong()->_pMachine[macOutput[i]]->Tick(channel,&pTemp);
				}
			}
			bisTicking=false;
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
				if ((macOutput[numparam] != -1 ) &&( Global::pSong()->_pMachine[macOutput[numparam]] != NULL))
				{
					sprintf(parVal,"%X -%s",macOutput[numparam],Global::pSong()->_pMachine[macOutput[numparam]]->_editName);
				}else if (macOutput[numparam] != -1) sprintf(parVal,"%X (none)",macOutput[numparam]);
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

		void DuplicatorMac::Work(int numSamples)
		{
			_worked = true;
		}
		bool DuplicatorMac::LoadSpecificChunk(DeSerializer* pFile, int version)
		{
			UINT size;
			size = pFile->getInt(); // size of this part params to load
			pFile->read(reinterpret_cast<char*>(&macOutput),sizeof(macOutput));
			pFile->read(reinterpret_cast<char*>(&noteOffset),sizeof(noteOffset));
			return true;
		}

		/*void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof macOutput+ sizeof noteOffset;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&macOutput,sizeof macOutput);
			pFile->Write(&noteOffset,sizeof noteOffset);
		}*/

		// Mixer

		Mixer::Mixer(int index)
		{
			_macIndex = index;
			_numPars = 255;
			_type = MACH_MIXER;
			_mode = MACHMODE_FX;
			sprintf(_editName, "Mixer");
		}

		void Mixer::Init(void)
		{
			Machine::Init();
			for (int j=0;j<MAX_CONNECTIONS;j++)
			{
				_sendGrid[j][mix]=1.0f;
				for (int i=send0;i<sendmax;i++)
				{
					_sendGrid[j][i]=0.0f;
				}
				_send[j]=0;
				_sendVol[j]=1.0f;
				_sendVolMulti[j]=1.0f;
				_sendValid[j]=false;
			}
		}

		void Mixer::Work(int numSamples)
		{
			// Step One, do the usual work, except mixing all the inputs to a single stream.
			Machine::WorkNoMix(numSamples);
			// Step Two, prepare input signals for the Send Fx, and make them work
			FxSend(numSamples);
			// Step Three, Mix the returns of the Send Fx's with the leveled input signal
			if(!_mute && !_stopped )
			{
				CPUCOST_INIT(cost);
				Mix(numSamples);
				Machine::SetVolumeCounter(numSamples);
				if ( Global::pConfig()->autoStopMachines )
				{
					if (_volumeCounter < 8.0f)
					{
						_volumeCounter = 0.0f;
						_volumeDisplay = 0;
						_stopped = true;
					}
					else _stopped = false;
				}
//				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
			}

			CPUCOST_INIT(wcost);
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
//			CPUCOST_CALC(wcost,numSamples);
			_wireCost+=wcost;
			_worked = true;
		}

		void Mixer::FxSend(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::pSong()->_pMachine[_send[i]];
					if (pSendMachine)
					{
						if (!pSendMachine->_worked && !pSendMachine->_waitingForSound)
						{ 
							// Mix all the inputs and route them to the send fx.
							CPUCOST_INIT(cost);
							for (int j=0; j<MAX_CONNECTIONS; j++)
							{
								if (_inputCon[j])
								{
									Machine* pInMachine = Global::pSong()->_pMachine[_inputMachines[j]];
									if (pInMachine)
									{
										if(!_mute && !_stopped && _sendGrid[j][send0+i]!= 0.0f)
										{
											dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[j]*_sendGrid[j][send0+i]);
											dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[j]*_sendGrid[j][send0+i]);
										}
									}
								}
							}
//							CPUCOST_CALC(cost, numSamples);
							_cpuCost += cost;

							// tell the FX to work, now that the input is ready.
							{
#if PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS
								processor::fpu::exception_mask fpu_exception_mask(pSendMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
#endif
								pSendMachine->Work(numSamples);
							}
							CPUCOST_INIT(cost2);

							pSendMachine->_waitingForSound = false;
							dsp::Clear(_pSamplesL, numSamples);
							dsp::Clear(_pSamplesR, numSamples);
//							CPUCOST_CALC(cost2, numSamples);
							_cpuCost += cost2;

						}
						if(!pSendMachine->_stopped) _stopped = false;
					}
				}
			}
		}
		void Mixer::Mix(int
 numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::pSong()->_pMachine[_send[i]];
					if (pSendMachine)
					{
						if(!_mute && !_stopped)
						{
							dsp::Add(pSendMachine->_pSamplesL, _pSamplesL, numSamples, pSendMachine->_lVol*_sendVol[i]);
							dsp::Add(pSendMachine->_pSamplesR, _pSamplesR, numSamples, pSendMachine->_rVol*_sendVol[i]);
						}
					}
				}
			}
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_inputCon[i])
				{
					Machine* pInMachine = Global::pSong()->_pMachine[_inputMachines[i]];
					if (pInMachine)
					{
						if(!_mute && !_stopped && _sendGrid[i][mix] != 0.0f)
						{
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]*_sendGrid[i][mix]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]*_sendGrid[i][mix]);
						}
					}
				}
			}
		}

		int Mixer::GetNumCols()
		{
			int cols=0;
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_inputCon[i]) cols++;
			}
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i]) cols++;
			}
			return cols==0?1:cols;
		}

		void Mixer::GetParamName(int numparam,char *name)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) { name[0] = '\0'; return; }
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					if ( send == mix )sprintf(name,"Channel %d - Mix",channel+1);
					else sprintf(name,"Channel %d - Send %d",channel+1,send);
				}
				else name[0] = '\0';
			}
			else if  ( send == 0){ name[0] = '\0'; return; }
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) sprintf(name,"Input level Ch %d",send+send0);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && _sendValid[send]) sprintf(name,"Input level Fx %d",send+send0);
				else name[0] = '\0';
			}
		}
		int Mixer::GetParamValue(int numparam)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) return 0;
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					return (int)(_sendGrid[channel][send]*100.0f);
				}
				else return 0;
			}
			else if  ( send == 0) return 0;
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) return (int)(_inputConVol[send]*_wireMultiplier[send]*100.0f);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && _sendValid[send]) return (int)(_sendVol[send]*_sendVolMulti[send]*100.0f);
				else return 0;
			}
		}
		void Mixer::GetParamValue(int numparam, char *parVal)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) { parVal[0] = '\0'; return; }
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					sprintf(parVal,"%.0f%%",_sendGrid[channel][send]*100.0f);
				}
				else  parVal[0] = '\0';
			}
			else if  ( send == 0) { parVal[0] = '\0'; return; }
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) sprintf(parVal,"%.0f%%",_inputConVol[send]*_wireMultiplier[send]*100.0f);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && _sendValid[send]) sprintf(parVal,"%.0f%%",_sendVol[send]*_sendVolMulti[send]*100.0f);
				else parVal[0] = '\0';
			}
		}
		bool Mixer::SetParameter(int numparam, int value)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) return false;
			if ( value>100 ) value=100;
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					_sendGrid[channel][send]=value/100.0f;
					return true;
				}
				else return false;
			}
			else if  ( send == 0) return false;
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS)
				{
					SetWireVolume(send,value/100.0f);
					return true;
				}
				else if ( channel == 0x0F && send < MAX_CONNECTIONS) 
				{
					_sendVol[send]= value / (_sendVolMulti[send] * 100.0f);
					return true;
				}
				else return false;
			}
		}

void Machine::WorkNoMix(int numSamples)
{
  _waitingForSound=true;
  for (int i=0; i<MAX_CONNECTIONS; i++)
  {
     if (_inputCon[i])
     {
       Machine* pInMachine = Global::pSong()->_pMachine[_inputMachines[i]];
       if (pInMachine)
       {
         if (!pInMachine->_worked && !pInMachine->_waitingForSound)
         {
             {
               pInMachine->Work(numSamples);
             }
             pInMachine->_waitingForSound = false;
         }
         if(!pInMachine->_stopped) _stopped = false;
        }
      }
   }
}

Machine * Machine::LoadFileChunk( DeSerializer * pFile, int index, int version, bool fullopen )
{
   // assume version 0 for now
   bool bDeleted(false);
   Machine* pMachine;
   MachineType type;//,oldtype;
   char dllName[256];
   pFile->read(reinterpret_cast<char*>(&type),sizeof(type));
   pFile->readString(dllName,256);

   switch (type)
   {
     case MACH_MASTER:
        if ( !fullopen ) pMachine = new Dummy(index); else pMachine = new Master(index);
     break;
     case MACH_SAMPLER:
         pMachine = new Dummy(index);
         //if ( !fullopen ) pMachine = new Dummy(index); else pMachine = new Sampler(index);
     break;
     case MACH_XMSAMPLER:
        pMachine = new Dummy(index);
        //if ( !fullopen ) pMachine = new Dummy(index); else pMachine = new XMSampler(index);
     break;
     case MACH_DUPLICATOR:
        if ( !fullopen ) pMachine = new Dummy(index); else pMachine = new DuplicatorMac(index);
     break;
     case MACH_PLUGIN: {
        if(!fullopen) pMachine = new Dummy(index); else
        {
          pMachine = new Dummy(index);
          type = MACH_DUMMY;
          Plugin * p;
          pMachine = p = new Plugin(index);
          if(!p->LoadDll(dllName)) {
              std::cout << "creating Dummy" << std::endl; fflush(stdout);
              //char sError[MAX_PATH + 100];
             // sprintf(sError,"Replacing Native plug-in \"%s\" with Dummy.",dllName);
             // MessageBox(NULL,sError, "Loading Error", MB_OK);
              pMachine = new Dummy(index);
              type = MACH_DUMMY;
              delete p;
              bDeleted = true;
          }
        }
     }
     break;
    case MACH_VST: {
        if(!fullopen) pMachine = new Dummy(index); else
        {
          pMachine = new Dummy(index);
          type = MACH_DUMMY;
           /*vst::instrument * p;
           pMachine = p = new vst::instrument(index);
           if(!p->LoadDll(dllName)) {
             char sError[MAX_PATH + 100];
             sprintf(sError,"Replacing VST Generator plug-in \"%s\" with Dummy.",dllName);
             MessageBox(NULL,sError, "Loading Error", MB_OK);
             pMachine = new Dummy(index);
             type = MACH_DUMMY;
             delete p;
             bDeleted = true;
           }*/
        }
    }
    break;
    case MACH_VSTFX: {
       if(!fullopen) pMachine = new Dummy(index); else {
        pMachine = new Dummy(index);
        type = MACH_DUMMY;
        /* vst::fx * p;
         pMachine = p = new vst::fx(index);
         if(!p->LoadDll(dllName)) {
             char sError[MAX_PATH + 100];
             sprintf(sError,"Replacing VST Effect plug-in \"%s\" with Dummy.",dllName);
             MessageBox(NULL,sError, "Loading Error", MB_OK);
             pMachine = new Dummy(index);
             type = MACH_DUMMY;
             delete p;
             bDeleted = true;
         }*/
       }
     }
     break;
    default:
      /*if (type != MACH_DUMMY ) MessageBox(0, "Please inform the devers about this message: unknown kind of machine while loading new file format", "Loading Error", MB_OK | MB_ICONERROR);*/
      pMachine = new Dummy(index);
      break;
    }
    pMachine->Init();
    pMachine->_type = type;
    pMachine->_bypass     = pFile->getByte();
    pMachine->_mute       = pFile->getByte();
    pMachine->_panning    = pFile->getInt();
    pMachine->_x          = pFile->getInt();
    pMachine->_y          = pFile->getInt();
    pMachine->_numInputs  = pFile->getInt(); // number of Incoming connections
    pMachine->_numOutputs = pFile->getInt(); // number of Outgoing connections

    for(int i = 0; i < MAX_CONNECTIONS; i++) {
      // Incoming connections Machine number
        pFile->read(&pMachine->_inputMachines[i],sizeof(pMachine->_inputMachines[i]));
      // Outgoing connections Machine number
        pFile->read(&pMachine->_outputMachines[i],sizeof(pMachine->_outputMachines[i]));
      // Incoming connections Machine vol
        pFile->read(&pMachine->_inputConVol[i],sizeof(pMachine->_inputConVol[i]));
      // Value to multiply _inputConVol[] to have a 0.0...1.0 range
        pFile->read(&pMachine->_wireMultiplier[i],sizeof(pMachine->_wireMultiplier[i]));
      // Outgoing connections activated
        pFile->read(&pMachine->_connection[i],sizeof(pMachine->_connection[i]));
      // Incoming connections activated
        pFile->read(&pMachine->_inputCon[i],sizeof(pMachine->_inputCon[i]));
    }
    pFile->readString(pMachine->_editName,32);
    if(bDeleted) {
       char buf[34];
       sprintf(buf,"X %s",pMachine->_editName);
       buf[31]=0;
       strcpy(pMachine->_editName,buf);
    }
    if(!fullopen) return pMachine;
      if(!pMachine->LoadSpecificChunk(pFile,version)) {
        int MAX_PATH = 256;
        char sError[MAX_PATH + 100];
        sprintf(sError,"Missing or Corrupted Machine Specific Chunk \"%s\" - replacing with Dummy.",dllName);
        //MessageBox(NULL,sError, "Loading Error", MB_OK);
        Machine* p = new Dummy(index);
        p->Init();
        p->_type=MACH_DUMMY;
        p->_mode=pMachine->_mode;
        p->_bypass=pMachine->_bypass;
        p->_mute=pMachine->_mute;
        p->_panning=pMachine->_panning;
        p->_x=pMachine->_x;
        p->_y=pMachine->_y;
        p->_numInputs=pMachine->_numInputs;   // number of Incoming connections
        p->_numOutputs=pMachine->_numOutputs; // number of Outgoing connections
        for(int i = 0; i < MAX_CONNECTIONS; i++)
        {
          p->_inputMachines[i]=pMachine->_inputMachines[i];
          p->_outputMachines[i]=pMachine->_outputMachines[i];
          p->_inputConVol[i]=pMachine->_inputConVol[i];
          p->_wireMultiplier[i]=pMachine->_wireMultiplier[i];
          p->_connection[i]=pMachine->_connection[i];
          p->_inputCon[i]=pMachine->_inputCon[i];
        }

        // dummy name goes here
        sprintf(p->_editName,"X %s",pMachine->_editName);
        p->_numPars=0;
        delete pMachine;
        pMachine=p;
      }
    if(index < MAX_BUSES) {
      pMachine->_mode = MACHMODE_GENERATOR;
      /*if(pMachine->_x > Global::pSong()->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width)
        pMachine->_x = Global::pSong()->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
        if(pMachine->_y > Global::pSong()->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height)
        pMachine->_y = Global::pSong()->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;*/
    } else if (index < MAX_BUSES*2) {
       pMachine->_mode = MACHMODE_FX;
       /*if(pMachine->_x > Global::pSong()->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width)
       pMachine->_x = Global::pSong()->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
       if(pMachine->_y > Global::pSong()->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height)
       pMachine->_y = Global::pSong()->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;*/
    } else {
        pMachine->_mode = MACHMODE_MASTER;
        /*if(pMachine->_x > Global::pSong()->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width)
        pMachine->_x = Global::pSong()->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
        if(pMachine->_y > Global::pSong()->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height)
        pMachine->_y = Global::pSong()->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;*/
     }
   pMachine->SetPan(pMachine->_panning);
   return pMachine;
}

 bool Mixer::LoadSpecificChunk(DeSerializer* pFile, int version)
 {
    UINT size;
    size = pFile->getInt();
    pFile->read((char*)&_sendGrid,sizeof(_sendGrid));
    pFile->read((char*)&_send,sizeof(_send));
    pFile->read((char*)&_sendVol,sizeof(_sendVol));
    pFile->read((char*)&_sendVolMulti,sizeof(_sendVolMulti));
    pFile->read((char*)&_sendValid,sizeof(_sendValid));
    return true;
 };


void Machine::SaveFileChunk(Serializer * pFile) {
  pFile->PutPChar(reinterpret_cast<char*>(&_type),sizeof(_type));
  //SaveDllName(pFile);
  pFile->PutBool(_bypass);
  pFile->PutBool(_mute);
  pFile->PutInt(_panning);
  pFile->PutInt(_x);
  pFile->PutInt(_y);
  pFile->PutInt(_numInputs);  // number of Incoming connections
  pFile->PutInt(_numOutputs); // number of Incoming connections

  for(int i = 0; i < MAX_CONNECTIONS; i++)
  {
     pFile->PutInt(_inputMachines[i]);   // Incoming connections Machine number
     pFile->PutInt(_outputMachines[i]); // Outgoing connections Machine number
     pFile->PutFloat(_inputConVol[i]);    // Incoming connections Machine vol
     pFile->PutFloat(_wireMultiplier[i]); // Value to multiply _inputConVol[] to have a 0.0...1.0 range
     pFile->PutBool(_connection[i]);       // Outgoing connections activated
     pFile->PutBool(_inputCon[i]);         // Incoming connections activated
  }
  pFile->PutString(_editName);
  //SaveSpecificChunk(pFile);
}


void Machine::SaveSpecificChunk(Serializer* pFile)
{
  UINT count = GetNumParams();
  UINT size = sizeof(count)+(count*sizeof(int));
  pFile->PutInt(size);
  pFile->PutInt(count);
  for(UINT i = 0; i < count; i++)
  {
    int temp = GetParamValue(i);
    pFile->PutInt(temp);
  }
}

void DuplicatorMac::SaveSpecificChunk(Serializer* pFile)
{
  UINT size = sizeof macOutput+ sizeof noteOffset;
  pFile->PutInt(size); // size of this part params to save
  pFile->PutPChar(reinterpret_cast<char*>(&macOutput),sizeof(macOutput));
  pFile->PutPChar(reinterpret_cast<char*>(&noteOffset),sizeof(noteOffset));
}


void Master::SaveSpecificChunk(Serializer* pFile)
{
  UINT size = sizeof _outDry + sizeof decreaseOnClip;
  pFile->PutInt(size);
  pFile->PutInt(_outDry);
  pFile->PutBool(decreaseOnClip);
};


void Mixer::SaveSpecificChunk(Serializer* pFile)
{
  UINT size = sizeof(_sendGrid) + sizeof(_send) + sizeof(_sendVol) + sizeof(_sendVolMulti) + sizeof(_sendValid);
  pFile->PutInt(size);
  pFile->PutPChar( (char*)&_sendGrid,sizeof(_sendGrid));
  pFile->PutPChar( (char*)&_send,sizeof(_send));
  pFile->PutPChar( (char*)&_sendVol,sizeof(_sendVol));
  pFile->PutPChar( (char*)&_sendVolMulti,sizeof(_sendVolMulti));
  pFile->PutPChar( (char*)&_sendValid,sizeof(_sendValid));
}

void Machine::SaveDllName( Serializer * pFile )
{
  pFile->PutBool(0);
}

