#include "stdafx.h"
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.7 $
 */
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

#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
	#include "Machine.h"
	#include "Dsp.h"
	#include "Song.h"
#include "Instrument.h"
	#include "FileIO.h"
	#include "configuration.h"
#else
//	#include "Psycle2.h"
	#include "Machine.h"
	#include "Dsp.h"
	#include "Song.h"
	#include "Instrument.h"
	#include "FileIO.h"
	#include "configuration.h"
	#include "WireDlg.h"
	#include "MainFrm.h"

//	extern CPsycleApp theApp;
#endif // _WINAMP_PLUGIN_

#include "Sampler.h"
#include "XMInstrument.h"
#include "XMSampler.h"
#include "Plugin.h"
#include "VSTHost.h"
	
#include "InputHandler.h"


TCHAR* Master::_psName = _T("Master");
TCHAR* Dummy::_psName = _T("DummyPlug");

#ifndef CONVERT_INTERNAL_MACHINES
TCHAR* Gainer::_psName = _T("Gainer");
CIntMachParam Gainer::pars[] = {{_T("Reserved"),0,0},{_T("Gain"),0,1024}};
TCHAR* Sine::_psName = "PsychOsc AM";
CIntMachParam Sine::pars[] = {{_T("Reserved"),0,0},{_T("OSC Frequency"),0,128},{_T("OSC Glide Speed"),0,128}, \
								{_T("LFO Frequency"),0,128},{_T("LFO Amplitude"),0,128}};
TCHAR* Distortion::_psName = _T("Distortion");
CIntMachParam Distortion::pars[] = {{_T("Reserved"),0,0},{_T("Pos. Threshold"),0,128},{_T("Positive Clamp"),0,128}, \
								{_T("Neg. Threshold"),0,128},{_T("Negative Clamp"),0,128}};
TCHAR* Delay::_psName = _T("Dalay Delay");
CIntMachParam Delay::pars[] = {{_T("Reserved"),0,0},{_T("Left Delay"),1,MAX_DELAY_BUFFER-1},{_T("Left Feedback"),-100,100}, \
								{_T("Right Delay"),1,MAX_DELAY_BUFFER-1}, {_T("Right Feedback"),-100,100},{_T("Dry"),-256,256},{_T("Wet"),-256,256}};
TCHAR* Flanger::_psName = _T("Flanger");
CIntMachParam Flanger::pars[] = {{_T("Reserved"),0,0},{_T("Left Delay"),0,1024},{_T("Left Amp"),0,256},{_T("Left Speed"),1,32768}, \
								{_T("Left Feeback"),-100,100},{_T("Right Phase"),0,256},{_T("Right Feedback"),-100,100}, \
								{_T("Dry"),-256,256},{_T("Wet"),-256,256}};
TCHAR* Filter2p::_psName = _T("2p Filter");
CIntMachParam Filter2p::pars[] = {{_T("Reserved"),0,0},{_T("Filter Type"),0,1},{_T("Filter Cuttoff"),0,256}, \
								{_T("Filter Ressonance"),0,256},{_T("LFO Speed"),0,32768},{_T("LFO Amplitude"),0,256},{_T("LFO Phase"),0,256}};

