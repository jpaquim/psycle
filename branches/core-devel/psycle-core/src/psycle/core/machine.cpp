// -*- mode:c++; indent-tabs-mode:t -*-
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/

///\implementation psy::core::Machine
#include <psycle/core/psycleCorePch.hpp>

#include "machine.h"

#include "song.h"
#include "dsp.h"
#include "helpers/math/round.hpp"
#include "fileio.h"

#include <cstddef>
#include <cstdlib> // for posix_memalign
#include <iostream> // only for debug output
#include <sstream>

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
				// note: free with _aligned_free
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

	AudioBuffer::AudioBuffer(int numChannels,int numSamples)
		:numchannels_(numChannels),numsamples_(numSamples)
	{
		aligned_malloc(16, buffer_, numChannels*numSamples);
	}

	AudioBuffer::~AudioBuffer()
	{
		aligned_dealloc(buffer_);
	}

	void AudioBuffer::Clear()
	{
		dsp::Clear(buffer_,numsamples_*numchannels_);
	}
	
	void Wire::Connect(AudioPort *senderp,AudioPort *receiverp)
	{
		senderport=senderp;
		receiverport=receiverp;
		multiplier=receiverport->GetMachine().GetAudioRange()/senderport->GetMachine().GetAudioRange();
		SetVolume(volume);
		senderport->Connected(this);
		receiverport->Connected(this);
	}

	void Wire::ChangeSource(AudioPort* newsource)
	{
		assert(senderport); Disconnect(senderport);
		senderport=newsource;
		multiplier=receiverport->GetMachine().GetAudioRange()/senderport->GetMachine().GetAudioRange();
		SetVolume(volume);
		senderport->Connected(this);
	}

	void Wire::ChangeDestination(AudioPort* newdest)
	{
		assert(receiverport); Disconnect(receiverport);
		receiverport=newdest;
		multiplier=receiverport->GetMachine().GetAudioRange()/senderport->GetMachine().GetAudioRange();
		SetVolume(volume);
		receiverport->Connected(this);
	}

	void Wire::CollectData(int numSamples)
	{
		senderport->CollectData(numSamples);
		///\todo : apply volume, panning and mapping.
		// Check if the Wire is working "inplace" or not, so that it does not
		// need to copy the contents of the output buffer into its intermediate buffer.
	}

	void Wire::SetVolume(float newvol)
	{
		volume=newvol;
		if ( pan > 0.0f )
		{
			rvol=newvol*multiplier;
			lvol=rvol*(1.0f-pan); // lvol=volume*multiplier*(1.0f-pan);
		} else {
			lvol=newvol*multiplier;
			rvol=lvol*(1.0f+pan); // rvol=volume*multiplier*(1.0f+pan);
		}
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
		///\todo : may want to notify to the parent that this wire is now free.
	}

	void InPort::CollectData(int numSamples)
	{
		///wire(0)processreplacing() while(wires) wire(1+).processadding() ?
		for(wires_type::const_iterator i(wires_.begin()); i != wires_.end(); ++i)
		{
			(**i).CollectData(numSamples);
			// do something with (**i).getBuffer().getBuffer()
			// check if the wire is working in "in-place", since maybe the 
			// buffer is shared with ours.
		}
	}

	void OutPort::CollectData(int /*numSamples*/)
	{
		//An outport, by default, does nothing.
	}

	Machine::Machine(MachineCallbacks* callbacks, Machine::id_type id)
	:
		crashed_(),
		//fpu_exception_mask_(),
		id_(id),
		callbacks(callbacks),
		playColIndex(0),
		_bypass(false),
		_standby(false),
		_mute(false),
		_waitingForSound(false),
		_worked(false),
		audio_range_(1.0f),
		numInPorts(0),
		numOutPorts(0),
		inports(0),
		outports(0),
		_isMixerSend(false),
		_connectedInputs(0),
		_connectedOutputs(0),
		_panning(0),
		_lVol(0),
		_rVol(0),
		_numPars(0),
		_nCols(1),
		_x(0),
		_y(0),
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
			#if 0
			CommandEvent event(0,0);
			TriggerDelay[c].SetCommand(0,event);
			#else
			TriggerDelay[c].setCommand(0);
			#endif
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
		aligned_dealloc(_pSamplesL);
		aligned_dealloc(_pSamplesR);
	}
	void Machine::CloneFrom(Machine& src) 
	{
		// Only allow to copy from a machine of the same type.
		if ( getMachineKey() != src.getMachineKey())
			return;
		SetPan(src.Pan());
		SetPosX(src.GetPosX()+32);
		SetPosY(src.GetPosY()+16);
		#if 1
		{
			std::stringstream s;
			s << src.GetEditName() << " " << std::hex << id() << " (cloned from " << std::hex << src.id() << ")";
			SetEditName(s.str());
		}
		#else ///\todo rewrite this for std::string
			int number = 1;
			char buf[sizeof(machine_[dst]->_editName)+4];
			strcpy (buf,machine_[dst]->_editName);
			char* ps = strrchr(buf,' ');
			if (ps)
			{
				number = atoi(ps);
				if (number < 1)
				{
					number =1;
				}
				else
				{
					ps[0] = 0;
					ps = strchr(machine_[dst]->_editName,' ');
					ps[0] = 0;
				}
			}

			for (int i = 0; i < MAX_MACHINES-1; i++)
			{
				if (i!=dst)
				{
					if (machine_[i])
					{
						if (strcmp(machine_[i]->_editName,buf)==0)
						{
							number++;
							sprintf(buf,"%s %d",machine_[dst]->_editName.c_str(),number);
							i = -1;
						}
					}
				}
			}

			buf[sizeof(machine_[dst]->_editName)-1] = 0;
			SetEditName(buf);
		#endif

		SetEditName(src.GetEditName());

		///\FIXME: Implement MemoryFile.
		MemoryFile file;
		file.OpenMem(1000);
		///FIXME: Version anyone?! 
		src.SaveSpecificChunk(&file);
		LoadSpecificChunk(&file,0);
		file.CloseMem();
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

	Wire::id_type Machine::ConnectTo(Machine & dstMac, InPort::id_type dsttype, OutPort::id_type srctype, float volume)
	{
		// Try to get free indexes from each machine
		Wire::id_type freebus=GetFreeOutputWire(srctype);
		Wire::id_type dfreebus=dstMac.GetFreeInputWire(dsttype);
		if(freebus == -1 || dfreebus == -1 )
			return -1;

		InsertOutputWire(dstMac,freebus,srctype);
		dstMac.InsertInputWire(*this, dfreebus,dsttype,volume);

		return dfreebus;
	}
	bool Machine::MoveWireDestTo(Machine& dstMac, OutPort::id_type srctype, Wire::id_type srcwire, InPort::id_type dsttype) 
	{
		if (srctype >= GetOutPorts() || dsttype >= dstMac.GetInPorts())
			return false;
		if ( !_connection[srcwire])
			return false;

		Machine *oldDst = callbacks->song().machine(_connection[srcwire]);
		if (oldDst)
		{
			Wire::id_type oldwire,dstwire;
			if ((oldwire = oldDst->FindInputWire(id()))== -1)
				return false;

			if ((dstwire = dstMac.GetFreeInputWire(dsttype)) == -1)
				return false;

			float volume = 1.0f;
			oldDst->GetWireVolume(oldwire,volume);
			///\todo: Error dsttype may not be the correct type. FindInputWire should give that info to us.
			oldDst->DeleteInputWire(oldwire,dsttype);
			InsertOutputWire(dstMac,srcwire,srctype);
			dstMac.InsertInputWire(*this,dstwire,dsttype,volume);
			return true;
		}
		return false;
	}
	bool Machine::MoveWireSourceTo(Machine& srcMac, InPort::id_type dsttype, Wire::id_type dstwire, OutPort::id_type srctype)
	{
		if (srctype >= srcMac.GetOutPorts() || dsttype >= GetInPorts())
			return false;
		if ( !_inputCon[dstwire])
			return false;
		if (_inputMachines[dstwire] == -1) 
			return false;

		Machine *oldDst = callbacks->song().machine(_connection[dstwire]);
		if (oldDst)
		{
			Wire::id_type oldwire;
			float volume = 1.0f;
			if ((oldwire =oldDst->FindOutputWire(id())) == -1)
				return false;
			///\todo: Error srctype may not be the correct type. FindOutputWire should give that info to us.
			oldDst->DeleteOutputWire(oldwire,srctype);
			srcMac.InsertOutputWire(*this,dstwire,dsttype);
			GetWireVolume(dstwire,volume);
			InsertInputWire(srcMac,dstwire,dsttype,volume);
			return true;
		}
		return false;
	}

	bool Machine::Disconnect( Machine& dstMac )
	{
		int wireIndex = FindOutputWire(dstMac.id());
		int dstWireIndex = dstMac.FindInputWire(this->id());

		if( wireIndex == -1 || dstWireIndex == -1)
			return false;

		///\todo: Error the type may not be the correct type. FindIn/OutputWire should give that info to us.
		DeleteOutputWire(wireIndex,OutPort::id_type(0));
		dstMac.DeleteInputWire(dstWireIndex,InPort::id_type(0));
		return true; 
	}
	void Machine::DeleteWires()
	{
		Machine *iMac;
		// Deleting the connections to/from other machines
		for(Wire::id_type w=0; w<MAX_CONNECTIONS; w++)
		{
			// Checking In-Wires
			if(_inputCon[w])
			{
				if((_inputMachines[w] >= 0) && (_inputMachines[w] < MAX_MACHINES))
				{
					iMac = callbacks->song().machine(_inputMachines[w]);
					if (iMac)
					{
						Wire::id_type wix = iMac->FindOutputWire(id());
						if (wix >=0)
						{
							iMac->DeleteOutputWire(wix,0);
						}
					}
				}
				DeleteInputWire(w,0);
			}
			// Checking Out-Wires
			if(_connection[w])
			{
				if((_outputMachines[w] >= 0) && (_outputMachines[w] < MAX_MACHINES))
				{
					iMac = callbacks->song().machine(_outputMachines[w]);
					if (iMac)
					{
						Wire::id_type wix = iMac->FindInputWire(id());
						if(wix >=0 )
						{
							iMac->DeleteInputWire(wix,0);
						}
					}
				}
				DeleteOutputWire(w,0);
			}
		}
	}

	void Machine::InsertOutputWire(Machine& mac, Wire::id_type wireIndex, OutPort::id_type /*srctype*/)
	{
		if (!_connection[wireIndex]) _connectedOutputs++;
		_outputMachines[wireIndex] = mac.id();
		_connection[wireIndex] = true;
	}
	void Machine::InsertInputWire(Machine& srcMac, Wire::id_type dstWire,InPort::id_type /*dstType*/, float initialVol)
	{
		if (!_inputCon[dstWire]) _connectedInputs++;
		_inputMachines[dstWire] = srcMac.id();
		_inputCon[dstWire] = true;
		_wireMultiplier[dstWire] = srcMac.GetAudioRange()/GetAudioRange();
		SetWireVolume(dstWire,initialVol);
		if ( _isMixerSend )
		{
			NotifyNewSendtoMixer(*this,srcMac);
		}
	}

	void Machine::DeleteOutputWire(Wire::id_type wireIndex, OutPort::id_type /*srctype*/)
	{
		if ( _isMixerSend)
		{
			ClearMixerSendFlag();
		}
		_connection[wireIndex] = false;
		_outputMachines[wireIndex] = -1;
		_connectedOutputs--;
	}
	void Machine::DeleteInputWire(Wire::id_type wireIndex, InPort::id_type /*dsttype*/)
	{
		_inputCon[wireIndex] = false;
		_inputMachines[wireIndex] = -1;
		_connectedInputs--;
		if ( _isMixerSend )
		{ 
			// Chain is broken, notify the mixer so that it replaces the send machine of the send/return.
			NotifyNewSendtoMixer(*this,*this);
		} 
	}
	void Machine::NotifyNewSendtoMixer(Machine& /*callerMac*/,Machine& senderMac)
	{
		//Work down the connection wires until finding the mixer.
		for (int i(0);i< MAX_CONNECTIONS; ++i)
			if ( _connection[i]) callbacks->song().machine(_outputMachines[i])->NotifyNewSendtoMixer(*this,senderMac);
	}
	void Machine::SetMixerSendFlag()
	{
		for (int i(0);i<MAX_CONNECTIONS;++i)
		{
			if (_inputCon[i]) callbacks->song().machine(_inputMachines[i])->SetMixerSendFlag();
		}
		_isMixerSend=true;
	}
	void Machine::ClearMixerSendFlag()
	{
		//Work up the connection wires to clear others' flag.
		for (int i(0);i< MAX_CONNECTIONS; ++i)
			if ( _inputCon[i])
			{
				callbacks->song().machine(_inputMachines[i])->ClearMixerSendFlag();
			}
			
		_isMixerSend=false;
	}


	void Machine::ExchangeInputWires(Wire::id_type first,Wire::id_type second, InPort::id_type /*firstType*/, InPort::id_type /*secondType*/)
	{
		int tmp = _inputMachines[first];
		_inputMachines[first]=_inputMachines[second];
		_inputMachines[second]=tmp;

		float tmp2 = _inputConVol[first];
		_inputConVol[first]=_inputConVol[second];
		_inputConVol[second]=tmp2;

		tmp2 = _wireMultiplier[first];
		_wireMultiplier[first]=_wireMultiplier[second];
		_wireMultiplier[second]=tmp2;

		bool tmp3 = _inputCon[first];
		_inputCon[first]=_inputCon[second];
		_inputCon[second]=tmp3;
	}
	void Machine::ExchangeOutputWires(Wire::id_type first,Wire::id_type second, OutPort::id_type /*firstType*/, InPort::id_type /*secondType*/)
	{
		int tmp = _outputMachines[first];
		_outputMachines[first]=_outputMachines[second];
		_outputMachines[second]=tmp;

		bool tmp3 = _connection[first];
		_connection[first]=_connection[second];
		_connection[second]=tmp3;
	}

	Wire::id_type Machine::FindInputWire(Machine::id_type id) const
	{
		for(Wire::id_type c(0); c < MAX_CONNECTIONS; ++c)
			if(_inputCon[c])
				if(_inputMachines[c] == id)
					return c;
		return Wire::id_type(-1);
	}

	Wire::id_type Machine::FindOutputWire(Machine::id_type id) const
	{
		for(Wire::id_type c(0); c < MAX_CONNECTIONS; ++c)
			if(_connection[c])
				if(_outputMachines[c] == id)
				return c;
		return Wire::id_type(-1);
	}

	Wire::id_type Machine::GetFreeInputWire(InPort::id_type /*slottype*/) const
	{
		for(int c=0; c<MAX_CONNECTIONS; c++)
		{
			if(!_inputCon[c]) return c;
		}
		return Wire::id_type(-1);
	}
	Wire::id_type Machine::GetFreeOutputWire(OutPort::id_type /*slottype*/) const
	{
		for(int c=0; c<MAX_CONNECTIONS; c++)
		{
			if(!_connection[c]) return c;
		}
		return Wire::id_type(-1);
	}
	bool Machine::SetDestWireVolume(Machine::id_type srcIndex, Wire::id_type WireIndex,float value)
	{
		// Get reference to the destination machine
		if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
		Machine *_pDstMachine = callbacks->song().machine(_outputMachines[WireIndex]);
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

	bool Machine::GetDestWireVolume(Machine::id_type srcIndex, Wire::id_type WireIndex,float &value) const
	{
		// Get reference to the destination machine
		if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
		const Machine *_pDstMachine = callbacks->song().machine(_outputMachines[WireIndex]);
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

	void Machine::PreWork(int numSamples,bool clear)
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
		if (clear)
		{
			dsp::Clear(_pSamplesL, numSamples);
			dsp::Clear(_pSamplesR, numSamples);
		}
		//PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
		//wire_cpu_cost(wire_cpu_cost() + cost);
	}


	// Low level Work function of machines. Takes care of audio generation and routing.
	// Each machine is expected to produce its output in its own _pSamplesX buffers.
	void Machine::Work(int numSamples )
	{
		WorkWires(numSamples);
		GenerateAudio( numSamples );
	}

	void Machine::WorkWires(int numSamples, bool mix )
	{
		// Variable to avoid feedback loops. Probably, it is not worth implement feedbacks.
		_waitingForSound=true;
		for (int i=0; i<MAX_CONNECTIONS; i++)
		{
			if (_inputCon[i])
			{
				Machine* pInMachine = callbacks->song().machine(_inputMachines[i]);
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
					if(!_mute && !Standby() && mix)
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
		
		//PSYCLE__CPU_COST__INIT(wcost);
		dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
		//PSYCLE__CPU_COST__CALCULATE(wcost,numSamples);
		//wire_cpu_cost(wire_cpu_cost() + wcost);
	}


	void Machine::defineInputAsStereo(int numports)
	{
		numInPorts=numports;
		#if 0
		///\todo: ArraY!!!!
		inports = new InPort(*this,0,"Stereo In");
		#endif
	}

	void Machine::defineOutputAsStereo(int numports)
	{
		numOutPorts=numports;
		#if 0
		///\todo: ArraY!!!!
		outports = new OutPort(*this,0,"Stereo Out");
		#endif
	}
	
	void Machine::UpdateVuAndStanbyFlag(int numSamples)
	{
	#if defined PSYCLE__CONFIGURATION__RMS_VUS
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
	}

	bool Machine::LoadFileChunk(RiffFile* pFile,int version)
	{
		std::uint32_t temp;
		pFile->Read(_bypass);
		pFile->Read(_mute);
		pFile->Read(_panning);
		pFile->Read(temp);
		SetPosX(temp);
		pFile->Read(temp);
		SetPosY(temp);
		pFile->Read(_connectedInputs);
		pFile->Read(_connectedOutputs);
		for(int i = 0; i < MAX_CONNECTIONS; i++)
		{
			pFile->Read(_inputMachines[i]);
			pFile->Read(_outputMachines[i]);
			pFile->Read(_inputConVol[i]);
			pFile->Read(_wireMultiplier[i]);
			pFile->Read(_connection[i]);
			pFile->Read(_inputCon[i]);
		}
		{
			//see? no char arrays! god bless the stl
			//it's still necessary to limit editname length, but i'm inclined to think 128 is plenty..
			std::vector<char> nametemp(128);
			pFile->ReadString(&nametemp[0], nametemp.size());
			editName_.assign( nametemp.begin(), std::find(nametemp.begin(), nametemp.end(), 0));
		}
		bool success = LoadSpecificChunk(pFile,version);
		if (success) {
			SetPan(_panning);
			if (_bypass) Bypass(true);
		}

		return success;
	}

	bool Machine::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
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
	}

	void Machine::SaveFileChunk(RiffFile* pFile) const
	{
		/* FIXME: Move this to the psyfilter saver.
		pFile->Write(type());
		SaveDllName(pFile);
		*/
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
		pFile->WriteArray(GetEditName().c_str(), GetEditName().length()+1); //a max of 256 chars will be read on song load, but there's no real
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

	void Machine::AddEvent( double offset, int track, const PatternEvent & event )
	{
		if ( !workEvents.empty() )
		{
			if ( workEvents.back().beatOffset() > offset )
			{
				// New event needs to be sorted in the queue
				std::deque<WorkEvent>::reverse_iterator it = workEvents.rbegin();
				while ( it != workEvents.rend() && it->beatOffset() > offset) ++it;
				if ( it == workEvents.rend() ) workEvents.push_front( WorkEvent(offset,track,event));
				else workEvents.insert(it.base(),WorkEvent(offset,track,event));
			}
			else workEvents.push_back( WorkEvent(offset,track,event));
		}
		else workEvents.push_back( WorkEvent(offset,track,event));
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

	int Machine::GenerateAudioInTicks(int /*startSample*/, int numsamples )
	{
		assert(numsamples >= 0);
		std::cerr << "ERROR!!!! Machine::GenerateAudioInTicks() called!"<<std::endl;
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
		const double positionInLines = positionInBeat*timeInfo.ticksSpeed();
		//position in samples of the next "Tick()" Line
		int nextLineInSamples = static_cast<int>( (1.0-(positionInLines-static_cast<int>(positionInLines)))* timeInfo.samplesPerTick() );
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
				nextLineInSamples += static_cast<int>( timeInfo.samplesPerTick() ); 
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
