///\file
///\brief implementation file for psycle::host::Machine

#include <psycle/host/detail/project.private.hpp>
#include "Machine.hpp"
// Included for "Work()" function and wirevolumes. Maybe this could be worked out
// in a different way
#include "Song.hpp"
#include "machineloader.hpp"

#if !defined WINAMP_PLUGIN
	// This one is included to update the buffers that wiredlg uses for display. 
	// Find a way to manage these buffers without its inclusion
	#include "WireDlg.hpp"
	// Included due to the plugin caching, which should be separated from the dialog.
#endif //!defined WINAMP_PLUGIN

// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include "internal_machines.hpp"
#include "Sampler.hpp"
#include "XMSampler.hpp"
#include "Plugin.hpp"
#include "VstHost24.hpp"

#include <universalis/os/aligned_alloc.hpp>
#include <psycle/helpers/value_mapper.hpp>
#include "cpu_time_clock.hpp"

namespace psycle
{
	namespace host
	{
		char* Master::_psName = "Master";
		bool Machine::autoStopMachine = false;

		void Machine::crashed(std::exception const & e) throw() {
			///\todo do we need thread synchronization?
			///\todo gui needs to update
			crashed_ = true;
			_bypass = true;
			_mute = true;
			std::ostringstream s;
			s
				<< "Machine: " << _editName << std::endl
				<< "DLL: "<< GetDllName() << std::endl
				<< e.what() << std::endl
				<< "The machine has been set to bypassed/muted to prevent it from making the host crash." << std::endl
				<< "You should save your work to a new file, and restart the host.";
			if(loggers::exception()) loggers::exception()(s.str());
			MessageBox(NULL, s.str().c_str(), "Error", MB_OK | MB_ICONERROR);
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
			, _macIndex(0)
			, _type(MACH_UNDEFINED)
			, _mode(MACHMODE_UNDEFINED)
			, _bypass(false)
			, _mute(false)
			, recursive_is_processing_(false)
			, _standby(false)
			, recursive_processed_(false)
			, sched_processed_(false)
			, _isMixerSend(false)
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
			universalis::os::aligned_memory_alloc(16, _pSamplesL, STREAM_SIZE);
			universalis::os::aligned_memory_alloc(16, _pSamplesR, STREAM_SIZE);

			// Clear machine buffer samples
			helpers::dsp::Clear(_pSamplesL,STREAM_SIZE);
			helpers::dsp::Clear(_pSamplesR,STREAM_SIZE);

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
				_inputCon[i]=false;
				_inputConVol[i]=0.0f;
				_wireMultiplier[i]=0.0f;
				_outputMachines[i]=-1;
				_connection[i]=false;
				_connectionPoint[i].x=0;
				_connectionPoint[i].y=0;
			}
#if PSYCLE__CONFIGURATION__RMS_VUS
			rms.count=0;
			rms.AccumLeft=0.;
			rms.AccumRight=0.;
			rms.previousLeft=0.;
			rms.previousRight=0.;
#endif
			reset_time_measurement();
		}
		Machine::Machine(Machine* mac)
			: crashed_()
			, _macIndex(mac->_macIndex)
			, _type(mac->_type)
			, _mode(mac->_mode)
			, _bypass(mac->_bypass)
			, _mute(mac->_mute)
			, recursive_is_processing_(false)
			, _standby(false)
			, recursive_processed_(false)
			, sched_processed_(false)
			, _isMixerSend(false)
			, _pSamplesL(0)
			, _pSamplesR(0)
			, _lVol(mac->_lVol)
			, _rVol(mac->_rVol)
			, _panning(mac->_panning)
			, _x(mac->_x)
			, _y(mac->_y)
			, _numPars(0)
			, _nCols(1)
			, _numInputs(mac->_numInputs)
			, _numOutputs(mac->_numOutputs)
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
			sprintf(_editName,mac->_editName);
			universalis::os::aligned_memory_alloc(16, _pSamplesL, STREAM_SIZE);
			universalis::os::aligned_memory_alloc(16, _pSamplesR, STREAM_SIZE);

			// Clear machine buffer samples
			helpers::dsp::Clear(_pSamplesL,STREAM_SIZE);
			helpers::dsp::Clear(_pSamplesR,STREAM_SIZE);

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

