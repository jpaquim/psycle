#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
	#include "Machine.h"
	#include "Song.h"
//	#include "FileIO.h"
	#include "Configuration.h"
#else
	#include "Psycle2.h"
	#include "Machine.h"
	#include "Song.h"
//	#include "FileIO.h"
	#include "Configuration.h"

	extern CPsycleApp theApp;
#endif // _WINAMP_PLUGIN_


char* Master::_psName = "Master";
char* Dummy::_psName = "DummyPlug";
char* Gainer::_psName = "Gainer";
CIntMachParam Gainer::pars[] = {{"Reserved",0,0},{"Gain",0,1024}};
char* Sine::_psName = "PsychOsc AM";
CIntMachParam Sine::pars[] = {{"Reserved",0,0},{"OSC Frequency",0,128},{"OSC Glide Speed",0,128}, \
								{"LFO Frequency",0,128},{"LFO Amplitude",0,128}};
char* Distortion::_psName = "Distortion";
CIntMachParam Distortion::pars[] = {{"Reserved",0,0},{"Pos. Threshold",0,128},{"Positive Clamp",0,128}, \
								{"Neg. Threshold",0,128},{"Negative Clamp",0,128}};
char* Delay::_psName = "Dalay Delay";
CIntMachParam Delay::pars[] = {{"Reserved",0,0},{"Left Delay",1,MAX_DELAY_BUFFER-1},{"Left Feedback",-100,100}, \
								{"Right Delay",1,MAX_DELAY_BUFFER-1}, {"Right Feedback",-100,100},{"Dry",-256,256},{"Wet",-256,256}};
char* Flanger::_psName = "Flanger";
CIntMachParam Flanger::pars[] = {{"Reserved",0,0},{"Left Delay",0,1024},{"Left Amp",0,256},{"Left Speed",1,32768}, \
								{"Left Feeback",-100,100},{"Right Phase",0,256},{"Right Feedback",-100,100}, \
								{"Dry",-256,256},{"Wet",-256,256}};
char* Filter2p::_psName = "2p Filter";
CIntMachParam Filter2p::pars[] = {{"Reserved",0,0},{"Filter Type",0,1},{"Filter Cuttoff",0,256}, \
								{"Filter Ressonance",0,256},{"LFO Speed",0,32768},{"LFO Amplitude",0,256},{"LFO Phase",0,256}};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
Machine::Machine()
{
	wasVST = false;

	
	_numPars = 0;
#if !defined(_WINAMP_PLUGIN_)
	_volumeCounter = 0;
	_volumeDisplay = 0;
	_volumeMaxDisplay = 0;
	_volumeMaxCounterLife = 0;
#endif // _WINAMP_PLUGIN_

	_pSamplesL = new float[STREAM_SIZE];
	_pSamplesR = new float[STREAM_SIZE];
	
	// Clear machine buffer samples
	for (int c=0; c<STREAM_SIZE; c++)
	{
		_pSamplesL[c] = 0;
		_pSamplesR[c] = 0;
	}
	for (c = 0; c<MAX_TRACKS; c++)
	{
		TriggerDelay[c]._cmd = 0;
	}
}

Machine::~Machine()
{
	delete _pSamplesL;
	delete _pSamplesR;
}

void Machine::Init(void)
{
	// Standard gear initalization
	//
	_outDry = 256;
	_outWet = 64;
#if !defined(_WINAMP_PLUGIN_)
	_cpuCost = 0;
	_wireCost = 0;
#endif // _WINAMP_PLUGIN_
	_mute = false;
	_stopped = false;
	_bypass = false;
	_waitingForSound = false;

	// Centering volume and panning
	//
	SetPan(64);

	// Clearing connections
	//
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		_inputConVol[i] = 1.0f;
		_wireMultiplier[i] = 1.0f;
		_connection[i] = false;
		_inputCon[i] = false;
	}
	_numInputs = 0;
	_numOutputs = 0;
}

void Machine::SetPan(
	int newPan)
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
int Machine::FindInputWire(Machine* pDstMac,int macIndex)
{
	for (int c=0; c<MAX_CONNECTIONS; c++)
	{
		if (pDstMac->_inputCon[c])
		{
			if (pDstMac->_inputMachines[c] == macIndex)
			{
				return c;
			}
		}
	}
	return -1;
}
int Machine::FindOutputWire(Machine* pSrcMac,int macIndex)
{
	for (int c=0; c<MAX_CONNECTIONS; c++)
	{
		if (pSrcMac->_connection[c])
		{
			if (pSrcMac->_outputMachines[c] == macIndex)
			{
				return c;
			}
		}
	}
	return -1;
}

bool Machine::SetDestWireVolume(int srcIndex, int WireIndex,int value)
{
	// Get reference to the destination machine
	if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
	Machine *_pDstMachine = Global::_pSong->_pMachines[_outputMachines[WireIndex]];

	if ( value == 255 ) value =256; // FF = 255
	const float invol = value*0.00390625f; // Convert a 0..256 value to a 0..1.0 value
	
	int c;
	if ( (c = FindInputWire(_pDstMachine,srcIndex)) != -1)
	{
		_pDstMachine->SetWireVolume(c,invol);
		return true;
	}
	return false;
}
bool Machine::GetDestWireVolume(int srcIndex, int WireIndex,int &value)
{
	// Get reference to the destination machine
	if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
	Machine *_pDstMachine = Global::_pSong->_pMachines[_outputMachines[WireIndex]];
	
	int c;
	if ( (c = FindInputWire(_pDstMachine,srcIndex)) != -1)
	{
		float val;
		_pDstMachine->GetWireVolume(c,val);
		value = f2i(val*256.0f);
		return true;
	}
	
	return false;
}