#endif
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
Machine::Machine()
{
	wasVST = false;

	_numPars = 0;
	_volumeCounter = 0.0f;
	_volumeDisplay = 0;
	_volumeMaxDisplay = 0;
	_volumeMaxCounterLife = 0;
	_pScopeBufferL = NULL;
	_pScopeBufferR = NULL;
	_scopeBufferIndex = 0;

	_pSamplesL = new float[STREAM_SIZE];
	_pSamplesR = new float[STREAM_SIZE];
	
	// Clear machine buffer samples
	for (int c = 0; c < STREAM_SIZE; c++)
	{
		_pSamplesL[c] = 0;
		_pSamplesR[c] = 0;
	}
	for (c = 0; c<MAX_TRACKS; c++)
	{
		TriggerDelay[c]._cmd = 0;
		TriggerDelayCounter[c]=0;
		RetriggerRate[c]=256;
	}
	for (c = 0; c<MAX_TWS; c++)
	{
		TWSInst[c] = 0;
		TWSDelta[c] = 0;
		TWSCurrent[c] = 0;
		TWSDestination[c] = 0;
	}
	TWSSamples = 0;
	TWSActive = FALSE;
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
void Machine::InitWireVolume(MachineType mType,int wireIndex,float value)
{
	if ( mType == MACH_VST || mType == MACH_VSTFX )
	{
		if (_type == MACH_VST || _type == MACH_VSTFX ) // VST to VST, no need to convert.
		{
			_inputConVol[wireIndex] = value;
			_wireMultiplier[wireIndex] = 1.0f;
		}
		else											// VST to native, multiply
		{
			_inputConVol[wireIndex] = value * 32768.0f;
			_wireMultiplier[wireIndex] = 0.000030517578125f;
		}
	}
	else if ( _type == MACH_VST || _type == MACH_VSTFX ) // native to VST, divide.
	{
		_inputConVol[wireIndex] = value * 0.000030517578125f;
		_wireMultiplier[wireIndex] = 32768.0f;
	}
	else												// native to native, no need to convert.
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

bool Machine::SetDestWireVolume(int srcIndex, int WireIndex,float value)
{
	// Get reference to the destination machine
	if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
	Machine *_pDstMachine 
		= Global::_pSong->pMachine(_outputMachines[WireIndex]);

	if (_pDstMachine)
	{
//		if ( value == 255 ) value =256; // FF = 255
//		const float invol = CValueMapper::Map_255_1(value); // Convert a 0..256 value to a 0..1.0 value
		
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
	Machine *_pDstMachine = 
		Global::_pSong->pMachine(_outputMachines[WireIndex]);
	
	if (_pDstMachine)
	{
		int c;
		if ( (c = _pDstMachine->FindInputWire(srcIndex)) != -1)
		{
//			float val;
			_pDstMachine->GetWireVolume(c,value);
			//value = f2i(val*256.0f);
			return true;
		}
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
	if (_pScopeBufferL && _pScopeBufferR)
	{
		float *pSamplesL = _pSamplesL;   
		float *pSamplesR = _pSamplesR;   
		int i = numSamples;   
		while (i > 0)   
		{   
			if (i+_scopeBufferIndex >= CWireDlg::SCOPE_BUF_SIZE)   
			{   
				 memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,(CWireDlg::SCOPE_BUF_SIZE-(_scopeBufferIndex))*sizeof(float));   
				 memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,(CWireDlg::SCOPE_BUF_SIZE-(_scopeBufferIndex))*sizeof(float));   
				 pSamplesL+=(CWireDlg::SCOPE_BUF_SIZE-(_scopeBufferIndex));   
				 pSamplesR+=(CWireDlg::SCOPE_BUF_SIZE-(_scopeBufferIndex));   
				 i -= (CWireDlg::SCOPE_BUF_SIZE-(_scopeBufferIndex));   
				 _scopeBufferIndex = 0;   
			}   
			else   
			{   
				 memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,i*sizeof(float));   
				 memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,i*sizeof(float));   
				 _scopeBufferIndex += i;   
				 i = 0;   
			}   
		} 
	}
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
			Machine* pInMachine =
				Global::_pSong->pMachine(_inputMachines[i]);
			if (pInMachine)
			{
				if (!pInMachine->_worked && !pInMachine->_waitingForSound)
				{ 
					pInMachine->Work(numSamples);
	/*				This could be a different Undenormalize funtion, using the already calculated
					"_volumeCounter".Note: It needs that muted&|bypassed machines set the variable
					correctly.
					if ( pInMachine->_volumeCounter*_inputConVol[i] < 0.004f ) //this gives for 24bit depth.
					{
						memset(pInMachine->_pSamplesL,0,numSamples*sizeof(float));
						memset(pInMachine->_pSamplesR,0,numSamples*sizeof(float));
					}
	*/
					pInMachine->_waitingForSound=false;
				}
				if ( !pInMachine->_stopped ) 
				{
					_stopped=false;
				}
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
#if defined( _WINAMP_PLUGIN_)
	Dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
#else
	CPUCOST_INIT(wcost);
	Dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
	CPUCOST_CALC(wcost,numSamples);
	_wireCost+=wcost;
#endif // _WINAMP_PLUGIN_
				
}

bool Machine::Load(
	RiffFile& riffFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	
	char _edit_name[16];

	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));

	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));

	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol

	riffFile.Read(&junk[0], sizeof(int)); // outdry
	riffFile.Read(&junk[0], sizeof(int)); // outwet

	riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
	riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distNegClamp

	riffFile.Read(&junk[0], sizeof(char)); // sinespeed
	riffFile.Read(&junk[0], sizeof(char)); // sineglide
	riffFile.Read(&junk[0], sizeof(char)); // sinevolume
	riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
	riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp

	riffFile.Read(&junk[0], sizeof(int)); // delayTimeL
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackL
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackR

	riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
	riffFile.Read(&junk[0], sizeof(int)); // filterResonance
	riffFile.Read(&junk[0], sizeof(int)); // filterLfospeed
	riffFile.Read(&junk[0], sizeof(int)); // filterLfoamp
	riffFile.Read(&junk[0], sizeof(int)); // filterLfophase
	riffFile.Read(&junk[0], sizeof(int)); // filterMode

	return true;
}

