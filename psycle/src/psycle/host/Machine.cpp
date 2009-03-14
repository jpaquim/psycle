///\file
///\brief implementation file for psycle::host::Machine

#include "Machine.hpp"
// Included for "Work()" function and wirevolumes. Maybe this could be worked out
// in a different way
#include "Song.hpp"

#if !defined WINAMP_PLUGIN
	// Included for the machine position on the machine view. This really should be
	// done in a different way.
	#include "MainFrm.hpp"
	// These two are included to update the buffers that wiredlg uses for display. 
	// Find a way to manage these buffers without its inclusion
	#include "Psycle.hpp"
	#include "WireDlg.hpp"
	// Included due to the plugin caching, which should be separated from the dialog.
	#include "NewMachine.hpp"
#else
	#include "player_plugins/winamp/shrunk_newmachine.hpp"
#endif //!defined WINAMP_PLUGIN

// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
#include "internal_machines.hpp"
#include "Sampler.hpp"
#include "XMSampler.hpp"
#include "Plugin.hpp"
#include "VstHost24.hpp"

#include "Loggers.hpp"
#include "configuration_options.hpp"

namespace psycle { namespace host 
{
#if !defined WINAMP_PLUGIN
		extern CPsycleApp theApp;
#endif //!defined WINAMP_PLUGIN

		char* Master::_psName = "Master";

		void Machine::crashed(std::exception const & e) throw() {
			///\todo do we need thread synchronization?
			///\todo gui needs to update
			crashed_ = true;
			_bypass = true;
			_mute = true;

			std::ostringstream s;
			s << "Machine: " << _editName;
			if(GetDllName()) s << ": " << GetDllName();
			s << std::endl << e.what() << std::endl;
			s
				<< "The machine has been set to bypassed/muted to prevent it from making the host crash."
				<< std::endl
				<< "You should save your work to a new file, and restart the host.";
			//loggers::crash(s.str()); // already colorized and reported as crash by the exception constructor
			loggers::exception(s.str());
			MessageBox(0, s.str().c_str(), "Exception", MB_OK | MB_ICONERROR);
		}

		Machine::Machine(MachineType msubclass, MachineMode mode, int id) {
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
			, _waitingForSound(false)
			, _standby(false)
			, _worked(false)
			, _isMixerSend(false)
			, _pSamplesL(0)
			, _pSamplesR(0)
			, _cpuCost(0)
			, _wireCost(0)
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
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			_pSamplesL = static_cast<float*>(_aligned_malloc(STREAM_SIZE*sizeof(float),16));
			_pSamplesR = static_cast<float*>(_aligned_malloc(STREAM_SIZE*sizeof(float),16));
		#elif defined DIVERSALIS__PROCESSOR__X86 &&  defined DIVERSALIS__COMPILER__GNU
			posix_memalign(reinterpret_cast<void**>(_pSamplesL),16,STREAM_SIZE*sizeof(float));
			posix_memalign(reinterpret_cast<void**>(_pSamplesR),16,STREAM_SIZE*sizeof(float));
		#else
			_pSamplesL = new float[STREAM_SIZE];
			_pSamplesR = new float[STREAM_SIZE];
		#endif

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
		}
		Machine::Machine(Machine* mac)
			: crashed_()
			, _macIndex(mac->_macIndex)
			, _type(mac->_type)
			, _mode(mac->_mode)
			, _bypass(mac->_bypass)
			, _mute(mac->_mute)
			, _waitingForSound(false)
			, _standby(false)
			, _worked(false)
			, _isMixerSend(false)
			, _pSamplesL(0)
			, _pSamplesR(0)
			, _cpuCost(0)
			, _wireCost(0)
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
#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			_pSamplesL = static_cast<float*>(_aligned_malloc(STREAM_SIZE*sizeof(float),16));
			_pSamplesR = static_cast<float*>(_aligned_malloc(STREAM_SIZE*sizeof(float),16));
#elif defined DIVERSALIS__PROCESSOR__X86 &&  defined DIVERSALIS__COMPILER__GNU
			posix_memalign(reinterpret_cast<void**>(_pSamplesL),16,STREAM_SIZE*sizeof(float));
			posix_memalign(reinterpret_cast<void**>(_pSamplesR),16,STREAM_SIZE*sizeof(float));
#else
			_pSamplesL = new float[STREAM_SIZE];
			_pSamplesR = new float[STREAM_SIZE];
#endif

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
		}
		Machine::~Machine() throw()
		{
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			_aligned_free(_pSamplesL);
			_aligned_free(_pSamplesR);
		#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__GNU
			free(_pSamplesL);
			free(_pSamplesR);
		#else
			delete [] _pSamplesL;
			delete [] _pSamplesR;
		#endif
			_pSamplesL = _pSamplesR=0;
		}