void Machine::PreWork(int numSamples)
{
	_worked = false;
	_waitingForSound= false;
#if defined( _WINAMP_PLUGIN_)
	Dsp::Clear(_pSamplesL, numSamples);
	Dsp::Clear(_pSamplesR, numSamples);
#else
	CPUCOST_INIT(cost);
	Dsp::Clear(_pSamplesL, numSamples);
	Dsp::Clear(_pSamplesR, numSamples);
	CPUCOST_CALC(cost, numSamples);
//	_cpuCost = cost;
	_wireCost+= cost;
#endif // _WINAMP_PLUGIN_
}

// Each machine is expected to produce its output in its own
// _pSamplesX buffers.
//
void Machine::Work(int numSamples)
{
	_waitingForSound=true;
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (_inputCon[i])
		{
			Machine* pInMachine = Global::_pSong->_pMachines[_inputMachines[i]];
			if (!pInMachine->_worked && !pInMachine->_waitingForSound)
			{ 
				pInMachine->Work(numSamples);
				pInMachine->_waitingForSound=false;
			}
			if ( !pInMachine->_stopped ) _stopped=false;
			if ((!_mute) && ( !_stopped ))
			{
#if defined( _WINAMP_PLUGIN_)
				Dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
				Dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
#else
				CPUCOST_INIT(wcost);
				Dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
				Dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
				CPUCOST_CALC(wcost,numSamples);
				_wireCost+=wcost;
#endif // _WINAMP_PLUGIN_
			}
		}
	}
}

bool Machine::Load(
	RiffFile* pFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	pFile->Read(&_editName, sizeof(_editName));

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junk[0], sizeof(int)); // distPosClamp
	pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junk[0], sizeof(int)); // distNegClamp

	pFile->Read(&junk[0], sizeof(char)); // sinespeed
	pFile->Read(&junk[0], sizeof(char)); // sineglide
	pFile->Read(&junk[0], sizeof(char)); // sinevolume
	pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Read(&junk[0], sizeof(int)); // delayTimeL
	pFile->Read(&junk[0], sizeof(int)); // delayTimeR
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Read(&junk[0], sizeof(int)); // filterCutoff
	pFile->Read(&junk[0], sizeof(int)); // filterResonance
	pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Read(&junk[0], sizeof(int)); // filterLfophase
	pFile->Read(&junk[0], sizeof(int)); // filterMode

	return true;
}
#if !defined(_WINAMP_PLUGIN_)
bool Machine::Save(
	RiffFile* pFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

	pFile->Write(&_editName, sizeof(_editName));

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Write(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junk[0], sizeof(int)); // distPosClamp
	pFile->Write(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junk[0], sizeof(int)); // distNegClamp

	pFile->Write(&junk[0], sizeof(char)); // sinespeed
	pFile->Write(&junk[0], sizeof(char)); // sineglide
	pFile->Write(&junk[0], sizeof(char)); // sinevolume
	pFile->Write(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Write(&junk[0], sizeof(int)); // delayTimeL
	pFile->Write(&junk[0], sizeof(int)); // delayTimeR
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Write(&junk[0], sizeof(int)); // filterCutoff
	pFile->Write(&junk[0], sizeof(int)); // filterResonance
	pFile->Write(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Write(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Write(&junk[0], sizeof(int)); // filterLfophase
	pFile->Write(&junk[0], sizeof(int)); // filterMode

	return true;
}
#endif // ndef _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Dummy::Dummy()
{
	_numPars = 0;
	_type = MACH_DUMMY;
	_mode = MACHMODE_FX;
	sprintf(_editName, "Dummy");
}
void Dummy::Work(int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
	Machine::SetVolumeCounter(numSamples);
	if ( Global::pConfig->autoStopMachines )
	{
//		Machine::SetVolumeCounterAccurate(numSamples);
		if (_volumeCounter < 8)	{
			_volumeCounter = 0;
			_volumeDisplay = 0;
			_stopped = true;
		}
	}
//	else Machine::SetVolumeCounter(numSamples);
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
float* Master::_pMasterSamples = NULL;

Master::Master()
{
	_numPars = 0;
	decreaseOnClip=false;
	_type = MACH_MASTER;
	_mode = MACHMODE_MASTER;
	sprintf(_editName, "Master");
}

void Master::Init(void)
{
	Machine::Init();
#if !defined(_WINAMP_PLUGIN_)
//	_LMAX = 1; // Min value should NOT be zero, because we use a log10() to calculate the vu-meter's value.
//	_RMAX = 1;
	currentpeak=0.0f;
	peaktime=1;
	_lMax = 1;
	_rMax = 1;
	vuupdated = false;
	_clip = false;
#endif // ndef _WINAMP_PLUGIN
}

void Master::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN

//	if (!_mute)
//	{
		float const mv = (float)_outDry*0.0039062f;
		
		float *pSamples = _pMasterSamples;
		float *pSamplesL = _pSamplesL;
		float *pSamplesR = _pSamplesR;
		
		--pSamples;
		--pSamplesL;
		--pSamplesR;
		

#if defined(_WINAMP_PLUGIN_)
		int i = numSamples;
		do
		{
			++pSamples;
			++pSamplesL;
			++pSamplesR;

			*pSamples = *pSamplesL * mv;
			++pSamples;
			*pSamples = *pSamplesR * mv;
		}
		while (--i);
#else
//		_lMax -= numSamples*8;
//		_rMax -= numSamples*8;
//		_lMax *= 0.5;
//		_rMax *= 0.5;
		if ( vuupdated ) { _lMax *= 0.5; _rMax *= 0.5; }

		int i = numSamples;
		do
		{
			++pSamples;
			++pSamplesL;
			++pSamplesR;
			

			// Left channel
			if (fabs( *pSamples = *pSamplesL * mv) > _lMax)
			{
				_lMax = fabsf(*pSamples);
			}
			
			// Right channel
			++pSamples;
			if (fabs(*pSamples = *pSamplesR * mv) > _rMax)
			{
				_rMax = fabsf(*pSamples);
			}
		}
		while (--i);
		
		if (_lMax > 32768.0f)
		{
			_clip=true;
			if (decreaseOnClip) 
			{
				_outDry = f2i((float)_outDry * 32768.0f / _lMax);
			}
			_lMax = 32768.0f; //_LMAX = 32768;
		}
		else if (_lMax < 1.0f) { _lMax = 1.0f; /*_LMAX = 1;*/ }
//		else _LMAX = Dsp::F2I(_lMax);

		if (_rMax > 32768.0f)
		{
			_clip=true;
			if (decreaseOnClip) 
			{
				_outDry = f2i((float)_outDry * 32768.0f / _rMax);
			}
			_rMax = 32768.0f; //_RMAX = 32768;
		}
		else if (_rMax < 1.0f) { _rMax = 1.0f; /*_RMAX = 1;*/ }
//		else _RMAX = Dsp::F2I(_rMax);

		if ( _lMax > currentpeak ) currentpeak = _lMax;
		if ( _rMax > currentpeak ) currentpeak = _rMax;


#endif // _WINAMP_PLUGIN_
//	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // _WINAMP_PLUGIN_
	_worked = true;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Gainer::Gainer()
{
	_numPars = 2;
	_type = MACH_GAIN;
	_mode = MACHMODE_FX;
	sprintf(_editName, "Gainer");
}

void Gainer::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_
	
	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		float const wet = (float)_outWet*0.00390625f;
		float *pSamplesL = _pSamplesL;
		float *pSamplesR = _pSamplesR;
		int i = numSamples;
		
		--pSamplesL;
		--pSamplesR;
		do
		{
			*pSamplesL = *++pSamplesL*wet;
			*pSamplesR = *++pSamplesR*wet;
		}
		while (--i);
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

void Gainer::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == 121 || pData->_note == 122 )
	{
		SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
	}
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Sine::Sine()
{
	_numPars = 5;
	_type = MACH_SINE;
	_mode = MACHMODE_FX;
	sprintf(_editName, "Psychosc");
}

void Sine::Init(void)
{
	Machine::Init();

	_speed = 1.0f;
	_volume = 1.0f;
	_osc = 0.0f;
	_lfoOsc = 0.0f;
	
	_realSpeed = 1.0f;
	_glide = 0.00001f;
	_lfoSpeed = 0.01f;
	_lfoAmp = 1.0f;
	
	_sineSpeed = 50;
	_sineVolume = 128;
	_sineGlide = 50;
	_sineLfoSpeed = 50;
	_sineLfoAmp = 50;

	Update();
}

void Sine::GetParamValue(int numparam,char* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,"None"); break;
		case 1:
#if defined(_WINAMP_PLUGIN_)
			sprintf(parval, "%.1f Hz.", Global::pConfig->_samplesPerSec * (_speed/6.2831853f));
#else
			sprintf(parval, "%.1f Hz.", Global::pConfig->_pOutputDriver->_samplesPerSec * (_speed/6.2831853f));
#endif // _WINAMP_PLUGIN
			break;
		case 2:
			sprintf(parval,"%.1f%%",_glide*155038.76f);
			break;
		case 3:
#if defined(_WINAMP_PLUGIN_)
			sprintf(parval, "%.1f Hz.", Global::pConfig->_samplesPerSec * (_lfoSpeed/6.2831853f));
#else
			sprintf(parval, "%.1f Hz.", Global::pConfig->_pOutputDriver->_samplesPerSec * (_lfoSpeed/6.2831853f));
#endif // _WINAMP_PLUGIN_
			break;
		case 4:
			sprintf(parval,"%.1f%%",_lfoAmp*1550.3876f);
			break;
		default: strcpy(parval,"Out Of Range"); break;
	}
}

void Sine::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == 121 || pData->_note == 122 ) SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
}

