#include "stdafx.h"
#include "Machine.h"
#include "Song.h"
#include "Dsp.h"
#include "Configuration.h"
#if !defined _WINAMP_PLUGIN_
	#include "psycle.h"
	#include "WireDlg.h"
	#include "MainFrm.h"
	#include "InputHandler.h"
#endif
// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include "Sampler.h"
#include "Plugin.h"
#include "VSTHost.h"
///\file
///\brief implementation file for psycle::host::Machine
namespace psycle
{
	namespace host
	{
		#if !defined _WINAMP_PLUGIN_
			extern CPsycleApp theApp;
		#endif

		char* Master::_psName = "Master";
		char* Dummy::_psName = "DummyPlug";

		void Machine::crashed(const std::exception & e) throw()
		{
			///\todo do we need thread synchronization?
			///\todo gui needs to update
			crashed_ = true;
			_bypass = true;
			_mute = true;
			std::ostringstream title; title
				<< "Machine crashed: "
				<< _editName;
			if(GetDllName()) title
				<< ": " << GetDllName();
			host::logger(host::logger::exception, title.str() + '\n' + e.what());
			//::MessageBox(0, e.what(), title.str().c_str(), MB_OK | MB_ICONWARNING);
		}

		Machine::Machine()
			: crashed_(false)
			, _numPars(0)
			, TWSSamples(0)
			, TWSActive(false)
			#if !defined _WINAMP_PLUGIN_
				, _volumeCounter(0.0f)
				, _volumeDisplay(0)
				, _volumeMaxDisplay(0)
				, _volumeMaxCounterLife(0)
				, _pScopeBufferL(0)
				, _pScopeBufferR(0)
				, _scopeBufferIndex(0)
				, _scopePrevNumSamples(0)
			#endif
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
			}
			for (int c = 0; c<MAX_TWS; c++)
			{
				TWSInst[c] = 0;
				TWSDelta[c] = 0;
				TWSCurrent[c] = 0;
				TWSDestination[c] = 0;
			}
		}

		Machine::~Machine() throw()
		{
			delete _pSamplesL;
			delete _pSamplesR;
		}

		void Machine::Init()
		{
			// Standard gear initalization
			#if !defined _WINAMP_PLUGIN_
				_cpuCost = 0;
				_wireCost = 0;
			#endif
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

		void Machine::SetPan(int newPan)
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
					_inputConVol[wireIndex] = value*32768.0f;
					_wireMultiplier[wireIndex] = 0.000030517578125f;
				}
			}
			else if ( _type == MACH_VST || _type == MACH_VSTFX ) // native to VST, divide.
			{
				_inputConVol[wireIndex] = value*0.000030517578125f;
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
			Machine *_pDstMachine = Global::_pSong->_pMachine[_outputMachines[WireIndex]];

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
			Machine *_pDstMachine = Global::_pSong->_pMachine[_outputMachines[WireIndex]];
			
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
					int i = _scopePrevNumSamples;
					while (i > 0)   
					{   
						if (i+_scopeBufferIndex >= SCOPE_BUF_SIZE)   
						{   
							memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1)*sizeof(float));
							memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1)*sizeof(float));
							pSamplesL+=(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
							pSamplesR+=(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
							i -= (SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
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
				_scopePrevNumSamples=numSamples;
				Dsp::Clear(_pSamplesL, numSamples);
				Dsp::Clear(_pSamplesR, numSamples);
				CPUCOST_CALC(cost, numSamples);
				//_cpuCost = cost;
				_wireCost+= cost;
			#endif // _WINAMP_PLUGIN_
		}

		/// Each machine is expected to produce its output in its own
		/// _pSamplesX buffers.
		void Machine::Work(int numSamples)
		{
			_waitingForSound=true;
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_inputCon[i])
				{
					Machine* pInMachine = Global::_pSong->_pMachine[_inputMachines[i]];
					if (pInMachine)
					{
						if (!pInMachine->_worked && !pInMachine->_waitingForSound)
						{ 
							pInMachine->Work(numSamples);
							/*
							This could be a different Undenormalize funtion, using the already calculated
							"_volumeCounter".Note: It needs that muted&|bypassed machines set the variable
							correctly.
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
							#if defined( _WINAMP_PLUGIN_)
								Dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
								Dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
							#else
								CPUCOST_INIT(wcost);
								Dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
								Dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
								CPUCOST_CALC(wcost,numSamples);
								_wireCost+=wcost;
							#endif
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
			#endif
		}

		bool Machine::LoadSpecificFileChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size,sizeof(size)); // size of this part params to load
			UINT count;
			pFile->Read(&count,sizeof(count)); // num params to load
			for (UINT i = 0; i < count; i++)
			{
				int temp;
				pFile->Read(&temp,sizeof(temp));
				SetParameter(i,temp);
			}
			pFile->Skip(size-sizeof(count)-(count*sizeof(int)));
			return true;
		};

		Machine* Machine::LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen)
		{
			// assume version 0 for now
			bool bDeleted(false);
			Machine* pMachine;
			MachineType type;//,oldtype;
			char dllName[256];
			pFile->Read(&type,sizeof(type));
			//oldtype=type;
			pFile->ReadString(dllName,256);
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
			case MACH_PLUGIN:
				{
					if(!fullopen) pMachine = new Dummy(index);
					else 
					{
						Plugin * p;
						pMachine = p = new Plugin(index);
						if(!p->LoadDll(dllName))
						{
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing Native plug-in \"%s\" with Dummy.",dllName);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);
							pMachine = new Dummy(index);
							type = MACH_DUMMY;
							delete p;
							bDeleted = true;
						}
					}
				}
				break;
			case MACH_VST:
				{
					if(!fullopen) pMachine = new Dummy(index);
					else 
					{
						vst::instrument * p;
						pMachine = p = new vst::instrument(index);
						if(!p->LoadDll(dllName))
						{
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing VST Generator plug-in \"%s\" with Dummy.",dllName);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);
							pMachine = new Dummy(index);
							type = MACH_DUMMY;
							delete p;
							bDeleted = true;
						}
					}
				}
				break;
			case MACH_VSTFX:
				{
					if(!fullopen) pMachine = new Dummy(index);
					else 
					{
						vst::fx * p;
						pMachine = p = new vst::fx(index);
						if(!p->LoadDll(dllName))
						{
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing VST Effect plug-in \"%s\" with Dummy.",dllName);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);
							pMachine = new Dummy(index);
							type = MACH_DUMMY;
							delete p;
							bDeleted = true;
						}
					}
				}
				break;
			default:
				::MessageBox(0, "Please inform the devers about this message: unknown kind of machine while loading new file format", "Loading Error", MB_OK | MB_ICONERROR);
				pMachine = new Dummy(index);
				break;
			}
			pMachine->Init();
			pMachine->_type = type;
			pFile->Read(&pMachine->_bypass,sizeof(pMachine->_bypass));
			pFile->Read(&pMachine->_mute,sizeof(pMachine->_mute));
			pFile->Read(&pMachine->_panning,sizeof(pMachine->_panning));
			pFile->Read(&pMachine->_x,sizeof(pMachine->_x));
			pFile->Read(&pMachine->_y,sizeof(pMachine->_y));
			pFile->Read(&pMachine->_numInputs,sizeof(pMachine->_numInputs));							// number of Incoming connections
			pFile->Read(&pMachine->_numOutputs,sizeof(pMachine->_numOutputs));						// number of Outgoing connections
			for(int i = 0; i < MAX_CONNECTIONS; i++)
			{
				pFile->Read(&pMachine->_inputMachines[i],sizeof(pMachine->_inputMachines[i]));	// Incoming connections Machine number
				pFile->Read(&pMachine->_outputMachines[i],sizeof(pMachine->_outputMachines[i]));	// Outgoing connections Machine number
				pFile->Read(&pMachine->_inputConVol[i],sizeof(pMachine->_inputConVol[i]));	// Incoming connections Machine vol
				pFile->Read(&pMachine->_wireMultiplier[i],sizeof(pMachine->_wireMultiplier[i]));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
				pFile->Read(&pMachine->_connection[i],sizeof(pMachine->_connection[i]));      // Outgoing connections activated
				pFile->Read(&pMachine->_inputCon[i],sizeof(pMachine->_inputCon[i]));		// Incoming connections activated
			}
			pFile->ReadString(pMachine->_editName,32);
			if(bDeleted)
			{
				char buf[34];
				sprintf(buf,"X %s",pMachine->_editName);
				buf[31]=0;
				strcpy(pMachine->_editName,buf);
			}
			if(!fullopen) return pMachine;
			if(!pMachine->LoadSpecificFileChunk(pFile,version))
			{
				char sError[MAX_PATH + 100];
				sprintf(sError,"Missing or Corrupted Machine Specific Chunk \"%s\" - replacing with Dummy.",dllName);
				::MessageBox(NULL,sError, "Loading Error", MB_OK);
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
			#if !defined _WINAMP_PLUGIN_
				if(index < MAX_BUSES)
				{
					pMachine->_mode = MACHMODE_GENERATOR;
					if(pMachine->_x > Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width)
						pMachine->_x = Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
					if(pMachine->_y > Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height)
						pMachine->_y = Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
				}
				else if (index < MAX_BUSES*2)
				{
					pMachine->_mode = MACHMODE_FX;
					if(pMachine->_x > Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width)
						pMachine->_x = Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
					if(pMachine->_y > Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height)
						pMachine->_y = Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
				}
				else
				{
					pMachine->_mode = MACHMODE_MASTER;
					if(pMachine->_x > Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width)
						pMachine->_x = Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
					if(pMachine->_y > Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height)
						pMachine->_y = Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;
				}
			#endif
			pMachine->SetPan(pMachine->_panning);
			return pMachine;
		}

		#if !defined _WINAMP_PLUGIN_

			void Machine::SaveFileChunk(RiffFile* pFile)
			{
				pFile->Write(&_type,sizeof(_type));
				SaveDllName(pFile);
				pFile->Write(&_bypass,sizeof(_bypass));
				pFile->Write(&_mute,sizeof(_mute));
				pFile->Write(&_panning,sizeof(_panning));
				pFile->Write(&_x,sizeof(_x));
				pFile->Write(&_y,sizeof(_y));
				pFile->Write(&_numInputs,sizeof(_numInputs));							// number of Incoming connections
				pFile->Write(&_numOutputs,sizeof(_numOutputs));						// number of Outgoing connections
				for(int i = 0; i < MAX_CONNECTIONS; i++)
				{
					pFile->Write(&_inputMachines[i],sizeof(_inputMachines[i]));	// Incoming connections Machine number
					pFile->Write(&_outputMachines[i],sizeof(_outputMachines[i]));	// Outgoing connections Machine number
					pFile->Write(&_inputConVol[i],sizeof(_inputConVol[i]));	// Incoming connections Machine vol
					pFile->Write(&_wireMultiplier[i],sizeof(_wireMultiplier[i]));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
					pFile->Write(&_connection[i],sizeof(_connection[i]));      // Outgoing connections activated
					pFile->Write(&_inputCon[i],sizeof(_inputCon[i]));		// Incoming connections activated
				}
				pFile->Write(_editName,strlen(_editName)+1);
				SaveSpecificChunk(pFile);
			}

			void Machine::SaveSpecificChunk(RiffFile* pFile) 
			{
				UINT count = GetNumParams();
				UINT size = sizeof(count)+(count*sizeof(int));
				pFile->Write(&size,sizeof(size));
				pFile->Write(&count,sizeof(count));
				for(UINT i = 0; i < count; i++)
				{
					int temp = GetParamValue(i);
					pFile->Write(&temp,sizeof(temp));
				}
			};

			void Machine::SaveDllName(RiffFile* pFile)
			{
				char temp=0;
				pFile->Write(&temp,1);
			};

		#endif // ndef _WINAMP_PLUGIN_



		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy



		Dummy::Dummy(int index)
		{
			_macIndex = index;
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
					//Machine::SetVolumeCounterAccurate(numSamples);
					if (_volumeCounter < 8.0f)	{
						_volumeCounter = 0.0f;
						_volumeDisplay = 0;
						_stopped = true;
					}
				}
				//else Machine::SetVolumeCounter(numSamples);
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
			#endif // ndef _WINAMP_PLUGIN_
			_worked = true;
		}

		bool Dummy::LoadSpecificFileChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Skip(size);
			return true;
		};
		


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Master



		float * Master::_pMasterSamples = 0;

		Master::Master(int index)
		{
			_macIndex = index;
			sampleCount = 0;
			_numPars = 0;
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
			#if !defined(_WINAMP_PLUGIN_)
				//_LMAX = 1; // Min value should NOT be zero, because we use a log10() to calculate the vu-meter's value.
				//_RMAX = 1;
				currentpeak=0.0f;
				peaktime=1;
				_lMax = 1;
				_rMax = 1;
				vuupdated = false;
				_clip = false;
			#endif
		}

		void Master::Work(int numSamples)
		{
			Machine::Work(numSamples);
			#if !defined(_WINAMP_PLUGIN_)
				CPUCOST_INIT(cost);
			#endif
			//if(!_mute)
			//{
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
				#endif // _WINAMP_PLUGIN_
			//}
			sampleCount+=numSamples;
			#if !defined(_WINAMP_PLUGIN_)
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
			#endif
			_worked = true;
		}

		bool Master::LoadSpecificFileChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size ); // size of this part params to load
			pFile->Read(&_outDry,sizeof _outDry);
			pFile->Read(&decreaseOnClip, sizeof decreaseOnClip); // numSubtracks
			return true;
		};

		#if !defined _WINAMP_PLUGIN_

			void Master::SaveSpecificChunk(RiffFile* pFile)
			{
				UINT size = sizeof _outDry + sizeof decreaseOnClip;
				pFile->Write(&size, sizeof size); // size of this part params to load
				pFile->Write(&_outDry,sizeof _outDry);
				pFile->Write(&decreaseOnClip, sizeof decreaseOnClip); 
			};

		#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// old file format vomit. don't look at it!



		// old file format vomit. don't look at it!
		bool Machine::Load(RiffFile* pFile)
		{
			char junk[256];
			std::memset(&junk, 0, sizeof(junk));
			pFile->Read(&_editName,16);
			_editName[15] = 0;
			pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
			pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
			pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
			pFile->Read(&_connection[0], sizeof(_connection));
			pFile->Read(&_inputCon[0], sizeof(_inputCon));
			#if defined (_WINAMP_PLUGIN_)
				pFile->Skip(96) ; // sizeof(CPoint) = 8.
			#else
				pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
			#endif
			pFile->Read(&_numInputs, sizeof(_numInputs));
			pFile->Read(&_numOutputs, sizeof(_numOutputs));

			pFile->Read(&_panning, sizeof(_panning));
			Machine::SetPan(_panning);
			pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
			pFile->Read(&junk[0], sizeof(int)); // numSubtracks
			pFile->Read(&junk[0], sizeof(int)); // interpol

			pFile->Read(&junk[0], sizeof(int)); // outdry
			pFile->Read(&junk[0], sizeof(int)); // outwet

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

		// old file format vomit. don't look at it!
		bool Master::Load(RiffFile* pFile)
		{
			char junk[256];
			memset(&junk, 0, sizeof(junk));
			
			pFile->Read(&_editName,16);
			_editName[15] = 0;
			
			pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
			pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
			pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
			pFile->Read(&_connection[0], sizeof(_connection));
			pFile->Read(&_inputCon[0], sizeof(_inputCon));
			#if defined (_WINAMP_PLUGIN_)
				pFile->Skip(96) ; // sizeof(CPoint) = 8.
			#else
				pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
			#endif
			pFile->Read(&_numInputs, sizeof(_numInputs));
			pFile->Read(&_numOutputs, sizeof(_numOutputs));
			
			pFile->Read(&_panning, sizeof(_panning));
			Machine::SetPan(_panning);
			pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
			pFile->Read(&junk[0], sizeof(int)); // numSubtracks
			pFile->Read(&junk[0], sizeof(int)); // interpol
			
			pFile->Read(&_outDry, sizeof(int)); // outdry
			pFile->Read(&junk[0], sizeof(int)); // outwet
			
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
	}
}