		void Machine::Init()
		{
			// Standard gear initalization
			_cpuCost = 0;
			_wireCost = 0;
			_mute = false;
			Standby(false);
			Bypass(false);
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
#if PSYCLE__CONFIGURATION__RMS_VUS
			rms.AccumLeft=0.;
			rms.AccumRight=0.;
			rms.count=0;
			rms.previousLeft=0.;
			rms.previousRight=0.;
#endif
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
		void Machine::InsertOutputWireIndex(Song* pSong,int wireIndex, int dstmac)
		{
			if (!_connection[wireIndex]) _numOutputs++;
			_outputMachines[wireIndex] = dstmac;
			_connection[wireIndex] = true;
		}
		void Machine::InsertInputWireIndex(Song* pSong,int wireIndex, int srcmac, float wiremultiplier,float initialvol)
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
				Machine* pMac = pSong->_pMachine[srcmac];
				for(int c=0; c<MAX_CONNECTIONS; c++)
				{
					if(pMac->_inputCon[c])	{
						pMac = pSong->_pMachine[pMac->_inputCon[c]];
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

		bool Machine::SetDestWireVolume(Song* pSong,int srcIndex, int WireIndex,float value)
		{
			// Get reference to the destination machine
			if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
			Machine *_pDstMachine = pSong->_pMachine[_outputMachines[WireIndex]];

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

		bool Machine::GetDestWireVolume(Song* pSong,int srcIndex, int WireIndex,float &value)
		{
			// Get reference to the destination machine
			if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
			Machine *_pDstMachine = pSong->_pMachine[_outputMachines[WireIndex]];
			
			if (_pDstMachine)
			{
				int c;
				if ( (c = _pDstMachine->FindInputWire(srcIndex)) != -1)
				{
					//float val;
					_pDstMachine->GetWireVolume(c,value);
					//value = helpers::math::rounded(val*256.0f);
					return true;
				}
			}
			
			return false;
		}

		void Machine::DeleteOutputWireIndex(Song* pSong,int wireIndex)
		{
			if ( _isMixerSend)
			{
				ClearMixerSendFlag(pSong);
			}
			_connection[wireIndex] = false;
			_outputMachines[wireIndex] = -1;
			_numOutputs--;
		}
		void Machine::DeleteInputWireIndex(Song* pSong,int wireIndex)
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
		void Machine::DeleteWires(Song* pSong)
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
						iMac = pSong->_pMachine[_inputMachines[w]];
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
						iMac = pSong->_pMachine[_outputMachines[w]];
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
		void Machine::NotifyNewSendtoMixer(Song* pSong,int callerMac,int senderMac)
		{
			//Work down the connection wires until finding the mixer.
			for (int i(0);i< MAX_CONNECTIONS; ++i)
				if ( _connection[i]) pSong->_pMachine[_outputMachines[i]]->NotifyNewSendtoMixer(pSong,_macIndex,senderMac);
		}
		void Machine::ClearMixerSendFlag(Song* pSong)
		{
			//Work up the connection wires to clear others' flag.
			for (int i(0);i< MAX_CONNECTIONS; ++i)
				if ( _inputCon[i])
				{
					pSong->_pMachine[_inputMachines[i]]->ClearMixerSendFlag(pSong);
				}
				
			_isMixerSend=false;
		}

		void Machine::PreWork(int numSamples,bool clear)
		{
			_worked = false;
			_waitingForSound= false;
			cpu::cycles_type wcost = cpu::cycles();
#if !defined WINAMP_PLUGIN
			if (_pScopeBufferL && _pScopeBufferR)
			{
				float *pSamplesL = _pSamplesL;   
				float *pSamplesR = _pSamplesR;   
				int i = _scopePrevNumSamples;
				if (i+_scopeBufferIndex >= SCOPE_BUF_SIZE)   
				{   
					//dsp::Mov SSE version needs 16byte (4 samples) aligned data.
					const int cont = (SCOPE_BUF_SIZE-_scopeBufferIndex)&0x0FFFFFFC;
					helpers::dsp::Mov(pSamplesL,&_pScopeBufferL[_scopeBufferIndex], cont);
					helpers::dsp::Mov(pSamplesR,&_pScopeBufferR[_scopeBufferIndex], cont);
					pSamplesL+=cont;
					pSamplesR+=cont;
					i -= cont;
					_scopeBufferIndex +=cont;
/*					if (_scopeBufferIndex < SCOPE_BUF_SIZE) {
						const int cont2 = SCOPE_BUF_SIZE-_scopeBufferIndex;
						memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,cont2);
						memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,cont2);
						pSamplesL+=cont2;
						pSamplesR+=cont2;
						i-=cont2;
					}
*/
					_scopeBufferIndex = 0;  
				}   
				//dsp::Mov SSE version needs 16byte (4 samples) aligned data.
				const int cont = i&0x0FFFFFFC;
				helpers::dsp::Mov(pSamplesL,&_pScopeBufferL[_scopeBufferIndex], cont);
				helpers::dsp::Mov(pSamplesR,&_pScopeBufferR[_scopeBufferIndex], cont);
				pSamplesL+=cont;
				pSamplesR+=cont;
				_scopeBufferIndex += cont;
				i -= cont;
/*				if (i > 0) {
					memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,i*sizeof(float));
					memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,i*sizeof(float));
					_scopeBufferIndex +=i;
				}
*/

			}
			_scopePrevNumSamples=numSamples;
#endif //!defined WINAMP_PLUGIN
			if (clear)
			{
				helpers::dsp::Clear(_pSamplesL, numSamples);
				helpers::dsp::Clear(_pSamplesR, numSamples);
			}
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
						* todo: Change the sound routing to understand what a feedback loop is,
						* creating a special type of wire that will have a buffer which will give as output,
						* and which will be triggered from the master, 
						* to fill again the buffer once all the other machines have done its job.
						*/
						if (!pInMachine->_worked && !pInMachine->_waitingForSound)
						{ 
							pInMachine->Work(numSamples);
							{
								//Disable bad-behaving machines
								///\todo: add a better approach later on, 
								/*
								if (pInMachine->_cpuCost >= Global::_cpuHz && !Global::pPlayer->_recording)
								{
									if ( pInMachine->_mode == MACHMODE_GENERATOR) pInMachine->_mute=true;
									else pInMachine->Bypass(true);
								}
								*/
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
						if(!pInMachine->_mute && !_mute && !pInMachine->Standby())
						{
							cpu::cycles_type wcost = cpu::cycles();
							helpers::dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
							helpers::dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
							_wireCost += cpu::cycles() - wcost;
						}
					}
				}
			}
			_waitingForSound = false;
			cpu::cycles_type wcost = cpu::cycles();
			helpers::dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
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
						if(!pInMachine->_worked && !pInMachine->_waitingForSound) pInMachine->Work(numSamples);
						if(!pInMachine->Standby()) Standby(false);
					}
				}
			}
			_waitingForSound = false;
		}