void Sine::Update(void)
{
	_speed = (float)_sineSpeed*0.0025f;
	_glide = (float)_sineGlide*0.000005f;
	_lfoSpeed = _sineLfoSpeed*0.000025f;
	_lfoAmp = (float)_sineLfoAmp*0.0005f;
}

void Sine::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_

	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		float const rad = 6.28318530717958647692528676655901f;
		float mod_value;
		
		float *pSamplesL = _pSamplesL;
		float *pSamplesR = _pSamplesR;
		--pSamplesL;
		--pSamplesR;
		
		int i = numSamples;
		do
		{
			mod_value = (float)sin(_osc)*_realSpeed;
			
			*pSamplesL = mod_value * *++pSamplesL;
			*pSamplesR = mod_value * *++pSamplesR;
			
			_osc += _realSpeed + (float)sin(_lfoOsc)*_lfoAmp;
			
			_lfoOsc += _lfoSpeed;
			if (_lfoOsc > rad)
			{
				_lfoOsc -= rad;
			}
			if (_realSpeed > _speed)
			{
				_realSpeed -= _glide;
				if (_realSpeed < _speed)
				{
					_realSpeed = _speed;
				}
			}
			
			if (_realSpeed < _speed)
			{
				_realSpeed += _glide;
				if (_realSpeed > _speed)
				{
					_realSpeed = _speed;
				}
			}
			
			if (_osc > rad)
			{
				_osc -= rad;
			}
		}
		while (--i);
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

bool Sine::Load(
	RiffFile* pFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	pFile->Read(&_editName, sizeof(_editName));

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junk[0], sizeof(int)); // distPosClamp
	pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junk[0], sizeof(int)); // distNegClamp

	pFile->Read(&_sineSpeed, sizeof(_sineSpeed));
	pFile->Read(&_sineGlide, sizeof(_sineGlide));
	pFile->Read(&_sineVolume, sizeof(_sineVolume));
	pFile->Read(&_sineLfoSpeed, sizeof(_sineLfoSpeed));
	pFile->Read(&_sineLfoAmp, sizeof(_sineLfoAmp));

	pFile->Read(&junk[0], sizeof(int)); // delayTimeL
	pFile->Read(&junk[0], sizeof(int)); // delayTimeR
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Read(&junk[0], sizeof(int)); // filterCutoff
	pFile->Read(&junk[0], sizeof(int)); // filterResonance
	pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Read(&junk[0], sizeof(int)); // filterLfophase
	pFile->Read(&junk[0], sizeof(int)); // filterMode
	Update();

	return true;
}

