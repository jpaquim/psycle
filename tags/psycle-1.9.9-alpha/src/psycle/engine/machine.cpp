///\file
///\brief implementation file for psycle::host::Machine
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/engine/machine.hpp>
#include <psycle/engine/song.hpp>
#include <psycle/engine/FileIO.hpp>
#include <psycle/engine/configuration.hpp>
#include <psycle/helpers/dsp.hpp>
#include <psycle/host/psycle.hpp> // Can this be removed?
#include <psycle/host/WireDlg.hpp> // Can this be removed?
#include <psycle/host/MainFrm.hpp> // Can this be removed?
//#include <psycle/host/InputHandler.hpp> // Can this be removed?
#include <universalis/processor/exception.hpp>
#include <algorithm>

#include <psycle/engine/internal_machine_package.hpp>
// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include <psycle/engine/internal_machines.hpp>
#include <psycle/engine/Sampler.hpp>
#include <psycle/engine/XMSampler.hpp>
#include <psycle/engine/plugin.hpp>
#include <psycle/engine/VSTHost.hpp>

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

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
						#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
							// todo, we need some portable exception codes!
						#else
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
						#endif
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
			s << "Machine: " << GetEditName() << ": " << GetDllName();
			s << std::endl << e.what() << std::endl;
			if(minor_problem)
			{
				s << "This is a minor problem: the machine won't be disabled and further occurences of the problem won't be reported anymore.";
				host::loggers::warning(s.str());
			}
			else
			{
				s
					<< "This is a serious error: the machine has been set to bypassed/muted to prevent it from making the host crash." << std::endl
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
			if ( port == senderport ) senderport=0; else receiverport=0;
			port->Disconnected(this);
			//\todo : need a way to indicate to the main Machine that this wire index is now free.
		}

		void AudioPort::Connected(Wire *wire)
		{
			wires_.push_back(wire);
		}

		void AudioPort::Disconnected(Wire *wire)
		{
			wires_type::iterator i(std::find(wires_.begin(), wires_.end(), wire));
			assert(i != wires_.end());
			wires_.erase(i);
		}

		void InPort::CollectData(int numSamples)
		{
			//\todo : need to clean the buffer first? wire(0)processreplacing() while(wires) wire(1+).processadding() ?
			for(wires_type::const_iterator i(wires_.begin()); i != wires_.end(); ++i)
			{
				(**i).CollectData(numSamples);
			}
		}

		void OutPort::CollectData(int numSamples)
		{
			parent_.Work(numSamples);
		}


