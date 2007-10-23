///\file
///\brief implementation file for psycle::host::Song.
#include <psycle/project.private.hpp>
#include "psycle.hpp"

#if !defined WINAMP_PLUGIN
	#include "NewMachine.hpp"
	#include "MainFrm.hpp"
	#include "ChildView.hpp"
	#include "ProgressDialog.hpp"
#else
	#include "player_plugins/winamp/fake_progressdialog.hpp"
	#include "player_plugins/winamp/shrunk_newmachine.hpp"
#endif //!defined WINAMP_PLUGIN

#include "Song.hpp"

#include "Machine.hpp" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.hpp"
#include "XMSampler.hpp"
#include "Plugin.hpp"
#include "VSTHost24.hpp"

#include "DataCompression.hpp"
#include "convert_internal_machines.hpp"

#if !defined WINAMP_PLUGIN
	#include "Riff.hpp" // for Wave file loading.
#endif //!defined WINAMP_PLUGIN

#include "zap.hpp"

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		/// the riff WAVE/fmt chunk.
		class WavHeader
		{
		public:
			char chunkID[4];
			long chunkSize;
			short wFormatTag;
			unsigned short wChannels;
			unsigned long  dwSamplesPerSec;
			unsigned long  dwAvgBytesPerSec;
			unsigned short wBlockAlign;
			unsigned short wBitsPerSample;
		};

		/// Helper class for the PSY2 loader.
		class VSTLoader
		{
		public:
			bool valid;
			char dllName[128];
			int numpars;
			float * pars;
		};

		bool Song::CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int songIdx,int shellIdx)
		{
			Machine* pMachine(0);
			Plugin* pPlugin(0);
			vst::plugin *vstPlug(0);
			if(songIdx < 0)
			{
				songIdx =	GetFreeMachine();
				if(songIdx < 0) return false;
			}
			switch (type)
			{
			case MACH_MASTER:
				if(_pMachine[MASTER_INDEX]) return false;
				pMachine = new Master(songIdx);
				songIdx = MASTER_INDEX;
				break;
			case MACH_SAMPLER:
				pMachine = new Sampler(songIdx);
				break;
			case MACH_XMSAMPLER:
				pMachine = new XMSampler(songIdx);
				break;
			case MACH_DUPLICATOR:
				pMachine = new DuplicatorMac(songIdx);
				break;
			case MACH_MIXER:
				pMachine = new Mixer(songIdx);
				break;
			case MACH_RECORDER:
				pMachine = new AudioRecorder(songIdx);
				break;
			case MACH_PLUGIN:
				{
					if(!CNewMachine::TestFilename(psPluginDll,shellIdx))
					{
						return false;
					}
					try
					{
						pMachine = pPlugin = new Plugin(songIdx);
						pPlugin->Instance(psPluginDll);
					}
					catch(std::exception const & e)
					{
						loggers::exception(e.what());
						zapObject(pMachine); 
						return false;
					}
					catch(...)
					{
						zapObject(pMachine); 
						return false;
					}
					break;
				}
			case MACH_VST:
			case MACH_VSTFX:
				{
					if(!CNewMachine::TestFilename(psPluginDll,shellIdx)) 
					{
						return false;
					}
					try
					{
						pMachine = vstPlug = dynamic_cast<vst::plugin*>(Global::vsthost().LoadPlugin(psPluginDll,shellIdx));
						if(vstPlug)
						{
							vstPlug->_macIndex=songIdx;
						}
					}
					catch(std::exception const & e)
					{
						loggers::exception(e.what());
						zapObject(pMachine); 
						return false;
					}
					catch(...)
					{
						zapObject(pMachine);
						return false;
					}
					break;
				}
			case MACH_DUMMY:
				pMachine = new Dummy(songIdx);
				break;
			default:
				return false; ///< hmm?
			}
			if(_pMachine[songIdx]) DestroyMachine(songIdx);
			pMachine->Init();
			pMachine->_x = x;
			pMachine->_y = y;
			// Finally, activate the machine
			_pMachine[songIdx] = pMachine;
			return true;
		}

		int Song::FindBusFromIndex(int smac)
		{
			if(!_pMachine[smac])  return 255;
			return smac;
		}

		Song::Song()
		{
			_machineLock = false;
			Invalided = false;
			
			for(int i(0) ; i < MAX_PATTERNS; ++i) ppPatternData[i] = NULL;
			for(int i(0) ; i < MAX_MACHINES; ++i) _pMachine[i] = NULL;
			CreateNewPattern(0);
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument;
			Reset();
		}

		Song::~Song()
		{
			DestroyAllMachines();
			DestroyAllInstruments();
			DeleteAllPatterns();
		}

		bool Song::ReplaceMachine(Machine* origmac, MachineType type, int x, int y, char const* psPluginDll, int songIdx,int shellIdx)
		{
			CSingleLock lock(&door,TRUE);
			///\todo: This has been copied from GearRack code. It needs to be converted (with multi-io and the mixer, this doesn't work at all)
			assert(origmac);

			// buffer all the connection info
			int outputMachines[MAX_CONNECTIONS];
			int inputMachines[MAX_CONNECTIONS];
			float inputConVol[MAX_CONNECTIONS];
			float outputConVol[MAX_CONNECTIONS];
			bool connection[MAX_CONNECTIONS];
			bool inputCon[MAX_CONNECTIONS];

			int numOutputs = origmac->_numOutputs;
			int numInputs = origmac->_numInputs;

			for (int i = 0; i < MAX_CONNECTIONS; i++)
			{
				outputMachines[i] = origmac->_outputMachines[i];
				inputMachines[i] = origmac->_inputMachines[i];
				inputConVol[i] = origmac->_inputConVol[i]*origmac->_wireMultiplier[i];
				connection[i] = origmac->_connection[i];
				inputCon[i] = origmac->_inputCon[i];
				// store out volumes aswell
				if (connection[i])
				{
					origmac->GetDestWireVolume(this,songIdx,i,outputConVol[i]);
				}
			}

			// CreateMachine automatically deletes the previous machine if exists.
			if (!CreateMachine(type,x,y,psPluginDll,songIdx,shellIdx))
				return false;

			// replace all the connection info
			Machine* newmac = _pMachine[songIdx];
			if (newmac)
			{
				newmac->_numOutputs = numOutputs;
				newmac->_numInputs = numInputs;

				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					// restore input connections
					if (inputCon[i])
					{
						InsertConnection(_pMachine[inputMachines[i]], _pMachine[songIdx],0,0, inputConVol[i]);
					}
					// restore output connections
					if (connection[i])
					{
						InsertConnection(_pMachine[songIdx], _pMachine[outputMachines[i]], 0,0, outputConVol[i]);
					}
				}
			}
			return true;
		}

		bool Song::ExchangeMachines(int one, int two)
		{
			CSingleLock lock(&door,TRUE);
			///\todo: This has been copied from GearRack code. It needs to be converted (with multi-io and the mixer, this doesn't work at all)
			Machine *mac1 = _pMachine[one];
			Machine *mac2 = _pMachine[two];

			// if they are both valid
			if (mac1 && mac2)
			{
				// exchange positions
				int temp = mac1->_x;
				mac1->_x = mac2->_x;
				mac2->_x = temp;

				temp = mac1->_y;
				mac1->_y = mac2->_y;
				mac2->_y = temp;

				float tmp1ivol[MAX_CONNECTIONS],tmp2ivol[MAX_CONNECTIONS], tmp1ovol[MAX_CONNECTIONS],tmp2ovol[MAX_CONNECTIONS];
				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					// Store the volumes of each wire and exchange.
					if (mac1->_connection[i]) {	mac1->GetDestWireVolume(this,mac1->_macIndex,i,tmp1ovol[i]);	}
					if (mac2->_connection[i]) {	mac2->GetDestWireVolume(this,mac2->_macIndex,i,tmp2ovol[i]); }				
					mac1->GetWireVolume(i,tmp1ivol[i]);
					mac2->GetWireVolume(i,tmp2ivol[i]);

					temp = mac1->_outputMachines[i];
					mac1->_outputMachines[i] = mac2->_outputMachines[i];
					mac2->_outputMachines[i] = temp;

					temp = mac1->_inputMachines[i];
					mac1->_inputMachines[i] = mac2->_inputMachines[i];
					mac2->_inputMachines[i] = temp;


					bool btemp = mac1->_connection[i];
					mac1->_connection[i] = mac2->_connection[i];
					mac2->_connection[i] = btemp;

					btemp = mac1->_inputCon[i];
					mac1->_inputCon[i] = mac2->_inputCon[i];
					mac2->_inputCon[i] = btemp;

				}

				temp = mac1->_numOutputs;
				mac1->_numOutputs = mac2->_numOutputs;
				mac2->_numOutputs = temp;

				temp = mac1->_numInputs;
				mac1->_numInputs = mac2->_numInputs;
				mac2->_numInputs = temp;

				// Exchange the Machine number.
				_pMachine[one] = mac2;
				_pMachine[two] = mac1;

				mac1->_macIndex = two;
				mac2->_macIndex = one;

				// Finally, Reinitialize the volumes of the wires. Remember that we have exchanged the wires, so the volume indexes are the opposite ones.
				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					if (mac1->_inputCon[i])
					{
						Machine* macsrc = _pMachine[mac1->_inputMachines[i]];
						mac1->InsertInputWireIndex(this,i,macsrc->_macIndex,macsrc->GetAudioRange()/mac1->GetAudioRange(),tmp2ivol[i]);
					}
					if (mac2->_inputCon[i])
					{
						Machine* macsrc = _pMachine[mac2->_inputMachines[i]];
						mac2->InsertInputWireIndex(this,i,macsrc->_macIndex,macsrc->GetAudioRange()/mac2->GetAudioRange(),tmp1ivol[i]);
					}

					if (mac1->_connection[i])
					{
						Machine* macdst = _pMachine[mac1->_outputMachines[i]];
						macdst->InsertInputWireIndex(this,macdst->FindInputWire(two),two,mac1->GetAudioRange()/macdst->GetAudioRange(),tmp2ovol[i]);
					}
					if (mac2->_connection[i])
					{
						Machine* macdst = _pMachine[mac2->_outputMachines[i]];
						macdst->InsertInputWireIndex(this,macdst->FindInputWire(one),one,mac2->GetAudioRange()/macdst->GetAudioRange(),tmp1ovol[i]);
					}					
				}

				return true;
			}
			else if (mac1)
			{
				// ok we gotta swap this one for a null one
				_pMachine[one] = NULL;
				_pMachine[two] = mac1;

				mac1->_macIndex = two;

				// and replace the index in any machine that pointed to this one.
				for (int i=0; i < MAX_CONNECTIONS; i++)
				{
					if ( mac1->_inputCon[i])
					{
						Machine* cmp = _pMachine[mac1->_inputMachines[i]];
						cmp->_outputMachines[cmp->FindOutputWire(one)]=two;
					}
					if ( mac1->_connection[i])
					{
						Machine* cmp = _pMachine[mac1->_outputMachines[i]];
						cmp->_inputMachines[cmp->FindInputWire(one)]=two;
					}
				}
				return true;
			}
			else if (mac2)
			{
				// ok we gotta swap this one for a null one
				_pMachine[one] = mac2;
				_pMachine[two] = NULL;

				mac2->_macIndex = one;

				// and replace the index in any machine that pointed to this one.
				for (int i=0; i < MAX_CONNECTIONS; i++)
				{
					if ( mac2->_inputCon[i])
					{
						Machine* cmp = _pMachine[mac2->_inputMachines[i]];
						cmp->_outputMachines[cmp->FindOutputWire(two)]=one;
					}
					if ( mac2->_connection[i])
					{
						Machine* cmp = _pMachine[mac2->_outputMachines[i]];
						cmp->_inputMachines[cmp->FindInputWire(two)]=one;
					}
				}
				return true;
			}
			return false;
		}

		void Song::DestroyAllMachines(bool write_locked)
		{
			_machineLock = true;
			for(int c(0) ;  c < MAX_MACHINES; ++c)
			{
				if(_pMachine[c])
				{
					for(int j(c + 1) ; j < MAX_MACHINES; ++j)
					{
						if(_pMachine[c] == _pMachine[j])
						{
							///\todo wtf? duplicate machine? could happen if loader messes up?
							char buf[128];
							std::sprintf(buf,"%d and %d have duplicate pointers", c, j);
							MessageBox(0, buf, "Duplicate Machine", 0);
							_pMachine[j] = 0;
						}
					}
					DestroyMachine(c, write_locked);
				}
				_pMachine[c] = 0;
			}
			_machineLock = false;
		}

		void Song::ExchangeInstruments(int one, int two)
		{
			CSingleLock lock(&door,TRUE);

			Instrument * tmpins;

			tmpins=_pInstrument[one];
			_pInstrument[one]=_pInstrument[two];
			_pInstrument[two]=tmpins;
			//The above works because we are not creating new objects, just swaping them.
			//this means that no new data is generated/deleted,and the information is just
			//copied. If not, we would have had to define the operator=() function and take
			//care of it.

		}
		void Song::DeleteLayer(int i)
		{
			_pInstrument[i]->DeleteLayer();
		}

		void Song::DeleteInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) DeleteInstrument(i);
		}

		void Song::DestroyAllInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) zapObject(_pInstrument[i]);
		}

		void Song::DeleteInstrument(int i)
		{
			Invalided=true;
			_pInstrument[i]->Delete();
			Invalided=false;
		}

		void Song::Reset()
		{
			cpuIdle=0;
			_sampCount=0;
			// Cleaning pattern allocation info
			for(int i(0) ; i < MAX_INSTRUMENTS; ++i) _pInstrument[i]->waveLength=0;
			for(int i(0) ; i < MAX_MACHINES ; ++i)
			{
					zapObject(_pMachine[i]);
			}
			for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
				XMSampler::rInstrument(i).Init();
			}
			for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
				XMSampler::SampleData(i).Init();
			}

			for(int i(0) ; i < MAX_PATTERNS; ++i)
			{
				// All pattern reset
				if(Global::pConfig) patternLines[i]=Global::pConfig->defaultPatLines;
				else patternLines[i]=64;
				std::sprintf(patternName[i], "Untitled"); 
			}
			_trackArmedCount = 0;
			for(int i(0) ; i < MAX_TRACKS; ++i)
			{
				_trackMuted[i] = false;
				_trackArmed[i] = false;
			}
			machineSoloed = -1;
			_trackSoloed = -1;
			playLength=1;
			for(int i(0) ; i < MAX_SONG_POSITIONS; ++i)
			{
				playOrder[i]=0; // All pattern reset
				playOrderSel[i]=false;
			}
			playOrderSel[0]=true;
		}

		void Song::New()
		{
			CSingleLock lock(&door,TRUE);
			seqBus=0;
			// Song reset
			name = "Untitled";
			author = "Unnamed";
			comments = "";
			currentOctave=4;
			// General properties
			m_BeatsPerMin=125;
			m_LinesPerBeat=4;
//			LineCounter=0;
//			LineChanged=false;
			//MessageBox(0, "Machines", 0, 0);
			// Clean up allocated machines.
			DestroyAllMachines(true);
			//MessageBox(0, "Insts", 0, 0);
			// Cleaning instruments
			DeleteInstruments();
			//MessageBox(0, "Pats", 0, 0);
			// Clear patterns
			DeleteAllPatterns();
			// Clear sequence
			Reset();
			instSelected = 0;
			midiSelected = 0;
			auxcolSelected = 0;
			_saved=false;
			fileName ="Untitled.psy";
#if !defined WINAMP_PLUGIN
			if((CMainFrame *)theApp.m_pMainWnd)
			{
				CreateMachine
					(
						MACH_MASTER, 
						(viewSize.x - static_cast<CMainFrame*>(theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width) / 2, 
						(viewSize.y - static_cast<CMainFrame*>(theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height) / 2, 
						0,
						MASTER_INDEX
					);
			}
			else
			{
				CreateMachine(MACH_MASTER, 320, 200, 0, MASTER_INDEX);
			}
#else
			CreateMachine(MACH_MASTER, 320, 200, 0, MASTER_INDEX);
#endif //!defined WINAMP_PLUGIN
		}

		int Song::GetFreeMachine()
		{
			int tmac = 0;
			for(;;)
			{
				if(!_pMachine[tmac]) return tmac;
				if(tmac++ >= MAX_MACHINES) return -1;
			}
		}
		bool Song::ValidateMixerSendCandidate(Machine* mac,bool rewiring)
		{
			// Basically, we dissallow a send comming from a generator as well as multiple-outs for sends.
			if ( mac->_mode == MACHMODE_GENERATOR) return false;
			if ( mac->_numOutputs > 1 || (mac->_numOutputs > 0 && !rewiring) ) return false;
			for (int i(0); i<MAX_CONNECTIONS; ++i)
			{
				if (mac->_inputCon[i])
				{
					if (!ValidateMixerSendCandidate(_pMachine[mac->_inputMachines[i]],false))
					{
						return false;
					}
				}
			}
			return true;
		}
		void Song::RestoreMixerSendFlags()
		{
			for (int i(0);i < MAX_MACHINES; ++i)
			{
				if (_pMachine[i])
				{
					if (_pMachine[i]->_type == MACH_MIXER)
					{
						Mixer* mac = static_cast<Mixer*>(_pMachine[i]);
						for (int j(0); j<mac->numreturns(); ++j)
						{
							if ( mac->Return(j).IsValid())
								mac->SetMixerSendFlag(this,_pMachine[mac->Return(j).Wire().machine_]);
						}
					}
				}
			}
		}

		int Song::InsertConnection(Machine* srcMac,Machine* dstMac, int srctype, int dsttype,float value)
		{
			CSingleLock lock(&door,TRUE);
			// Assert that we have two machines
			assert(srcMac); assert(dstMac);
			// Verify that the destination is not a generator
			if(dstMac->_mode == MACHMODE_GENERATOR) return -1;
			// Verify that src is not connected to dst already, and that destination is not connected to source.
			if (srcMac->FindOutputWire(dstMac->_macIndex) > -1 || dstMac->FindOutputWire(srcMac->_macIndex) > -1) return -1;
			// disallow mixer as a sender of another mixer
			if ( srcMac->_type == MACH_MIXER && dstMac->_type == MACH_MIXER && dsttype != 0) return -1;
			// If source is in a mixer chain, dissallow the new connection.
			if ( srcMac->_isMixerSend ) return -1;
			// If destination is in a mixer chain (or the mixer itself), validate the sender first
			if ( dstMac->_isMixerSend || (dstMac->_type == MACH_MIXER && dsttype == 1))
			{
				if (!ValidateMixerSendCandidate(srcMac)) return -1;
			}

			// Try to get free indexes from each machine
			int freebus=srcMac->GetFreeOutputWire(srctype);
			int dfreebus=dstMac->GetFreeInputWire(dsttype);
			if(freebus == -1 || dfreebus == -1 ) return -1;

			// If everything went right, connect them.
			srcMac->InsertOutputWireIndex(this,freebus,dstMac->_macIndex);
			dstMac->InsertInputWireIndex(this,dfreebus,srcMac->_macIndex,srcMac->GetAudioRange()/dstMac->GetAudioRange(),value);
			return dfreebus;
		}
		bool Song::ChangeWireDestMac(Machine* srcMac,Machine* dstMac, int wiresrc,int wiredest)
		{
			CSingleLock lock(&door,TRUE);
			// Assert that we have two machines
			assert(srcMac); assert(dstMac);
			// Verify that the destination is not a generator
			if(dstMac->_mode == MACHMODE_GENERATOR) return false;
			// Verify that src is not connected to dst already, and that destination is not connected to source.
			if (srcMac->FindOutputWire(dstMac->_macIndex) > -1 || dstMac->FindOutputWire(srcMac->_macIndex) > -1) return false;
			if ( srcMac->_type == MACH_MIXER && dstMac->_type == MACH_MIXER && wiredest >=MAX_CONNECTIONS) return false;
			// If source is in a mixer chain, dissallow the new connection.
			// If destination is in a mixer chain (or the mixer itself), validate the sender first
			if ( dstMac->_isMixerSend || (dstMac->_type == MACH_MIXER && wiredest >= MAX_CONNECTIONS))
			{
				///\todo: validate for the case whre srcMac->_isMixerSend
				if (!ValidateMixerSendCandidate(srcMac,true)) return false;
			}

			if (wiresrc == -1 || wiredest == -1 || srcMac->_outputMachines[wiresrc] == -1)
				return false;

			int w;
			float volume = 1.0f;
			///\todo: this assignation will need to change with multi-io.
			Machine *oldmac = _pMachine[srcMac->_outputMachines[wiresrc]];
			if (oldmac)
			{
				if ((w = oldmac->FindInputWire(srcMac->_macIndex))== -1)
					return false;

				oldmac->GetWireVolume(w,volume);
				oldmac->DeleteInputWireIndex(this,w);
				srcMac->InsertOutputWireIndex(this,wiresrc,dstMac->_macIndex);
				dstMac->InsertInputWireIndex(this,wiredest,srcMac->_macIndex,srcMac->GetAudioRange()/dstMac->GetAudioRange(),volume);
				return true;
			}
			return false;
		}
		bool Song::ChangeWireSourceMac(Machine* srcMac,Machine* dstMac, int wiresrc, int wiredest)
		{
			CSingleLock lock(&door,TRUE);
			// Assert that we have two machines
			assert(srcMac); assert(dstMac);
			// Verify that the destination is not a generator
			if(dstMac->_mode == MACHMODE_GENERATOR) return false;
			// Verify that src is not connected to dst already, and that destination is not connected to source.
			if (srcMac->FindOutputWire(dstMac->_macIndex) > -1 || dstMac->FindOutputWire(srcMac->_macIndex) > -1) return false;
			// disallow mixer as a sender of another mixer
			if ( srcMac->_type == MACH_MIXER && dstMac->_type == MACH_MIXER && wiredest >= MAX_CONNECTIONS) return false;
			// If source is in a mixer chain, dissallow the new connection.
			if ( srcMac->_isMixerSend ) return false;
			// If destination is in a mixer chain (or the mixer itself), validate the sender first
			if ( dstMac->_isMixerSend || (dstMac->_type == MACH_MIXER && wiredest >= MAX_CONNECTIONS))
			{
				if (!ValidateMixerSendCandidate(srcMac,false)) return false;
			}

			if (wiresrc == -1 || wiredest == -1)
				return false;

			Machine *oldmac(0);
			if ( dstMac->_type == MACH_MIXER)
			{
				if ( wiredest < MAX_CONNECTIONS)
				{
					if (dstMac->_inputMachines[wiredest] == -1)
						return false;
					oldmac = _pMachine[dstMac->_inputMachines[wiredest]];
				}
				else if (!((Mixer*)dstMac)->ReturnValid(wiredest-MAX_CONNECTIONS))
					return false;
				else oldmac = _pMachine[((Mixer*)dstMac)->Return(wiredest-MAX_CONNECTIONS).Wire().machine_];
			}
			else if (dstMac->_inputMachines[wiredest] == -1) 
				return false;
			else oldmac = _pMachine[dstMac->_inputMachines[wiredest]];

			int w;
			float volume = 1.0f;
			///\todo: this assignation will need to change with multi-io.
			 
			if (oldmac)
			{
				if ((w =oldmac->FindOutputWire(dstMac->_macIndex)) == -1)
					return false;

				oldmac->DeleteOutputWireIndex(this,w);
				srcMac->InsertOutputWireIndex(this,wiresrc,dstMac->_macIndex);
				dstMac->GetWireVolume(wiredest,volume);
				dstMac->InsertInputWireIndex(this,wiredest,srcMac->_macIndex,srcMac->GetAudioRange()/dstMac->GetAudioRange(),volume);
				return true;
			}
			return false;
		}

		void Song::DestroyMachine(int mac, bool write_locked)
		{
			CSingleLock lock(&door, TRUE);
			Machine *iMac = _pMachine[mac];
			if(iMac)
			{
				iMac->DeleteWires(this);
				if(mac == machineSoloed) machineSoloed = -1;
				// If it's a (Vst)Plugin, the destructor calls to release the underlying library
				try
				{
					zapObject(_pMachine[mac]);
				}catch(...){};
			}
		}

		void Song::DeleteAllPatterns()
		{
			SONGTRACKS = 16;
			for(int i=0; i<MAX_PATTERNS; i++) RemovePattern(i);
		}

		void Song::RemovePattern(int ps)
		{
			zapArray(ppPatternData[ps]);
		}

		unsigned char * Song::CreateNewPattern(int ps)
		{
			RemovePattern(ps);
			ppPatternData[ps] = new unsigned char[MULTIPLY2];
			PatternEntry blank;
			unsigned char * pData = ppPatternData[ps];
			for(int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
			{
				memcpy(pData,&blank,EVENT_SIZE);
				pData+= EVENT_SIZE;
			}
			return ppPatternData[ps];
		}

		bool Song::AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
		{
			PatternEntry blank;
			unsigned char *toffset;
			if(adaptsize)
			{
				float step;
				if( patternLines[pattern] > lines ) 
				{
					step= (float)patternLines[pattern]/lines;
					for(int t=0;t<SONGTRACKS;t++)
					{
						toffset=_ptrack(pattern,t);
						int l;
						for(l = 1 ; l < lines; ++l)
						{
							std::memcpy(toffset + l * MULTIPLY, toffset + helpers::math::rounded(l * step) * MULTIPLY,EVENT_SIZE);
						}
						while(l < patternLines[pattern])
						{
							// This wouldn't be necessary if we really allocate a new pattern.
							std::memcpy(toffset + (l * MULTIPLY), &blank, EVENT_SIZE);
							++l;
						}
					}
					patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
				}
				else if(patternLines[pattern] < lines)
				{
					step= (float)lines/patternLines[pattern];
					int nl= patternLines[pattern];
					for(int t=0;t<SONGTRACKS;t++)
					{
						toffset=_ptrack(pattern,t);
						for(int l=nl-1;l>0;l--)
						{
							std::memcpy(toffset + helpers::math::rounded(l * step) * MULTIPLY, toffset + l * MULTIPLY,EVENT_SIZE);
							int tz(helpers::math::rounded(l * step) - 1);
							while (tz > (l - 1) * step)
							{
								std::memcpy(toffset + tz * MULTIPLY, &blank, EVENT_SIZE);
								--tz;
							}
						}
					}
					patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
				}
			}
			else
			{
				int l(patternLines[pattern]);
				while(l < lines)
				{
					// This wouldn't be necessary if we really allocate a new pattern.
					for(int t(0) ; t < SONGTRACKS ; ++t)
					{
						toffset=_ptrackline(pattern,t,l);
						memcpy(toffset,&blank,EVENT_SIZE);
					}
					++l;
				}
				patternLines[pattern] = lines;
			}
			std::sprintf(patternName[pattern], name);
			return true;
		}

		int Song::GetLastPatternUsed()
		{
			int patternsUsedCount = GetNumPatternsUsed();
			int patternsFound = 0;
			int patternIndex = -1;

			while (patternsFound < patternsUsedCount)
			{
				patternIndex++;
				if (IsPatternUsed(patternIndex))
					patternsFound++;
			}

			return patternIndex;
		}

		int Song::GetNumPatternsUsed()
		{
			int rval(0);
			for(int c(0) ; c < playLength ; ++c) if(rval < playOrder[c]) rval = playOrder[c];
			++rval;
			if(rval > MAX_PATTERNS - 1) rval = MAX_PATTERNS - 1;
			return rval;
		}


		int Song::GetBlankPatternUnused(int rval)
		{
			//Check for one unexistant pattern.
			for(int i(0) ; i < MAX_PATTERNS; ++i) if(!IsPatternUsed(i)) return i;
			//if none found, try to find an empty used pattern.
			PatternEntry blank;
			bool bTryAgain(true);
			while(bTryAgain && rval < MAX_PATTERNS - 1)
			{
				for(int c(0) ; c < playLength ; ++c)
				{
					if(rval == playOrder[c]) 
					{
						++rval;
						c = -1;
					}
				}
				// now test to see if data is really blank
				bTryAgain = false;
				if(rval < MAX_PATTERNS - 1)
				{
					unsigned char *offset_source(_ppattern(rval));
					for(int t(0) ; t < MULTIPLY2 ; t += EVENT_SIZE)
					{
						if(memcmp(offset_source+t,&blank,EVENT_SIZE) != 0 )
						{
							++rval;
							bTryAgain = true;
							t = MULTIPLY2;
						}
					}
				}
			}
			if(rval > MAX_PATTERNS - 1)
			{
				rval = 0;
				for(int c(0) ; c < playLength ; ++c)
				{
					if(rval == playOrder[c]) 
					{
						++rval;
						c = -1;
					}
				}
				if(rval > MAX_PATTERNS - 1) rval = MAX_PATTERNS - 1;
			}
			return rval;
		}

		int Song::GetFreeBus()
		{
			for(int c(0) ; c < MAX_BUSES ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

		int Song::GetFreeFxBus()
		{
			for(int c(MAX_BUSES) ; c < MAX_BUSES * 2 ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

		// IFF structure ripped by krokpitr
		// Current Code Extremely modified by [JAZ] ( RIFF based )
		// Advise: IFF files use Big Endian byte ordering. That's why I use
		// the following structure.
		//
		// typedef struct {
		//   unsigned char hihi;
		//   unsigned char hilo;
		//   unsigned char lohi;
		//   unsigned char lolo;
		// } ULONGINV;
		// 
		//
		/*
		** IFF Riff Header
		** ----------------

		char Id[4]			// "FORM"
		ULONGINV hlength	// of the data contained in the file (except Id and length)
		char type[4]		// "16SV" == 16bit . 8SVX == 8bit

		char name_Id[4]		// "NAME"
		ULONGINV hlength	// of the data contained in the header "NAME". It is 22 bytes
		char name[22]		// name of the sample.

		char vhdr_Id[4]		// "VHDR"
		ULONGINV hlength	// of the data contained in the header "VHDR". it is 20 bytes
		ULONGINV Samplength	// length of the sample. It is in bytes, not in Samples.
		ULONGINV loopstart	// Start point for the loop. It is in bytes, not in Samples.
		ULONGINV loopLength	// Length of the loop (so loopEnd = loopstart+looplength) In bytes.
		unsigned char unknown2[5]; //Always $20 $AB $01 $00 //
		unsigned char volumeHiByte;
		unsigned char volumeLoByte;
		unsigned char unknown3;

		char body_Id[4]		// "BODY"
		ULONGINV hlength	// of the data contained in the file. It is the sample length as well (in bytes)
		char *data			// the sample.

		*/

		int Song::IffAlloc(int instrument,const char * str)
		{
			if(instrument != PREV_WAV_INS)
			{
				Invalided = true;
				///\todo lock/unlock
				::Sleep(256);
			}
			RiffFile file;
			RiffChunkHeader hd;
			ULONG data;
			ULONGINV tmp;
			int bits = 0;
			// opens the file and reads the "FORM" header.
			if(!file.Open(const_cast<char*>(str)))
			{
				Invalided = false;
				return 0;
			}
			DeleteLayer(instrument);
			file.Read(&data,4);
			if( data == file.FourCC("16SV")) bits = 16;
			else if(data == file.FourCC("8SVX")) bits = 8;
			file.Read(&hd,sizeof hd);
			if(hd._id == file.FourCC("NAME"))
			{
				file.Read(_pInstrument[instrument]->waveName, 22); ///\todo should be hd._size instead of "22", but it is incorrectly read.
				strncpy(_pInstrument[instrument]->_sName,str,31);
				_pInstrument[instrument]->_sName[31]='\0';
				file.Read(&hd,sizeof hd);
			}
			if ( hd._id == file.FourCC("VHDR"))
			{
				unsigned int Datalen, ls, le;
				file.Read(&tmp,sizeof tmp);
				Datalen = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				file.Read(&tmp,sizeof tmp);
				ls = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				file.Read(&tmp,sizeof tmp);
				le = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				if(bits == 16)
				{
					Datalen >>= 1;
					ls >>= 1;
					le >>= 1;
				}
				_pInstrument[instrument]->waveLength=Datalen;
				if(ls != le)
				{
					_pInstrument[instrument]->waveLoopStart = ls;
					_pInstrument[instrument]->waveLoopEnd = ls + le;
					_pInstrument[instrument]->waveLoopType = true;
				}
				file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
				file.Read(&hd,sizeof hd);
			}
			if(hd._id == file.FourCC("BODY"))
			{
				short * csamples;
				const unsigned int Datalen(_pInstrument[instrument]->waveLength);
				_pInstrument[instrument]->waveStereo = false;
				_pInstrument[instrument]->waveDataL = new signed short[Datalen];
				csamples = _pInstrument[instrument]->waveDataL;
				if(bits == 16)
				{
					for(unsigned int smp(0) ; smp < Datalen; ++smp)
					{
						file.Read(&tmp, 2);
						*csamples = tmp.hilo * 256 + tmp.hihi;
						++csamples;
					}
				}
				else
				{
					for(unsigned int smp(0) ; smp < Datalen; ++smp)
					{
						file.Read(&tmp, 1);
						*csamples = tmp.hihi * 256 + tmp.hihi;
						++csamples;
					}
				}
			}
			file.Close();
			Invalided = false;
			return 1;
		}

		int Song::WavAlloc(int iInstr, bool bStereo, long iSamplesPerChan, const char * sName)
		{
			assert(iSamplesPerChan<(1<<30)); ///< Since in some places, signed values are used, we cannot use the whole range.
			DeleteLayer(iInstr);
			_pInstrument[iInstr]->waveDataL = new signed short[iSamplesPerChan];
			if(bStereo)
			{	_pInstrument[iInstr]->waveDataR = new signed short[iSamplesPerChan];
				_pInstrument[iInstr]->waveStereo = true;
			} else {
				_pInstrument[iInstr]->waveStereo = false;
			}
			_pInstrument[iInstr]->waveLength = iSamplesPerChan;
			std::strncpy(_pInstrument[iInstr]->waveName, sName, 31);
			_pInstrument[iInstr]->waveName[31] = '\0';
			std::strncpy(_pInstrument[iInstr]->_sName,sName,31);
			_pInstrument[iInstr]->_sName[31]='\0';
			return true;
		}

		int Song::WavAlloc(int instrument,const char * Wavfile)
		{ 
#if !defined WINAMP_PLUGIN
			assert(Wavfile != 0);
			WaveFile file;
			ExtRiffChunkHeader hd;
			// opens the file and read the format Header.
			DDCRET retcode(file.OpenForRead((char*)Wavfile));
			if(retcode != DDC_SUCCESS) 
			{
				Invalided = false;
				return 0; 
			}
			Invalided = true;
			///\todo lock/unlock
			::Sleep(256);
			// sample type	
			int st_type(file.NumChannels());
			int bits(file.BitsPerSample());
			long Datalen(file.NumSamples());
			// Initializes the layer.
			WavAlloc(instrument, st_type == 2, Datalen, Wavfile);
			// Reading of Wave data.
			// We don't use the WaveFile "ReadSamples" functions, because there are two main differences:
			// We need to convert 8bits to 16bits, and stereo channels are in different arrays.
			short * sampL(_pInstrument[instrument]->waveDataL);

			///\todo use template code for all this semi-repetitive code.

			long io; ///< \todo why is this declared here?
			// mono
			if(st_type == 1)
			{
				UINT8 smp8;
				switch(bits)
				{
					case 8:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.ReadData(&smp8, 1);
							*sampL = (smp8 << 8) - 32768;
							++sampL;
						}
						break;
					case 16:
							file.ReadData(sampL, Datalen);
						break;
					case 24:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.ReadData(&smp8, 1);
							file.ReadData(sampL, 1);
							++sampL;
						}
						break;
					default:
						break;
				}
			}
			// stereo
			else
			{
				short *sampR(_pInstrument[instrument]->waveDataR);
				UINT8 smp8;
				switch(bits)
				{
					case 8:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.ReadData(&smp8, 1);
							*sampL = (smp8 << 8) - 32768;
							++sampL;
							file.ReadData(&smp8, 1);
							*sampR = (smp8 << 8) - 32768;
							++sampR;
						}
						break;
					case 16:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.ReadData(sampL, 1);
							file.ReadData(sampR, 1);
							++sampL;
							++sampR;
						}
						break;
					case 24:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.ReadData(&smp8, 1);
							file.ReadData(sampL, 1);
							++sampL;
							file.ReadData(&smp8, 1);
							file.ReadData(sampR, 1);
							++sampR;
						}
						break;
					default:
						break; ///< \todo should throw an exception
				}
			}
			retcode = file.Read(static_cast<void*>(&hd), 8);
			while(retcode == DDC_SUCCESS)
			{
				if(hd.ckID == FourCC("smpl"))
				{
					char pl(0);
					file.Skip(28);
					file.Read(static_cast<void*>(&pl), 1);
					if(pl == 1)
					{
						file.Skip(15);
						unsigned int ls(0);
						unsigned int le(0);
						file.Read(static_cast<void*>(&ls), 4);
						file.Read(static_cast<void*>(&le), 4);
						_pInstrument[instrument]->waveLoopStart = ls;
						_pInstrument[instrument]->waveLoopEnd = le;
						// only for my bad sample collection
						//if(!((ls <= 0) && (le >= Datalen - 1)))
						{
							_pInstrument[instrument]->waveLoopType = true;
						}
					}
					file.Skip(9);
				}
				else if(hd.ckSize > 0)
					file.Skip(hd.ckSize);
				else
					file.Skip(1);
				retcode = file.Read(static_cast<void*>(&hd), 8);
			}
			file.Close();
			Invalided = false;
#endif //!defined WINAMP_PLUGIN
			return 1;
		}

		bool Song::Load(RiffFile* pFile, bool fullopen)
		{
			char Header[9];
			pFile->Read(&Header, 8);
			Header[8]=0;

			if (strcmp(Header,"PSY3SONG")==0)
			{
				CProgressDialog Progress;
				Progress.Create();
				Progress.SetWindowText("Loading...");
				Progress.ShowWindow(SW_SHOW);
				UINT version = 0;
				UINT size = 0;
				UINT index = 0;
				int temp;
				int solo(0);
				int chunkcount=0;
				Header[4]=0;
				long filesize = pFile->FileSize();
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if(version > CURRENT_FILE_VERSION)
				{
					MessageBox(0,"This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning", MB_OK | MB_ICONERROR);
				}
				if (size == 4) // Since "version" is used for File version, we use size as version identifier
				{
					pFile->Read(&chunkcount,sizeof(chunkcount));
				}
				/*
				if (size == )
				{
				// This is left here if someday, extra data is added to the file version chunk.
				// Modify "pFile->Skip(size - 4);" as necessary. Ex:  pFile->Skip(size - bytesread);
				}
				*/
				if ( size-4 > 0) pFile->Skip(size - 4);// Size of the current Header DATA // This ensures that any extra data is skipped.

				DestroyAllMachines();
				_machineLock = true;
				DeleteInstruments();
				DeleteAllPatterns();
				Reset(); //added by sampler mainly to reset current pattern showed.
				while(pFile->Read(&Header, 4) && chunkcount)
				{
					Progress.m_Progress.SetPos(helpers::math::rounded((pFile->GetPos()*16384.0f)/filesize));
					::Sleep(1); ///< Allow screen refresh.
					// we should use the size to update the index, but for now we will skip it
					if(std::strcmp(Header,"INFO") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						if(version > CURRENT_FILE_VERSION_INFO)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Info Seqment of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							char name_[129]; char author_[65]; char comments_[65536];
							pFile->ReadString(name_, sizeof name_);
							pFile->ReadString(author_, sizeof author_);
							pFile->ReadString(comments_,sizeof comments_);
							name = name_;
							author = author_;
							comments = comments_;

						}
					}
					else if(std::strcmp(Header,"SNGI")==0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						if(version > CURRENT_FILE_VERSION_SNGI)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Song Segment of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// why all these temps?  to make sure if someone changes the defs of
							// any of these members, the rest of the file reads ok.  assume 
							// everything is an int, when we write we do the same thing.

							// # of tracks for whole song
							pFile->Read(&temp, sizeof temp);
							SONGTRACKS = temp;
							// bpm
							pFile->Read(&temp, sizeof temp);
							m_BeatsPerMin = temp;
							// tpb
							pFile->Read(&temp, sizeof temp);
							m_LinesPerBeat = temp;
							// current octave
							pFile->Read(&temp, sizeof temp);
							currentOctave = temp;
							// machineSoloed
							// we need to buffer this because destroy machine will clear it
							pFile->Read(&temp, sizeof temp);
							solo = temp;
							// trackSoloed
							pFile->Read(&temp, sizeof temp);
							_trackSoloed = temp;
							pFile->Read(&temp, sizeof temp);  
							seqBus = temp;
							pFile->Read(&temp, sizeof temp);  
							midiSelected = temp;
							pFile->Read(&temp, sizeof temp);  
							auxcolSelected = temp;
							pFile->Read(&temp, sizeof temp);  
							instSelected = temp;
							// sequence width, for multipattern
							pFile->Read(&temp,sizeof(temp));
							_trackArmedCount = 0;
							for(int i(0) ; i < SONGTRACKS; ++i)
							{
								pFile->Read(&_trackMuted[i],sizeof(_trackMuted[i]));
								// remember to count them
								pFile->Read(&_trackArmed[i],sizeof(_trackArmed[i]));
								if(_trackArmed[i]) ++_trackArmedCount;
							}
							if (fullopen)
							{
								///\todo: Warning! This is done here, because the plugins, when loading, need an up-to-date information.
								/// It should be coded in some way to get this information from the loading song, since doing it here
								/// is bad for the Winamp plugin (or any other multi-document situation).
								Global::pPlayer->SetBPM(BeatsPerMin(), LinesPerBeat());
							}
						}
					}
					else if(std::strcmp(Header,"SEQD")==0)
					{
						--chunkcount;
						pFile->Read(&version,sizeof version);
						pFile->Read(&size,sizeof size);
						if(version > CURRENT_FILE_VERSION_SEQD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Sequence section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// index, for multipattern - for now always 0
							pFile->Read(&index, sizeof index);
							if (index < MAX_SEQUENCES)
							{
								char pTemp[256];
								// play length for this sequence
								pFile->Read(&temp, sizeof temp);
								playLength = temp;
								// name, for multipattern, for now unused
								pFile->ReadString(pTemp, sizeof pTemp);
								for (int i(0) ; i < playLength; ++i)
								{
									pFile->Read(&temp, sizeof temp);
									playOrder[i] = temp;
								}
							}
							else
							{
								//MessageBox(0, "Sequence section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"PATD") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						if(version > CURRENT_FILE_VERSION_PATD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Pattern section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// index
							pFile->Read(&index, sizeof index);
							if(index < MAX_PATTERNS)
							{
								// num lines
								pFile->Read(&temp, sizeof temp );
								// clear it out if it already exists
								RemovePattern(index);
								patternLines[index] = temp;
								// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
								pFile->Read(&temp, sizeof temp );
								pFile->ReadString(patternName[index], sizeof *patternName);
								pFile->Read(&size, sizeof size);
								byte* pSource = new byte[size];
								pFile->Read(pSource, size);
								byte* pDest;
								BEERZ77Decomp2(pSource, &pDest);
								zapArray(pSource,pDest);
								for(int y(0) ; y < patternLines[index] ; ++y)
								{
									unsigned char* pData(_ppattern(index) + (y * MULTIPLY));
									std::memcpy(pData, pSource, SONGTRACKS * EVENT_SIZE);
									pSource += SONGTRACKS * EVENT_SIZE;
								}
								zapArray(pDest);
							}
							else
							{
								//MessageBox(0, "Pattern section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"MACD") == 0)
					{
						int curpos(0);
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						--chunkcount;
						if(!fullopen)
						{
							curpos = pFile->GetPos();
						}
						if(version > CURRENT_FILE_VERSION_MACD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Machine section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(&index, sizeof index);
							if(index < MAX_MACHINES)
							{
								// we had better load it
								DestroyMachine(index);
								_pMachine[index] = Machine::LoadFileChunk(pFile, index, version, fullopen);
								// skips specific chunk.
								if(!fullopen) pFile->Seek(curpos + size);
							}
							else
							{
								//MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"INSD") == 0)
					{
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						--chunkcount;
						if(version > CURRENT_FILE_VERSION_INSD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(&index, sizeof index);
							if(index < MAX_INSTRUMENTS)
							{
								_pInstrument[index]->LoadFileChunk(pFile, version, fullopen);
							}
							else
							{
								//MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"EINS") == 0)
					{
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						long filepos;
						bool wrongState=false;
						filepos=pFile->GetPos();
						--chunkcount;
						// Check higher bits of version (AAAABBBB). 
						// different A, incompatible, different B, compatible
						if ( (version&0x11110000) == (XMSampler::VERSION&0x11110000) )
						{
							// Instrument Data Load
							int numInstruments;
							pFile->Read(numInstruments);
							int idx;
							for(int i = 0;i < numInstruments;i++)
							{
								pFile->Read(idx);
								if (!XMSampler::rInstrument(idx).Load(*pFile)) { wrongState=true; break; }
								//m_Instruments[idx].IsEnabled(true); // done in the loader.
							}
							if (!wrongState)
							{
								int numSamples;
								pFile->Read(numSamples);
								int idx;
								for(int i = 0;i < numSamples;i++)
								{
									pFile->Read(idx);
									if (!XMSampler::SampleData(idx).Load(*pFile)) { wrongState=true; break; }
								}
							}
						}
						else wrongState=true;

						if (wrongState)
						{
							pFile->Seek(filepos+size);
						}
					}
					else 
					{
						// we are not at a valid header for some weird reason.  
						// probably there is some extra data.
						// shift back 3 bytes and try again
						pFile->Seek(pFile->GetPos()-3);
					}
				}
				// now that we have loaded all the modules, time to prepare them.
				Progress.m_Progress.SetPos(16384);
				::Sleep(1); ///< ???
				// test all connections for invalid machines. disconnect invalid machines.
				for(int i(0) ; i < MAX_MACHINES ; ++i)
				{
					if(_pMachine[i])
					{
						_pMachine[i]->_numInputs = 0;
						_pMachine[i]->_numOutputs = 0;
						for (int c(0) ; c < MAX_CONNECTIONS ; ++c)
						{
							if(_pMachine[i]->_connection[c])
							{
								if(_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_connection[c] = false;
									_pMachine[i]->_outputMachines[c] = -1;
								}
								else if(!_pMachine[_pMachine[i]->_outputMachines[c]])
								{
									_pMachine[i]->_connection[c] = false;
									_pMachine[i]->_outputMachines[c] = -1;
								}
								else 
								{
									_pMachine[i]->_numOutputs++;
								}
							}
							else
							{
								_pMachine[i]->_outputMachines[c] = -1;
							}

							if (_pMachine[i]->_inputCon[c])
							{
								if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_inputCon[c] = false;
									_pMachine[i]->_inputMachines[c] = -1;
								}
								else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
								{
									_pMachine[i]->_inputCon[c] = false;
									_pMachine[i]->_inputMachines[c] = -1;
								}
								else
								{
									_pMachine[i]->_numInputs++;
								}
							}
							else
							{
								_pMachine[i]->_inputMachines[c] = -1;
							}
						}
					}
				}

				RestoreMixerSendFlags();
				for (int i(0); i < MAX_MACHINES;++i) if ( _pMachine[i]) _pMachine[i]->PostLoad();
				// translate any data that is required
#if !defined WINAMP_PLUGIN
				static_cast<CMainFrame*>(theApp.m_pMainWnd)->UpdateComboGen();
#endif //!defined WINAMP_PLUGIN
				machineSoloed = solo;
				// allow stuff to work again
				_machineLock = false;
				Progress.OnCancel();
				if((!pFile->Close()) || (chunkcount))
				{
					if (!_pMachine[MASTER_INDEX] )
					{
						_pMachine[MASTER_INDEX] = new Master(MASTER_INDEX);
						_pMachine[MASTER_INDEX]->Init();
					}
					std::ostringstream s;
					s << "Error reading from file '" << pFile->szName << "'" << std::endl;
					if(chunkcount) s << "some chunks were missing in the file";
					else s << "could not close the file";
					MessageBox(0, s.str().c_str(), "File Error!!!", 0);
					return false;
				}
				return true;
			}
			else if(std::strcmp(Header, "PSY2SONG") == 0)
			{
				CProgressDialog Progress;
				Progress.Create();
				Progress.SetWindowText("Loading old format...");
				Progress.ShowWindow(SW_SHOW);
//				int i;
				int num,sampR;
				bool _machineActive[128];
				unsigned char busEffect[64];
				unsigned char busMachine[64];
				New();
				char name_[129]; char author_[65]; char comments_[65536];
				pFile->Read(name_, 32);
				pFile->Read(author_, 32);
				pFile->Read(comments_,128);
				name = name_;
				author = author_;
				comments = comments_;

				pFile->Read(&m_BeatsPerMin, sizeof m_BeatsPerMin);
				pFile->Read(&sampR, sizeof sampR);
				if( sampR <= 0)
				{
					// Shouldn't happen but has happened.
					m_LinesPerBeat= 4;
				}
				// The old format assumes we output at 44100 samples/sec, so...
				else m_LinesPerBeat = 44100 * 60 / (sampR * m_BeatsPerMin);

				if (fullopen)
				{
					///\todo: Warning! This is done here, because the plugins, when loading, need an up-to-date information.
					/// It should be coded in some way to get this information from the loading song, since doing it here
					/// is bad for the Winamp plugin (or any other multi-document situation).
					Global::pPlayer->SetBPM(BeatsPerMin(), LinesPerBeat());
	//				Global::pPlayer->bpm = m_BeatsPerMin;
	//				Global::pPlayer->tpb = m_LinesPerBeat;
	//				Global::pPlayer->SamplesPerRow(sampR * Global::pConfig->_pOutputDriver->_samplesPerSec / 44100);
				}
				pFile->Read(&currentOctave, sizeof(char));
				pFile->Read(busMachine, 64);
				pFile->Read(playOrder, 128);
				pFile->Read(&playLength, sizeof(int));
				pFile->Read(&SONGTRACKS, sizeof(int));
				// Patterns
				pFile->Read(&num, sizeof num);
				for(int i =0 ; i < num; ++i)
				{
					pFile->Read(&patternLines[i], sizeof *patternLines);
					pFile->Read(&patternName[i][0], sizeof *patternName);
					if(patternLines[i] > 0)
					{
						unsigned char * pData(CreateNewPattern(i));
						for(int c(0) ; c < patternLines[i] ; ++c)
						{
							pFile->Read(reinterpret_cast<char*>(pData), OLD_MAX_TRACKS * sizeof(PatternEntry));
							pData += MAX_TRACKS * sizeof(PatternEntry);
						}
					}
					else
					{
						patternLines[i] = 64;
						RemovePattern(i);
					}
				}
				Progress.m_Progress.SetPos(2048);
				::Sleep(1); ///< ???
				// Instruments
				pFile->Read(&instSelected, sizeof instSelected);
				for(int i=0 ; i < OLD_MAX_INSTRUMENTS ; ++i)
				{
					pFile->Read(&_pInstrument[i]->_sName, sizeof(_pInstrument[0]->_sName));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_NNA, sizeof(_pInstrument[0]->_NNA));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_AT, sizeof(_pInstrument[0]->ENV_AT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_DT, sizeof(_pInstrument[0]->ENV_DT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_SL, sizeof(_pInstrument[0]->ENV_SL));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_RT, sizeof(_pInstrument[0]->ENV_RT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_AT, sizeof(_pInstrument[0]->ENV_F_AT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_DT, sizeof(_pInstrument[0]->ENV_F_DT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_SL, sizeof(_pInstrument[0]->ENV_F_SL));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_RT, sizeof(_pInstrument[0]->ENV_F_RT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_CO, sizeof(_pInstrument[0]->ENV_F_CO));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_RQ, sizeof(_pInstrument[0]->ENV_F_RQ));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_EA, sizeof(_pInstrument[0]->ENV_F_EA));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_TP, sizeof(_pInstrument[0]->ENV_F_TP));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_pan, sizeof(_pInstrument[0]->_pan));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_RPAN, sizeof(_pInstrument[0]->_RPAN));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_RCUT, sizeof(_pInstrument[0]->_RCUT));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_RRES, sizeof(_pInstrument[0]->_RRES));
				}
				
				Progress.m_Progress.SetPos(4096);
				::Sleep(1);
				// Waves
				//
				int tmpwvsl;
				pFile->Read(&tmpwvsl, sizeof(int));

				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					for (int w=0; w<OLD_MAX_WAVES; w++)
					{
						int wltemp;
						pFile->Read(&wltemp, sizeof(_pInstrument[0]->waveLength));
						if (wltemp > 0)
						{
							if ( w == 0 )
							{
								short tmpFineTune;
								_pInstrument[i]->waveLength=wltemp;
								pFile->Read(&_pInstrument[i]->waveName, 32);
								pFile->Read(&_pInstrument[i]->waveVolume, sizeof(_pInstrument[0]->waveVolume));
								pFile->Read(&tmpFineTune, sizeof(short));
								_pInstrument[i]->waveFinetune=(int)tmpFineTune;
								pFile->Read(&_pInstrument[i]->waveLoopStart, sizeof(_pInstrument[0]->waveLoopStart));
								pFile->Read(&_pInstrument[i]->waveLoopEnd, sizeof(_pInstrument[0]->waveLoopEnd));
								pFile->Read(&_pInstrument[i]->waveLoopType, sizeof(_pInstrument[0]->waveLoopType));
								pFile->Read(&_pInstrument[i]->waveStereo, sizeof(_pInstrument[0]->waveStereo));
								_pInstrument[i]->waveDataL = new signed short[_pInstrument[i]->waveLength];
								pFile->Read(_pInstrument[i]->waveDataL, _pInstrument[i]->waveLength*sizeof(short));
								if (_pInstrument[i]->waveStereo)
								{
									_pInstrument[i]->waveDataR = new signed short[_pInstrument[i]->waveLength];
									pFile->Read(_pInstrument[i]->waveDataR, _pInstrument[i]->waveLength*sizeof(short));
								}
							}
							else 
							{
								bool stereo;
								char *junk =new char[42+sizeof(bool)];
								pFile->Read(junk,sizeof(junk));
								delete junk;
								pFile->Read(&stereo,sizeof(bool));
								short *junk2 = new signed short[wltemp];
								pFile->Read(junk2, sizeof(junk2));
								if ( stereo )
								{
									pFile->Read(junk2, sizeof(junk2));
								}
								delete junk2;
							}
						}
					}
				}
				
				Progress.m_Progress.SetPos(4096+2048);
				::Sleep(1);
				// VST DLLs
				//

				VSTLoader vstL[OLD_MAX_PLUGINS]; 
				for (int i=0; i<OLD_MAX_PLUGINS; i++)
				{
					pFile->Read(&vstL[i].valid,sizeof(bool));
					if( vstL[i].valid )
					{
						pFile->Read(vstL[i].dllName,sizeof(vstL[i].dllName));
						_strlwr(vstL[i].dllName);
						pFile->Read(&(vstL[i].numpars), sizeof(int));
						vstL[i].pars = new float[vstL[i].numpars];

						for (int c=0; c<vstL[i].numpars; c++)
						{
							pFile->Read(&(vstL[i].pars[c]), sizeof(float));
						}
					}
				}
				
				Progress.m_Progress.SetPos(8192);
				::Sleep(1);
				// Machines
				//
				_machineLock = true;

				pFile->Read(&_machineActive[0], sizeof(_machineActive));
				Machine* pMac[128];
				memset(pMac,0,sizeof(pMac));

				convert_internal_machines::Converter converter;

				for (int i=0; i<128; i++)
				{
					Sampler* pSampler;
					XMSampler* pXMSampler;
					Plugin* pPlugin;
					vst::plugin * pVstPlugin(0);
					int x,y,type;
					if (_machineActive[i])
					{
						Progress.m_Progress.SetPos(8192+i*(4096/128));
						::Sleep(1);

						pFile->Read(&x, sizeof(x));
						pFile->Read(&y, sizeof(y));

						pFile->Read(&type, sizeof(type));

						if(converter.plugin_names().exists(type))
							pMac[i] = &converter.redirect(i, type, *pFile);
						else switch (type)
						{
						case MACH_MASTER:
							pMac[i] = _pMachine[MASTER_INDEX];
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						case MACH_SAMPLER:
							pMac[i] = pSampler = new Sampler(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						case MACH_XMSAMPLER:
							pMac[i] = pXMSampler = new XMSampler(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						case MACH_PLUGIN:
							{
							pMac[i] = pPlugin = new Plugin(i);
							// Should the "Init()" function go here? -> No. Needs to load the dll first.
							if (!pMac[i]->Load(pFile))
							{
								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(pOldMachine);
								pMac[i]->_macIndex=i;
								std::stringstream s;
								s << "X!" << pOldMachine->GetEditName();
								pMac[i]->SetEditName(s.str());
								zapObject(pOldMachine);
								// Warning: It cannot be known if the missing plugin is a generator
								// or an effect. This will be guessed from the busMachine array.
							}
							break;
							}
						case MACH_VST:
						case MACH_VSTFX:
							{
								std::string temp;
								char sPath[_MAX_PATH];
								char sError[128];
								bool berror=false;
								vst::plugin* pTempMac = new vst::plugin(0);
								unsigned char program;
								int instance;
								// The trick: We need to load the information from the file in order to know the "instance" number
								// and be able to create a plugin from the corresponding dll. Later, we will set the loaded settings to
								// the newly created plugin.
								pTempMac->PreLoad(pFile,program,instance);
								assert(instance < OLD_MAX_PLUGINS);
								int shellIdx=0;
								if((!vstL[instance].valid) || (!CNewMachine::lookupDllName(vstL[instance].dllName,temp,MACH_VST,shellIdx)))
								{
									berror=true;
									sprintf(sError,"VST plug-in missing, or erroneous data in song file \"%s\"",vstL[instance].dllName);
								}
								else
								{
									strcpy(sPath,temp.c_str());
									if (!CNewMachine::TestFilename(sPath,shellIdx))
									{
										berror=true;
										sprintf(sError,"This VST plug-in is Disabled \"%s\" - replacing with Dummy.",sPath);
									}
									else
									{
										try
										{
											pMac[i] = pVstPlugin = dynamic_cast<vst::plugin*>(Global::vsthost().LoadPlugin(sPath,shellIdx));

											if (pVstPlugin)
											{
												pVstPlugin->LoadFromMac(pTempMac);
												pVstPlugin->SetProgram(program);
												pVstPlugin->_macIndex=i;
												const int numpars = vstL[instance].numpars;
												for (int c(0) ; c < numpars; ++c)
												{
													try
													{
														pVstPlugin->SetParameter(c, vstL[instance].pars[c]);
													}
													catch(const std::exception &)
													{
														// o_O`
													}
												}
											}
										}
										catch(...)
										{
											berror=true;
											sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath);
										}
									}
								}
								if (berror)
								{
									MessageBox(NULL,sError, "Loading Error", MB_OK);

									pMac[i] = new Dummy(pTempMac);
									pMac[i]->_macIndex=i;
									std::stringstream s;
									s << "X!" << pTempMac->GetEditName();
									pMac[i]->SetEditName(s.str());
									zapObject(pTempMac);
									if (type == MACH_VST ) pMac[i]->_mode = MACHMODE_FX;
									else pMac[i]->_mode = MACHMODE_GENERATOR;
								}
							break;
							}
						case MACH_SCOPE:
						case MACH_DUMMY:
							pMac[i] = new Dummy(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						default:
							{
								char buf[MAX_PATH];
								sprintf(buf,"unkown machine type: %i",type);
								MessageBox(0, buf, "Loading old song", MB_ICONERROR);
							}
							pMac[i] = new Dummy(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
						}

#if !defined WINAMP_PLUGIN
						switch (pMac[i]->_mode)
						{
						case MACHMODE_GENERATOR:
							if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
							if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
							break;
						case MACHMODE_FX:
							if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
							if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
							break;

						case MACHMODE_MASTER:
							if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
							if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;
							break;
						}
#endif //!defined WINAMP_PLUGIN

						pMac[i]->_x = x;
						pMac[i]->_y = y;
					}
				}

				// Patch 0: Some extra data added around the 1.0 release.
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_loop, sizeof(_pInstrument[0]->_loop));
				}
				for (int i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_lines, sizeof(_pInstrument[0]->_lines));
				}

				// Validate the machine arrays. At the same time we fill volMatrix that
				// we will use later on to correctly initialize the wire volumes.
				float volMatrix[128][MAX_CONNECTIONS];
				for (int i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
				{
					if (!_machineActive[i])
					{
						zapObject(pMac[i]);
					}
					else if (!pMac[i])
					{
						_machineActive[i] = false;
					}
					else 
					{
						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							volMatrix[i][c] = pMac[i]->_inputConVol[c];
						}
					}
				}

				// Patch 1: BusEffects (twf). Try to read it, and if it doesn't exist, generate it.
				Progress.m_Progress.SetPos(8192+4096);
				::Sleep(1);
				if ( pFile->Read(&busEffect[0],sizeof(busEffect)) == false )
				{
					int j=0;
					unsigned char invmach[128];
					memset(invmach,255,sizeof(invmach));
					// The guessing procedure does not rely on the machmode because if a plugin
					// is missing, then it is always tagged as a generator.
					for (int i = 0; i < 64; i++)
					{
						if (busMachine[i] != 255) invmach[busMachine[i]]=i;
					}
					for ( int i=1;i<128;i++ ) // machine 0 is the Master machine.
					{
						if (_machineActive[i])
						{
							if (invmach[i] == 255)
							{
								busEffect[j]=i;	
								j++;
							}
						}
					}
					while(j < 64)
					{
						busEffect[j] = 255;
						j++;
					}
				}
				// Validate that there isn't any duplicated machine.
				for ( int i=0;i<64;i++ ) 
				{
					for ( int j=i+1;j<64;j++ ) 
					{
						if  (busMachine[i] == busMachine[j]) busMachine[j]=255;
						if  (busEffect[i] == busEffect[j]) busEffect[j]=255;
					}
					for (int j=0;j<64;j++)
					{
						if  (busMachine[i] == busEffect[j]) busEffect[j]=255;
					}
				}

				// Patch 1.2: Fixes erroneous machine mode when a dummy replaces a bad plugin
				// (missing dll, or when the load process failed).
				// At the same time, we validate the indexes of the busMachine and busEffects arrays.
				for ( int i=0;i<64;i++ ) 
				{
					if (busEffect[i] != 255)
					{
						if ( busEffect[i] > 128 || !_machineActive[busEffect[i]] )
							busEffect[i] = 255;
						// If there's a dummy, force it to be an effect
						else if (pMac[busEffect[i]]->_type == MACH_DUMMY ) 
						{
							pMac[busEffect[i]]->_mode = MACHMODE_FX;
						}
						// Else if the machine is a generator, move it to gens bus.
						// This can't happen, but it is here for completeness
						else if (pMac[busEffect[i]]->_mode == MACHMODE_GENERATOR)
						{
							int k=0;
							while (busEffect[k] != 255 && k<MAX_BUSES) 
							{
								k++;
							}
							busMachine[k]=busEffect[i];
							busEffect[i]=255;
						}
					}
					if (busMachine[i] != 255)
					{
						if (busMachine[i] > 128 || !_machineActive[busMachine[i]])
							busMachine[i] = 255;
						 // If there's a dummy, force it to be a Generator
						else if (pMac[busMachine[i]]->_type == MACH_DUMMY ) 
						{
							pMac[busMachine[i]]->_mode = MACHMODE_GENERATOR;
						}
						// Else if the machine is an fx, move it to FXs bus.
						// This can't happen, but it is here for completeness
						else if ( pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)
						{
							int j=0;
							while (busEffect[j] != 255 && j<MAX_BUSES) 
							{
								j++;
							}
							busEffect[j]=busMachine[i];
							busMachine[i]=255;
						}
					}
				}

				 // Patch 2: VST Chunks.
				Progress.m_Progress.SetPos(8192+4096+1024);
				::Sleep(1);
				bool chunkpresent=false;
				pFile->Read(&chunkpresent,sizeof(chunkpresent));

				if ( fullopen && chunkpresent ) for ( int i=0;i<128;i++ ) 
				{
					if (_machineActive[i])
					{
						if ( pMac[i]->_type == MACH_DUMMY ) 
						{
							if (((Dummy*)pMac[i])->wasVST && chunkpresent )
							{
								// Since we don't know if the plugin saved it or not, 
								// we're stuck on letting the loading crash/behave incorrectly.
								// There should be a flag, like in the VST loading Section to be correct.
								MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
							}
						}
						else if (( pMac[i]->_type == MACH_VST ) || 
							( pMac[i]->_type == MACH_VSTFX))
						{
							bool chunkread = false;
							try
							{
								vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
								if(chunkpresent) chunkread = plugin.LoadChunk(pFile);
							}
							catch(const std::exception &)
							{
								// o_O`
							}
						}
					}
				}


				//////////////////////////////////////////////////////////////////////////
				//Finished all the file loading. Now Process the data to the current structures

				// The old fileformat stored the volumes on each output, 
				// so what we have in inputConVol is really the output
				// and we have to convert it.
				Progress.m_Progress.SetPos(8192+4096+2048);
				::Sleep(1);
				for (int i=0; i<128; i++) // we go to fix this for each
				{
					if (_machineActive[i])		// valid machine (important, since we have to navigate!)
					{
						for (int c=0; c<MAX_CONNECTIONS; c++) // all for each of its input connections.
						{
							if (pMac[i]->_inputCon[c] && pMac[i]->_inputMachines[c] > -1 && pMac[pMac[i]->_inputMachines[c]])	// If there's a valid machine in this inputconnection,
							{
								Machine* pOrigMachine = pMac[pMac[i]->_inputMachines[c]]; // We get that machine
								int d = pOrigMachine->FindOutputWire(i);				// and wire

								if ( d == -1 )
								{
									 pMac[i]->_inputCon[c] = false; pMac[i]->_inputMachines[c] = -1; 
								}
								else
								{
									float val = volMatrix[pMac[i]->_inputMachines[c]][d];
									if( val > 4.1f )
									{
										val*=0.000030517578125f; // BugFix
									}
									else if ( val < 0.00004f) 
									{
										val*=32768.0f; // BugFix
									}
									// and set the volume.
									pMac[i]->InsertInputWireIndex(this,c,pOrigMachine->_macIndex,pOrigMachine->GetAudioRange()/pMac[i]->GetAudioRange(),val);
								}
							}
							else { pMac[i]->_inputCon[c] = false; pMac[i]->_inputMachines[c] = -1; }
						}
					}
				}
				
				// Psycle no longer uses busMachine and busEffect, since the pMachine Array directly maps
				// to the real machine.
				// Due to this, we have to move machines to where they really are, 
				// and remap the inputs and outputs indexes again... ouch
				// At the same time, we validate each wire, and the number count.
				Progress.m_Progress.SetPos(8192+4096+2048+1024);
				::Sleep(1);
				unsigned char invmach[128];
				memset(invmach,255,sizeof(invmach));
				for (int i = 0; i < 64; i++)
				{
					if (busMachine[i] != 255) invmach[busMachine[i]]=i;
					if (busEffect[i] != 255) invmach[busEffect[i]]=i+64;
				}
				invmach[0]=MASTER_INDEX;

				for (int i = 0; i < 128; i++)
				{
					if (invmach[i] != 255)
					{
						;
						Machine *cMac = _pMachine[invmach[i]] = pMac[i];
						cMac->_macIndex = invmach[i];
						_machineActive[i] = false; // mark as "converted"
						cMac->_numInputs = 0;
						cMac->_numOutputs = 0;
						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							if (cMac->_inputCon[c])
							{
								if (cMac->_inputMachines[c] < 0 || cMac->_inputMachines[c] >= MAX_MACHINES-1)
								{
									cMac->_inputCon[c]=false;
									cMac->_inputMachines[c]=-1;
								}
								else if (!pMac[cMac->_inputMachines[c]])
								{
									cMac->_inputCon[c]=false;
									cMac->_inputMachines[c]=-1;
								}
								else
								{
									cMac->_inputMachines[c] = invmach[cMac->_inputMachines[c]];
									cMac->_numInputs++;
								}
							}
							if (cMac->_connection[c])
							{
								if (cMac->_outputMachines[c] < 0 || cMac->_outputMachines[c] >= MAX_MACHINES)
								{
									cMac->_connection[c]=false;
									cMac->_outputMachines[c]=-1;
								}
								else if (!pMac[cMac->_outputMachines[c]])
								{
									cMac->_connection[c]=false;
									cMac->_outputMachines[c]=-1;
								}
								else 
								{
									cMac->_outputMachines[c] = invmach[cMac->_outputMachines[c]];
									cMac->_numOutputs++;
								}
							}
						}
					}
				}
				// verify that there isn't any machine that hasn't been copied into _pMachine
				// Shouldn't happen. It would mean a damaged file.
				int j=0;
				int k=64;
				for (int i=0;i < 128; i++)
				{
					if (_machineActive[i])
					{
						if ( pMac[i]->_mode == MACHMODE_GENERATOR)
						{
							while (_pMachine[j] && j<64) j++;
							_pMachine[j]=pMac[i];
						}
						else
						{
							while (_pMachine[k] && k<128) k++;
							_pMachine[k]=pMac[i];
						}
					}
				}

				Progress.m_Progress.SetPos(16384);
				::Sleep(1);
				if(fullopen) converter.retweak(*this);
				for (int i(0); i < MAX_MACHINES;++i) if ( _pMachine[i]) _pMachine[i]->PostLoad();
				_machineLock = false;
				seqBus=0;
				// Clean the vst loader helpers.
				for (int i=0; i<OLD_MAX_PLUGINS; i++)
				{
					if( vstL[i].valid )
					{
						zapObject(vstL[i].pars);
					}
				}
				Progress.OnCancel();
				if (!pFile->Close())
				{
					std::ostringstream s;
					s << "Error reading from file '" << pFile->szName << "'" << std::endl;
					MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
					return false;
				}

				return true;
			}

			// load did not work
			MessageBox(NULL,"Incorrect file format","Error",MB_OK);
			return false;
		}


		bool Song::Save(RiffFile* pFile,bool autosave)
		{
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			// header, this has to be at the top of the file

			CProgressDialog Progress;
			if ( !autosave ) 
			{
				Progress.Create();
				Progress.SetWindowText("Saving...");
				Progress.ShowWindow(SW_SHOW);
			}

			int chunkcount = 3; // 3 chunks plus:
			for (int i = 0; i < MAX_PATTERNS; i++)
			{
				// check every pattern for validity
				if (IsPatternUsed(i))
				{
					chunkcount++;
				}
			}

			for (int i = 0; i < MAX_MACHINES; i++)
			{
				// check every pattern for validity
				if (_pMachine[i])
				{
					chunkcount++;
				}
			}

			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!_pInstrument[i]->Empty())
				{
					chunkcount++;
				}
			}
			// Instrument Data Save
			int numInstruments = 0;	
			for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
				if(XMSampler::rInstrument(i).IsEnabled()){
					numInstruments++;
				}
			}
			if (numInstruments >0)
			{
				chunkcount++;
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.SetRange(0,chunkcount);
				Progress.m_Progress.SetStep(1);
			}

			/*
			===================
			FILE HEADER
			===================
			id = "PSY3SONG"; // PSY2 was 1.66
			*/

			pFile->Write("PSY3SONG", 8);

			UINT version = CURRENT_FILE_VERSION;
			UINT size = sizeof(chunkcount);
			UINT index = 0;
			int temp;

			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));
			pFile->Write(&chunkcount,sizeof(chunkcount));

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			// the rest of the modules can be arranged in any order

			/*
			===================
			SONG INFO TEXT
			===================
			id = "INFO"; 
			*/

			pFile->Write("INFO",4);
			version = CURRENT_FILE_VERSION_INFO;