#if !defined(_WINAMP_PLUGIN_)
bool Sine::Save(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

	pFile->Write(&_editName, sizeof(_editName));

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Write(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junk[0], sizeof(int)); // distPosClamp
	pFile->Write(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junk[0], sizeof(int)); // distNegClamp

	pFile->Write(&_sineSpeed, sizeof(_sineSpeed));
	pFile->Write(&_sineGlide, sizeof(_sineGlide));
	pFile->Write(&_sineVolume, sizeof(_sineVolume));
	pFile->Write(&_sineLfoSpeed, sizeof(_sineLfoSpeed));
	pFile->Write(&_sineLfoAmp, sizeof(_sineLfoAmp));

	pFile->Write(&junk[0], sizeof(int)); // delayTimeL
	pFile->Write(&junk[0], sizeof(int)); // delayTimeR
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Write(&junk[0], sizeof(int)); // filterCutoff
	pFile->Write(&junk[0], sizeof(int)); // filterResonance
	pFile->Write(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Write(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Write(&junk[0], sizeof(int)); // filterLfophase
	pFile->Write(&junk[0], sizeof(int)); // filterMode

	return true;
}
#endif // ndef _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Distortion::Distortion()
{
	_numPars = 5;
	_type = MACH_DIST;
	_mode = MACHMODE_FX;
	sprintf(_editName, "Distortion");
}

void Distortion::Init(void)
{
	Machine::Init();

	_posThreshold = 128;
	_posClamp = 128;
	_negThreshold = 128;
	_negClamp = 128;
}

void Distortion::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_
	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		float dpt = _posThreshold*128.0f;
		float dpc = _posClamp*128.0f;
		float dnt = -_negThreshold*128.0f;
		float dnc = -_negClamp*128.0f;
		float in;
		
		float* pSamplesL = _pSamplesL;
		float* pSamplesR = _pSamplesR;
		
		--pSamplesL;
		--pSamplesR;
		
		int i = numSamples;
		do
		{
			in = *++pSamplesL;
			if (in > dpt)
			{
				in = dpc;
			}
			if (in < dnt)
			{
				in = dnc;
			}
			*pSamplesL = in;
			
			in = *++pSamplesR;
			if (in > dpt)
			{
				in = dpc;
			}
			if (in < dnt)
			{
				in = dnc;
			}
			*pSamplesR = in;
		}
		while (--i);
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

void Distortion::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == 121 || pData->_note == 122 ) SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
}