//////////////////////////////////////////////////////////////////////////

		Machine* CreateFromType(Machine::class_type msubclass,Machine::id_type id,std::string dllname)
		{
			const InternalMachineInfo* minfo = Machine::infopackage().GetInfo(msubclass); 
			if (minfo)
			{
				return	minfo->CreateFromType(id,dllname);
			}
			return 0;
		}

		Machine::Machine(Machine::class_type msubclass, Machine::mode_type mode, Machine::id_type id)
		:
			_subclass(msubclass),
			_mode(mode),
			_macIndex(id),
			crashed_(),
			fpu_exception_mask_(),
			_bypass(false),
			_mute(false),
			_waitingForSound(false),
			_stopped(false),
			_worked(false),
			_audiorange(1.0f),
			_pSamplesL(0),
			_pSamplesR(0),
			_lVol(0),
			_rVol(0),
			_panning(0),
			_x(0),
			_y(0),
			_nCols(1),
			_connectedInputs(0),
			numInPorts(0),
			numOutPorts(0),
			_connectedOutputs(0),
			TWSSamples(0),
			TWSActive(false),
			_volumeCounter(0.0f),
			_volumeDisplay(0),
			_volumeMaxDisplay(0),
			_volumeMaxCounterLife(0),
			_pScopeBufferL(0),
			_pScopeBufferR(0),
			_scopeBufferIndex(0),
			_scopePrevNumSamples(0)
		{
			_editName = infopackage().GetInfo(subclass())->shortname;
			_numPars = infopackage().GetInfo(subclass())->parameters;
			_pSamplesL = new float[MAX_BUFFER_LENGTH];
			_pSamplesR = new float[MAX_BUFFER_LENGTH];
			// Clear machine buffer samples
			for (int c=0; c<MAX_BUFFER_LENGTH; c++)
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
			delete[] _pSamplesL;
			delete[] _pSamplesR;
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

		bool Machine::ConnectTo(Machine & dst_machine, InPort::id_type dstport, OutPort::id_type outport, float volume)
		{
			if(dst_machine._mode == MACHMODE_GENERATOR)
			{
				std::ostringstream s;
				s << "attempted to use a generator as destination for wire" << this->id() << " -> " << dst_machine.id();
				loggers::warning(s.str());
				return false;
			}

			// Get a free output slot on the source machine
			Wire::id_type freebus(-1);
			{
				bool error = false;
				for(int c(MAX_CONNECTIONS - 1); c >= 0; --c)
				{
					if(!_connection[c]) freebus = c;
					// Checking that there's not a slot to the dest. machine already
					else if(_outputMachines[c] == dst_machine.id()) error = true;
				}
				// lamely abandon
				if(freebus == -1 || error) return false;
			}

			// Get a free input slot on the destination machine
			Wire::id_type dfreebus(-1);
			{
				bool error = false;
				for(int c(MAX_CONNECTIONS - 1); c >= 0; --c)
				{
					if(!dst_machine._inputCon[c]) dfreebus = c;
					// Checking if the destination machine is connected with the source machine to avoid a loop.
					if(dst_machine._connection[c] && dst_machine._outputMachines[c] == this->id()) error = true;
				}
				// lamely abandon
				if(dfreebus == -1 || error) return false;
			}

			// Calibrating in/out properties
			this->_outputMachines[freebus] = dst_machine.id();
			this->_connection[freebus] = true;
			this->_connectedOutputs++;
			dst_machine._inputMachines[dfreebus] = this->id();
			dst_machine._inputCon[dfreebus] = true;
			dst_machine._connectedInputs++;
			dst_machine.InitWireVolume(subclass(), dfreebus, volume);
			return true;
		}

		bool Machine::Disconnect(Machine& dst_machine)
		{
			int wireIndex = FindOutputWire( dstMac._macIndex );
			int dstWireIndex = dstMac.FindInputWire( _macIndex );

			_connection[wireIndex] = false;
			_outputMachines[wireIndex] = -1;
			_connectedOutputs--;

			dstMac._inputCon[dstWireIndex] = false;
			dstMac._inputMachines[dstWireIndex]=-1;
			dstMac._connectedInputs--;
			return true; 
		}

		void Machine::InitWireVolume(Machine::class_type msubclass, Wire::id_type wire, float value)
		{
			if (msubclass == MACH_VST || msubclass == MACH_VSTFX )
			{
				if (this->subclass() == MACH_VST || this->subclass() == MACH_VSTFX ) // VST to VST, no need to convert.
				{
					_inputConVol[wire] = value;
					_wireMultiplier[wire] = 1.0f;
				}
				else											// VST to native, multiply
				{
					_inputConVol[wire] = value * 32768.0f;
					_wireMultiplier[wire] = 0.000030517578125f; // what is it?
				}
			}
			else if (this->subclass() == MACH_VST || this->subclass() == MACH_VSTFX ) // native to VST, divide.
			{
				_inputConVol[wire] = value * 0.000030517578125f; // what is it?
				_wireMultiplier[wire] = 32768.0f;
			}
			else												// native to native, no need to convert.
			{
				_inputConVol[wire] = value;
				_wireMultiplier[wire] = 1.0f;
			}	
			// The reason of the conversions in the case of MACH_VST is because VST's output wave data
			// in the range -1.0 to +1.0, while native and internal output at -32768.0 to +32768.0
			// Initially (when the format was made), Psycle did convert this in the "Work" function,
			// but since it already needs to multiply the output by inputConVol, I decided to remove
			// that extra conversion and use directly the volume to do so.
		}

		Wire::id_type Machine::FindInputWire(Machine::id_type id)
		{
			for(Wire::id_type c(0); c < MAX_CONNECTIONS; ++c)
				if(_inputCon[c])
					if(_inputMachines[c] == id)
						return c;
			return Wire::id_type(-1);
		}

		Wire::id_type Machine::FindOutputWire(Machine::id_type id)
		{
			for(Wire::id_type c(0); c < MAX_CONNECTIONS; ++c)
				if(_connection[c])
					if(_outputMachines[c] == id)
						return c;
			return Wire::id_type(-1);
		}

		bool Machine::SetDestWireVolume(Machine::id_type srcIndex, Wire::id_type WireIndex,float value)
		{
			// Get reference to the destination machine
			if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
			Machine *_pDstMachine = Global::song()._pMachine[_outputMachines[WireIndex]];
			if (_pDstMachine)
			{
				Wire::id_type c;
				if((c = _pDstMachine->FindInputWire(srcIndex)) != -1)
				{
					_pDstMachine->SetWireVolume(c,value);
					return true;
				}
			}
			return false;
		}

		bool Machine::GetDestWireVolume(Machine::id_type srcIndex, Wire::id_type WireIndex,float &value)
		{
			// Get reference to the destination machine
			if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
			Machine *_pDstMachine = Global::song()._pMachine[_outputMachines[WireIndex]];
			if (_pDstMachine)
			{
				Wire::id_type c;
				if((c = _pDstMachine->FindInputWire(srcIndex)) != -1)
				{
					//float val;
					_pDstMachine->GetWireVolume(c,value);
					//value = f2i(val*256.0f);
					return true;
				}
			}
			return false;
		}

		InternalMachinePackage& Machine::infopackage()
		{
			static InternalMachinePackage* p = new InternalMachinePackage();
			return *p;
		}

		const InternalMachineInfo* GetInfoFromType(Machine::class_type msubclass)
		{
			return Machine::infopackage().GetInfo(msubclass);
		}
		const std::string Machine::GetBrand() { return infopackage().GetInfo(subclass())->brandname; }
		const std::string Machine::GetVendorName() { return infopackage().GetInfo(subclass())->vendor; }
		const std::uint32_t Machine::GetVersion() { return infopackage().GetInfo(subclass())->version; }
		const std::uint32_t Machine::GetCategory() { return infopackage().GetInfo(subclass())->category; }

		void Machine::PreWork(int numSamples)
		{
			_worked = false;
			_waitingForSound= false;
			cpu::cycles_type cost(cpu::cycles());
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
			cost = cpu::cycles() - cost;
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
					Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
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

						}
						if(!pInMachine->_stopped) _stopped = false;
						if(!_mute && !_stopped)
						{
							cpu::cycles_type wcost(cpu::cycles());
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
							wcost = cpu::cycles() - wcost;
							wire_cpu_cost(wire_cpu_cost() + wcost);
						}
					}
				}
			}
			_waitingForSound = false;
			{
				cpu::cycles_type wcost(cpu::cycles());
					dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
				wcost = cpu::cycles() - wcost;
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

		void Machine::DefineStereoInput(int numinputs)
		{
			//\todo : pending implementation of multi-io.
//			numInPorts=numinputs;
//			inports = new InPort(*this,0,"Stereo In");
		}

		void Machine::DefineStereoOutput(int numoutputs)
		{
			//\todo : pending implementation of multi-io.
//			numOutPorts=numoutputs;
//			outports = new OutPort(*this,0,"Stereo Out");
		}

		bool Machine::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			std::uint32_t count;
			pFile->Read(count);
			for(std::uint32_t i(0); i < count; ++i)
			{
				std::uint32_t temp;
				pFile->Read(temp);
				SetParameter(i,temp);
			}
			pFile->Skip(size - sizeof count - count * sizeof(std::uint32_t));
			return true;
		};

		Machine* Machine::LoadFileChunk(RiffFile* pFile, Machine::id_type index, int version,bool fullopen)
		{
			// assume version 0 for now
			bool bDeleted(false);
			Machine* pMachine;
			Machine::class_type msubclass;//,oldtype;
			char dllName[256];
			pFile->Read(msubclass);
			//oldtype=type;
			pFile->ReadString(dllName,256);
			switch (msubclass)
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
			case MACH_AUTOMATOR:
				if ( !fullopen ) pMachine = new Dummy(index);
				else pMachine = new Automator(index);
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
							pMachine = new Dummy(index);
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
							pMachine = new Dummy(index);
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
							pMachine = new Dummy(index);
							delete p;
							bDeleted = true;
						}
					}
				}
				break;
			case MACH_SCOPE:
				//deprecated. Loading a dummy in its place.
				pMachine = new Dummy(index);
				break;
			default:
				if (msubclass != MACH_DUMMY ) MessageBox(0, "Please inform the devers about this message: unknown kind of machine while loading new file format", "Loading Error", MB_OK | MB_ICONERROR);
				pMachine = new Dummy(index);
				break;
			}
			pMachine->Init();
			std::uint32_t temp;
			pMachine->_subclass = msubclass;
			pFile->Read(pMachine->_bypass);
			pFile->Read(pMachine->_mute);
			pFile->Read(pMachine->_panning);
			pFile->Read(temp);
			pMachine->SetPosX(temp);
			pFile->Read(temp);
			pMachine->SetPosY(temp);
			pFile->Read(pMachine->_connectedInputs);
			pFile->Read(pMachine->_connectedOutputs);
			for(int i = 0; i < MAX_CONNECTIONS; i++)
			{
				pFile->Read(pMachine->_inputMachines[i]);
				pFile->Read(pMachine->_outputMachines[i]);
				pFile->Read(pMachine->_inputConVol[i]);
				pFile->Read(pMachine->_wireMultiplier[i]);
				pFile->Read(pMachine->_connection[i]);
				pFile->Read(pMachine->_inputCon[i]);
			}
			{
				//see? no char arrays! god bless the stl
				//it's still necessary to limit editname length, but i'm inclined to think 128 is plenty..
				std::vector<char> nametemp(128);
				pFile->ReadString(&nametemp[0], nametemp.size());
				//\todo: Use SetEditName. Btw... was this *really* necessary?
				pMachine->_editName.assign( nametemp.begin(), std::find(nametemp.begin(), nametemp.end(), 0));
			}
			//\todo: use SetEditName
			if(bDeleted) pMachine->_editName += " (replaced)";
			if(!fullopen) return pMachine;
			if(!pMachine->LoadSpecificChunk(pFile,version))
			{
				{
					std::ostringstream s;
					s << "Missing or Corrupted Machine Specific Chunk " << dllName << std::endl << "Replacing with Dummy.";
					MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
				}
				Machine* p = new Dummy(index);
				p->Init();
				p->_subclass=MACH_DUMMY;
				p->_mode=pMachine->_mode;
				p->_bypass=pMachine->_bypass;
				p->_mute=pMachine->_mute;
				p->_panning=pMachine->_panning;
				p->SetPosX(pMachine->GetPosX());
				p->SetPosY(pMachine->GetPosY());
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
				//\todo: use SetEditName()
				p->_editName += " (replaced)";
				p->_numPars = 0;
				delete pMachine;
				pMachine = p;
			}
			if(index < MAX_BUSES)
			{
				pMachine->_mode = MACHMODE_GENERATOR;
				if(pMachine->GetPosX() > Global::song().viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width)
					pMachine->SetPosX(Global::song().viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width);
				if(pMachine->GetPosY() > Global::song().viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height)
					pMachine->SetPosY(Global::song().viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height);
			}
			else if (index < MAX_BUSES*2)
			{
				pMachine->_mode = MACHMODE_FX;
				if(pMachine->GetPosX() > Global::song().viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width)
					pMachine->SetPosX(Global::song().viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width);
				if(pMachine->GetPosY() > Global::song().viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height)
					pMachine->SetPosY(Global::song().viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height);
			}
			else
			{
				pMachine->_mode = MACHMODE_MASTER;
				if(pMachine->GetPosX() > Global::song().viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width)
					pMachine->SetPosX(Global::song().viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width);
				if(pMachine->GetPosY() > Global::song().viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height)
					pMachine->SetPosY(Global::song().viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height);
			}
			pMachine->SetPan(pMachine->_panning);
			return pMachine;
		}


		void Machine::SaveFileChunk(RiffFile* pFile)
		{
			pFile->Write(subclass());
			SaveDllName(pFile);
			pFile->Write(_bypass);
			pFile->Write(_mute);
			pFile->Write(_panning);
			pFile->Write(_x);
			pFile->Write(_y);
			pFile->Write(_connectedInputs);
			pFile->Write(_connectedOutputs);
			for(int i = 0; i < MAX_CONNECTIONS; i++)
			{
				pFile->Write(_inputMachines[i]);
				pFile->Write(_outputMachines[i]);
				pFile->Write(_inputConVol[i]);
				pFile->Write(_wireMultiplier[i]);
				pFile->Write(_connection[i]);
				pFile->Write(_inputCon[i]);
			}
			pFile->WriteChunk(GetEditName().c_str(), GetEditName().length()+1);	//a max of 128 chars will be read on song load, but there's no real
																			//reason to limit what gets saved here.. (is there?)
			SaveSpecificChunk(pFile);
		}

		void Machine::SaveSpecificChunk(RiffFile* pFile) 
		{
			std::uint32_t count = GetNumParams();
			std::uint32_t const size(sizeof count  + count * sizeof(std::uint32_t));
			pFile->Write(size);
			pFile->Write(count);
			for(unsigned int i = 0; i < count; i++)
			{
				std::uint32_t temp = GetParamValue(i);
				pFile->Write(temp);
			}
		}

		void Machine::SaveDllName(RiffFile* pFile)
		{
			std::uint8_t const temp(0);
			pFile->Write(temp);
		}
	}
}