Machine* Machine::LoadFileChunk(RiffFile& riffFile,const int index,const int version,const bool fullopen)
{
	// assume version 0 for now
	BOOL bDeleted = FALSE;

	Machine* pMachine;
	MachineType type;//,oldtype;
	TCHAR dllName[256];
	riffFile.Read(&type,sizeof(type));
//	oldtype=type;
	riffFile.ReadStringA2T(dllName,256);
	switch (type)
	{
	case MACH_MASTER:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Master(index);
		break;
	case MACH_SAMPLER:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Sampler(index);
		break;
	case MACH_XMSAMPLER:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new SF::XMSampler(index);
		break;

#ifndef CONVERT_INTERNAL_MACHINES

	case MACH_SINE:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Sine(index);
		break;
	case MACH_DIST:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Distortion(index);
		break;
	case MACH_DELAY:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Delay(index);
		break;
	case MACH_2PFILTER:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Filter2p(index);
		break;
	case MACH_GAIN:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Gainer(index);
		break;
	case MACH_FLANGER:
		if ( !fullopen ) pMachine = new Dummy(index);
		else pMachine = new Flanger(index);
		break;
#endif
	case MACH_PLUGIN:
		{
			if ( !fullopen ) pMachine = new Dummy(index);
			else 
			{
				Plugin * p;
				pMachine = p = new Plugin(index);
				if (!p->LoadDll(dllName))
				{
					::MessageBox(NULL,
						(SF::tformat(SF::CResourceString(IDS_ERR_MSG0099)) % dllName).str().data(),
						SF::CResourceString(IDS_ERR_MSG0064), MB_OK);
					pMachine = new Dummy(index);
					type = MACH_DUMMY;
					delete p;
					bDeleted = TRUE;
				}
			}
		}
		break;
	case MACH_VST:
		{
			if ( !fullopen ) pMachine = new Dummy(index);
			else 
			{
				VSTInstrument * p;
				pMachine = p = new VSTInstrument(index);
				if (!p->LoadDll(dllName))
				{
					::MessageBox(NULL,
						(SF::tformat(SF::CResourceString(IDS_ERR_MSG0100)) % dllName).str().data(),
						SF::CResourceString(IDS_ERR_MSG0064), MB_OK);

					pMachine = new Dummy(index);
					type = MACH_DUMMY;
					delete p;
					bDeleted = TRUE;
				}
			}
		}
		break;
	case MACH_VSTFX:
		{
			if ( !fullopen ) pMachine = new Dummy(index);
			else 
			{
				VSTFX * p;
				pMachine = p = new VSTFX(index);
				if (!p->LoadDll(dllName))
				{
					::MessageBox(NULL,
						(SF::tformat(SF::CResourceString(IDS_ERR_MSG0101)) % dllName).str().data(),
						SF::CResourceString(IDS_ERR_MSG0064), MB_OK);

					pMachine = new Dummy(index);
					type = MACH_DUMMY;
					delete p;
					bDeleted = TRUE;
				}
			}
		}
		break;
	default:
		pMachine = new Dummy(index);
		break;
	}
	pMachine->Init();
	pMachine->_type = type;

	riffFile.Read(&pMachine->_bypass,sizeof(pMachine->_bypass));
	riffFile.Read(&pMachine->_mute,sizeof(pMachine->_mute));

	riffFile.Read(&pMachine->_panning,sizeof(pMachine->_panning));

	riffFile.Read(&pMachine->_x,sizeof(pMachine->_x));
	riffFile.Read(&pMachine->_y,sizeof(pMachine->_y));
	riffFile.Read(&pMachine->_numInputs,sizeof(pMachine->_numInputs));							// number of Incoming connections
	riffFile.Read(&pMachine->_numOutputs,sizeof(pMachine->_numOutputs));						// number of Outgoing connections
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		riffFile.Read(&pMachine->_inputMachines[i],sizeof(pMachine->_inputMachines[i]));	// Incoming connections Machine number
		riffFile.Read(&pMachine->_outputMachines[i],sizeof(pMachine->_outputMachines[i]));	// Outgoing connections Machine number
		riffFile.Read(&pMachine->_inputConVol[i],sizeof(pMachine->_inputConVol[i]));	// Incoming connections Machine vol
		riffFile.Read(&pMachine->_wireMultiplier[i],sizeof(pMachine->_wireMultiplier[i]));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		riffFile.Read(&pMachine->_connection[i],sizeof(pMachine->_connection[i]));      // Outgoing connections activated
		riffFile.Read(&pMachine->_inputCon[i],sizeof(pMachine->_inputCon[i]));		// Incoming connections activated
	}
	riffFile.ReadStringA2T(pMachine->_editName,32);
	if (bDeleted)
	{
		TCHAR buf[34];
		_stprintf(buf,_T("X %s"),pMachine->_editName);
		buf[31]=0;
		_tcscpy(pMachine->_editName,buf);
	}
	
	if ( !fullopen ) return pMachine;
	
	if (!pMachine->LoadSpecificFileChunk(riffFile,version))
	{
		::MessageBox(NULL,
				(SF::tformat(SF::CResourceString(IDS_ERR_MSG0102)) % dllName).str().data(),
				SF::CResourceString(IDS_ERR_MSG0064), MB_OK);

		Machine* p = new Dummy(index);
		p->Init();
		p->_type=MACH_DUMMY;
		p->_mode=pMachine->_mode;

		p->_bypass=pMachine->_bypass;
		p->_mute=pMachine->_mute;

		p->_panning=pMachine->_panning;

		p->_x=pMachine->_x;
		p->_y=pMachine->_y;
		p->_numInputs=pMachine->_numInputs;							// number of Incoming connections
		p->_numOutputs=pMachine->_numOutputs;						// number of Outgoing connections
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			p->_inputMachines[i]=pMachine->_inputMachines[i];
			p->_outputMachines[i]=pMachine->_outputMachines[i];
			p->_inputConVol[i]=pMachine->_inputConVol[i];
			p->_wireMultiplier[i]=pMachine->_wireMultiplier[i];
			p->_connection[i]=pMachine->_connection[i];
			p->_inputCon[i]=pMachine->_inputCon[i];
		}
		// dummy name goes here
		_stprintf(p->_editName,_T("X %s"),pMachine->_editName);
		p->_numPars=0;

		delete pMachine;
		pMachine=p;
	}

	if (index < MAX_BUSES)
	{
		pMachine->_mode = MACHMODE_GENERATOR;
		if ( pMachine->_x > Global::_pSong->ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.width ) 
		{
			pMachine->_x = Global::_pSong->ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.width;
		}
		if ( pMachine->_y > Global::_pSong->ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.height ) 
		{
			pMachine->_y = Global::_pSong->ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.height;
		}
	}
	else if (index < MAX_BUSES * 2)
	{
		pMachine->_mode = MACHMODE_FX;
		if ( pMachine->_x > Global::_pSong->ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sEffect.width ) 
		{
			pMachine->_x = Global::_pSong->ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sEffect.width;
		}
		if ( pMachine->_y > Global::_pSong->ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sEffect.height ) 
		{
			pMachine->_y = Global::_pSong->ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sEffect.height;
		}
	}
	else
	{
		pMachine->_mode = MACHMODE_MASTER;
		if ( pMachine->_x > Global::_pSong->ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sMaster.width ) 
		{
			pMachine->_x = Global::_pSong->ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sMaster.width;
		}
		if ( pMachine->_y > Global::_pSong->ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sMaster.height ) 
		{
			pMachine->_y = Global::_pSong->ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sMaster.height;
		}
	}

	pMachine->SetPan(pMachine->_panning);
	return pMachine;
}

#if !defined(_WINAMP_PLUGIN_)