bool Distortion::Load(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Read(&_editName, sizeof(_editName));

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&_posThreshold, sizeof(_posThreshold));
	pFile->Read(&_posClamp, sizeof(_posClamp));
	pFile->Read(&_negThreshold, sizeof(_negThreshold));
	pFile->Read(&_negClamp, sizeof(_negClamp));

	pFile->Read(&junk[0], sizeof(char)); // sinespeed
	pFile->Read(&junk[0], sizeof(char)); // sineglide
	pFile->Read(&junk[0], sizeof(char)); // sinevolume
	pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Read(&junk[0], sizeof(int)); // delayTimeL
	pFile->Read(&junk[0], sizeof(int)); // delayTimeR
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Read(&junk[0], sizeof(int)); // filterCutoff
	pFile->Read(&junk[0], sizeof(int)); // filterResonance
	pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Read(&junk[0], sizeof(int)); // filterLfophase
	pFile->Read(&junk[0], sizeof(int)); // filterMode

	return true;
}
#if !defined(_WINAMP_PLUGIN_)
bool Distortion::Save(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

	pFile->Write(&_editName, sizeof(_editName));

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Write(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&_posThreshold, sizeof(_posThreshold));
	pFile->Write(&_posClamp, sizeof(_posClamp));
	pFile->Write(&_negThreshold, sizeof(_negThreshold));
	pFile->Write(&_negClamp, sizeof(_negClamp));

	pFile->Write(&junk[0], sizeof(char)); // sinespeed
	pFile->Write(&junk[0], sizeof(char)); // sineglide
	pFile->Write(&junk[0], sizeof(char)); // sinevolume
	pFile->Write(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Write(&junk[0], sizeof(int)); // delayTimeL
	pFile->Write(&junk[0], sizeof(int)); // delayTimeR
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Write(&junk[0], sizeof(int)); // filterCutoff
	pFile->Write(&junk[0], sizeof(int)); // filterResonance
	pFile->Write(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Write(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Write(&junk[0], sizeof(int)); // filterLfophase
	pFile->Write(&junk[0], sizeof(int)); // filterMode

	return true;
}
#endif // ndef _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Delay::Delay()
{
	_numPars = 7;
	_type = MACH_DELAY;
	_mode = MACHMODE_FX;
	sprintf(_editName, "Delay");

	_pBufferL = NULL;
	_pBufferR = NULL;
}

Delay::~Delay()
{
	if (_pBufferL != NULL)
	{
		delete _pBufferL;
	}
	if (_pBufferR != NULL)
	{
		delete _pBufferR;
	}
}

void Delay::Init(void)
{
	Machine::Init();

	_pBufferL = new float[MAX_DELAY_BUFFER];
	_pBufferR = new float[MAX_DELAY_BUFFER];

	Dsp::Clear(_pBufferL, MAX_DELAY_BUFFER);
	Dsp::Clear(_pBufferR, MAX_DELAY_BUFFER);
	Update(11050,11050,50,50);
}

void Delay::GetParamValue(int numparam,char* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,"None"); break;
		case 1:
#if defined(_WINAMP_PLUGIN_)
			sprintf(parval,"%.2fms (%d)",_timeL/Global::pConfig->_samplesPerSec,_timeL);
#else
			sprintf(parval,"%.2fms (%d)",_timeL/Global::pConfig->_pOutputDriver->_samplesPerSec,_timeL);
#endif // _WINAMP_PLUGIN_
			break;
		case 2:
			sprintf(parval, "%d%%", _feedbackL);
			break;
		case 3:
#if defined(_WINAMP_PLUGIN_)
			sprintf(parval,"%.2fms (%d)",_timeR/Global::pConfig->_samplesPerSec,_timeR);
#else
			sprintf(parval,"%.2fms (%d)",_timeR/Global::pConfig->_pOutputDriver->_samplesPerSec,_timeR);
#endif // _WINAMP_PLUGIN_
			break;
		case 4:
			sprintf(parval, "%d%%", _feedbackR);
			break;
		case 5:
			sprintf(parval, "%d%%", _outDry*0.390625f);
			break;
		case 6:
			sprintf(parval, "%d%%", _outWet*0.390625f);
			break;
		default: strcpy(parval,"Out Of Range"); break;
	}
}

void Delay::Update(int delayL, int delayR, int feedbackL, int feedbackR)
{
	if((delayL != 0) && (delayL != _timeL))
	{
		_timeL = delayL;
		_counterL = MAX_DELAY_BUFFER-1;
		_delayedCounterL = _counterL-_timeL;
		
		if (_delayedCounterL < 0)
		{
			_delayedCounterL = 0;
		}
	}

	if((delayR != 0)  && (delayR != _timeR))
	{
		_timeR = delayR;
		_counterR = MAX_DELAY_BUFFER-1;
		_delayedCounterR = _counterR-_timeR;
		
		if (_delayedCounterR < 0)
		{
			_delayedCounterR = 0;
		}
	}

	if (feedbackL != 0)
	{
		_feedbackL = feedbackL;
	}
	if (feedbackR != 0)
	{
		_feedbackR = feedbackR;
	}
}

void Delay::Work(int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_

	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		float inputL;
		float inputR;
		
		float fdbkL = (float)_feedbackL*0.01f;
		float fdbkR = (float)_feedbackR*0.01f;
		float dry = (float)_outDry*0.0039062f;
		float wet = (float)_outWet*0.0039062f;
		
		float* pSamplesL = _pSamplesL;
		float* pSamplesR = _pSamplesR;
		
		--pSamplesL;
		--pSamplesR;
		
		int i = numSamples;
		do
		{
			inputL = *++pSamplesL;
			inputR = *++pSamplesR;
			
			inputL++;
			inputR++;
			
			if (++_counterL >= MAX_DELAY_BUFFER)
			{
				_counterL = 0;
			}
			if (++_counterR >= MAX_DELAY_BUFFER)
			{
				_counterR = 0;
			}
			
			if (++_delayedCounterL >= MAX_DELAY_BUFFER)
			{
				_delayedCounterL = 0;
			}
			if (++_delayedCounterR >= MAX_DELAY_BUFFER)
			{
				_delayedCounterR = 0;
			}
			
			_pBufferL[_counterL] = inputL + _pBufferL[_delayedCounterL] * fdbkL;
			_pBufferR[_counterR] = inputR + _pBufferR[_delayedCounterR] * fdbkR;
			
			*pSamplesL = inputL*dry + _pBufferL[_delayedCounterL]*wet;
			*pSamplesR = inputR*dry + _pBufferR[_delayedCounterR]*wet;
		}
		while(--i);
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

bool Delay::Load(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Read(&_editName, sizeof(_editName));

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junk[0], sizeof(int)); // distPosClamp
	pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junk[0], sizeof(int)); // distNegClamp

	pFile->Read(&junk[0], sizeof(char)); // sinespeed
	pFile->Read(&junk[0], sizeof(char)); // sineglide
	pFile->Read(&junk[0], sizeof(char)); // sinevolume
	pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

	int timeL;
	int timeR;
	int feedbackL;
	int feedbackR;
	pFile->Read(&timeL, sizeof(_timeL));
	pFile->Read(&timeR, sizeof(_timeR));
	pFile->Read(&feedbackL, sizeof(_feedbackL));
	pFile->Read(&feedbackR, sizeof(_feedbackR));

	pFile->Read(&junk[0], sizeof(int)); // filterCutoff
	pFile->Read(&junk[0], sizeof(int)); // filterResonance
	pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Read(&junk[0], sizeof(int)); // filterLfophase
	pFile->Read(&junk[0], sizeof(int)); // filterMode

	Update(timeL, timeR, feedbackL, feedbackR);
	return true;
}
#if !defined(_WINAMP_PLUGIN_)
bool Delay::Save(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

	pFile->Write(&_editName, sizeof(_editName));

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Write(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junk[0], sizeof(int)); // distPosClamp
	pFile->Write(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junk[0], sizeof(int)); // distNegClamp

	pFile->Write(&junk[0], sizeof(char)); // sinespeed
	pFile->Write(&junk[0], sizeof(char)); // sineglide
	pFile->Write(&junk[0], sizeof(char)); // sinevolume
	pFile->Write(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Write(&_timeL, sizeof(_timeL));
	pFile->Write(&_timeR, sizeof(_timeR));
	pFile->Write(&_feedbackL, sizeof(_feedbackL));
	pFile->Write(&_feedbackR, sizeof(_feedbackR));

	pFile->Write(&junk[0], sizeof(int)); // filterCutoff
	pFile->Write(&junk[0], sizeof(int)); // filterResonance
	pFile->Write(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Write(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Write(&junk[0], sizeof(int)); // filterLfophase
	pFile->Write(&junk[0], sizeof(int)); // filterMode

	return true;
}
#endif // _WINAMP_PLUGIN_

void Delay::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == 121 || pData->_note == 122 )
	{
		if ( pData->_cmd & 0x80 ) SetParameter(pData->_inst,(((pData->_cmd&0x7F)<<8) + pData->_parameter)*(-1));
		else SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Flanger::Flanger()
{
	_numPars = 9;
	_type = MACH_FLANGER;
	_mode = MACHMODE_FX;
	sprintf(_editName, "Flanger");

	_pBufferL = NULL;
	_pBufferR = NULL;
}

Flanger::~Flanger()
{
	if (_pBufferL != NULL)
	{
		delete _pBufferL;
	}
	if (_pBufferR != NULL)
	{
		delete _pBufferR;
	}
}

void Flanger::Init(void)
{
	Machine::Init();
	SetPreset(0);

	_pBufferL = new float[2048];
	_pBufferR = new float[2048];
	
	Dsp::Clear(_pBufferL, 2048);
	Dsp::Clear(_pBufferR, 2048);
	_counter = 0;
	_dLfoPos=0;
}
void Flanger::GetParamValue(int numparam,char* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,"None"); break;
		case 1:
#if defined(_WINAMP_PLUGIN_)
			sprintf(parval,"%.2fms (%d)",_time/Global::pConfig->_samplesPerSec,_time);
#else
			sprintf(parval,"%.2fms (%d)",_time/Global::pConfig->_pOutputDriver->_samplesPerSec,_time);
#endif // _WINAMP_PLUGIN_
			break;
		case 2:
			sprintf(parval, "%d%%", _lfoAmp*0.390625f);
			break;
		case 3:
#if defined(_WINAMP_PLUGIN_)
			sprintf(parval,"%.2fms (%d)",_lfoSpeed/Global::pConfig->_samplesPerSec,_lfoSpeed);
#else
			sprintf(parval,"%.2fms (%d)",_lfoSpeed/Global::pConfig->_pOutputDriver->_samplesPerSec,_lfoSpeed);
#endif // _WINAMP_PLUGIN_)
			break;
		case 4:
			sprintf(parval, "%d%%", _feedbackL);
			break;
		case 5:
			sprintf(parval, "%d Degrees", _lfoPhase*1.40625);
			break;
		case 6:
			sprintf(parval, "%d%%", _feedbackR);
			break;
		case 7:
			sprintf(parval, "%.1f%%", _outDry*0.390625f);
			break;
		case 8:
			sprintf(parval, "%.1f%%", _outWet*0.390625f);
			break;
		default: strcpy(parval,"Out Of Range"); break;
	}
}

void Flanger::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_

	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		static float const TWOPI = 6.28318530717958647692528676655901f;
		
		float inputL;
		float inputR;
		float fdbkL =(float)_feedbackL*0.01f;
		float fdbkR =(float)_feedbackR*0.01f;
		float dry =(float)_outDry*0.00390625f;
		float wet =(float)_outWet*0.00390625f;
		
		float* pSamplesL = _pSamplesL;
		float* pSamplesR = _pSamplesR;
		
		--pSamplesL;
		--pSamplesR;

		double ipR = _dLfoPos+_fLfoPhase;
		double b1 = 2.0 * cos(_dLfoSpeed);
		double y0L=0;
		double y1L= sin(_dLfoPos-_dLfoSpeed);
		double y2L= sin(_dLfoPos-2*_dLfoSpeed);
		double y0R=0;
		double y1R= sin(ipR-_dLfoSpeed);
		double y2R= sin(ipR-2*_dLfoSpeed);

		int i = numSamples;
		do
		{
			y0L = b1*y1L - y2L;
			y2L = y1L;
			y1L = y0L;
			y0R = b1*y1R - y2R;
			y2R = y1R;
			y1R = y0R;
			
			const float acc_timeL = (float)_time + (float)(y0L*_fLfoAmp);
			const float acc_timeR = (float)_time + (float)(y0R*_fLfoAmp);
//			const float acc_timeL = _time + (float)(sin(_fLfoDegree)*_fLfoAmp);	// 0 <= _fLfoDegree <= TWOPI
//			const float acc_timeR = _time + (float)(sin(_fLfoDegree+_fLfoPhase)*_fLfoAmp); // 0 <= _fLfoPhase <= PI

			if (++_counter >= 2048)	{ _counter = 0; }
			
			int _delayedCounterL = _counter-f2i(acc_timeL);
			if (_delayedCounterL < 0) _delayedCounterL += 2048;

			int _delayedCounterR = _counter-f2i(acc_timeR);
			if (_delayedCounterR < 0) _delayedCounterR += 2048;

			float y_l = _pBufferL[_delayedCounterL];
			if (IS_DENORMAL(y_l) ) y_l=0.0f;

			float y_r = _pBufferR[_delayedCounterR];
			if (IS_DENORMAL(y_r) ) y_r=0.0f;
			
			inputL = *++pSamplesL;
			inputR = *++pSamplesR;

			_pBufferL[_counter] = inputL + y_l*fdbkL;
			_pBufferR[_counter] = inputR + y_r*fdbkR;
			
			*pSamplesL = inputL*dry + y_l*wet;
			*pSamplesR = inputR*dry + y_r*wet;
			
			_dLfoPos += _dLfoSpeed;
			if (_dLfoPos > TWOPI)
			{
				_dLfoPos -= TWOPI;
			}
		}
		while (--i);
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
	else
	{
		static float const TWOPI = 6.28318530717958647692528676655901f;
		_dLfoPos += _dLfoSpeed*numSamples;
		while (_dLfoPos > TWOPI)
		{
			_dLfoPos -= TWOPI;
		}
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

void Flanger::Tick(int channel, PatternEntry *pData)
{
	int cmd;
	if ( pData->_cmd & 0x80 ) cmd = (((pData->_cmd & 0x7F)<<8) +pData->_parameter)*(-1);
	else cmd = ( pData->_cmd&0x7F<<8) + pData->_parameter;

	if ( pData->_note == 121 || pData->_note == 122 ) {
		switch(pData->_inst)
		{
		case 1: _time=cmd;Update();break;
		case 2: _lfoAmp=cmd & 0x7F;Update();break;
		case 3: _lfoSpeed=cmd;Update();break;
		case 4:
			if ( cmd < -100 || cmd > 100 ) _feedbackL=100;
			else _feedbackL=cmd;
			Update();
			break;
		case 5: _lfoPhase=cmd & 0x7F;Update();break;
		case 6: 
			if ( cmd < -100 || cmd > 100 ) _feedbackR=100;
			else _feedbackR=cmd;
			Update();
			break;
		case 7: _outWet=cmd & 0x7F;break;
		case 8: _outDry=cmd & 0x7F;break;
		default:break;
		}
	}
}

void Flanger::Update(void)
{
	_dLfoSpeed = _lfoSpeed*0.000000003f;
	_fLfoAmp = (float)_lfoAmp*0.00390625f*float(_time-1);
	_fLfoPhase = (float)_lfoPhase*0.012271846f;
}

void Flanger::SetPreset(
	int preset)
{
	switch (preset)
	{
	case 0:	/* Default */
		_time = 75;
		_lfoAmp = 246;
		_lfoSpeed = 3748;
		_feedbackL = 65;
		_lfoPhase = 32;
		_feedbackR = 49;
		_outDry = 256;
		_outWet = 256;
		break;
		
	case 1:	/* Chorus */
		_time = 227;
		_lfoAmp = 246;
		_lfoSpeed = 23543;
		_feedbackL = 28;
		_lfoPhase = 112;
		_feedbackR = -24;
		_outDry = 236;
		_outWet = 256;
		break;
		
	case 2:	/* Chorus 2*/
		_time = 325;
		_lfoAmp = 100;
		_lfoSpeed = 12973;
		_feedbackL = 64;
		_lfoPhase = 124;
		_feedbackR = 65;
		_outDry = 144;
		_outWet = 256;
		break;
	}
	Update();
}

bool Flanger::Load(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Read(&_editName, sizeof(_editName));

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junk[0], sizeof(int)); // distPosClamp
	pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junk[0], sizeof(int)); // distNegClamp

	pFile->Read(&junk[0], sizeof(char)); // sinespeed
	pFile->Read(&junk[0], sizeof(char)); // sineglide
	pFile->Read(&junk[0], sizeof(char)); // sinevolume
	pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Read(&_time, sizeof(_time));
	pFile->Read(&junk[0], sizeof(int)); // delayTimeR
	pFile->Read(&_feedbackL, sizeof(_feedbackL));
	pFile->Read(&_feedbackR, sizeof(_feedbackR));

	pFile->Read(&junk[0], sizeof(int)); // filterCutoff
	pFile->Read(&junk[0], sizeof(int)); // filterResonance
	pFile->Read(&_lfoSpeed, sizeof(_lfoSpeed));
	pFile->Read(&_lfoAmp, sizeof(_lfoAmp));
	pFile->Read(&_lfoPhase, sizeof(_lfoPhase));
	pFile->Read(&junk[0], sizeof(int)); // filterMode

	Update();
	return true;
}

