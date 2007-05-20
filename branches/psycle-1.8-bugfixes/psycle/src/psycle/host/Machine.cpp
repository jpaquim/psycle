///\file
///\brief implementation file for psycle::host::Machine
#include <project.private.hpp>
#include "Machine.hpp"
#include "Song.hpp"
#include "Dsp.hpp"
#include "Configuration.hpp"
#include "psycle.hpp"
#include "WireDlg.hpp"
#include "MainFrm.hpp"
#include "InputHandler.hpp"
#include "NewMachine.hpp"
// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include "Sampler.hpp"
#include "XMSampler.hpp"
#include "Plugin.hpp"
#include "VSTHost24.hpp"
namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		char* Master::_psName = "Master";
		char* Dummy::_psName = "DummyPlug";
		char* DuplicatorMac::_psName = "Dupe it!";
		char* Mixer::_psName = "Mixer";

		void Machine::crashed(std::exception const & e) throw()
		{
			bool minor_problem(false);
			bool crash(false);
			{
				exceptions::function_error const * const function_error(dynamic_cast<exceptions::function_error const * const>(&e));
				if(function_error)
				{
					operating_system::exceptions::translated const * const translated(dynamic_cast<operating_system::exceptions::translated const * const>(function_error->exception()));
					if(translated)
					{
						crash = true;
						switch(translated->code())
						{
							// grows the fpu exception mask so that each type of exception is only reported once
							case STATUS_FLOAT_INEXACT_RESULT:    fpu_exception_mask().inexact(true)     ; minor_problem = true ; break;
							case STATUS_FLOAT_DENORMAL_OPERAND:  fpu_exception_mask().denormal(true)    ; minor_problem = true ; break;
							case STATUS_FLOAT_DIVIDE_BY_ZERO:    fpu_exception_mask().divide_by_0(true) ;                        break;
							case STATUS_FLOAT_OVERFLOW:          fpu_exception_mask().overflow(true)    ;                        break;
							case STATUS_FLOAT_UNDERFLOW:         fpu_exception_mask().underflow(true)   ; minor_problem = true ; break;
							case STATUS_FLOAT_STACK_CHECK:                                                                       break;
							case STATUS_FLOAT_INVALID_OPERATION: fpu_exception_mask().invalid(true)     ;                        break;
						}
					}
				}
			}
			if(!minor_problem)
			{
				///\todo do we need thread synchronization?
				///\todo gui needs to update
				crashed_ = true;
				_bypass = true;
				_mute = true;
			}
			std::ostringstream s;
			s << "Machine: " << _editName;
			if(GetDllName()) s << ": " << GetDllName();
			s << std::endl << e.what() << std::endl;
			if(minor_problem)
			{
				s << "This is a minor problem: the machine won't be disabled and further occurences of the problem won't be reported anymore.";
				host::loggers::warning(s.str());
			}
			else
			{
				s
					<< "This is a serious error: the machine has been set to bypassed/muted to prevent it from making the host crash."
					<< std::endl
					<< "You should save your work to a new file, and restart the host.";
				if(crash)
				{
					//host::loggers::crash(s.str()); // already colorized and reported as crash by the exception constructor
					host::loggers::exception(s.str());
				}
				else
				{
					host::loggers::exception(s.str());
				}
			}
			MessageBox(0, s.str().c_str(), crash ? "Exception (Crash)" : "Exception (Software)", MB_OK | (minor_problem ? MB_ICONWARNING : MB_ICONERROR));
			///\todo in the case of a minor_problem, we would rather continue the execution at the point the cpu/os exception was triggered.
		}
		Machine::Machine(MachineType msubclass, MachineMode mode, int id)
		{
		//	Machine();
			_type=msubclass;
			_mode=mode;
			_macIndex=id;
		}

		Machine::Machine()
			: crashed_()
			, fpu_exception_mask_()
			, _macIndex(0)
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

		Machine::~Machine() throw()
		{
			zapArray(_pSamplesL);
			zapArray(_pSamplesR);
		}

		void Machine::Init()
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
				_inputMachines[i]=-1;
				_outputMachines[i]=-1;
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
			cpu::cycles_type wcost = cpu::cycles();
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
			dsp::Clear(_pSamplesL, numSamples);
			dsp::Clear(_pSamplesR, numSamples);
			_wireCost += cpu::cycles() - wcost;
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
									processor::fpu::exception_mask fpu_exception_mask(pInMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
								#endif
								pInMachine->Work(numSamples);
							}
							{
								//Disable bad-behaving machines
								///\todo: add a better approach later on, 
								if (pInMachine->_cpuCost >= Global::_cpuHz && !Global::pPlayer->_recording)
								{
									if ( pInMachine->_mode == MACHMODE_GENERATOR) pInMachine->_mute=true;
									else pInMachine->_bypass=true;
								}
							}
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
						}
						if(!pInMachine->_stopped) _stopped = false;
						if(!_mute && !_stopped)
						{
							cpu::cycles_type wcost = cpu::cycles();
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
							_wireCost += cpu::cycles() - wcost;
						}
					}
				}
			}
			_waitingForSound = false;
			cpu::cycles_type wcost = cpu::cycles();
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			_wireCost += cpu::cycles() - wcost;
		}

		//Modified version of Machine::Work(). The only change is the removal of mixing inputs into one stream.
		void Machine::WorkNoMix(int numSamples)
		{
			_waitingForSound=true;
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_inputCon[i])
				{
					Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
					if (pInMachine)
					{
						if (!pInMachine->_worked && !pInMachine->_waitingForSound)
						{ 
							{
#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
								universalis::processor::exceptions::fpu::mask fpu_exception_mask(pInMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
#endif
								pInMachine->Work(numSamples);
							}
						}
						if(!pInMachine->_stopped) _stopped = false;
					}
				}
			}
			_waitingForSound = false;
		}

		bool Machine::LoadSpecificChunk(RiffFile* pFile, int version)
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
			case MACH_XMSAMPLER:
				if ( !fullopen ) pMachine = new Dummy(index);
				else pMachine = new XMSampler(index);
				break;
			case MACH_DUPLICATOR:
				if ( !fullopen ) pMachine = new Dummy(index);
				else pMachine = new DuplicatorMac(index);
				break;
			case MACH_MIXER:
				if ( !fullopen ) pMachine = new Dummy(index);
				else pMachine = new Mixer(index);
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
							MessageBox(NULL,sError, "Loading Error", MB_OK);
							pMachine = new Dummy(index);
							delete p;
							bDeleted = true;
						}
					}
				}
				break;
			case MACH_VST:
			case MACH_VSTFX:
				{
					if(!fullopen) pMachine = new Dummy(index);
					else 
					{
						std::string sPath;
						vst::plugin *vstPlug=0;
						int shellIdx=0;

						if(!CNewMachine::lookupDllName(dllName,sPath,shellIdx)) 
						{
							// Check Compatibility Table.
							// Probably could be done with the dllNames lockup.
							//GetCompatible(psFileName,sPath2) // If no one found, it will return a null string.
							sPath = dllName;
						}
						if(CNewMachine::TestFilename(sPath,shellIdx) ) 
						{
							vstPlug = dynamic_cast<vst::plugin*>(Global::vsthost().LoadPlugin(sPath.c_str(),shellIdx));
						}

						if(!vstPlug)
						{
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing VST plug-in \"%s\" with Dummy.",dllName);
							MessageBox(NULL,sError, "Loading Error", MB_OK);
							pMachine = new Dummy(index);
							((Dummy*)pMachine)->wasVST=true;
							bDeleted = true;
						}
						else
						{
							vstPlug->_macIndex=index;
							pMachine = vstPlug;
						}
					}
				}
				break;
			default:
				if (type != MACH_DUMMY ) MessageBox(0, "Please inform the devers about this message: unknown kind of machine while loading new file format", "Loading Error", MB_OK | MB_ICONERROR);
				pMachine = new Dummy(index);
				break;
			}
			pMachine->Init();
			if(!bDeleted)
			{
				///\todo: Is it even necessary???
				pMachine->_type = type;
			}
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
			if(!pMachine->LoadSpecificChunk(pFile,version))
			{
				char sError[MAX_PATH + 100];
				sprintf(sError,"Missing or Corrupted Machine Specific Chunk \"%s\" - replacing with Dummy.",dllName);
				MessageBox(NULL,sError, "Loading Error", MB_OK);
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
			pMachine->SetPan(pMachine->_panning);
			return pMachine;
		}


		void Machine::SaveFileChunk(RiffFile* pFile)
		{
			pFile->Write(&_type,sizeof(_type));
			SaveDllNameAndIndex(pFile,GetShellIdx());
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

		void Machine::SaveDllNameAndIndex(RiffFile* pFile,int index)
		{
			CString str = GetDllName();
			char str2[256];
			if ( str.IsEmpty()) str2[0]=0;
			else strcpy(str2,str.Mid(str.ReverseFind('\\')+1));

			if (index != 0)
			{
				char idxtext[5];
				int divisor=16777216;
				idxtext[4]=0;
				for (int i=0; i < 4; i++)
				{
					int residue = index%divisor;
					idxtext[3-i]=index/divisor;
					index = residue;
					divisor=divisor/256;
				}
				strcat(str2,idxtext);
			}
			pFile->Write(&str2,strlen(str2)+1);		};



		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy



		Dummy::Dummy(int index)
		{
			_macIndex = index;
			_type = MACH_DUMMY;
			_mode = MACHMODE_FX;
			sprintf(_editName, _psName);
			wasVST = false;
		}
		void Dummy::Work(int numSamples)
		{
			Machine::Work(numSamples);
			cpu::cycles_type cost = cpu::cycles();
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
			_cpuCost += cpu::cycles() - cost;
			_worked = true;
		}

		// Since Dummy is used by the loader to load broken/missing plugins, 
		// its "LoadSpecificChunk" skips the data of the chunk so that the
		// song loader can continue the sequence.
		bool Dummy::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Skip(size);
			return true;
		};
		

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// NoteDuplicator
		DuplicatorMac::DuplicatorMac(int index)
		{
			_macIndex = index;
			_numPars = NUMMACHINES*2;
			_nCols = 2;
			_type = MACH_DUPLICATOR;
			_mode = MACHMODE_GENERATOR;
			bisTicking = false;
			sprintf(_editName, _psName);
			for (int i=0;i<NUMMACHINES;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
				for (int j=0;j<MAX_TRACKS;j++)
				{
					allocatedchans[j][i] = -1;
				}
			}
			for (int i=0;i<MAX_MACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					availablechans[i][j] = true;
				}
			}
		}
		void DuplicatorMac::Init()
		{
			Machine::Init();
			for (int i=0;i<NUMMACHINES;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
				for (int j=0;j<MAX_TRACKS;j++)
				{
					allocatedchans[j][i] = -1;
				}
			}
			for (int i=0;i<MAX_MACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					availablechans[i][j] = true;
				}
			}
		}
		void DuplicatorMac::Stop()
		{
			for (int i=0;i<NUMMACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					allocatedchans[j][i] = -1;
				}
			}
			for (int i=0;i<MAX_MACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					availablechans[i][j] = true;
				}
			}
		}
		void DuplicatorMac::Tick()
		{

		}

		void DuplicatorMac::Tick( int channel,PatternEntry* pData)
		{
			if ( !_mute && !bisTicking) // Prevent possible loops of dupe machines.
			{
				bisTicking=true;
				for (int i=0;i<NUMMACHINES;i++)
				{
					if (macOutput[i] != -1 && Global::_pSong->_pMachine[macOutput[i]] != NULL )
					{
						AllocateVoice(channel,i);
						PatternEntry pTemp = *pData;
						if ( pTemp._note < 120 )
						{
							int note = pTemp._note+noteOffset[i];
							if ( note>=120) note=119;
							else if (note<0 ) note=0;
							pTemp._note=(uint8)note;
						}
						
						// the first part can happen if the parameter is the machine itself.
						if (Global::_pSong->_pMachine[macOutput[i]] != this) 
						{
							Global::_pSong->_pMachine[macOutput[i]]->Tick(allocatedchans[channel][i],&pTemp);
							if (pTemp._note >= 120 )
							{
								DeallocateVoice(channel,i);
							}
						}
					}
				}
			}
			bisTicking=false;
		}
		void DuplicatorMac::AllocateVoice(int channel,int machine)
		{
			// If this channel already has allocated channels, use them.
			if ( allocatedchans[channel][machine] != -1 )
				return;
			// If not, search an available channel
			int j=channel;
			while (j<MAX_TRACKS && !availablechans[macOutput[machine]][j]) j++;
			if ( j == MAX_TRACKS)
			{
				j=0;
				while (j<MAX_TRACKS && !availablechans[macOutput[machine]][j]) j++;
				if (j == MAX_TRACKS)
				{
					j= MAX_TRACKS * static_cast<unsigned int>(rand())/((RAND_MAX+1)*2);
				}
			}
			allocatedchans[channel][machine]=j;
			availablechans[macOutput[machine]][j]=false;
		}
		void DuplicatorMac::DeallocateVoice(int channel, int machine)
		{
			if ( allocatedchans[channel][machine] == -1 )
				return;
			availablechans[macOutput[machine]][allocatedchans[channel][machine]]= true;
			allocatedchans[channel][machine]=-1;
		}
		void DuplicatorMac::GetParamName(int numparam,char *name)
		{
			if (numparam >=0 && numparam<NUMMACHINES)
			{
				sprintf(name,"Output Machine %d",numparam);
			} else if (numparam >=NUMMACHINES && numparam<NUMMACHINES*2) {
				sprintf(name,"Note Offset %d",numparam-NUMMACHINES);
			}
			else name[0] = '\0';
		}
		void DuplicatorMac::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if ( numparam < NUMMACHINES) { minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if ( numparam < NUMMACHINES*2) { minval = -48; maxval = 48; }
		}
		int DuplicatorMac::GetParamValue(int numparam)
		{
			if (numparam >=0 && numparam<NUMMACHINES)
			{
				return macOutput[numparam];
			} else if (numparam >=NUMMACHINES && numparam <NUMMACHINES*2) {
				return noteOffset[numparam-NUMMACHINES];
			}
			else return 0;
		}
		void DuplicatorMac::GetParamValue(int numparam, char *parVal)
		{
			if (numparam >=0 && numparam <NUMMACHINES)
			{
				if ((macOutput[numparam] != -1 ) &&( Global::_pSong->_pMachine[macOutput[numparam]] != NULL))
				{
					sprintf(parVal,"%X -%s",macOutput[numparam],Global::_pSong->_pMachine[macOutput[numparam]]->_editName);
				}else if (macOutput[numparam] != -1) sprintf(parVal,"%X (none)",macOutput[numparam]);
				else sprintf(parVal,"(disabled)");

			} else if (numparam >= NUMMACHINES && numparam <NUMMACHINES*2) {
				char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
				sprintf(parVal,"%s%d",notes[(noteOffset[numparam-NUMMACHINES]+60)%12],(noteOffset[numparam-NUMMACHINES]+60)/12);
			}
			else parVal[0] = '\0';
		}
		bool DuplicatorMac::SetParameter(int numparam, int value)
		{
			if (numparam >=0 && numparam<NUMMACHINES)
			{
				macOutput[numparam]=value;
				return true;
			} else if (numparam >=NUMMACHINES && numparam<NUMMACHINES*2) {
				noteOffset[numparam-NUMMACHINES]=value;
				return true;
			}
			else return false;
		}

		void DuplicatorMac::Work(int numSamples)
		{
			_worked = true;
			_stopped = false;
		}
		bool DuplicatorMac::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Read(&macOutput,sizeof macOutput);
			pFile->Read(&noteOffset,sizeof noteOffset);
			return true;
		}

		void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof macOutput+ sizeof noteOffset;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&macOutput,sizeof macOutput);
			pFile->Write(&noteOffset,sizeof noteOffset);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Master



		float * Master::_pMasterSamples = 0;

		Master::Master(int index)
		{
			_macIndex = index;
			sampleCount = 0;
			_outDry = 256;
			decreaseOnClip=false;
			_type = MACH_MASTER;
			_mode = MACHMODE_MASTER;
			sprintf(_editName, _psName);
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
			#if PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS
				processor::fpu::exception_mask fpu_exception_mask(this->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
			#endif
			Machine::Work(numSamples);
			cpu::cycles_type cost = cpu::cycles();
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
			_cpuCost += cpu::cycles() - cost;
			_worked = true;
		}

		bool Master::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size ); // size of this part params to load
			pFile->Read(&_outDry,sizeof _outDry);
			pFile->Read(&decreaseOnClip, sizeof decreaseOnClip);
			return true;
		};

		void Master::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof _outDry + sizeof decreaseOnClip;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&_outDry,sizeof _outDry);
			pFile->Write(&decreaseOnClip, sizeof decreaseOnClip); 
		};



		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mixer

		Mixer::Mixer(int id)
		{
			_macIndex = id;
			_type = MACH_MIXER;
			_mode = MACHMODE_FX;
			sprintf(_editName, _psName);

		}

		Mixer::~Mixer() throw()
		{
		}

		void Mixer::Init()
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

		void Mixer::Tick( int channel,PatternEntry* pData)
		{
			if(pData->_note == notecommands::tweak)
			{
				int nv = (pData->_cmd<<8)+pData->_parameter;
				SetParameter(pData->_inst,nv);
				Global::player().Tweaker = true;
			}
			else if(pData->_note == notecommands::tweakslide)
			{
				//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
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
				cpu::cycles_type cost = cpu::cycles();
				Mix(numSamples);
				Machine::SetVolumeCounter(numSamples);
				if ( Global::configuration().autoStopMachines )
				{
					if (_volumeCounter < 8.0f)
					{
						_volumeCounter = 0.0f;
						_volumeDisplay = 0;
						_stopped = true;
					}
					else _stopped = false;
				}
				_cpuCost += cpu::cycles() - cost;
			}

			cpu::cycles_type cost = cpu::cycles();
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			_cpuCost += cpu::cycles() - cost;

			_worked = true;
		}

		void Mixer::FxSend(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::song()._pMachine[_send[i]];
					if (pSendMachine)
					{
						if (!pSendMachine->_worked && !pSendMachine->_waitingForSound)
						{ 
							// Mix all the inputs and route them to the send fx.
							{
								cpu::cycles_type cost = cpu::cycles();
								for (int j=0; j<MAX_CONNECTIONS; j++)
								{
									if (_inputCon[j])
									{
										Machine* pInMachine = Global::song()._pMachine[_inputMachines[j]];
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
								_cpuCost += cpu::cycles() - cost ;
							}

							// tell the FX to work, now that the input is ready.
							{
#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
								universalis::processor::exceptions::fpu::mask fpu_exception_mask(pSendMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
#endif
								pSendMachine->Work(numSamples);
							}

							{
								cpu::cycles_type cost = cpu::cycles();
								pSendMachine->_waitingForSound = false;
								dsp::Clear(_pSamplesL, numSamples);
								dsp::Clear(_pSamplesR, numSamples);
								_cpuCost += cpu::cycles() - cost;
							}

						}
						if(!pSendMachine->_stopped) _stopped = false;
					}
				}
			}
		}

		void Mixer::Mix(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::song()._pMachine[_send[i]];
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
					Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
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

/*		std::string Mixer::GetAudioInputName(int port)
		{
			std::string rettxt;
			if (port < return1 )
			{	
				rettxt = "Input ";
				rettxt += ('0'+port-chan1);
				return rettxt;
			}
			else if ( port <= return12)
			{
				rettxt = "Return ";
				rettxt += ('0'+port-return1);
				return rettxt;
			}
			rettxt = "-";
			return rettxt;
		}
*/
		std::string Mixer::GetAudioInputName(int port)
		{
			std::string rettxt;
			if (port < return1 )
			{	
				rettxt = "Input ";
				rettxt += ('0'+port-chan1);
				return rettxt;
			}
			else if ( port <= return12)
			{
				rettxt = "Return ";
				rettxt += ('0'+port-return1);
				return rettxt;
			}
			rettxt = "-";
			return rettxt;
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
			int send(numparam % 16); // 0 is for channel mix, others are send.
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

		bool Mixer::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(&size,sizeof(size));
			pFile->Read(_sendGrid,sizeof(_sendGrid));
			pFile->Read(_send,sizeof(_send));
			pFile->Read(_sendVol,sizeof(_sendVol));
			pFile->Read(_sendVolMulti,sizeof(_sendVolMulti));
			pFile->Read(_sendValid,sizeof(_sendValid));
			return true;
		}

		void Mixer::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof _sendGrid + sizeof _send + sizeof _sendVol + sizeof _sendVolMulti + sizeof _sendValid);
			pFile->Write(&size,sizeof(size));
			pFile->Write(_sendGrid,sizeof(_sendGrid));
			pFile->Write(_send,sizeof(_send));
			pFile->Write(_sendVol,sizeof(_sendVol));
			pFile->Write(_sendVolMulti,sizeof(_sendVolMulti));
			pFile->Write(_sendValid,sizeof(_sendValid));
		}

		float Mixer::VuChan(int idx)
		{
			float vol;
			GetWireVolume(idx,vol);
			if ( _inputCon[idx] ) return (Global::song()._pMachine[_inputMachines[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}

		float Mixer::VuSend(int idx)
		{
			float vol = _sendVol[idx] * _sendVolMulti[idx];
			if ( _sendValid[idx] ) return (Global::song()._pMachine[_send[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}

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
			pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
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
			pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
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