void Machine::SaveFileChunk(RiffFile& riffFile)
{
	riffFile.Write(&_type,sizeof(_type));
	SaveDllName(riffFile);
//	riffFile.Write(&_mode,sizeof(_mode));
	riffFile.Write(&_bypass,sizeof(_bypass));
	riffFile.Write(&_mute,sizeof(_mute));

	riffFile.Write(&_panning,sizeof(_panning));

	riffFile.Write(&_x,sizeof(_x));
	riffFile.Write(&_y,sizeof(_y));
	riffFile.Write(&_numInputs,sizeof(_numInputs));							// number of Incoming connections
	riffFile.Write(&_numOutputs,sizeof(_numOutputs));						// number of Outgoing connections
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		riffFile.Write(&_inputMachines[i],sizeof(_inputMachines[i]));	// Incoming connections Machine number
		riffFile.Write(&_outputMachines[i],sizeof(_outputMachines[i]));	// Outgoing connections Machine number
		riffFile.Write(&_inputConVol[i],sizeof(_inputConVol[i]));	// Incoming connections Machine vol
		riffFile.Write(&_wireMultiplier[i],sizeof(_wireMultiplier[i]));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		riffFile.Write(&_connection[i],sizeof(_connection[i]));      // Outgoing connections activated
		riffFile.Write(&_inputCon[i],sizeof(_inputCon[i]));		// Incoming connections activated
//		riffFile.Write(&_connectionPoint[i],sizeof(_connectionPoint[i]));// point for wire? 
	}

	CT2A _edit_name(_editName);
	riffFile.Write(_edit_name,strlen(_edit_name) + 1);
	
	SaveSpecificChunk(riffFile);
}

#endif // ndef _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Dummy::Dummy(int index)
{
	_macIndex = index;
	_numPars = 0;
	_type = MACH_DUMMY;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("Dummy"));
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
		if (_volumeCounter < 8.0f)	{
			_volumeCounter = 0.0f;
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

Master::Master(int index)
{
	_macIndex = index;
	sampleCount = 0;
	_numPars = 0;
	_outDry = 256;
	decreaseOnClip=false;
	_type = MACH_MASTER;
	_mode = MACHMODE_MASTER;
	_stprintf(_editName, _T("Master"));
}

void Master::Init(void)
{
	Machine::Init();
	sampleCount = 0;
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
	
		float mv = CValueMapper::Map_255_1(_outDry);
		
		float *pSamples = _pMasterSamples;
		float *pSamplesL = _pSamplesL;
		float *pSamplesR = _pSamplesR;
		
#if defined(_WINAMP_PLUGIN_)
		int i = numSamples;
		do
		{
			*pSamples++ = *pSamplesL++ * mv;
			*pSamples++ = *pSamplesR++ * mv;
		}
		while (--i);
#else
//		_lMax -= numSamples*8;
//		_rMax -= numSamples*8;
//		_lMax *= 0.5;
//		_rMax *= 0.5;
		if ( vuupdated ) 
		{ 
			_lMax *= 0.5; 
			_rMax *= 0.5; 
		}

		int i = numSamples;
		if (decreaseOnClip)
		{
			do
			{
				// Left channel
				if (fabs( *pSamples = *pSamplesL = *pSamplesL * mv) > _lMax)
				{
					_lMax = fabsf(*pSamplesL);
				}
				if (*pSamples > 32767.0f)
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
				if (fabs(*pSamples = *pSamplesR = *pSamplesR * mv) > _rMax)
				{
					_rMax = fabsf(*pSamplesR);
				}
				if (*pSamples > 32767.0f)
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
				if (fabs( *pSamples++ = *pSamplesL = *pSamplesL * mv) > _lMax)
				{
					_lMax = fabsf(*pSamplesL);
				}
				pSamplesL++;
				
				// Right channel
				if (fabs(*pSamples++ = *pSamplesR = *pSamplesR * mv) > _rMax)
				{
					_rMax = fabsf(*pSamplesR);
				}
				pSamplesR++;
			}
			while (--i);
		}
		
		if (_lMax > 32767.0f)
		{
			_clip=true;
			_lMax = 32767.0f; //_LMAX = 32768;
		}
		else if (_lMax < 1.0f) { _lMax = 1.0f; /*_LMAX = 1;*/ }
//		else _LMAX = Dsp::F2I(_lMax);

		if (_rMax > 32767.0f)
		{
			_clip=true;
			_rMax = 32767.0f; //_RMAX = 32768;
		}
		else if (_rMax < 1.0f) { _rMax = 1.0f; /*_RMAX = 1;*/ }
//		else _RMAX = Dsp::F2I(_rMax);

		if ( _lMax > currentpeak ) currentpeak = _lMax;
		if ( _rMax > currentpeak ) currentpeak = _rMax;


#endif // _WINAMP_PLUGIN_
//	}

	sampleCount+=numSamples;
	
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // _WINAMP_PLUGIN_
	_worked = true;
}

bool Master::Load(
				   RiffFile& riffFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));
	
	char _edit_name[16];
	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));
	
	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));
	
	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol
	
	riffFile.Read(&_outDry, sizeof(int)); // outdry
	riffFile.Read(&junk[0], sizeof(int)); // outwet
	
	riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
	riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distNegClamp
	
	riffFile.Read(&junk[0], sizeof(char)); // sinespeed
	riffFile.Read(&junk[0], sizeof(char)); // sineglide
	riffFile.Read(&junk[0], sizeof(char)); // sinevolume
	riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
	riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp
	
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeL
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackL
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackR
	
	riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
	riffFile.Read(&junk[0], sizeof(int)); // filterResonance
	riffFile.Read(&junk[0], sizeof(int)); // filterLfospeed
	riffFile.Read(&junk[0], sizeof(int)); // filterLfoamp
	riffFile.Read(&junk[0], sizeof(int)); // filterLfophase
	riffFile.Read(&junk[0], sizeof(int)); // filterMode
	
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef CONVERT_INTERNAL_MACHINES
Gainer::Gainer(int index)
{
	_macIndex = index;
	_outWet = 256;
	_numPars = 2;
	_type = MACH_GAIN;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("Gainer"));
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
		float const wet = CValueMapper::Map_255_1(_outWet);
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
			if (_volumeCounter < 8.0f)	{
				_volumeCounter = 0.0f;
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
	if ( pData->_note == cdefTweakM || pData->_note == cdefTweakE )
	{
		SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Sine::Sine(int index)
{
	_macIndex = index;
	_numPars = 5;
	_type = MACH_SINE;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("Psychosc"));
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

void Sine::GetParamValue(int numparam,TCHAR* parval)
{
	switch(numparam)
	{
		case 0: _tcscpy(parval,_T("None")); break;
		case 1:
#if defined(_WINAMP_PLUGIN_)
			_stprintf(parval, _T("%.1f Hz."), Global::pConfig->_samplesPerSec * (_speed/6.2831853f));
#else
			_stprintf(parval, _T("%.1f Hz."), Global::pConfig->_pOutputDriver->_samplesPerSec * (_speed/6.2831853f));
#endif // _WINAMP_PLUGIN
			break;
		case 2:
			_stprintf(parval,_T("%.1f%%"),_glide*155038.76f);
			break;
		case 3:
#if defined(_WINAMP_PLUGIN_)
			_stprintf(parval, _T("%.1f Hz."), Global::pConfig->_samplesPerSec * (_lfoSpeed/6.2831853f));
#else
			_stprintf(parval, _T("%.1f Hz."), Global::pConfig->_pOutputDriver->_samplesPerSec * (_lfoSpeed/6.2831853f));
#endif // _WINAMP_PLUGIN_
			break;
		case 4:
			_stprintf(parval,_T("%.1f%%"),_lfoAmp*1550.3876f);
			break;
		default: strcpy(parval,_T("Out Of Range")); break;
	}
}

void Sine::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == cdefTweakM || pData->_note == cdefTweakE ) SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
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
			if (_volumeCounter < 8.0f)	{
				_volumeCounter = 0.0f;
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
	RiffFile& riffFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	char _edit_name[16];
	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));

	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));

	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol

	riffFile.Read(&junk[0], sizeof(int)); // outwet
	riffFile.Read(&junk[0], sizeof(int)); // outdry

	riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
	riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distNegClamp

	riffFile.Read(&_sineSpeed, sizeof(_sineSpeed));
	riffFile.Read(&_sineGlide, sizeof(_sineGlide));
	riffFile.Read(&_sineVolume, sizeof(_sineVolume));
	riffFile.Read(&_sineLfoSpeed, sizeof(_sineLfoSpeed));
	riffFile.Read(&_sineLfoAmp, sizeof(_sineLfoAmp));

	riffFile.Read(&junk[0], sizeof(int)); // delayTimeL
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackL
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackR

	riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
	riffFile.Read(&junk[0], sizeof(int)); // filterResonance
	riffFile.Read(&junk[0], sizeof(int)); // filterLfospeed
	riffFile.Read(&junk[0], sizeof(int)); // filterLfoamp
	riffFile.Read(&junk[0], sizeof(int)); // filterLfophase
	riffFile.Read(&junk[0], sizeof(int)); // filterMode
	Update();

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Distortion::Distortion(int index)
{
	_macIndex = index;
	_numPars = 5;
	_type = MACH_DIST;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("Distortion"));
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
			if (_volumeCounter < 8.0f)	{
				_volumeCounter = 0.0f;
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
	if ( pData->_note == cdefTweakM || pData->_note == cdefTweakE ) SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
}