#if !defined(_WINAMP_PLUGIN_)
bool Flanger::Save(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

	pFile->Write(&_editName, sizeof(_editName));

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Write(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junk[0], sizeof(int)); // distPosClamp
	pFile->Write(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junk[0], sizeof(int)); // distNegClamp

	pFile->Write(&junk[0], sizeof(char)); // sinespeed
	pFile->Write(&junk[0], sizeof(char)); // sineglide
	pFile->Write(&junk[0], sizeof(char)); // sinevolume
	pFile->Write(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Write(&_time, sizeof(_time));
	pFile->Write(&junk[0], sizeof(int)); // delayTimeR
	pFile->Write(&_feedbackL, sizeof(_feedbackL));
	pFile->Write(&_feedbackR, sizeof(_feedbackR));

	pFile->Write(&junk[0], sizeof(int)); // filterCutoff
	pFile->Write(&junk[0], sizeof(int)); // filterResonance
	pFile->Write(&_lfoSpeed, sizeof(_lfoSpeed));
	pFile->Write(&_lfoAmp, sizeof(_lfoAmp));
	pFile->Write(&_lfoPhase, sizeof(_lfoPhase));
	pFile->Write(&junk[0], sizeof(int)); // filterMode

	return true;
}
#endif // _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Filter2p::Filter2p()
{
	_numPars = 7;
	_type = MACH_2PFILTER;
	_mode = MACHMODE_FX;
	sprintf(_editName, "2p Filter");
}

void Filter2p::Init(void)
{
	Machine::Init();

	_cutoff = 128;
	_resonance = 128;
	_lfoSpeed = 0;
	_lfoAmp = 0;
	_lfoPhase = 0;
	_filterMode = 0;
	_fLfoDegree = 0;
	_bufferL0 = 0;
	_bufferL1 = 0;
	_bufferR0 = 0;
	_bufferR1 = 0;

	Update();
}

void Filter2p::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_

	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		static float const TWOPI = 6.28318530717958647692528676655901f;
		
		float co1 = _fCutoff + (float)sin(_fLfoDegree) * _fLfoAmp;
		float co2 = _fCutoff + (float)sin(_fLfoDegree + _fLfoPhase) * _fLfoAmp;
		
		// Init filter intermediate vars
		if (co1 < 0.01f)
		{
			co1 = 0.01f;
		}
		if (co1 > 0.99f)
		{
			co1 = 0.99f;
		}
		float fa1 = float(1.0 - co1); 
		float fb1 = float(_fResonance * (1.0 + (1.0/fa1)));
		
		if (co2 < 0.01f)
		{
			co2 = 0.01f;
		}
		if (co2 > 0.99f)
		{
			co2 = 0.99f;
		}
		float fa2 = float(1.0 - co2); 
		float fb2 = float(_fResonance * (1.0 + (1.0/fa2)));
		
		float* pSamplesL = _pSamplesL;
		float* pSamplesR = _pSamplesR;
		
		int i = numSamples;
		do
		{
			*pSamplesL++ = WorkL(*pSamplesL, co1, _fResonance, fa1, fb1);
			*pSamplesR++ = WorkR(*pSamplesR, co2, _fResonance, fa2, fb2);
			
			_fLfoDegree += _fLfoSpeed;
			if (_fLfoDegree > TWOPI)
			{
				_fLfoDegree -= TWOPI;
			}
		}
		while (--i);
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

float Filter2p::WorkL(
	float input,
	float f,
	float q,
	float fa,
	float fb)
{
	input += 1.0f;
	
	_bufferL0 = fa * _bufferL0 + f * (input + fb * (_bufferL0 - _bufferL1)); 
	_bufferL1 = fa * _bufferL1 + f * _bufferL0;
	
	switch(_filterMode)
	{
	case 0:
		return _bufferL1;
		break;
		
	case 1:
		return input-_bufferL1; 
		break;
	}
	return 0;
}

float Filter2p::WorkR(
	float input,
	float f,
	float q,
	float fa,
	float fb)
{
	input += 1.0f;
	
	_bufferR0 = fa * _bufferR0 + f * (input + fb * (_bufferR0 - _bufferR1));
	_bufferR1 = fa * _bufferR1 + f * _bufferR0;

	switch (_filterMode)
	{
	case 0:
		return _bufferR1; 
	case 1:
		return input-_bufferR1; 
	}
	return 0;
}

void Filter2p::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == 121 || pData->_note == 122 ) {
		switch(pData->_inst)
		{
		case 1: _filterMode=pData->_parameter;break;
		case 2: _cutoff=pData->_parameter;Update();break;
		case 3: _resonance=pData->_parameter;Update();break;
		case 4: _lfoSpeed=(pData->_cmd<<8)+pData->_parameter;Update();break;
		case 5: _lfoAmp=pData->_parameter;Update();break;
		case 6: _lfoPhase=pData->_parameter;Update();break;
		default:break;
		}
	}
}