		void Machine::UpdateVuAndStanbyFlag(int numSamples)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			_volumeCounter = helpers::dsp::GetRMSVol(rms,_pSamplesL,_pSamplesR,numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(helpers::math::rounded((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > 0)
			{
				_volumeDisplay = temp;
			}
			else if (_volumeDisplay>1 ) _volumeDisplay -=2;

			if ( Global::pConfig->autoStopMachines )
			{
				if (rms.AccumLeft < 0.00024*GetAudioRange() && rms.count >= numSamples)	{
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
			_volumeCounter = helpers::dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples)*(1.f/GetAudioRange());
			//Transpose scale from -40dbs...0dbs to 0 to 97pix. (actually 100px)
			int temp(helpers::math::rounded((50.0f * log10f(_volumeCounter)+100.0f)));
			// clip values
			if(temp > 97) temp = 97;
			if(temp > _volumeDisplay) _volumeDisplay = temp;
			if (_volumeDisplay>0 )--_volumeDisplay;
			if ( Global::pConfig->autoStopMachines )
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
							char sError[MAX_PATH + 100];
							sprintf(sError,"Replacing Native plug-in \"%s\" with Dummy.",dllName);
							MessageBox(NULL,sError, "Loading Error", MB_OK);
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

						if(!CNewMachine::lookupDllName(dllName,sPath,MACH_VST,shellIdx)) 
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
					else pMachine = new Dummy(index);
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
				/// for the winamp plugin maybe?
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
#if !defined WINAMP_PLUGIN
/*				if(pMachine->_x > Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width)
					pMachine->_x = Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
				if(pMachine->_y > Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height)
					pMachine->_y = Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;*/
#endif //!defined WINAMP_PLUGIN
			}
			else if (index < MAX_BUSES*2)
			{
				pMachine->_mode = MACHMODE_FX;
#if !defined WINAMP_PLUGIN
/*				if(pMachine->_x > Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width)
					pMachine->_x = Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
				if(pMachine->_y > Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height)
					pMachine->_y = Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;*/
#endif //!defined WINAMP_PLUGIN
			}
			else
			{
				pMachine->_mode = MACHMODE_MASTER;
#if !defined WINAMP_PLUGIN
/*				if(pMachine->_x > Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width)
					pMachine->_x = Global::_pSong->viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
				if(pMachine->_y > Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height)
					pMachine->_y = Global::_pSong->viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;*/
#endif //!defined WINAMP_PLUGIN
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
			Machine::Work(numSamples);
			cpu::cycles_type cost = cpu::cycles();

			float mv = helpers::CValueMapper::Map_255_1(_outDry);
				
			float *pSamples = _pMasterSamples;
			float *pSamplesL = _pSamplesL;
			float *pSamplesR = _pSamplesR;
			
			if(vuupdated)
			{ 
					// Auto decrease effect for the Master vu-meters
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
						_outDry = helpers::math::rounded((float)_outDry * 32767.0f / (*pSamples));
						mv = helpers::CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesL = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = helpers::math::rounded((float)_outDry * -32767.0f / (*pSamples));
						mv = helpers::CValueMapper::Map_255_1(_outDry);
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
						_outDry = helpers::math::rounded((float)_outDry * 32767.0f / (*pSamples));
						mv = helpers::CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesR = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = helpers::math::rounded((float)_outDry * -32767.0f / (*pSamples));
						mv = helpers::CValueMapper::Map_255_1(_outDry);
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
				_lMax = 32767.0f;
			}
			else if (_lMax < 1.0f) { _lMax = 1.0f; }
			if(_rMax > 32767.0f)
			{
				_clip=true;
				_rMax = 32767.0f;
			}
			else if(_rMax < 1.0f) { _rMax = 1.0f; }
			if( _lMax > currentpeak ) currentpeak = _lMax;
			if( _rMax > currentpeak ) currentpeak = _rMax;

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
}}