bool Distortion::Load(
	RiffFile& riffFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	char _edit_name[16];
	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));

	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));

	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol

	riffFile.Read(&junk[0], sizeof(int)); // outwet
	riffFile.Read(&junk[0], sizeof(int)); // outdry

	riffFile.Read(&_posThreshold, sizeof(_posThreshold));
	riffFile.Read(&_posClamp, sizeof(_posClamp));
	riffFile.Read(&_negThreshold, sizeof(_negThreshold));
	riffFile.Read(&_negClamp, sizeof(_negClamp));

	riffFile.Read(&junk[0], sizeof(char)); // sinespeed
	riffFile.Read(&junk[0], sizeof(char)); // sineglide
	riffFile.Read(&junk[0], sizeof(char)); // sinevolume
	riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
	riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp

	riffFile.Read(&junk[0], sizeof(int)); // delayTimeL
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackL
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackR

	riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
	riffFile.Read(&junk[0], sizeof(int)); // filterResonance
	riffFile.Read(&junk[0], sizeof(int)); // filterLfospeed
	riffFile.Read(&junk[0], sizeof(int)); // filterLfoamp
	riffFile.Read(&junk[0], sizeof(int)); // filterLfophase
	riffFile.Read(&junk[0], sizeof(int)); // filterMode

	return true;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Delay::Delay(int index)
{
	_macIndex = index;
	_numPars = 7;
	_outWet = 256;
	_outDry = 256;
	_type = MACH_DELAY;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("Delay"));

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

void Delay::GetParamValue(int numparam,TCHAR* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,_T("None")); break;
		case 1:
#if defined(_WINAMP_PLUGIN_)
			_stprintf(parval,_T("%.2fms (%d)"),_timeL/Global::pConfig->_samplesPerSec,_timeL);
#else
			_stprintf(parval,_T("%.2fms (%d)"),_timeL/Global::pConfig->_pOutputDriver->_samplesPerSec,_timeL);
#endif // _WINAMP_PLUGIN_
			break;
		case 2:
			_stprintf(parval, _T("%d%%"), _feedbackL);
			break;
		case 3:
#if defined(_WINAMP_PLUGIN_)
			_stprintf(parval,_T("%.2fms (%d)"),_timeR/Global::pConfig->_samplesPerSec,_timeR);