void Filter2p::GetParamValue(int numparam,char* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,"None"); break;
		case 1: sprintf(parval,"%s",_filterMode?"Low Pass 2Pole -12db":"High Pass 2Pole -12db"); break;
#if defined(_WINAMP_PLUGIN_)
		case 2: sprintf(parval,"%d (%d Hz)",_cutoff, Global::pConfig->_samplesPerSec * asin( _fCutoff) / 3.1415926f); break;
#else
		case 2: sprintf(parval,"%d (%d Hz)",_cutoff, Global::pConfig->_pOutputDriver->_samplesPerSec * asin( _fCutoff) / 3.1415926f); break;
#endif // _WINAMP_PLUGIN_
		case 3: sprintf(parval,"%d (%d%%)",_resonance,_resonance*0.390625f); break;
#if defined(_WINAMP_PLUGIN_)
		case 4: sprintf(parval,"%d (%.2f Hz)",_lfoSpeed,_fLfoSpeed*Global::pConfig->_samplesPerSec / 6.283185f); break;
#else
		case 4: sprintf(parval,"%d (%.2f Hz)",_lfoSpeed,_fLfoSpeed*Global::pConfig->_pOutputDriver->_samplesPerSec / 6.283185f); break;
#endif // _WINAMP_PLUGIN_
		case 5: sprintf(parval,"%%d (%d%%)",_lfoAmp,_lfoAmp*0.390625f); break;
		case 6: sprintf(parval,"%d (%d deg)",_lfoPhase,_lfoPhase*0.703125f); break;
		default: strcpy(parval,"Out Of Range"); break;
	}
}


