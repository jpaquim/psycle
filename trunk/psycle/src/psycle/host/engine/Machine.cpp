///\file
///\brief implementation file for psycle::host::Machine
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/engine/machine.hpp>
#include <psycle/host/engine/song.hpp>
#include <psycle/host/engine/dsp.hpp>
#include <psycle/host/configuration.hpp>
#include <psycle/host/gui/psycle.hpp> // Can this be removed?
#include <psycle/host/gui/WireDlg.hpp> // Can this be removed?
#include <psycle/host/gui/MainFrm.hpp> // Can this be removed?
#include <psycle/host/gui/InputHandler.hpp> // Can this be removed?
#include <universalis/processor/exception.hpp>

// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include <psycle/host/engine/Sampler.hpp>
#include <psycle/host/engine/XMSampler.hpp>
#include <psycle/host/engine/plugin.hpp>
#include <psycle/host/engine/VSTHost.hpp>
namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		char* Master::_psName = "Master";
		char* Dummy::_psName = "DummyPlug";
		char* DuplicatorMac::_psName = "Dupe it!";
		char* Mixer::_psName = "Mixer";
		char* LFO::_psName = "LFO";

		void Machine::crashed(std::exception const & e) throw()
		{
			bool minor_problem(false);
			bool crash(false);
			{
				exceptions::function_error const * const function_error(dynamic_cast<exceptions::function_error const * const>(&e));
				if(function_error)
				{
					universalis::processor::exception const * const translated(dynamic_cast<universalis::processor::exception const * const>(function_error->exception()));
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
		void Wire::Connect(AudioPort *senderp,AudioPort *receiverp)
		{
			senderport=senderp;
			receiverport=receiverp;
			multiplier=receiverport->GetMachine()->GetAudioRange()/senderport->GetMachine()->GetAudioRange();
			SetVolume(volume);
			senderport->Connected(this);
			receiverport->Connected(this);
			//\todo : need a way get a wire index.
		}
		void Wire::ChangeSource(AudioPort* newsource)
		{
			assert(senderport); Disconnect(senderport);
			senderport=newsource;
			multiplier=receiverport->GetMachine()->GetAudioRange()/senderport->GetMachine()->GetAudioRange();
			SetVolume(volume);
			senderport->Connected(this);
		}
		void Wire::ChangeDestination(AudioPort* newdest)
		{
			assert(receiverport); Disconnect(receiverport);
			receiverport=newdest;
			multiplier=receiverport->GetMachine()->GetAudioRange()/senderport->GetMachine()->GetAudioRange();
			SetVolume(volume);
			receiverport->Connected(this);
		}
		void Wire::CollectData(int numSamples)
		{
			senderport->CollectData(numSamples);
			//\todo : apply volume, panning and mapping.
		}
		void Wire::SetVolume(float newvol)
		{
			volume=newvol;
			rvol=volume*pan*multiplier;
			lvol=volume*(1.0f-pan)*multiplier;
		}
		void Wire::SetPan(float newpan)
		{
			pan=newpan;
			SetVolume(volume);
		}

		void Wire::Disconnect(AudioPort* port)
		{
			if ( port == senderport ) { senderport=0; }
			else { receiverport=0; }
			port->Disconnected(this);
			//\todo : need a way to indicate to the main Machine that this wire index is now free.
		}

		void AudioPort::Connected(Wire *wire)
		{
			wires.push_back(wire);
		}
		void AudioPort::Disconnected(Wire *wire)
		{
			for (unsigned int i(0);i<wires.size();i++)
			{
				if (wires[i]==wire) 
				{
					wires.erase(wires.begin()+i);
					return;
				}
			}
		}

		void InPort::CollectData(int numSamples)
		{
			//\todo : need to clean the buffer first? wire(0)processreplacing() while(wires) wire(1+).processadding() ?
			for (unsigned int i(0);i<wires.size();i++)
			{
				GetWire(i)->CollectData(numSamples);
			}
		}
		void OutPort::CollectData(int numSamples)
		{
			pParent->Work(numSamples);
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
			, _audiorange(1.0f)
			, _pSamplesL(0)
			, _pSamplesR(0)
			, _lVol(0)
			, _rVol(0)
			, _panning(0)
			, _x(0)
			, _y(0)
			, _numPars(0)
			, _nCols(1)
			, _connectedInputs(0)
			, numInPorts(0)
			, numOutPorts(0)
			, _connectedOutputs(0)
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
			work_cpu_cost(0);
			wire_cpu_cost(0);
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
				_inputMachines[i]=-1;
				_outputMachines[i]=-1;
				_inputCon[i] = false;
				_connection[i] = false;
			}
			_connectedInputs = 0;
			_connectedOutputs = 0;
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

		bool Machine::ConnectTo(Machine* dstMac,int dstport,int outport,float volume)
		{
			ASSERT(dstMac);
			if (dstMac->_mode == MACHMODE_GENERATOR) return false;

			int freebus=-1;
			int dfreebus=-1;
			bool error=false;

			// Get a free output slot on the source machine
			for(int c(MAX_CONNECTIONS - 1) ; c >= 0 ; --c)
			{
				if(!_connection[c]) freebus = c;
				// Checking that there's not a slot to the dest. machine already
				else if(_outputMachines[c] == dstMac->_macIndex) error = true;
			}
			if(freebus == -1 || error) return false;
			// Get a free input slot on the destination machine
			error=false;
			for(int c=MAX_CONNECTIONS-1; c>=0; c--)
			{
				if(!dstMac->_inputCon[c]) dfreebus = c;
				// Checking if the destination machine is connected with the source machine to avoid a loop.
				else if(dstMac->_outputMachines[c] == _macIndex) error = true;
			}
			if(dfreebus == -1 || error) return false;

			// Calibrating in/out properties
			_outputMachines[freebus] = dstMac->_macIndex;
			_connection[freebus] = true;
			_connectedOutputs++;
			dstMac->_inputMachines[dfreebus] = _macIndex;
			dstMac->_inputCon[dfreebus] = true;
			dstMac->_connectedInputs++;
			dstMac->InitWireVolume(_type,dfreebus,volume);
			return true;
		}
		bool Machine::Disconnect(Machine* dstMac)
		{
			return false;
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
			PSYCLE__CPU_COST__INIT(cost);
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
			PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
			wire_cpu_cost(wire_cpu_cost() + cost);
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
								#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
									universalis::processor::exceptions::fpu::mask fpu_exception_mask(pInMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
								#endif
								pInMachine->Work(numSamples);
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
							pInMachine->_waitingForSound = false;
						}
						if(!pInMachine->_stopped) _stopped = false;
						if(!_mute && !_stopped)
						{
							PSYCLE__CPU_COST__INIT(wcost);
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
							PSYCLE__CPU_COST__CALCULATE(wcost,numSamples);
							wire_cpu_cost(wire_cpu_cost() + wcost);
						}
					}
				}
			}
			{
				PSYCLE__CPU_COST__INIT(wcost);
					dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
				PSYCLE__CPU_COST__CALCULATE(wcost,numSamples);
				wire_cpu_cost(wire_cpu_cost() + wcost);
			}
		}

		//Modified version of Machine::Work(). The only change is the removal of mixing inputs into one stream.
		void Machine::WorkNoMix(int numSamples)
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
							{
								#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
									universalis::processor::exceptions::fpu::mask fpu_exception_mask(pInMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
								#endif
								pInMachine->Work(numSamples);
							}
							pInMachine->_waitingForSound = false;
						}
						if(!pInMachine->_stopped) _stopped = false;
					}
				}
			}
		}
		void Machine::DefineStereoInput(int numinputs)
		{
			numInPorts=numinputs;
			inports = new InPort(this,0,"Stereo In");
		}
		void Machine::DefineStereoOutput(int numoutputs)
		{
			numOutPorts=numoutputs;
			outports = new OutPort(this,0,"Stereo Out");
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
			case MACH_LFO:
				if ( !fullopen ) pMachine = new Dummy(index);
				else pMachine = new LFO(index);
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
							MessageBox(NULL,sError, "Loading Error", MB_OK);
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
							MessageBox(NULL,sError, "Loading Error", MB_OK);
							pMachine = new Dummy(index);
							type = MACH_DUMMY;
							delete p;
							bDeleted = true;
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
			pMachine->_type = type;
			pFile->Read(&pMachine->_bypass,sizeof(pMachine->_bypass));
			pFile->Read(&pMachine->_mute,sizeof(pMachine->_mute));
			pFile->Read(&pMachine->_panning,sizeof(pMachine->_panning));
			pFile->Read(&pMachine->_x,sizeof(pMachine->_x));
			pFile->Read(&pMachine->_y,sizeof(pMachine->_y));
			pFile->Read(&pMachine->_connectedInputs,sizeof(pMachine->_connectedInputs));							// number of Incoming connections
			pFile->Read(&pMachine->_connectedOutputs,sizeof(pMachine->_connectedOutputs));						// number of Outgoing connections
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
				p->_connectedInputs=pMachine->_connectedInputs;							// number of Incoming connections
				p->_connectedOutputs=pMachine->_connectedOutputs;						// number of Outgoing connections
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
			SaveDllName(pFile);
			pFile->Write(&_bypass,sizeof(_bypass));
			pFile->Write(&_mute,sizeof(_mute));
			pFile->Write(&_panning,sizeof(_panning));
			pFile->Write(&_x,sizeof(_x));
			pFile->Write(&_y,sizeof(_y));
			pFile->Write(&_connectedInputs,sizeof(_connectedInputs));							// number of Incoming connections
			pFile->Write(&_connectedOutputs,sizeof(_connectedOutputs));						// number of Outgoing connections
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



		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy



		Dummy::Dummy(int index)
		{
			DefineStereoInput(1);
			DefineStereoOutput(1);
			_macIndex = index;
			_type = MACH_DUMMY;
			_mode = MACHMODE_FX;
			sprintf(_editName, "Dummy");
		}
		void Dummy::Work(int numSamples)
		{
			Machine::Work(numSamples);
			PSYCLE__CPU_COST__INIT(cost);
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
			PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
			work_cpu_cost(work_cpu_cost() + cost);
			_worked = true;
		}

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
					if (macOutput[i] != -1 && Global::_pSong->_pMachine[macOutput[i]] != NULL 
						&& Global::_pSong->_pMachine[macOutput[i]] != this) Global::_pSong->_pMachine[macOutput[i]]->Tick(channel,&pTemp);
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
				if ((macOutput[numparam] != -1 ) &&( Global::_pSong->_pMachine[macOutput[numparam]] != NULL))
				{
					sprintf(parVal,"%X -%s",macOutput[numparam],Global::_pSong->_pMachine[macOutput[numparam]]->_editName);
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
			: sampleCount(0)
			, _outDry(256)
			, decreaseOnClip(false)
		{
			_audiorange=32768.0f;

			DefineStereoInput(1);
			_macIndex = index;
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
			#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
				universalis::processor::exceptions::fpu::mask fpu_exception_mask(this->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
			#endif
			Machine::Work(numSamples);
			PSYCLE__CPU_COST__INIT(cost);
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
			PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
			work_cpu_cost(work_cpu_cost() + cost);
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
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mixer

		Mixer::Mixer(int index)
		{
			_macIndex = index;
			_numPars = 255;
			DefineStereoInput(24);
			DefineStereoOutput(1);
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
		void Mixer::Tick( int channel,PatternEntry* pData)
		{
			if(pData->_note == cdefTweakM)
			{
				int nv = (pData->_cmd<<8)+pData->_parameter;
				SetParameter(pData->_inst,nv);
				Global::pPlayer->Tweaker = true;
			}
			else if(pData->_note == cdefTweakS)
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
				PSYCLE__CPU_COST__INIT(cost);
					Mix(numSamples);
					Machine::SetVolumeCounter(numSamples);
					if ( Global::pConfig->autoStopMachines )
					{
						if (_volumeCounter < 8.0f)
						{
							_volumeCounter = 0.0f;
							_volumeDisplay = 0;
							_stopped = true;
						}
						else _stopped = false;
					}
				PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
				work_cpu_cost(work_cpu_cost() + cost);
			}

			{
				PSYCLE__CPU_COST__INIT(wcost);
					dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
				PSYCLE__CPU_COST__CALCULATE(wcost,numSamples);
				wire_cpu_cost(wire_cpu_cost() + wcost);
			}

			_worked = true;
		}

		void Mixer::FxSend(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::_pSong->_pMachine[_send[i]];
					if (pSendMachine)
					{
						if (!pSendMachine->_worked && !pSendMachine->_waitingForSound)
						{ 
							// Mix all the inputs and route them to the send fx.
							{
								PSYCLE__CPU_COST__INIT(cost);
									for (int j=0; j<MAX_CONNECTIONS; j++)
									{
										if (_inputCon[j])
										{
											Machine* pInMachine = Global::_pSong->_pMachine[_inputMachines[j]];
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
								PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
								work_cpu_cost(work_cpu_cost() + cost);
							}

							// tell the FX to work, now that the input is ready.
							{
								#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
									universalis::processor::exceptions::fpu::mask fpu_exception_mask(pSendMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
								#endif
								pSendMachine->Work(numSamples);
							}

							{
								PSYCLE__CPU_COST__INIT(cost);
									pSendMachine->_waitingForSound = false;
									dsp::Clear(_pSamplesL, numSamples);
									dsp::Clear(_pSamplesR, numSamples);
								PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
								work_cpu_cost(work_cpu_cost() + cost);
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
					Machine* pSendMachine = Global::_pSong->_pMachine[_send[i]];
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
					Machine* pInMachine = Global::_pSong->_pMachine[_inputMachines[i]];
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
		bool Mixer::ConnectTo(Machine* dstMac,int dstport,int outport,float volume)
		{
			//
			//
			//
			return Machine::ConnectTo(dstMac,dstport,outport,volume);
		}
		std::string Mixer::GetAudioInputName(int port) {
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

		bool Mixer::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(size);
			pFile->Read(&_sendGrid,sizeof(_sendGrid));
			pFile->Read(&_send,sizeof(_send));
			pFile->Read(&_sendVol,sizeof(_sendVol));
			pFile->Read(&_sendVolMulti,sizeof(_sendVolMulti));
			pFile->Read(&_sendValid,sizeof(_sendValid));
			return true;
		};

		void Mixer::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof(_sendGrid) + sizeof(_send) + sizeof(_sendVol) + sizeof(_sendVolMulti) + sizeof(_sendValid);
			pFile->Write(size);
			pFile->Write(&_sendGrid,sizeof(_sendGrid));
			pFile->Write(&_send,sizeof(_send));
			pFile->Write(&_sendVol,sizeof(_sendVol));
			pFile->Write(&_sendVolMulti,sizeof(_sendVolMulti));
			pFile->Write(&_sendValid,sizeof(_sendValid));
		};
		float Mixer::VuChan(int idx)
		{
			float vol;
			GetWireVolume(idx,vol);
			if ( _inputCon[idx] ) return (Global::_pSong->_pMachine[_inputMachines[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}
		float Mixer::VuSend(int idx)
		{
			float vol = _sendVol[idx] * _sendVolMulti[idx];
			if ( _sendValid[idx] ) return (Global::_pSong->_pMachine[_send[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}




		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LFO


		//		todo:
		//	- as is, control rate is proportional to STREAM_SIZE.. we update in work, which (at the moment) means once every 256
		//      samples. at 44k, this means a cr of 142hz.  this is probably good enough for most purposes, but i believe it also
		//		means that the lfo can and likely will be phased by 5.8ms depending on where it is placed in the machine view..
		//		if we want to take the idea of modulation machines much further, we should probably put together some kind of
		//		standard place in the processing chain where these machines will work, preferably -before- any audio
		//		processing.  this should also eliminate the need for the lfo to be connected to something to work.
		//  - prettify gui
		//	- implement baseline control, or at least decide if we actually want it
		//  - pulse width for sq. wave
		//  - vst support??


		LFO::LFO(int index)
		{
			_macIndex = index;
			_numPars = 24;
			_nCols = 3;
			_type = MACH_LFO;
			_mode = MACHMODE_GENERATOR;
			bisTicking = false;
			strcpy(_editName, "LFO");
			for (int i=0;i<5;i++)
			{
				macOutput[i]=-1;
				paramOutput[i]=-1;
				level[i]=100;
				phase[i]=MAX_PHASE/2.0f;
				prevVal[i]=centerVal[i]=0.0;
			}
			lfoPos=0.0;
			lSpeed=MAX_SPEED/6;
			waveform=lt_sine;
			baseline=100;
			FillTable();
		}
		void LFO::Init()
		{
			Machine::Init();
			for (int i=0;i<5;i++)
			{
				macOutput[i]=-1;
				paramOutput[i]=-1;
				level[i]=100;
				phase[i]=MAX_PHASE/2.0f;
				prevVal[i]=centerVal[i]=0.0;
			}
			lfoPos=0.0;
			lSpeed=MAX_SPEED/6;
			waveform=lt_sine;
			baseline=100;
			FillTable();
		}
		void LFO::Tick( int channel,PatternEntry* pData)
		{
			if(!bisTicking)
			{
				bisTicking=true;
				if(pData->_cmd==0x01)	// 0x01.. seems appropriate for a machine with exactly one command, but if this goes
					lfoPos=0.0;			// against any established practices or something, let me know
			}
			bisTicking=false;
		}
		void LFO::GetParamName(int numparam,char *name)
		{
			if(numparam==0)
				sprintf(name,"Waveform");
			else if(numparam==1)
				sprintf(name,"Baseline");
			else if(numparam==2)
				sprintf(name,"Speed");
			else if (numparam<8)
				sprintf(name,"Output Machine %d",numparam-3);
			else if (numparam<13)
				sprintf(name,"Output Param %d",numparam-8);
			else if (numparam<18)
				sprintf(name,"Output Level %d",numparam-13);
			else if (numparam<23)
				sprintf(name,"Output Phase %d",numparam-18);
			else if (numparam==23)
				sprintf(name,"LFO Position");
			else name[0] = '\0';
		}
		void LFO::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if(numparam==0) { minval = 0; maxval = 4;}
			else if (numparam==1) {minval = 0; maxval = 200;}
			else if (numparam==2) {minval = 0; maxval = MAX_SPEED;}
			else if (numparam <8) {minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if (numparam <13){minval = -1; maxval = 128;}	//totally arbitrary.. better ideas?
			else if (numparam <18){minval = 0; maxval = MAX_DEPTH*2;}
			else if (numparam <23){minval = 0; maxval = MAX_PHASE; }
			else if (numparam==23){minval=0;maxval=LFO_SIZE; }
			else {minval=0;maxval=0; }
		}
		int LFO::GetParamValue(int numparam)
		{
			if(numparam==0)			return waveform;
			else if(numparam==1)	return baseline;
			else if(numparam==2)	return lSpeed;
			else if(numparam <8)	return macOutput[numparam-3];
			else if(numparam <13)	return paramOutput[numparam-8];
			else if(numparam <18)	return level[numparam-13];
			else if(numparam <23)	return phase[numparam-18];
			else if(numparam==23)   return (int)lfoPos;
			else return 0;
		}
		void LFO::GetParamValue(int numparam, char *parVal)
		{
			if(numparam==0)
			{
				switch(waveform)
				{
				case lt_sine: sprintf(parVal, "sine"); break;
				case lt_tri: sprintf(parVal, "triangle"); break;
				case lt_sawup: sprintf(parVal, "saw up"); break;
				case lt_sawdown: sprintf(parVal, "saw down"); break;
				case lt_square: sprintf(parVal, "square"); break;
				}
			} else if(numparam==1) {
				sprintf(parVal, "%i", baseline-100);
			} else if(numparam==2) {
				if(lSpeed==0)
					sprintf(parVal, "inf.");
				else
				{
					sprintf(  parVal, "%.1f ms", 100 / float(lSpeed/float(MAX_SPEED))  );		
				}
			} else if(numparam<8) {
				if ((macOutput[numparam-3] != -1 ) &&( Global::_pSong->_pMachine[macOutput[numparam-3]] != NULL))
					sprintf(parVal,"%X -%s",macOutput[numparam-3],Global::_pSong->_pMachine[macOutput[numparam-3]]->_editName);
				else if (macOutput[numparam-3] != -1)
					sprintf(parVal,"%X (none)",macOutput[numparam-3]);
				else 
					sprintf(parVal,"(disabled)");
			} else if(numparam<13) {
				if(		(macOutput[numparam-8] != -1) 
					&&	(Global::_pSong->_pMachine[macOutput[numparam-8]] != NULL)
					&&  (paramOutput[numparam-8] >= 0)	)
				{
					if		(paramOutput[numparam-8] < Global::_pSong->_pMachine[macOutput[numparam-8]]->GetNumParams())
					{
						char name[128];
						Global::_pSong->_pMachine[macOutput[numparam-8]]->GetParamName(paramOutput[numparam-8], name);
						sprintf(parVal,"%X -%s", paramOutput[numparam-8], name);
					} else
						sprintf(parVal,"%X -none", paramOutput[numparam-8]);					
				} else 
					sprintf(parVal,"(disabled)");
				
			} else if(numparam<18)
				sprintf(parVal,"%i%%", level[numparam-13]-MAX_DEPTH);
			else if(numparam<23)
				sprintf(parVal,"%.1f deg.", (phase[numparam-18]-MAX_PHASE/2.0f)/float(MAX_PHASE/2.0f) * 180.0f);
			else if(numparam==23)
				sprintf(parVal,"%.1f%%", lfoPos/(float)LFO_SIZE * 100);
			else
				parVal[0] = '\0';
		}
		bool LFO::SetParameter(int numparam, int value)
		{
			if(numparam==0)
			{
				waveform = value;
				FillTable();
				return true;
			}
			else if(numparam==1)	
			{
				baseline = value;
				return true;
			}
			else if(numparam==2)
			{
				lSpeed = value;
				return true;
			}
			else if(numparam <8)
			{
				if(macOutput[numparam-3] != value)	//afaik this if is only necessary when ctrl and shift are both down..
				{									// so keep an eye on it, it may stop being needed once we stop using
					ParamEnd(numparam-3);			// CFrameMachine for our gui
					macOutput[numparam-3] = value;
					ParamStart(numparam-3);
				}
				return true;
			}
			else if(numparam <13)
			{
				if(paramOutput[numparam-8] != value)
				{
					ParamEnd(numparam-8);
					paramOutput[numparam-8] = value;
					ParamStart(numparam-8);
				}
				return true;
			}
			else if(numparam <18)
			{
				level[numparam-13] = value;
				return true;
			}
			else if(numparam <23)
			{
				phase[numparam-18] = value;
				return true;
			}
			else return false;
		}


		void LFO::Work(int numSamples)
		{
			PSYCLE__CPU_COST__INIT(cost);

			int maxVal=0, minVal=0;
			int curVal=0, newVal=0;
			float curLFO=0.0;
			float lfoAmt=0.0;
			bool bRedraw=false;

			for(int j(0);j<5;++j)
			{
				if	(	macOutput[j] != -1		&&	Global::_pSong->_pMachine[macOutput[j]] != NULL
					&&  paramOutput[j] != -1	&&	paramOutput[j] < Global::_pSong->_pMachine[macOutput[j]]->GetNumParams() )
				{
					Global::_pSong->_pMachine[macOutput[j]]->GetParamRange(paramOutput[j], minVal, maxVal);
					curVal = Global::_pSong->_pMachine[macOutput[j]]->GetParamValue(paramOutput[j]);
					curLFO = waveTable[	int(lfoPos+phase[j]+(MAX_PHASE/2.0f)) % LFO_SIZE];
					lfoAmt = (level[j]-MAX_DEPTH)/(float)MAX_DEPTH;

					centerVal[j] -= prevVal[j] - curVal;  //compensate for external movement

					newVal = curLFO * ((maxVal-minVal)/2.0f) * lfoAmt + centerVal[j];

					if(newVal>maxVal) newVal=maxVal;
					else if(newVal<minVal) newVal=minVal;

					Global::_pSong->_pMachine[macOutput[j]]->SetParameter(paramOutput[j], newVal);	//make it happen!
					bRedraw=true;
					prevVal[j] = newVal;
				}
			}

			if(bRedraw)
				Global::pPlayer->Tweaker=true;
			bRedraw=false;

			float minms = Global::pConfig->GetSamplesPerSec()/1000.0f * 100.0f;	//100ms in samples
			lfoPos += (lSpeed/ float(MAX_SPEED)) * (LFO_SIZE/float(minms/float(numSamples)));
			if(lfoPos>LFO_SIZE) lfoPos-=LFO_SIZE;

			PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
			work_cpu_cost(work_cpu_cost() + cost);
		}

		bool LFO::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size,			sizeof size); // size of this part params to load
			pFile->Read(&waveform,		sizeof waveform);
			pFile->Read(&lSpeed,		sizeof lSpeed);
			pFile->Read(&baseline,		sizeof baseline);
			pFile->Read(&macOutput,		sizeof macOutput);
			pFile->Read(&paramOutput,	sizeof paramOutput);
			pFile->Read(&level,			sizeof level);
			pFile->Read(&phase,			sizeof phase);
			return true;
		}

		void LFO::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size	= sizeof waveform	+ sizeof lSpeed +	sizeof baseline 
						+ sizeof macOutput	+ sizeof paramOutput
						+ sizeof level		+ sizeof phase;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&waveform,		sizeof waveform);
			pFile->Write(&lSpeed,		sizeof lSpeed);
			pFile->Write(&baseline,		sizeof baseline);
			pFile->Write(&macOutput,	sizeof macOutput);
			pFile->Write(&paramOutput,	sizeof paramOutput);
			pFile->Write(&level,		sizeof level);
			pFile->Write(&phase,		sizeof phase);
		}

		void LFO::FillTable()
		{
			switch(waveform)
			{
			case lt_sine:	for (int i(0);i<LFO_SIZE;++i)
							{
								waveTable[i]=sin(i/float(LFO_SIZE-1) * 6.2831853071795864769252);
							}
							break;
			case lt_tri:	for (int i(0);i<LFO_SIZE/2;++i)
							{
								waveTable[i] = waveTable[LFO_SIZE-i-1] = i/float(LFO_SIZE/4) - 1;
							}
							break;
			case lt_sawup:	for (int i(0);i<LFO_SIZE;++i)
							{
								waveTable[i] = i/float((LFO_SIZE-1)/2) - 1;
							}
							break;
			case lt_sawdown: for(int i(0);i<LFO_SIZE;++i)
							{
								waveTable[i] = (LFO_SIZE-i)/float((LFO_SIZE-1)/2) - 1;
							}
							break;
			case lt_square:	for (int i(0);i<LFO_SIZE/2;++i)
							{
								waveTable[i] = 1;
								waveTable[LFO_SIZE-1-i]=-1;
							}
							break;
			default:		for(int i(0);i<LFO_SIZE;++i)	//????
							{
								waveTable[i] = 0;
							}
							break;
			}
		}

		void LFO::ParamStart(int which)
		{
			if(which<0 || which>4) return;  //jic
			int destMac = macOutput[which];
			int destParam = paramOutput[which];

			if	(	destMac		!= -1	&&	Global::_pSong->_pMachine[destMac] != NULL
				&&  destParam	!= -1	&&	destParam < Global::_pSong->_pMachine[destMac]->GetNumParams())
			{
				int minVal, maxVal;
				float curLFO, lfoAmt;

				Global::_pSong->_pMachine[destMac]->GetParamRange(destParam, minVal, maxVal);
				curLFO = waveTable[	int(lfoPos+phase[which]+(MAX_PHASE/2.0f)) % LFO_SIZE];
				lfoAmt = (level[which]-MAX_DEPTH)/(float)MAX_DEPTH;

				//bad! bad!
				//prevVal[which] = Global::_pSong->_pMachine[macOutput[which]]->GetParamValue(paramOutput[which]);
				//centerVal[which] = prevVal[which] - (curLFO * ((maxVal-minVal)/2.0f) * lfoAmt);

				centerVal[which] = Global::_pSong->_pMachine[destMac]->GetParamValue(destParam);
				prevVal[which] = centerVal[which];
				
				// the way i've set this up, a control will 'jump' if the lfo is at a peak or dip when a control is first selected.
				// that may seem like a bad thing, but it means that the center of the lfo does -not- depend on the lfo position
				// when a control is selected.. and i think that this would be the preferred behavior.

			}
			else
			{
				prevVal[which]=0;
				centerVal[which]=0;
			}
		}


		//currently, this function resets the value of an output parameter to where it would be at lfo==0.  this behavior deserves
		// some consideration, because it is conceivable that some people would want turning off an output to leave the parameter where it is
		// instead of jerking it back to its original position..  on the other hand, without this code, sweeping through a list of parameters
		// carelessly can wreak havoc on an entire plugin's settings.  i may decide just to let the user choose which s/he prefers..
		void LFO::ParamEnd(int which)
		{
			if(which<0 || which>4) return;
			int destMac = macOutput[which];
			int destParam = paramOutput[which];

			if	(	destMac		!= -1	&&	Global::_pSong->_pMachine[destMac] != NULL
				&&  destParam	!= -1	&&	destParam < Global::_pSong->_pMachine[destMac]->GetNumParams())
			{
				int minVal, maxVal;
				int newVal;
				Global::_pSong->_pMachine[destMac]->GetParamRange(destParam, minVal, maxVal);
				newVal = centerVal[which];
				if(newVal<minVal) newVal=minVal;
				else if(newVal>maxVal) newVal=maxVal;

				if(destMac != _macIndex)	//craziness may ensue without this check.. folks routing the lfo to itself are on their own
					Global::_pSong->_pMachine[destMac]->SetParameter(destParam, newVal); //set to value at lfo==0
			}
		}


	}
}