#else
			_stprintf(parval,_T("%.2fms (%d)"),_timeR/Global::pConfig->_pOutputDriver->_samplesPerSec,_timeR);
#endif // _WINAMP_PLUGIN_
			break;
		case 4:
			_stprintf(parval, _T("%d%%"), _feedbackR);
			break;
		case 5:
			_stprintf(parval, _T("%d%%"), CValueMapper::Map_255_100(_outDry));
			break;
		case 6:
			_stprintf(parval, _T("%d%%"), CValueMapper::Map_255_100(_outWet));
			break;
		default: strcpy(parval,_T("Out Of Range")); break;
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
		float dry = CValueMapper::Map_255_1(_outDry);
		float wet = CValueMapper::Map_255_1(_outWet);
		
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
			if (_volumeCounter < 8.0f)	{
				_volumeCounter = 0.0f;
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
	RiffFile& riffFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	
	char _edit_name[16];
	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));

	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));

	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol

	riffFile.Read(&_outDry, sizeof(_outDry));
	riffFile.Read(&_outWet, sizeof(_outWet));

	riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
	riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distNegClamp

	riffFile.Read(&junk[0], sizeof(char)); // sinespeed
	riffFile.Read(&junk[0], sizeof(char)); // sineglide
	riffFile.Read(&junk[0], sizeof(char)); // sinevolume
	riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
	riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp

	int timeL;
	int timeR;
	int feedbackL;
	int feedbackR;
	riffFile.Read(&timeL, sizeof(_timeL));
	riffFile.Read(&timeR, sizeof(_timeR));
	riffFile.Read(&feedbackL, sizeof(_feedbackL));
	riffFile.Read(&feedbackR, sizeof(_feedbackR));

	riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
	riffFile.Read(&junk[0], sizeof(int)); // filterResonance
	riffFile.Read(&junk[0], sizeof(int)); // filterLfospeed
	riffFile.Read(&junk[0], sizeof(int)); // filterLfoamp
	riffFile.Read(&junk[0], sizeof(int)); // filterLfophase
	riffFile.Read(&junk[0], sizeof(int)); // filterMode

	Update(timeL, timeR, feedbackL, feedbackR);
	return true;
}

void Delay::Tick(int channel, PatternEntry *pData)
{
	if ( pData->_note == cdefTweakM || pData->_note == cdefTweakE )
	{
		if ( pData->_cmd & 0x80 ) SetParameter(pData->_inst,(((pData->_cmd&0x7F)<<8) + pData->_parameter)*(-1));
		else SetParameter(pData->_inst,((pData->_cmd&0x7F)<<8) + pData->_parameter);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Flanger::Flanger(int index)
{
	_macIndex = index;
	_numPars = 9;
	_outWet = 256;
	_outDry = 256;
	_type = MACH_FLANGER;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("Flanger"));
//	_resampler.SetQuality(RESAMPLE_LINEAR);// ADVISE!!!! Only linear resample can be done. SPLINE needs samples ahead
	useResample=true;
	
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
void Flanger::GetParamValue(int numparam,TCHAR* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,_T("None")); break;
		case 1:
#if defined(_WINAMP_PLUGIN_)
			_stprintf(parval,_T("%.2fms (%d)"),_time/Global::pConfig->_samplesPerSec,_time);
#else
			_stprintf(parval,_T("%.2fms (%d)"),_time/Global::pConfig->_pOutputDriver->_samplesPerSec,_time);
#endif // _WINAMP_PLUGIN_
			break;
		case 2:
			_stprintf(parval, _T("%d%%"), CValueMapper::Map_255_100(_lfoAmp));
			break;
		case 3:
#if defined(_WINAMP_PLUGIN_)
			_stprintf(parval,_T("%.2fms (%d)"),_lfoSpeed/Global::pConfig->_samplesPerSec,_lfoSpeed);
#else
			_stprintf(parval,_T("%.2fms (%d)"),_lfoSpeed/Global::pConfig->_pOutputDriver->_samplesPerSec,_lfoSpeed);
#endif // _WINAMP_PLUGIN_)
			break;
		case 4:
			_stprintf(parval, _T("%d%%"), _feedbackL);
			break;
		case 5:
			_stprintf(parval, _T("%d Degrees"), _lfoPhase*1.40625);
			break;
		case 6:
			_stprintf(parval, _T("%d%%"), _feedbackR);
			break;
		case 7:
			_stprintf(parval, _T("%.1f%%"), CValueMapper::Map_255_100(_outDry));
			break;
		case 8:
			_stprintf(parval, _T("%.1f%%"), CValueMapper::Map_255_100(_outWet));
			break;
		default: strcpy(parval,_T("Out Of Range")); break;
	}
}