void Filter2p::Update(void)
{
	_fCutoff = _cutoff * 0.00390625f;
	_fResonance = _resonance * 0.00390625f;
	_fLfoSpeed = _lfoSpeed * 0.00000003f;
	_fLfoAmp = _lfoAmp * 0.00390625f;
	_fLfoPhase = _lfoPhase * 0.0122718f;
}

bool Filter2p::Load(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Read(&_editName, sizeof(_editName));

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junk[0], sizeof(int)); // distPosClamp
	pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junk[0], sizeof(int)); // distNegClamp

	pFile->Read(&junk[0], sizeof(char)); // sinespeed
	pFile->Read(&junk[0], sizeof(char)); // sineglide
	pFile->Read(&junk[0], sizeof(char)); // sinevolume
	pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Read(&junk[0], sizeof(int)); // delayTimeL
	pFile->Read(&junk[0], sizeof(int)); // delayTimeR
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Read(&_cutoff, sizeof(_cutoff));
	pFile->Read(&_resonance, sizeof(_resonance));
	pFile->Read(&_lfoSpeed, sizeof(_lfoSpeed));
	pFile->Read(&_lfoAmp, sizeof(_lfoAmp));
	pFile->Read(&_lfoPhase, sizeof(_lfoPhase));
	pFile->Read(&_filterMode, sizeof(_filterMode));

	Update();
	return true;
}

#if !defined(_WINAMP_PLUGIN_)
bool Filter2p::Save(
	RiffFile* pFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

	pFile->Write(&_editName, sizeof(_editName));

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Write(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junk[0], sizeof(int)); // distPosClamp
	pFile->Write(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junk[0], sizeof(int)); // distNegClamp

	pFile->Write(&junk[0], sizeof(char)); // sinespeed
	pFile->Write(&junk[0], sizeof(char)); // sineglide
	pFile->Write(&junk[0], sizeof(char)); // sinevolume
	pFile->Write(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Write(&junk[0], sizeof(int)); // delayTimeL
	pFile->Write(&junk[0], sizeof(int)); // delayTimeR
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Write(&_cutoff, sizeof(_cutoff));
	pFile->Write(&_resonance, sizeof(_resonance));
	pFile->Write(&_lfoSpeed, sizeof(_lfoSpeed));
	pFile->Write(&_lfoAmp, sizeof(_lfoAmp));
	pFile->Write(&_lfoPhase, sizeof(_lfoPhase));
	pFile->Write(&_filterMode, sizeof(_filterMode));

	return true;
}
#endif // ndef _WINAMP_PLUGIN_