			for (int i(0);i<MAX_CONNECTIONS;i++)
			{
				_inputMachines[i] = mac->_inputMachines[i];
				_inputCon[i] = mac->_inputCon[i];
				_inputConVol[i] = mac->_inputConVol[i];
				//I am unsure that this conversion will always work. Would need some testing.
				_wireMultiplier[i] = (mac->_wireMultiplier[i]*mac->GetAudioRange()/GetAudioRange());
				_outputMachines[i] = mac->_outputMachines[i];
				_connection[i] = mac->_connection[i];
				_connectionPoint[i].x=mac->_connectionPoint[i].x;
				_connectionPoint[i].y=mac->_connectionPoint[i].y;
			}
#if PSYCLE__CONFIGURATION__RMS_VUS
			rms.count=0;
			rms.AccumLeft=0.;
			rms.AccumRight=0.;
			rms.previousLeft=0.;
			rms.previousRight=0.;
#endif
			reset_time_measurement();
		}
		Machine::~Machine() throw()
		{
			universalis::os::aligned_memory_dealloc(_pSamplesL);
			universalis::os::aligned_memory_dealloc(_pSamplesR);
			_pSamplesL = _pSamplesR=0;
		}

		void Machine::Init()
		{
			// Standard gear initalization
			_mute = false;
			Standby(false);
			Bypass(false);
			recursive_is_processing_ = false;
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
#if PSYCLE__CONFIGURATION__RMS_VUS
			rms.AccumLeft=0.;
			rms.AccumRight=0.;
			rms.count=0;
			rms.previousLeft=0.;
			rms.previousRight=0.;
#endif
			reset_time_measurement();
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
		void Machine::InsertOutputWireIndex(Song& pSong,int wireIndex, int dstmac)
		{
			if (!_connection[wireIndex]) _numOutputs++;
			_outputMachines[wireIndex] = dstmac;
			_connection[wireIndex] = true;
		}
		void Machine::InsertInputWireIndex(Song& pSong,int wireIndex, int srcmac, float wiremultiplier,float initialvol)
		{
			if (!_inputCon[wireIndex]) _numInputs++;
			_inputMachines[wireIndex] = srcmac;
			_inputCon[wireIndex] = true;
			_wireMultiplier[wireIndex] = wiremultiplier;
			SetWireVolume(wireIndex,initialvol);
			if ( _isMixerSend )
			{
				//Let's find if the new machine has still other machines connected to it.
				// Right now the UI doesn't allow such configurations, but there isn't a reason
				// not to allow it in the future.
				Machine* pMac = pSong._pMachine[srcmac];
				for(int c=0; c<MAX_CONNECTIONS; c++)
				{
					if(pMac->_inputCon[c])	{
						pMac = pSong._pMachine[pMac->_inputMachines[c]];
						c=0;
						continue;
					}
				}
				NotifyNewSendtoMixer(pSong,_macIndex,pMac->_macIndex);
			}
		}
		int Machine::GetFreeInputWire(int slottype)
		{
			for(int c=0; c<MAX_CONNECTIONS; c++)
			{
				if(!_inputCon[c]) return c;
			}
			return -1;
		}
		int Machine::GetFreeOutputWire(int slottype)
		{
			for(int c=0; c<MAX_CONNECTIONS; c++)
			{
				if(!_connection[c]) return c;
			}
			return -1;
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

		bool Machine::SetDestWireVolume(Song& thesong,int srcIndex, int WireIndex,float value)
		{
			// Get reference to the destination machine
			if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
			Machine *_pDstMachine = thesong._pMachine[_outputMachines[WireIndex]];

			if (_pDstMachine)
			{
				int c;
				if ( (c = _pDstMachine->FindInputWire(srcIndex)) != -1)
				{
					_pDstMachine->SetWireVolume(c,value);
					return true;
				}
			}
			return false;
		}

		bool Machine::GetDestWireVolume(Song& pSong,int srcIndex, int WireIndex,float &value)
		{
			// Get reference to the destination machine
			if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
			Machine *_pDstMachine = pSong._pMachine[_outputMachines[WireIndex]];
			
			if (_pDstMachine)
			{
				int c;
				if ( (c = _pDstMachine->FindInputWire(srcIndex)) != -1)
				{
					//float val;
					_pDstMachine->GetWireVolume(c,value);
					//value = helpers::math::lround<int,float>(val*256.0f);
					return true;
				}
			}
			
			return false;
		}

		void Machine::DeleteOutputWireIndex(Song& pSong,int wireIndex)
		{
			if ( _isMixerSend)
			{
				ClearMixerSendFlag(pSong);
			}
			_connection[wireIndex] = false;
			_outputMachines[wireIndex] = -1;
			_numOutputs--;
		}
		void Machine::DeleteInputWireIndex(Song& pSong,int wireIndex)
		{
			_inputCon[wireIndex] = false;
			_inputMachines[wireIndex] = -1;
			_numInputs--;
			if ( _isMixerSend )
			{
				// Chain is broken, notify the mixer so that it replaces the send machine of the send/return.
				NotifyNewSendtoMixer(pSong,_macIndex,_macIndex);
			}
		}
		void Machine::DeleteWires(Song& pSong)
		{
			Machine *iMac;
			// Deleting the connections to/from other machines
			for(int w=0; w<MAX_CONNECTIONS; w++)
			{
				// Checking In-Wires
				if(_inputCon[w])
				{
					if((_inputMachines[w] >= 0) && (_inputMachines[w] < MAX_MACHINES))
					{
						iMac = pSong._pMachine[_inputMachines[w]];
						if (iMac)
						{
							int wix = iMac->FindOutputWire(_macIndex);
							if (wix >=0)
							{
								iMac->DeleteOutputWireIndex(pSong,wix);
							}
						}
					}
					DeleteInputWireIndex(pSong,w);
				}
				// Checking Out-Wires
				if(_connection[w])
				{
					if((_outputMachines[w] >= 0) && (_outputMachines[w] < MAX_MACHINES))
					{
						iMac = pSong._pMachine[_outputMachines[w]];
						if (iMac)
						{
							int wix = iMac->FindInputWire(_macIndex);
							if(wix >=0 )
							{
								iMac->DeleteInputWireIndex(pSong,wix);
							}
						}
					}
					DeleteOutputWireIndex(pSong,w);
				}
			}
		}

		void Machine::ExchangeInputWires(int first, int second)
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
		void Machine::ExchangeOutputWires(int first, int second)
		{
			int tmp = _outputMachines[first];
			_outputMachines[first]=_outputMachines[second];
			_outputMachines[second]=tmp;

			bool tmp3 = _connection[first];
			_connection[first]=_connection[second];
			_connection[second]=tmp3;
		}
		void Machine::NotifyNewSendtoMixer(Song& pSong,int callerMac,int senderMac)
		{
			//Work down the connection wires until finding the mixer.
			for (int i(0);i< MAX_CONNECTIONS; ++i)
				if ( _connection[i]) pSong._pMachine[_outputMachines[i]]->NotifyNewSendtoMixer(pSong,_macIndex,senderMac);
		}
		void Machine::ClearMixerSendFlag(Song& pSong)
		{
			//Work up the connection wires to clear others' flag.
			for (int i(0);i< MAX_CONNECTIONS; ++i)
				if ( _inputCon[i])
				{
					pSong._pMachine[_inputMachines[i]]->ClearMixerSendFlag(pSong);
				}
				
			_isMixerSend=false;
		}
		void Machine::GetCurrentPreset(CPreset & preset)
		{
			int numParameters = GetNumParams();
			preset.Init(numParameters);
			for(int i=0; i < numParameters; ++i)
			{
				preset.SetParam(i , GetParamValue(i));
			}
		}
		void Machine::Tweak(CPreset const & preset)
		{
			int num=preset.GetNumPars();
			for(int i(0) ; i < num ; ++i)
			{
				try
				{
					SetParameter(i, preset.GetParam(i));
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				catch(...) // reinterpret_cast sucks
				{
					// o_O`
				}
			}
		}
		void Machine::PreWork(int numSamples,bool clear, bool measure_cpu_usage)
		{
			sched_processed_ = recursive_processed_ = recursive_is_processing_ = false;
			cpu_time_clock::time_point t0;
			if(measure_cpu_usage) t0 = cpu_time_clock::now();
#if !defined WINAMP_PLUGIN
			if (_pScopeBufferL && _pScopeBufferR)
			{
				float *pSamplesL = _pSamplesL;   
				float *pSamplesR = _pSamplesR;
				int i = _scopePrevNumSamples & ~0x3; // & ~0x3 to ensure aligned to 16byte
				if (i+_scopeBufferIndex >= SCOPE_BUF_SIZE)   
				{   
					int const amountSamples=SCOPE_BUF_SIZE-_scopeBufferIndex;
					helpers::dsp::Mov(pSamplesL,&_pScopeBufferL[_scopeBufferIndex], amountSamples);
					helpers::dsp::Mov(pSamplesR,&_pScopeBufferR[_scopeBufferIndex], amountSamples);
					pSamplesL+=amountSamples;
					pSamplesR+=amountSamples;
					i -= amountSamples;
					_scopeBufferIndex = 0;
				}
				helpers::dsp::Mov(pSamplesL,&_pScopeBufferL[_scopeBufferIndex], i);
				helpers::dsp::Mov(pSamplesR,&_pScopeBufferR[_scopeBufferIndex], i);
				_scopeBufferIndex += i;
				i = 0;
			}
			_scopePrevNumSamples=numSamples;
#endif //!defined WINAMP_PLUGIN
			if (clear)
			{
				helpers::dsp::Clear(_pSamplesL, numSamples);
				helpers::dsp::Clear(_pSamplesR, numSamples);
			}
			if(measure_cpu_usage) {
				cpu_time_clock::time_point const t1(cpu_time_clock::now());
				Global::song().accumulate_routing_time(t1 - t0);
			}
		}


// Low level process function of machines. Takes care of audio generation and routing.
// Each machine is expected to produce its output in its own _pSamplesX buffers.
void Machine::recursive_process(unsigned int frames, bool measure_cpu_usage) {
	recursive_process_deps(frames, true, measure_cpu_usage);

	cpu_time_clock::time_point t1;
	if(measure_cpu_usage) t1 = cpu_time_clock::now();

	GenerateAudio(frames, measure_cpu_usage);
	if(measure_cpu_usage) {
		cpu_time_clock::time_point const t2(cpu_time_clock::now());
		accumulate_processing_time(t2 - t1);
	}
}

void Machine::recursive_process_deps(unsigned int frames, bool mix, bool measure_cpu_usage) {
	recursive_is_processing_ = true;
	for(int i(0); i < MAX_CONNECTIONS; ++i) {
		if(_inputCon[i]) {
			Machine * pInMachine = Global::song()._pMachine[_inputMachines[i]];
			if(pInMachine) {
				if(!pInMachine->recursive_processed_ && !pInMachine->recursive_is_processing_)
					pInMachine->recursive_process(frames,measure_cpu_usage);
				if(!pInMachine->Standby()) Standby(false);
				if(!_mute && !Standby() && mix) {
					cpu_time_clock::time_point t0;
					if(measure_cpu_usage) t0 = cpu_time_clock::now();
					helpers::dsp::Add(pInMachine->_pSamplesL, _pSamplesL, frames, pInMachine->_lVol * _inputConVol[i]);
					helpers::dsp::Add(pInMachine->_pSamplesR, _pSamplesR, frames, pInMachine->_rVol * _inputConVol[i]);
					if(measure_cpu_usage) { 
						cpu_time_clock::time_point const t1(cpu_time_clock::now());
						Global::song().accumulate_routing_time(t1 - t0);
					}
				}
			}
		}
	}

	cpu_time_clock::time_point t0;
	if(measure_cpu_usage) t0 = cpu_time_clock::now();
	helpers::dsp::Undenormalize(_pSamplesL, _pSamplesR, frames);
	if(measure_cpu_usage) {
		cpu_time_clock::time_point const t1(cpu_time_clock::now());
		Global::song().accumulate_routing_time(t1 - t0);
	}
	recursive_is_processing_ = false;
}

/// tells the scheduler which machines to process before this one
void Machine::sched_inputs(sched_deps & result) const {
	for(int c(0); c < MAX_CONNECTIONS; ++c) if(_inputCon[c]) {
		Machine & input(*Global::song()._pMachine[_inputMachines[c]]);
		result.push_back(&input);
	}
	// Mixer creates virtual connections from itself to the sends/returns.
	// The following process identifies such a situation.
	// Also note that a send can be "mixer -(send)> Fx1 -> Fx2 -(return)> mixer".
	// In this case, Fx2 doesn't depend on mixer, but on Fx1, and result would already have it.
	if (_isMixerSend && result.empty()) {
		//Work down the connection wires until finding the mixer.
		const Machine* nmac = this;
		while(true) {
			for(int i(0); i < MAX_CONNECTIONS; ++i) if(nmac->_connection[i]) {
				nmac = Global::song()._pMachine[nmac->_outputMachines[i]];
				break;
			}
			if (nmac->_type == MACH_MIXER) {
				break;
			}
			if (nmac->_outputMachines == 0) {
				// we are on the wrong machine, better return
				return;
			}
		}
		result.push_back(nmac);
	}
}

/// tells the scheduler which machines may be processed after this one
void Machine::sched_outputs(sched_deps & result) const {
	for(int c(0); c < MAX_CONNECTIONS; ++c) if(_connection[c]) {
		Machine & output(*Global::song()._pMachine[_outputMachines[c]]);
		result.push_back(&output);
	}
}

/// called by the scheduler to ask for the actual processing of the machine
bool Machine::sched_process(unsigned int frames, bool measure_cpu_usage) {
	cpu_time_clock::time_point t0;
	if(measure_cpu_usage) t0 = cpu_time_clock::now();

	if(!_mute) for(int i(0); i < MAX_CONNECTIONS; ++i) if(_inputCon[i]) {
		Machine & input_node(*Global::song()._pMachine[_inputMachines[i]]);
		if(!input_node.Standby()) Standby(false);
		// Mixer already prepares the buffers onto the sends.
		if(!Standby() && (input_node._type != MACH_MIXER || !_isMixerSend)) {
			helpers::dsp::Add(input_node._pSamplesL, _pSamplesL, frames, input_node._lVol * _inputConVol[i]);
			helpers::dsp::Add(input_node._pSamplesR, _pSamplesR, frames, input_node._rVol * _inputConVol[i]);
		}
	}
	helpers::dsp::Undenormalize(_pSamplesL, _pSamplesR, frames);

	cpu_time_clock::time_point t1;
	if(measure_cpu_usage) {
		t1 = cpu_time_clock::now();
		Global::song().accumulate_routing_time(t1 - t0);
	}

	GenerateAudio(frames,measure_cpu_usage);
	if(measure_cpu_usage) {
		cpu_time_clock::time_point const t2(cpu_time_clock::now());
		accumulate_processing_time(t2 - t1);
	}

	++processing_count_;

	return true;
}
int Machine::GenerateAudio(int numsamples, bool measure_cpu_usage) {
	//Current implementation limited to work in ticks. check psycle-core's in trunk for the other implementation.
	return GenerateAudioInTicks(0,numsamples);
}
int Machine::GenerateAudioInTicks(int /*startSample*/, int numsamples) {
	return 0;
}
		bool Machine::playsTrack(const int track) const {
			if (Standby()) {
				return false;
			}
#if PSYCLE__CONFIGURATION__RMS_VUS
			//This is made to prevent calculating it when the count has been reseted.
			if ( rms.count > 512) {
				double bla = std::sqrt(std::max(rms.AccumLeft,rms.AccumRight)*(1.0/GetAudioRange())  / (double)rms.count);
				if( bla < 0.00024 )
				{
					return false;
				}
			}
#else
			if (_volumeCounter < 8.0f)	{
				return false;
			}
#endif
			return true;
		}

		void Machine::UpdateVuAndStanbyFlag(int numSamples)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			_volumeCounter = helpers::dsp::GetRMSVol(rms,_pSamplesL,_pSamplesR,numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(helpers::math::lround<int,float>((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > 0)
			{
				_volumeDisplay = temp;
			}
			else if (_volumeDisplay>1 ) _volumeDisplay -=2;
			else {_volumeDisplay = 0;}

			if ( autoStopMachine && !Standby())
			{
				//This is made to prevent calculating it when the count has been reseted.
				if(rms.count >= 512) {
					double bla = std::sqrt(std::max(rms.AccumLeft,rms.AccumRight)*(1.0/GetAudioRange())  / (double)rms.count);
					if( bla < 0.00024 )
					{
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
			}
#else
			_volumeCounter = helpers::dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(helpers::math::lround<int,float>((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > _volumeDisplay) _volumeDisplay = temp;
			if (_volumeDisplay>0 )--_volumeDisplay;
			if ( Global::configuration().UsesAutoStopMachines() )
			{
				if (_volumeCounter < 8.0f)	{
					_volumeCounter = 0.0f;
					_volumeDisplay = 0;
					Standby(true);
				}
			}
#endif
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
		}

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
				if ( fullopen ) pMachine = new Master(index);
				else pMachine = new Dummy(index);
				break;
			case MACH_SAMPLER:
				if ( fullopen ) pMachine = new Sampler(index);
				else pMachine = new Dummy(index);
				break;
			case MACH_XMSAMPLER:
				if ( fullopen ) pMachine = new XMSampler(index);
				else pMachine = new Dummy(index);
				break;
			case MACH_DUPLICATOR:
				if ( fullopen ) pMachine = new DuplicatorMac(index);
				else pMachine = new Dummy(index);
				break;
			case MACH_MIXER:
				if ( fullopen ) pMachine = new Mixer(index);
				else pMachine = new Dummy(index);
				break;
			case MACH_RECORDER:
				if ( fullopen ) pMachine = new AudioRecorder(index);
				else pMachine = new Dummy(index);
				break;
			case MACH_PLUGIN:
				{
					if(fullopen)
					{
						Plugin * p;
						pMachine = p = new Plugin(index);
						if(!p->LoadDll(dllName))
						{
#if !defined WINAMP_PLUGIN
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing Native plug-in \"%s\" with Dummy.",dllName);
							MessageBox(NULL,sError, "Loading Error", MB_OK);
#endif //!defined WINAMP_PLUGIN
							pMachine = new Dummy(index);
							
							delete p;
							bDeleted = true;
						}
					}
					else pMachine = new Dummy(index);
				}
				break;
			case MACH_VST:
			case MACH_VSTFX:
				{
					if(fullopen)
					{
						std::string sPath;
						vst::plugin *vstPlug=0;
						int shellIdx=0;

						if(!Global::machineload().lookupDllName(dllName,sPath,MACH_VST,shellIdx)) 
						{
							// Check Compatibility Table.
							// Probably could be done with the dllNames lookup.
							//GetCompatible(psFileName,sPath2) // If no one found, it will return a null string.
							sPath = dllName;
						}
						if(Global::machineload().TestFilename(sPath,shellIdx) ) 
						{
							vstPlug = dynamic_cast<vst::plugin*>(Global::vsthost().LoadPlugin(sPath.c_str(),shellIdx));
						}

						if(!vstPlug)
						{
#if !defined WINAMP_PLUGIN
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing VST plug-in \"%s\" with Dummy.",dllName);
							MessageBox(NULL,sError, "Loading Error", MB_OK);
#endif //!defined WINAMP_PLUGIN
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
					else pMachine = new Dummy(index);
				}
				break;
			default:
#if !defined WINAMP_PLUGIN
				if (type != MACH_DUMMY ) MessageBox(0, "Please inform the devers about this message: unknown kind of machine while loading new file format", "Loading Error", MB_OK | MB_ICONERROR);
#endif //!defined WINAMP_PLUGIN
				pMachine = new Dummy(index);
				break;
			}
			pMachine->Init();
			if(!bDeleted)
			{
				//this is when "fullopen=false", since then only dummys are loaded
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
				((Dummy*)pMachine)->dllName=dllName;
				std::stringstream s;
				s << "X!" << pMachine->GetEditName();
				((Dummy*)pMachine)->SetEditName(s.str());
			}
			if(!fullopen) return pMachine;

			if(!pMachine->LoadSpecificChunk(pFile,version))
			{
#if !defined WINAMP_PLUGIN
				char sError[MAX_PATH + 100];
				sprintf(sError,"Missing or Corrupted Machine Specific Chunk \"%s\" - replacing with Dummy.",dllName);
				MessageBox(NULL,sError, "Loading Error", MB_OK);
#endif //!defined WINAMP_PLUGIN
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
			}
			else if (index < MAX_BUSES*2)
			{
				pMachine->_mode = MACHMODE_FX;
			}
			else
			{
				pMachine->_mode = MACHMODE_MASTER;
			}
			pMachine->SetPan(pMachine->_panning);
			if (pMachine->_bypass) pMachine->Bypass(true);
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
		}

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
			pFile->Write(&str2,strlen(str2)+1);
		}




		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Master



		float * Master::_pMasterSamples = 0;

		Master::Master(int index)
		{
			_macIndex = index;
			_outDry = 256;
			decreaseOnClip=false;
			_type = MACH_MASTER;
			_mode = MACHMODE_MASTER;
			sprintf(_editName, _psName);
			volumeDisplayLeft=0;
			volumeDisplayRight=0;
		}

		void Master::Init(void)
		{
			Machine::Init();
			currentpeak=0.0f;
			currentrms=0.0f;
			peaktime=1;
			_lMax = 0.f;
			_rMax = 0.f;
			vuupdated = false;
			_clip = false;
		}

		int Master::GenerateAudio(int numSamples, bool measure_cpu_usage)
		{
			float mv = helpers::value_mapper::map_256_1(_outDry);
				
			float *pSamples = _pMasterSamples;
			float *pSamplesL = _pSamplesL;
			float *pSamplesR = _pSamplesR;
			
			if(vuupdated)
			{ 
				// Auto decrease effect for the Master peak vu-meters (rms one is always showing the current value)
				_lMax *= 0.707f; 
				_rMax *= 0.707f; 
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
						_outDry = helpers::math::lround<int,float>((float)_outDry * 32767.0f / (*pSamples));
						mv = helpers::value_mapper::map_256_1(_outDry);
						*pSamples = *pSamplesL = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = helpers::math::lround<int,float>((float)_outDry * -32767.0f / (*pSamples));
						mv = helpers::value_mapper::map_256_1(_outDry);
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
						_outDry = helpers::math::lround<int,float>((float)_outDry * 32767.0f / (*pSamples));
						mv = helpers::value_mapper::map_256_1(_outDry);
						*pSamples = *pSamplesR = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = helpers::math::lround<int,float>((float)_outDry * -32767.0f / (*pSamples));
						mv = helpers::value_mapper::map_256_1(_outDry);
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
			UpdateVuAndStanbyFlag(numSamples);
			if(_lMax > 32767.0f)
			{
				_clip=true;
			}
			else if (_lMax < 0.f) { _lMax = 0.f; }
			if(_rMax > 32767.0f)
			{
				_clip=true;
			}
			else if(_rMax < 0.f) { _rMax = 0.f; }
			if( _lMax > currentpeak ) currentpeak = _lMax;
			if( _rMax > currentpeak ) currentpeak = _rMax;
			
			float maxrms = std::max(rms.previousLeft, rms.previousRight);
			if(maxrms > currentrms) currentrms = maxrms;

			recursive_processed_ = true;
			return numSamples;
		}

		void Master::UpdateVuAndStanbyFlag(int numSamples)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			helpers::dsp::GetRMSVol(rms,_pSamplesL,_pSamplesR,numSamples);
			float volumeLeft = rms.previousLeft*(1.f/GetAudioRange());
			float volumeRight = rms.previousRight*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(helpers::math::lround<int,float>((50.0f * log10f(volumeLeft)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > 0)
			{
				volumeDisplayLeft = temp;
			}
			else if (volumeDisplayLeft>1 ) volumeDisplayLeft -=2;
			else {volumeDisplayLeft = 0;}
			temp = helpers::math::lround<int,float>((50.0f * log10f(volumeRight)+100.0f));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > 0)
			{
				volumeDisplayRight = temp;
			}
			else if (volumeDisplayRight>1 ) volumeDisplayRight -=2;
			else {volumeDisplayRight = 0;}
			_volumeDisplay = std::max(volumeDisplayLeft,volumeDisplayRight);
#else
			_volumeCounter = helpers::dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(helpers::math::lround<int,float>((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > _volumeDisplay) _volumeDisplay = temp;
			if (_volumeDisplay>0 )--_volumeDisplay;
			//Cannot calculate the volume display with GetMaxVol method.
			volumeDisplayLeft = _volumeDisplay;
			volumeDisplayRight = _volumeDisplay;
#endif
		}
		bool Master::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size ); // size of this part params to load
			pFile->Read(&_outDry,sizeof _outDry);
			pFile->Read(&decreaseOnClip, sizeof decreaseOnClip);
			return true;
		}

		void Master::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof _outDry + sizeof decreaseOnClip;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&_outDry,sizeof _outDry);
			pFile->Write(&decreaseOnClip, sizeof decreaseOnClip); 
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