void Flanger::Work(
	int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_

	static float const TWOPI = 6.28318530717958647692528676655901f;

	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		const float fdbkL =(float)_feedbackL*0.01f;
		const float fdbkR =(float)_feedbackR*0.01f;
		const float dry =CValueMapper::Map_255_1(_outDry);
		const float wet =CValueMapper::Map_255_1(_outWet);
		
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

		if ( useResample)
		{
			PRESAMPLERFN pResamplerWork;
			pResamplerWork = Global::pResampler->_pWorkFn;

			int i = numSamples;
			do
			{
				y0L = b1*y1L - y2L;
				y2L = y1L;
				y1L = y0L;
				y0R = b1*y1R - y2R;
				y2R = y1R;
				y1R = y0R;
				
				ULARGE_INTEGER tmposcL;
				tmposcL.QuadPart = (__int64)((y0L*_fLfoAmp) *0x100000000);
				int _delayedCounterL = _counter - _time + tmposcL.HighPart;
				
				if (_delayedCounterL < 0) _delayedCounterL += 2048;
				int c = (_delayedCounterL==2047) ? 0 : _delayedCounterL+1;
				
				float y_l = pResamplerWork( 0 ,
					_pBufferL[_delayedCounterL],
					_pBufferL[c], 0,
					tmposcL.LowPart, _delayedCounterL, 2050); // Since we already take care or buffer overrun, we set the length bigger.
				if (IS_DENORMAL(y_l) ) y_l=0.0f;
				
				ULARGE_INTEGER tmposcR;
				tmposcR.QuadPart = (__int64)((y0R*_fLfoAmp) *0x100000000);
				int _delayedCounterR = _counter - _time + tmposcR.HighPart;
				
				if (_delayedCounterR < 0) _delayedCounterR += 2048;
				c = (_delayedCounterR==2047) ? 0 : _delayedCounterR+1;
				
				float y_r = pResamplerWork(	0,
					_pBufferR[_delayedCounterR],
					_pBufferR[c], 0,
					tmposcR.LowPart, _delayedCounterR, 2050); // Since we already take care or buffer overrun, we set the length bigger.
				if (IS_DENORMAL(y_r) ) y_r=0.0f;
				
				const float inputL = *++pSamplesL;
				const float inputR = *++pSamplesR;
				
				_pBufferL[_counter] = inputL + y_l*fdbkL;
				_pBufferR[_counter] = inputR + y_r*fdbkR;
				
				*pSamplesL = inputL*dry + y_l*wet;
				*pSamplesR = inputR*dry + y_r*wet;
				
				_dLfoPos += _dLfoSpeed;
				if (_dLfoPos >= TWOPI)
				{
					_dLfoPos -= TWOPI;
				}
				if (++_counter >= 2048)	{ _counter = 0; }
			}
			while (--i);
		}
		else
		{
			int i = numSamples;
			do
			{
				y0L = b1*y1L - y2L;
				y2L = y1L;
				y1L = y0L;
				y0R = b1*y1R - y2R;
				y2R = y1R;
				y1R = y0R;
				
//		Unoptimized ones
//			sin(_fLfoDegree)*_fLfoAmp;	// 0 <= _fLfoDegree <= TWOPI
//			sin(_fLfoDegree+_fLfoPhase)*_fLfoAmp; // 0 <= _fLfoPhase <= 3PI/2
//		Optimized ones
//			y0L*_fLfoAmp;
//			y0R*_fLfoAmp;
				
				int _delayedCounterL, _delayedCounterR;
				if ( y0L< 0 ) _delayedCounterL = _counter - _time + (int)(y0L*_fLfoAmp)-1; //  (int)0.9 = (int)-0.9
				else _delayedCounterL = _counter - _time + (int)(y0L*_fLfoAmp);
				if (_delayedCounterL < 0) _delayedCounterL += 2048;

				if ( y0R< 0 ) _delayedCounterR = _counter - _time + (int)(y0R*_fLfoAmp)-1;
				else _delayedCounterR = _counter - _time + (int)(y0R*_fLfoAmp);
				if (_delayedCounterR < 0) _delayedCounterR += 2048;


				float y_l = _pBufferL[_delayedCounterL];
				if (IS_DENORMAL(y_l) ) y_l=0.0f;

				float y_r = _pBufferR[_delayedCounterR];
				if (IS_DENORMAL(y_r) ) y_r=0.0f;
	
				const float inputL = *++pSamplesL;
				const float inputR = *++pSamplesR;
				
				_pBufferL[_counter] = inputL + y_l*fdbkL;
				_pBufferR[_counter] = inputR + y_r*fdbkR;
				
				*pSamplesL = inputL*dry + y_l*wet;
				*pSamplesR = inputR*dry + y_r*wet;

				_dLfoPos += _dLfoSpeed;
				if (_dLfoPos >= TWOPI)
				{
					_dLfoPos -= TWOPI;
				}
				if (++_counter >= 2048)	{ _counter = 0; }
			}
			while (--i);
		}
#if !defined(_WINAMP_PLUGIN_)
		Machine::SetVolumeCounter(numSamples);
		if ( Global::pConfig->autoStopMachines )
		{
//			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8.0f)	{
				_volumeCounter = 0.0f;
				_volumeDisplay = 0;
				_stopped = true;
			}
		}
//		else Machine::SetVolumeCounter(numSamples);
#endif // ndef _WINAMP_PLUGIN_
	}
	else
	{
		_dLfoPos += _dLfoSpeed*numSamples;
		while (_dLfoPos > TWOPI)
		{
			_dLfoPos -= TWOPI;
		}
		memset(_pBufferL,0,numSamples*sizeof(float));
		memset(_pBufferR,0,numSamples*sizeof(float));
		_counter = 0;
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

	if ( pData->_note == cdefTweakM || pData->_note == cdefTweakE ) {
		switch(pData->_inst)
		{
		case 1: _time=cmd & 0x400;Update();break;
		case 2: _lfoAmp=(cmd & 0x7F)+1;Update();break;
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
	_fLfoAmp = CValueMapper::Map_255_1(_lfoAmp)*(float)(_time-1);
	_fLfoPhase = (float)_lfoPhase*0.012271846f;
}

void Flanger::SetPreset(
	int preset)
{
	switch (preset)
	{
	case 0:	// default
		_time = 75;
		_lfoAmp = 246;
		_lfoSpeed = 3748;
		_feedbackL = 65;
		_lfoPhase = 32;
		_feedbackR = 49;
		_outDry = 256;
		_outWet = 256;
		break;
		
	case 1:	// Chorus 
		_time = 227;
		_lfoAmp = 246;
		_lfoSpeed = 23543;
		_feedbackL = 28;
		_lfoPhase = 112;
		_feedbackR = -24;
		_outDry = 236;
		_outWet = 256;
		break;
		
	case 2:	// Chorus 2
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
	RiffFile& riffFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	char _edit_name[16];
	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));

	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));

	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol

	riffFile.Read(&_outDry, sizeof(_outDry));
	riffFile.Read(&_outWet, sizeof(_outWet));

	riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
	riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distNegClamp

	riffFile.Read(&junk[0], sizeof(char)); // sinespeed
	riffFile.Read(&junk[0], sizeof(char)); // sineglide
	riffFile.Read(&junk[0], sizeof(char)); // sinevolume
	riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
