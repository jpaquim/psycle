///\file
///\brief implementation file for psy::core::Machine
#include <psycle/core/psycleCorePch.hpp>

#include "machine.h"

//#include "analyzer.h"
#include "song.h"
#include "dsp.h"
#include "helpers/math/round.hpp"

// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include "internal_machines.h"
#include "plugin.h"
#include "sampler.h"

#include <cstddef>
#include <cstdlib> // for posix_memalign

namespace psy { namespace core {

	///\todo general purpose => move this to universalis/operating_system/aligned_malloc.hpp or something
	template<typename X>
	void aligned_malloc(std::size_t alignment, X *& x, std::size_t count) {
		std::size_t const size(count * sizeof(X));
		#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				void * address;
				posix_memalign(&address, alignment, size);
				x = static_cast<X*>(address);
				// note: free with std::free
		#elif 0///\todo defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__GNU
				x = static_cast<X*>(__mingw_aligned_malloc(size, alignment));
				// note: free with _mingw_aligned_free
		#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
				x = static_cast<X*>(_aligned_malloc(size, alignment));
				///\todo note: free with what?
		#else
			// could also try _mm_malloc (#include <xmmintr.h> or <emmintr.h>?)
			// memalign on SunOS but not BSD (#include both <cstdlib> and <cmalloc>)
			// note that memalign is declared obsolete and does not specify how to free the allocated memory.
			
			size; // unused
			x = new X[count];
			// note: free with delete[]
		#endif
	}

	///\todo general purpose => move this to universalis/operating_system/aligned_dealloc.hpp or something
	template<typename X>
	void aligned_dealloc(X *& address)
	{
		#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
			free(address); address=0;
		#elif 0///\todo: defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__GNU
			_aligned_free(address); address=0;
		#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
			_aligned_free(address); address=0;
		#else
			delete[] address; address=0;
		#endif
	}

	///\todo: This is the official panning formula for MIDI. Implement it in psycle?
	// Left Channel Gain [dB] = 20*log (cos (Pi/2* max(0,CC#10 – 1)/126)
	// Right Channel Gain [dB] = 20*log (sin (Pi /2* max(0,CC#10 – 1)/126)

	void Machine::crashed(std::exception const & e) throw()
	{
		bool minor_problem(false);
		bool crash(false);
		{
			#if 0 ///\todo
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
			#endif
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
			//host::loggers::warning(s.str());
			std::cerr << s.str() << std::endl;
		}
		else
		{
			s
				<< "This is a serious error: the machine has been set to bypassed/muted to prevent it from making the host crash." << std::endl
				<< "You should save your work to a new file, and restart the host.";
			if(crash)
			{
				//host::loggers::crash(s.str()); // already colorized and reported as crash by the exception constructor
				//host::loggers::exception(s.str());
				std::cerr << s.str() << std::endl;
			}
			else
			{
				//host::loggers::exception(s.str());
				std::cerr << s.str() << std::endl;
			}
		}
		//MessageBox(0, s.str().c_str(), crash ? "Exception (Crash)" : "Exception (Software)", MB_OK | (minor_problem ? MB_ICONWARNING : MB_ICONERROR));
		//std::cerr << (crash) ? "Exception (Crash)" : "Exception (Software)" << std::endl;
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
		///\todo : need a way get a wire index.
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
		///\todo : apply volume, panning and mapping.
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
		///\todo : need a way to indicate to the main Machine that this wire index is now free.
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
		///\todo : need to clean the buffer first? wire(0)processreplacing() while(wires) wire(1+).processadding() ?
		for(wires_type::const_iterator i(wires_.begin()); i != wires_.end(); ++i)
		{
			(**i).CollectData(numSamples);
		}
	}

	void OutPort::CollectData(int numSamples)
	{
		//parent_.Work(numSamples);
	}