///\todo: BUG!!!! Fileformat does NOT define this 3*sizeof(int)
			size = name.length() + author.length() + comments.length() + 3*sizeof(int) + 3; // +3 for \0

			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));

			pFile->Write(name.c_str(),name.length()+1);
			pFile->Write(author.c_str(),author.length()+1);
			pFile->Write(comments.c_str(),comments.length()+1);

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			/*
			===================
			SONG INFO
			===================
			id = "SNGI"; 
			*/

			pFile->Write("SNGI",4);
			version = CURRENT_FILE_VERSION_SNGI;
			size = (11*sizeof(temp))+(SONGTRACKS*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])));
			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));

			temp = SONGTRACKS;
			pFile->Write(&temp,sizeof(temp));
			temp = m_BeatsPerMin;
			pFile->Write(&temp,sizeof(temp));
			temp = m_LinesPerBeat;
			pFile->Write(&temp,sizeof(temp));
			temp = currentOctave;
			pFile->Write(&temp,sizeof(temp));
			temp = machineSoloed;
			pFile->Write(&temp,sizeof(temp));
			temp = _trackSoloed;
			pFile->Write(&temp,sizeof(temp));

			temp = seqBus;
			pFile->Write(&temp,sizeof(temp));

			temp = midiSelected;
			pFile->Write(&temp,sizeof(temp));
			temp = auxcolSelected;
			pFile->Write(&temp,sizeof(temp));
			temp = instSelected;
			pFile->Write(&temp,sizeof(temp));

			temp = 1; // sequence width
			pFile->Write(&temp,sizeof(temp));

			for (int i = 0; i < SONGTRACKS; i++)
			{
				pFile->Write(&_trackMuted[i],sizeof(_trackMuted[i]));
				pFile->Write(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			/*
			===================
			SEQUENCE DATA
			===================
			id = "SEQD"; 
			*/
			index = 0; // index
			for (index=0;index<MAX_SEQUENCES;index++)
			{
				char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequence.

				pFile->Write("SEQD",4);
				version = CURRENT_FILE_VERSION_SEQD;
				size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
				pFile->Write(&version,sizeof(version));
				pFile->Write(&size,sizeof(size));
				
				pFile->Write(&index,sizeof(index)); // Sequence Track number
				temp = playLength;
				pFile->Write(&temp,sizeof(temp)); // Sequence length

				pFile->Write(pSequenceName,strlen(pSequenceName)+1); // Sequence Name

				for (int i = 0; i < playLength; i++)
				{
					temp = playOrder[i];
					pFile->Write(&temp,sizeof(temp));	// Sequence data.
				}
			}
			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			/*
			===================
			PATTERN DATA
			===================
			id = "PATD"; 
			*/

			for (int i = 0; i < MAX_PATTERNS; i++)
			{
				// check every pattern for validity
				if (IsPatternUsed(i))
				{
					// ok save it
					byte* pSource=new byte[SONGTRACKS*patternLines[i]*EVENT_SIZE];
					byte* pCopy = pSource;

					for (int y = 0; y < patternLines[i]; y++)
					{
						unsigned char* pData = ppPatternData[i]+(y*MULTIPLY);
						memcpy(pCopy,pData,EVENT_SIZE*SONGTRACKS);
						pCopy+=EVENT_SIZE*SONGTRACKS;
					}
					
					int sizez77 = BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[i]*EVENT_SIZE);
					zapArray(pSource);

					pFile->Write("PATD",4);
					version = CURRENT_FILE_VERSION_PATD;

					pFile->Write(&version,sizeof(version));
					size = sizez77+(4*sizeof(temp))+strlen(patternName[i])+1;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));
					temp = patternLines[i];
					pFile->Write(&temp,sizeof(temp));
					temp = SONGTRACKS; // eventually this may be variable per pattern
					pFile->Write(&temp,sizeof(temp));

					pFile->Write(&patternName[i],strlen(patternName[i])+1);

					pFile->Write(&sizez77,sizeof(sizez77));
					pFile->Write(pCopy,sizez77);
					zapArray(pCopy);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}
			/*
			===================
			MACHINE DATA
			===================
			id = "MACD"; 
			*/
			// machine and instruments handle their save and load in their respective classes

			for (int i = 0; i < MAX_MACHINES; i++)
			{
				if (_pMachine[i])
				{
					pFile->Write("MACD",4);
					version = CURRENT_FILE_VERSION_MACD;
					pFile->Write(&version,sizeof(version));
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));

					_pMachine[i]->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(&size,sizeof(size));
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}
			/*
			===================
			Instrument DATA
			===================
			id = "INSD"; 
			*/
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!_pInstrument[i]->Empty())
				{
					pFile->Write("INSD",4);
					version = CURRENT_FILE_VERSION_INSD;
					pFile->Write(&version,sizeof(version));
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));

					_pInstrument[i]->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(&size,sizeof(size));
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}

			/*
			===================================
			Extended Instrument DATA (Sampulse)
			===================================
			id = "EINS"; 
			*/

			// Instrument Data Save
			if (numInstruments >0)
			{
				pFile->Write("EINS",4);
				version = XMSampler::VERSION;
				pFile->Write(&version,sizeof(version));
				long pos = pFile->GetPos();
				size = 0;
				pFile->Write(&size,sizeof(size));

				pFile->Write(numInstruments);

				for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
					if(XMSampler::rInstrument(i).IsEnabled()){
						pFile->Write(i);
						XMSampler::rInstrument(i).Save(*pFile);
					}
				}

				// Sample Data Save
				int numSamples = 0;	
				for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
					if(XMSampler::SampleData(i).WaveLength() != 0){
						numSamples++;
					}
				}

				pFile->Write(numSamples);

				for(int i = 0;i < XMSampler::MAX_INSTRUMENT;i++){
					if(XMSampler::SampleData(i).WaveLength() != 0){
						pFile->Write(i);
						XMSampler::SampleData(i).Save(*pFile);
					}
				}
				long pos2 = pFile->GetPos(); 
				size = pos2-pos-sizeof(size);
				pFile->Seek(pos);
				pFile->Write(&size,sizeof(size));
				pFile->Seek(pos2);
			}


			if ( !autosave ) 
			{
				Progress.m_Progress.SetPos(chunkcount);
				::Sleep(1);

				Progress.OnCancel();
			}

			if (!pFile->Close())
			{
				std::ostringstream s;
				s << "Error writing to file '" << pFile->szName << "'" << std::endl;
				MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
			}

			return true;
		}
		void Song::DoPreviews(int amount)
		{
#if !defined WINAMP_PLUGIN
			//todo do better.. use a vector<InstPreview*> or something instead
			if(wavprev.IsEnabled())
			{
				wavprev.Work(_pMachine[MASTER_INDEX]->_pSamplesL, _pMachine[MASTER_INDEX]->_pSamplesR, amount);
			}
			if(waved.IsEnabled())
			{
				waved.Work(_pMachine[MASTER_INDEX]->_pSamplesL, _pMachine[MASTER_INDEX]->_pSamplesR, amount);
			}
#endif // !defined WINAMP_PLUGIN
		}

		bool Song::CloneMac(int src,int dst)
		{
			// src has to be occupied and dst must be empty
			if (_pMachine[src] && _pMachine[dst])
			{
				return false;
			}
			if (_pMachine[dst])
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (!_pMachine[src])
			{
				return false;
			}
			// check to see both are same type
			if (((dst < MAX_BUSES) && (src >= MAX_BUSES))
				|| ((dst >= MAX_BUSES) && (src < MAX_BUSES)))
			{
				return false;
			}

			if ((src >= MAX_MACHINES-1) || (dst >= MAX_MACHINES-1))
			{
				return false;
			}

			// save our file
#if !defined WINAMP_PLUGIN
			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();
#endif //!defined WINAMP_PLUGIN
			///\todo: Wrong song dir causes "machine cloning failed"! 
			///\todo: the process should be chagned and save the data in memory.
			CString filepath = Global::pConfig->GetSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			OldPsyFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.Write("MACD",4);
			UINT version = CURRENT_FILE_VERSION_MACD;
			file.Write(&version,sizeof(version));
			long pos = file.GetPos();
			UINT size = 0;
			file.Write(&size,sizeof(size));

			int index = dst; // index
			file.Write(&index,sizeof(index));

			_pMachine[src]->SaveFileChunk(&file);

			long pos2 = file.GetPos(); 
			size = pos2-pos-sizeof(size);
			file.Seek(pos);
			file.Write(&size,sizeof(size));
			file.Close();

			// now load it

			if (!file.Open(filepath.GetBuffer(1)))
			{
				DeleteFile(filepath);
				return false;
			}
			char Header[5];
			file.Read(&Header, 4);
			Header[4] = 0;
			if (strcmp(Header,"MACD")==0)
			{
				file.Read(&version,sizeof(version));
				file.Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(&index,sizeof(index));
					index = dst;
					if (index < MAX_MACHINES)
					{
						// we had better load it
						DestroyMachine(index);
						_pMachine[index] = Machine::LoadFileChunk(&file,index,version);
					}
					else
					{
						file.Close();
						DeleteFile(filepath);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
			file.Close();
			DeleteFile(filepath);

			// randomize the dst's position

#if !defined WINAMP_PLUGIN
			int xs,ys,x,y;
			if (src >= MAX_BUSES)
			{
				xs = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
				ys = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
			}
			else 
			{
				xs = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
				ys = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
			}
			x=_pMachine[dst]->_x+32;
			y=_pMachine[dst]->_y+ys+8;

			bool bCovered = TRUE;
			while (bCovered)
			{
				bCovered = FALSE;
				for (int i=0; i < MAX_MACHINES; i++)
				{
					if (i != dst)
					{
						if (_pMachine[i])
						{
							if ((abs(_pMachine[i]->_x - x) < 32) &&
								(abs(_pMachine[i]->_y - y) < 32))
							{
								bCovered = TRUE;
								i = MAX_MACHINES;
								x = (rand())%(((CMainFrame *)theApp.m_pMainWnd)->m_wndView.CW-xs);
								y = (rand())%(((CMainFrame *)theApp.m_pMainWnd)->m_wndView.CH-ys);
							}
						}
					}
				}
			}
			_pMachine[dst]->_x = x;
			_pMachine[dst]->_y = y;
#endif //!defined WINAMP_PLUGIN

			// delete all connections

			_pMachine[dst]->DeleteWires(this);

			int number = 1;
			char buf[sizeof(_pMachine[dst]->_editName)+4];
			strcpy (buf,_pMachine[dst]->_editName);
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
					ps = strchr(_pMachine[dst]->_editName,' ');
					ps[0] = 0;
				}
			}

			for (int i = 0; i < MAX_MACHINES-1; i++)
			{
				if (i!=dst)
				{
					if (_pMachine[i])
					{
						if (strcmp(_pMachine[i]->_editName,buf)==0)
						{
							number++;
							sprintf(buf,"%s %d",_pMachine[dst]->_editName,number);
							i = -1;
						}
					}
				}
			}

			buf[sizeof(_pMachine[dst]->_editName)-1] = 0;
			strcpy(_pMachine[dst]->_editName,buf);

			return true;
		}


		bool Song::CloneIns(int src,int dst)
		{
			// src has to be occupied and dst must be empty
			if (!_pInstrument[src]->Empty() && !_pInstrument[dst]->Empty())
			{
				return false;
			}
			if (!_pInstrument[dst]->Empty())
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (_pInstrument[src]->Empty())
			{
				return false;
			}
			// ok now we get down to business
#if !defined WINAMP_PLUGIN

			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();
#endif //!defined WINAMP_PLUGIN

			// save our file

			CString filepath = Global::pConfig->GetSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			OldPsyFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.Write("INSD",4);
			UINT version = CURRENT_FILE_VERSION_INSD;
			file.Write(&version,sizeof(version));
			long pos = file.GetPos();
			UINT size = 0;
			file.Write(&size,sizeof(size));

			int index = dst; // index
			file.Write(&index,sizeof(index));

			_pInstrument[src]->SaveFileChunk(&file);

			long pos2 = file.GetPos(); 
			size = pos2-pos-sizeof(size);
			file.Seek(pos);
			file.Write(&size,sizeof(size));

			file.Close();

			// now load it

			if (!file.Open(filepath.GetBuffer(1)))
			{
				DeleteFile(filepath);
				return false;
			}
			char Header[5];
			file.Read(&Header, 4);
			Header[4] = 0;

			if (strcmp(Header,"INSD")==0)
			{
				file.Read(&version,sizeof(version));
				file.Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(&index,sizeof(index));
					index = dst;
					if (index < MAX_INSTRUMENTS)
					{
						// we had better load it
						_pInstrument[index]->LoadFileChunk(&file,version);
					}
					else
					{
						file.Close();
						DeleteFile(filepath);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
			file.Close();
			DeleteFile(filepath);
			return true;
		}

		bool Song::IsPatternUsed(int i)
		{
			bool bUsed = FALSE;
			if (ppPatternData[i])
			{
				// we could also check to see if pattern is unused AND blank.
				for (int j = 0; j < playLength; j++)
				{
					if (playOrder[j] == i)
					{
						bUsed = TRUE;
						break;
					}
				}

				if (!bUsed)
				{
					// check to see if it is empty
					PatternEntry blank;
					unsigned char * pData = ppPatternData[i];
					for (int j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
					{
						if (memcmp(pData+j,&blank,EVENT_SIZE) != 0 )
						{
							bUsed = TRUE;
							j = MULTIPLY2;
							break;
						}
					}
				}
			}
			return bUsed;
		}
	}
}