//	riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp <- old meaning
	riffFile.Read(&useResample, sizeof(bool)); // <- new meaning
	
	riffFile.Read(&_time, sizeof(_time));
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
	riffFile.Read(&_feedbackL, sizeof(_feedbackL));
	riffFile.Read(&_feedbackR, sizeof(_feedbackR));

	riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
	riffFile.Read(&junk[0], sizeof(int)); // filterResonance
	riffFile.Read(&_lfoSpeed, sizeof(_lfoSpeed));
	riffFile.Read(&_lfoAmp, sizeof(_lfoAmp));
	riffFile.Read(&_lfoPhase, sizeof(_lfoPhase));
	riffFile.Read(&junk[0], sizeof(int)); // filterMode

	Update();
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Filter2p::Filter2p(int index)
{
	_macIndex = index;
	_numPars = 7;
	_type = MACH_2PFILTER;
	_mode = MACHMODE_FX;
	_stprintf(_editName, _T("2p Filter"));
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
			if (_volumeCounter < 8.0f)	{
				_volumeCounter = 0.0f;
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
	if ( pData->_note == cdefTweakM || pData->_note == cdefTweakE ) {
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

void Filter2p::GetParamValue(int numparam,TCHAR* parval)
{
	switch(numparam)
	{
		case 0: strcpy(parval,_T("None")); break;
		case 1: _stprintf(parval,_T("%s"),_filterMode?_T("Low Pass 2Pole -12db"):_T("High Pass 2Pole -12db")); break;
#if defined(_WINAMP_PLUGIN_)
		case 2: _stprintf(parval,_T("%d (%d Hz)"),_cutoff, Global::pConfig->_samplesPerSec * asin( _fCutoff) / 3.1415926f); break;
#else
		case 2: _stprintf(parval,_T("%d (%d Hz)"),_cutoff, Global::pConfig->_pOutputDriver->_samplesPerSec * asin( _fCutoff) / 3.1415926f); break;
#endif // _WINAMP_PLUGIN_
		case 3: _stprintf(parval,_T("%d (%d%%)"),_resonance,CValueMapper::Map_255_100(_resonance)); break;
#if defined(_WINAMP_PLUGIN_)
		case 4: _stprintf(parval,_T("%d (%.2f Hz)"),_lfoSpeed,_fLfoSpeed*Global::pConfig->_samplesPerSec / 6.283185f); break;
#else
		case 4: _stprintf(parval,_T("%d (%.2f Hz)"),_lfoSpeed,_fLfoSpeed*Global::pConfig->_pOutputDriver->_samplesPerSec / 6.283185f); break;
#endif // _WINAMP_PLUGIN_
		case 5: _stprintf(parval,_T("%%d (%d%%)"),_lfoAmp,CValueMapper::Map_255_100(_lfoAmp)); break;
		case 6: _stprintf(parval,_T("%d (%d deg)"),_lfoPhase,_lfoPhase*0.703125f); break;
		default: strcpy(parval,_T("Out Of Range")); break;
	}
}


void Filter2p::Update(void)
{
	_fCutoff = CValueMapper::Map_255_1(_cutoff);
	_fResonance = CValueMapper::Map_255_1(_resonance );
	_fLfoSpeed = _lfoSpeed * 0.00000003f;
	_fLfoAmp = CValueMapper::Map_255_1(_lfoAmp);
	_fLfoPhase = _lfoPhase * 0.0122718f;
}

bool Filter2p::Load(
	RiffFile& riffFile)
{
	char junk[256];
		memset(&junk, 0, sizeof(junk));

	char _edit_name[16];
	riffFile.Read(&_edit_name,16);
	_edit_name[15] = 0;
	_tcscpy(_editName,CA2T(_edit_name));

	riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
	riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
	riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
	riffFile.Read(&_connection[0], sizeof(_connection));
	riffFile.Read(&_inputCon[0], sizeof(_inputCon));
#if defined (_WINAMP_PLUGIN_)
	riffFile.Skip(96) ; // sizeof(CPoint) = 8.
#else
	riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	riffFile.Read(&_numInputs, sizeof(_numInputs));
	riffFile.Read(&_numOutputs, sizeof(_numOutputs));

	riffFile.Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	riffFile.Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	riffFile.Read(&junk[0], sizeof(int)); // numSubtracks
	riffFile.Read(&junk[0], sizeof(int)); // interpol

	riffFile.Read(&junk[0], sizeof(int)); // outwet
	riffFile.Read(&junk[0], sizeof(int)); // outdry

	riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
	riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
	riffFile.Read(&junk[0], sizeof(int)); // distNegClamp

	riffFile.Read(&junk[0], sizeof(char)); // sinespeed
	riffFile.Read(&junk[0], sizeof(char)); // sineglide
	riffFile.Read(&junk[0], sizeof(char)); // sinevolume
	riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
	riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp

	riffFile.Read(&junk[0], sizeof(int)); // delayTimeL
	riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackL
	riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackR

	riffFile.Read(&_cutoff, sizeof(_cutoff));
	riffFile.Read(&_resonance, sizeof(_resonance));
	riffFile.Read(&_lfoSpeed, sizeof(_lfoSpeed));
	riffFile.Read(&_lfoAmp, sizeof(_lfoAmp));
	riffFile.Read(&_lfoPhase, sizeof(_lfoPhase));
	riffFile.Read(&_filterMode, sizeof(_filterMode));

	Update();
	return true;
}

#endif