	Machine::Machine(MachineCallbacks* callbacks, Machine::type_type type, Machine::mode_type mode, Machine::id_type id, CoreSong * song)
	:
		crashed_(),
		type_(type),
		mode_(mode),
		id_(id),
		callbacks(callbacks),
		song_(song),
		audio_range_(1.0f),
		editName_(),
		//fpu_exception_mask_(),
		numInPorts(0),
		numOutPorts(0),
		_bypass(false),
		_mute(false),
		_waitingForSound(false),
		_standby(false),
		_worked(false),
		_lVol(0),
		_rVol(0),
		_panning(0),
		_x(0),
		_y(0),
		_numPars(0),
		_nCols(1),
		_connectedInputs(0),
		_connectedOutputs(0),
		_volumeCounter(0.0f),
		_volumeDisplay(0),
		_volumeMaxDisplay(0),
		_volumeMaxCounterLife(0),
		_scopePrevNumSamples(0),
		_scopeBufferIndex(0),
		_pScopeBufferL(0),
		_pScopeBufferR(0),
		TWSActive(false),
		TWSSamples(0)
	{
		aligned_malloc(16, _pSamplesL, MAX_BUFFER_LENGTH);
		aligned_malloc(16, _pSamplesR, MAX_BUFFER_LENGTH);

		// Clear machine buffer samples
		dsp::Clear(_pSamplesL,MAX_BUFFER_LENGTH);
		dsp::Clear(_pSamplesR,MAX_BUFFER_LENGTH);
		
		for (int c = 0; c<MAX_TRACKS; c++)
		{
			TriggerDelay[c].setCommand( 0 );
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
	Machine::Machine(Machine* mac,MachineType type,MachineMode mode)
	:crashed_()
	,type_(type)
	,mode_(mode)
	,id_(mac->id_)
	,callbacks(mac->callbacks)
	,song_(mac->song_)
	,audio_range_(1.0f)
	,editName_(mac->GetEditName())
	,numInPorts(0)
	,numOutPorts(0)
	,_bypass(mac->_bypass)
	,_mute(mac->_mute)
	,_waitingForSound(false)
	,_standby(false)
	,_worked(false)
	,_lVol(mac->_lVol)
	,_rVol(mac->_rVol)
	,_panning(mac->_panning)
	,_x(mac->_x)
	,_y(mac->_y)
	,_numPars(0)
	,_nCols(1)
	,_connectedInputs(mac->_connectedInputs)
	,_connectedOutputs(mac->_connectedOutputs)
	,_volumeCounter(0.0f)
	,_volumeDisplay(0)
	,_volumeMaxDisplay(0)
	,_volumeMaxCounterLife(0)
	,_scopePrevNumSamples(0)
	,_scopeBufferIndex(0)
	,_pScopeBufferL(0)
	,_pScopeBufferR(0)
	,TWSActive(false)
	,TWSSamples(0)
	{
		aligned_malloc(16, _pSamplesL, MAX_BUFFER_LENGTH);
		aligned_malloc(16, _pSamplesR, MAX_BUFFER_LENGTH);
		
		// Clear machine buffer samples
		dsp::Clear(_pSamplesL,MAX_BUFFER_LENGTH);
		dsp::Clear(_pSamplesR,MAX_BUFFER_LENGTH);

		for (int c = 0; c<MAX_TRACKS; c++)
		{
			TriggerDelay[c].setCommand( 0 );
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
			_inputMachines[i]=mac->_inputMachines[i];
			_outputMachines[i]=mac->_outputMachines[i];
			_inputConVol[i]=mac->_inputConVol[i];
			_wireMultiplier[i]=(mac->_wireMultiplier[i]*mac->GetAudioRange()/GetAudioRange());
			_connection[i]=mac->_connection[i];
			_inputCon[i]=mac->_inputCon[i];
		}
	}

	Machine::~Machine()
	{
		aligned_dealloc(_pSamplesL);
		aligned_dealloc(_pSamplesR);
	}

	void Machine::Init()
	{
		// Standard gear initalization
		//work_cpu_cost(0);
		//wire_cpu_cost(0);
		_mute = false;
		Standby(false);
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
		if(dst_machine.mode() == MACHMODE_GENERATOR)
		{
			std::ostringstream s;
			s << "attempted to use a generator as destination for wire" << this->id() << " -> " << dst_machine.id();
			//loggers::warning(s.str());
															std::cerr << s.str() << std::endl;
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
		dst_machine.InitWireVolume(type(), dfreebus, volume);
		return true;
	}

	bool Machine::Disconnect( Machine& dstMac )
	{
		int wireIndex = FindOutputWire(dstMac.id());
		int dstWireIndex = dstMac.FindInputWire(this->id());

		_connection[wireIndex] = false;
		_outputMachines[wireIndex] = -1;
		_connectedOutputs--;

		dstMac._inputCon[dstWireIndex] = false;
		dstMac._inputMachines[dstWireIndex]=-1;
		dstMac._connectedInputs--;
		return true; 
	}

	void Machine::InitWireVolume(Machine::type_type type, Wire::id_type wire, float value)
	{
		if (type == MACH_VST || type == MACH_VSTFX  || type == MACH_LADSPA)
		{
			if (this->type() == MACH_VST || this->type() == MACH_VSTFX || this->type() == MACH_LADSPA) // VST to VST, no need to convert.
			{
				_inputConVol[wire] = value;
				_wireMultiplier[wire] = 1.0f;
			}
			else // VST to native, multiply
			{
				_inputConVol[wire] = value * 32768.0f;
				_wireMultiplier[wire] = 0.000030517578125f; // what is it?
			}
		}
		else if (this->type() == MACH_VST || this->type() == MACH_VSTFX || this->type() == MACH_LADSPA ) // native to VST, divide.
		{
			_inputConVol[wire] = value * 0.000030517578125f; // what is it?
			_wireMultiplier[wire] = 32768.0f;
		}
		else // native to native, no need to convert.
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

							bool Machine::acceptsConnections()
							{
											if (mode() == MACHMODE_FX || mode() == MACHMODE_MASTER) {
															return true;
											} else {
															return false;  
											}
							}

							bool Machine::emitsConnections()
							{
											if (mode() == MACHMODE_GENERATOR || mode() == MACHMODE_FX) {
															return true;
											} else {
															return false;  
											}
							}

	bool Machine::SetDestWireVolume(Machine::id_type srcIndex, Wire::id_type WireIndex,float value)
	{
		// Get reference to the destination machine
		if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
		Machine *_pDstMachine = song()->machine(_outputMachines[WireIndex]);
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
		Machine *_pDstMachine = song()->machine(_outputMachines[WireIndex]);
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

	void Machine::PreWork(int numSamples)
	{
		_worked = false;
		_waitingForSound= false;
		//PSYCLE__CPU_COST__INIT(cost);
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
		//PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
		//wire_cpu_cost(wire_cpu_cost() + cost);
	}

	/// Each machine is expected to produce its output in its own
	/// _pSamplesX buffers.
	void Machine::Work(int numSamples )
	{
		_waitingForSound=true;
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_inputCon[i])
			{
				Machine* pInMachine = song()->machine(_inputMachines[i]);
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
							//std::cout << pInMachine->id() << "before work" << numSamples << std::endl;
							pInMachine->Work( numSamples );
							//std::cout << pInMachine->id() << "after work" << numSamples << std::endl;
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
					if(!pInMachine->Standby()) Standby(false);
					if(!_mute && !Standby())
					{
						//PSYCLE__CPU_COST__INIT(wcost);
						dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
						dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
						//PSYCLE__CPU_COST__CALCULATE(wcost,numSamples);
						//wire_cpu_cost(wire_cpu_cost() + wcost);
					}
				}
			}
		}
		_waitingForSound = false;
		{
			//PSYCLE__CPU_COST__INIT(wcost);
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			//PSYCLE__CPU_COST__CALCULATE(wcost,numSamples);
			//wire_cpu_cost(wire_cpu_cost() + wcost);
		}
		GenerateAudio( numSamples );
	}

	//Modified version of Machine::Work(). The only change is the removal of mixing inputs into one stream.
	void Machine::WorkNoMix( int numSamples )
	{
		_waitingForSound=true;
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_inputCon[i])
			{
				Machine* pInMachine = song()->machine(_inputMachines[i]);
				if (pInMachine)
				{
					if (!pInMachine->_worked && !pInMachine->_waitingForSound)
					{ 
						{
							#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
								universalis::processor::exceptions::fpu::mask fpu_exception_mask(pInMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
							#endif
							pInMachine->Work( numSamples );
						}
					}
					if(!pInMachine->Standby()) Standby(false);
				}
			}
		}
		_waitingForSound = false;
	}

	void Machine::DefineStereoInput(int numinputs)
	{
		numInPorts=numinputs;
		inports = new InPort(*this,0,"Stereo In");
	}

	void Machine::DefineStereoOutput(int numoutputs)
	{
		numOutPorts=numoutputs;
		outports = new OutPort(*this,0,"Stereo Out");
	}
		void Machine::UpdateVuAndStanbyFlag(int numSamples)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			_volumeCounter = dsp::GetRMSVol(rms,_pSamplesL,_pSamplesR,numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(common::math::rounded((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > 0)
			{
				_volumeDisplay = temp;
			}
			else if (_volumeDisplay>1 ) _volumeDisplay -=2;

			if ( callbacks->autoStopMachines() )
			{
				if (rms.AccumLeft < 0.00024*GetAudioRange() && rms.count >= numSamples) {
					rms.count=0;
					rms.AccumLeft=0.;
					rms.AccumRight=0.;
					rms.previousLeft=0.;
					rms.previousRight=0.;
					_volumeCounter = 0.0f;
					_volumeDisplay = 0;
					Standby(true);
				}
			}
#else
			_volumeCounter = core::dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(common::math::rounded((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > _volumeDisplay) _volumeDisplay = temp;
			if (_volumeDisplay>0 )--_volumeDisplay;
			if ( callbacks->autoStopMachines() )
			{
				if (_volumeCounter < 8.0f) {
					_volumeCounter = 0.0f;
					_volumeDisplay = 0;
					Standby(true);
				}
			}
#endif
		};

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

	Machine* Machine::LoadFileChunk(std::string const & plugin_path, CoreSong* pSong, RiffFile* pFile, MachineCallbacks* callbacks, Machine::id_type index, int version,bool fullopen)
	{
		// assume version 0 for now
		bool bDeleted(false);
		Machine* pMachine;
		MachineType type;//,oldtype;
		char dllName[256];
		pFile->Read(type);
		//oldtype=type;
		pFile->ReadString(dllName,256);
		switch (type)
		{
		case MACH_MASTER:
			if (pSong->machine(MASTER_INDEX)) pMachine = pSong->machine(MASTER_INDEX);
			else if ( !fullopen ) pMachine = new Dummy(callbacks, index, pSong);
			else pMachine = new Master(callbacks, index, pSong);
			break;
		case MACH_SAMPLER:
			if ( !fullopen ) pMachine = new Dummy(callbacks, index, pSong);
			else pMachine = new Sampler(callbacks, index, pSong );
			break;
		case MACH_XMSAMPLER:
			//if ( !fullopen ) 
			pMachine = new Dummy(callbacks, index, pSong);
			type = MACH_DUMMY;
			//else pMachine = new XMSampler(index);
			break;
		case MACH_DUPLICATOR:
			if ( !fullopen ) pMachine = new Dummy(callbacks, index, pSong);
			else pMachine = new DuplicatorMac(callbacks, index, pSong);
			break;
		case MACH_MIXER:
			if ( !fullopen ) pMachine = new Dummy(callbacks, index, pSong);
			else pMachine = new Mixer(callbacks, index, pSong);
			break;
		case MACH_LFO:
			if ( !fullopen ) pMachine = new Dummy(callbacks, index, pSong);
			else pMachine = new LFO(callbacks, index, pSong);
			break;
		case MACH_PLUGIN:
			{
				if(!fullopen) pMachine = new Dummy(callbacks, index, pSong);
				else 
				{
					Plugin * p;
					pMachine = p = new Plugin(callbacks, index, pSong);
					if(!p->LoadDll(plugin_path, dllName))
					{
						pMachine = new Dummy(callbacks, index, pSong);
						type = MACH_DUMMY;
						delete p;
						bDeleted = true;
					}
				}
			}
			break;
		case MACH_VST:
			{
				if(!fullopen) pMachine = new Dummy(callbacks, index, pSong);
				else 
				{
					//vst::instrument * p;
					//pMachine = p = new vst::instrument(index);
					//if(!p->LoadDll(dllName))
					//{
					pMachine = new Dummy(callbacks, index, pSong);
					type = MACH_DUMMY;
					//delete p;
					//bDeleted = true;
					//}
				}
			}
			break;
		case MACH_VSTFX:
			{
				//if(!fullopen) pMachine = new Dummy(callbacks, index);
				//else 
				//{
				//vst::fx * p;
				//pMachine = p = new vst::fx(callbacks, index);
				//if(!p->LoadDll(dllName))
				//{
				pMachine = new Dummy(callbacks, index, pSong);
				type = MACH_DUMMY;
				//delete p;
				//bDeleted = true;
				//}
				//}
			}
			break;
		default:
			//if (type != MACH_DUMMY ) MessageBox(0, "Please inform the devers about this message: unknown kind of machine while loading new file format", "Loading Error", MB_OK | MB_ICONERROR);
			std::cerr << "Please inform the devers about this message: unknown kind of machine while loading new file format" << std::endl;
			pMachine = new Dummy(callbacks, index, pSong);
			break;
		}
		pMachine->Init();
		int temp;
		pMachine->type_ = type;
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
			pMachine->editName_.assign( nametemp.begin(), std::find(nametemp.begin(), nametemp.end(), 0));
		}
		if(bDeleted) pMachine->editName_ += " (replaced)";
		if(!fullopen) return pMachine;
		if(!pMachine->LoadSpecificChunk(pFile,version))
		{
			{
				std::ostringstream s;
				s << "Missing or Corrupted Machine Specific Chunk " << dllName << std::endl << "Replacing with Dummy.";
				std::cerr << s.str() << std::endl;
				//MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
			}
			Machine* p = new Dummy(callbacks, index, pSong);
			p->Init();
			p->type(MACH_DUMMY);
			p->mode(pMachine->mode());
			p->_bypass=pMachine->_bypass;
			p->_mute=pMachine->_mute;
			p->_panning=pMachine->_panning;
			p->SetPosX(pMachine->GetPosX());
			p->SetPosY(pMachine->GetPosY());
			p->_connectedInputs=pMachine->_connectedInputs; // number of Incoming connections
			p->_connectedOutputs=pMachine->_connectedOutputs; // number of Outgoing connections
			for(int i = 0; i < MAX_CONNECTIONS; i++)
			{
				p->_inputMachines[i]=pMachine->_inputMachines[i];
				p->_outputMachines[i]=pMachine->_outputMachines[i];
				p->_inputConVol[i]=pMachine->_inputConVol[i];
				p->_wireMultiplier[i]=pMachine->_wireMultiplier[i];
				p->_connection[i]=pMachine->_connection[i];
				p->_inputCon[i]=pMachine->_inputCon[i];
			}
			pMachine->editName_ += " (replaced)";
			p->_numPars = 0;
			delete pMachine;
			pMachine = p;
		}

		if(index < MAX_BUSES) pMachine->mode(MACHMODE_GENERATOR);
		else if (index < MAX_BUSES * 2) pMachine->mode(MACHMODE_FX);
		else pMachine->mode(MACHMODE_MASTER);

		pMachine->SetPan(pMachine->_panning);
		return pMachine;
	}

	void Machine::SaveFileChunk(RiffFile* pFile) const
	{
		pFile->Write(type());
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
		pFile->WriteChunk(GetEditName().c_str(), GetEditName().length()+1); //a max of 128 chars will be read on song load, but there's no real
		// reason to limit what gets saved here.. (is there?)
		SaveSpecificChunk(pFile);
	}

	void Machine::SaveSpecificChunk(RiffFile* pFile) const
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

	void Machine::SaveDllName(RiffFile* pFile) const
	{
		char temp=0;
		pFile->Write(temp);
	}

	void Machine::AddEvent( double offset, int track, const PatternEvent & event )
	{
		workEvents.push_back( WorkEvent(offset,track,event));
	}

	WorkEvent::WorkEvent( )
	{
	}

	WorkEvent::WorkEvent( double beatOffset, int track, const PatternEvent & patternEvent )
	: offset_(beatOffset)
	, track_(track)
	, event_(patternEvent)
	{
	}

	const PatternEvent &  psy::core::WorkEvent::event( ) const
	{
		return event_;
	}

	double WorkEvent::beatOffset( ) const
	{
		return offset_;
	}

	int WorkEvent::track( ) const
	{
		return track_;
	}

	int Machine::GenerateAudioInTicks(int startSample, int numsamples )
	{
		assert(numsamples >= 0);
		//std::cout << "ERROR!!!! Machine::GenerateAudioInTicks() called!"<<std::endl;
		workEvents.clear();
		return 0;
	}

	int Machine::GenerateAudio( int numsamples )
	{
		assert(numsamples >= 0);
		const PlayerTimeInfo & timeInfo = callbacks->timeInfo();
		//position [0.0-1.0] inside the current beat.
		const double positionInBeat = timeInfo.playBeatPos() - static_cast<int>(timeInfo.playBeatPos()); 
		//position [0.0-linesperbeat] converted to "Tick()" lines
		const double positionInLines = positionInBeat*timeInfo.linesPerBeat();
		//position in samples of the next "Tick()" Line
		int nextLineInSamples = static_cast<int>( (1.0-(positionInLines-static_cast<int>(positionInLines)))* timeInfo.samplesPerRow() );
		assert(nextLineInSamples >= 0);
		//Next event, initialized to "out of scope".
		int nextevent = numsamples+1;
		int previousline = nextLineInSamples;
		std::map<int,int>::iterator colsIt;

		// check for next event.
		if (!workEvents.empty())
		{
			WorkEvent & workEvent = workEvents.front();
			nextevent = static_cast<int>( workEvent.beatOffset() * timeInfo.samplesPerBeat() );
		assert(nextevent >= 0);
			// correcting rounding errors.
			if ( nextevent == nextLineInSamples+1 ) nextLineInSamples = nextevent;
		}
		int samplestoprocess = 0;
		int processedsamples = 0;
		for(;processedsamples<numsamples; processedsamples+=samplestoprocess)
		{
			if ( processedsamples == nextLineInSamples )
			{
				Tick( );
				previousline = nextLineInSamples;
				nextLineInSamples += static_cast<int>( timeInfo.samplesPerRow() ); 
			}

			
			while ( processedsamples == nextevent  )
			{
				if ( !workEvents.empty() ) {
					WorkEvent & workEvent = *workEvents.begin();
					///\todo: beware of using more than MAX_TRACKS. "Stop()" resets the list, but until that, playColIndex keeps increasing.
					colsIt = playCol.find(workEvent.track());
					if ( colsIt == playCol.end() ) { playCol[workEvent.track()]=playColIndex++;  colsIt = playCol.find(workEvent.track()); }
					Tick(colsIt->second, workEvent.event() );
					workEvents.pop_front();
					if (!workEvents.empty())
					{
						WorkEvent & workEvent1 = *workEvents.begin();
						//nextevent = (workEvent.beatOffset() - beatOffset) * Gloxxxxxxxxxxxxxxxbal::player().SamplesPerBeat();
						nextevent = static_cast<int>( workEvent1.beatOffset() * timeInfo.samplesPerBeat() );
				assert(nextevent >= processedsamples);
					} else nextevent = numsamples+1;
				} else nextevent = numsamples+1;
			}
			assert(nextLineInSamples >= processedsamples);
			assert(nextevent >= processedsamples);

			//minimum between remaining samples, next "Tick()" and next event
			samplestoprocess= std::min(numsamples,std::min(nextLineInSamples,nextevent))-processedsamples;
			//samplestoprocess= std::min(numsamples,nextevent)-processedsamples;
			assert(samplestoprocess >= 0);
			#if 0
			if ( (processedsamples !=0 && processedsamples+ samplestoprocess != numsamples) || samplestoprocess <= 0)
			{
				std::cout << "GenerateAudio:" << processedsamples << "-" << samplestoprocess << "-" << nextLineInSamples << "(" << previousline << ")" << "-" << nextevent << std::endl;
			}
			#endif
			GenerateAudioInTicks( processedsamples, samplestoprocess );
		}
		// Reallocate events remaining in the buffer, This happens when soundcard buffer is bigger than STREAM_SIZE (machine buffer).
		// Since events are generated once per soundcard work(), events have to be reallocated for the next machine Work() call.
		reallocateRemainingEvents( numsamples/ timeInfo.samplesPerBeat() ); 
		
		return processedsamples;
	}

	void Machine::reallocateRemainingEvents(double beatOffset)
	{
		std::deque<WorkEvent>::iterator it = workEvents.begin();
		while(it != workEvents.end())
		{
			it->changeposition(it->beatOffset()-beatOffset);
			it++;
		}
	}

}}
