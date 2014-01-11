///\file
///\brief implementation file for psycle::host::Song.

#include <psycle/host/detail/project.private.hpp>
#include "Song.hpp"
#include "machineloader.hpp"

#if !defined WINAMP_PLUGIN
	#include "PsycleConfig.hpp"
	#include "ProgressDialog.hpp"
	#include "InputHandler.hpp"
	#include <psycle/helpers/riff.hpp> // for Wave file loading.
#else
	#include "Configuration.hpp"
	#include "player_plugins/winamp/fake_progressDialog.hpp"
#endif //!defined WINAMP_PLUGIN

#include "Machine.hpp" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.hpp"
#include "XMSampler.hpp"
#include "Plugin.hpp"
#include "VstHost24.hpp"
#include "LuaPlugin.hpp"
#include "LuaHost.hpp"

#include <psycle/helpers/datacompression.hpp>
#include <psycle/helpers/math.hpp>
namespace loggers = universalis::os::loggers;
#include "convert_internal_machines.private.hpp"

#include "Zap.hpp"

namespace psycle
{
	namespace host
	{
		int Song::defaultPatLines = 64;

		/// Helper class for the PSY2 loader.
		class VSTLoader
		{
		public:
			bool valid;
			char dllName[128];
			std::int32_t numpars;
			float * pars;
		};

		void Song::SetDefaultPatLines(int lines)
		{
			defaultPatLines = lines;
		}
		bool Song::CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int songIdx,std::int32_t shellIdx)
		{
			Machine* pMachine = CreateMachine(type, psPluginDll, songIdx, shellIdx);
			if (pMachine) {
				pMachine->_x = x;
				pMachine->_y = y;
				if(_pMachine[songIdx]) DestroyMachine(songIdx);
				_pMachine[songIdx] = pMachine;
				return true;
			}
			else {
				return false;
			}
		}
		Machine* Song::CreateMachine(MachineType type, char const* psPluginDll,int songIdx,std::int32_t shellIdx)
		{
			Machine* pMachine(0);
			Plugin* pPlugin(0);
			LuaPlugin *luaPlug(0);
			vst::Plugin *vstPlug(0);
			if(songIdx < 0)
			{
				songIdx =	GetFreeMachine();
				if(songIdx < 0) return NULL;
			}
			switch (type)
			{
			case MACH_MASTER:
				if(_pMachine[MASTER_INDEX]) pMachine = _pMachine[MASTER_INDEX];
				else pMachine = new Master(songIdx);
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
			case MACH_DUPLICATOR2:
				pMachine = new DuplicatorMac2(songIdx);
				break;
			case MACH_LUA:
				{
					if(Global::machineload().TestFilename(psPluginDll,shellIdx))
					{
						try
						{
							pMachine = luaPlug = dynamic_cast<LuaPlugin*>(LuaHost::LoadPlugin(psPluginDll,songIdx));
						}
						catch(const std::exception& e)
						{
							loggers::exception()(e.what());
							zapObject(pMachine); 
						}
						catch(...)
						{
							zapObject(pMachine); 
						}
					}
					break;
				}
			case MACH_MIXER:
				pMachine = new Mixer(songIdx);
				break;
			case MACH_RECORDER:
				pMachine = new AudioRecorder(songIdx);
				break;
			case MACH_PLUGIN:
				{
					if(Global::machineload().TestFilename(psPluginDll,shellIdx))
					{
						try
						{
							pMachine = pPlugin = new Plugin(songIdx);
							pPlugin->Instance(psPluginDll);
						}
						catch(const std::exception& e)
						{
							loggers::exception()(e.what());
							zapObject(pMachine); 
						}
						catch(...)
						{
							zapObject(pMachine); 
						}
					}
					break;
				}
			case MACH_VST:
			case MACH_VSTFX:
				{
					if(Global::machineload().TestFilename(psPluginDll,shellIdx)) 
					{
						try
						{
							pMachine = vstPlug = dynamic_cast<vst::Plugin*>(Global::vsthost().LoadPlugin(psPluginDll,shellIdx));
							if(vstPlug)
							{
								vstPlug->_macIndex=songIdx;
							}
						}
						catch(const std::runtime_error & e)
						{
							std::ostringstream s; s << typeid(e).name() << std::endl;
							if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
							loggers::exception()(s.str());
						}
						//TODO: Warning! This is not std::exception, but universalis::stdlib::exception
						catch(const std::exception & e)
						{
							loggers::exception()(e.what());
							zapObject(pMachine); 
						}
						catch(...)
						{
#ifndef NDEBUG 
							throw;
#else
							loggers::exception()("unknown exception");
							zapObject(pMachine);
#endif
						}
					}
					break;
				}
			case MACH_DUMMY:
				pMachine = new Dummy(songIdx);
				break;
			default:
				break;
			}
			if (pMachine) {
				pMachine->Init();
			}
			return pMachine;
		}


		int Song::FindBusFromIndex(int smac) const
		{
			if(!_pMachine[smac])  return 255;
			return smac;
		}

		Song::Song()
			:semaphore(2,2,NULL,NULL)
		{
			for(int i(0) ; i < MAX_PATTERNS; ++i) ppPatternData[i] = NULL;
			for(int i(0) ; i < MAX_MACHINES; ++i) _pMachine[i] = NULL;
			CreateNewPattern(0);
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument();
			Reset();
		}

		Song::~Song()
		{
			DestroyAllMachines();
			DestroyAllInstruments();
			DeleteAllPatterns();
		}

		bool Song::ReplaceMachine(Machine* origmac, MachineType type, int x, int y, char const* psPluginDll, int songIdx,std::int32_t shellIdx)
		{
			CExclusiveLock lock(&semaphore, 2, true);
			///\todo: This has been copied from GearRack code. It needs to be converted (with multi-io and the mixer, this doesn't work at all)
			assert(origmac);

			Machine* newmac = CreateMachine(type,psPluginDll,songIdx,shellIdx);
			// replace all the connection info
			if (newmac)
			{
				// Finally, Rewire.
				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					if (origmac->inWires[i].Enabled()) {
						origmac->inWires[i].ChangeDest(newmac->inWires[i]);
					}
					if (origmac->outWires[i] && origmac->outWires[i]->Enabled()) {
						origmac->outWires[i]->ChangeSource(*newmac, 0, i
							, &origmac->outWires[i]->GetMapping());
					}
				}
				if(_pMachine[songIdx]) DestroyMachine(songIdx);
				_pMachine[songIdx] = newmac;
				newmac->_x = x;
				newmac->_y = y;
				return true;
			}

			return false;
		}

		bool Song::ExchangeMachines(int one, int two)
		{
			CExclusiveLock lock(&semaphore, 2, true);
			Machine *mac1 = _pMachine[one];
			Machine *mac2 = _pMachine[two];

			///\todo: This has been added as a safety measure. This method with the mixer (and multi-io) does not work.
			if( (mac1  && (mac1->_isMixerSend || mac1->_type == MACH_MIXER))
				|| (mac2 && (mac2->_isMixerSend || mac2->_type == MACH_MIXER)))
			{
				char buf[128];
				std::sprintf(buf,"Cannot exchange the mixer with any other machine, or any send effect of the mixer");
				MessageBox(0, buf, "Exchange Machine", 0);
				return false;
			}
			// if they are both valid
			if (mac1 && mac2)
			{
				std::vector<Wire> inWire1;
				std::vector<Wire> inWire2;
				std::vector<Wire*> outWire1;
				std::vector<Wire*> outWire2;
				Wire unused(mac1);
				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					inWire1.push_back(mac1->inWires[i]);
					inWire2.push_back(mac2->inWires[i]);
					mac1->inWires[i].Disconnect();
					mac2->inWires[i].Disconnect();
					if (mac1->outWires[i]) {
						outWire1.push_back(mac1->outWires[i]);
						mac1->outWires[i]->Disconnect();
					}
					else {
						outWire1.push_back(&unused);
					}
					if (mac2->outWires[i]) {
						outWire2.push_back(mac2->outWires[i]);
						mac2->outWires[i]->Disconnect();
					}
					else {
						outWire2.push_back(&unused);
					}
				}

				// Finally, Rewire.
				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					if (inWire1[i].Enabled()) {
						if (inWire1[i].GetSrcMachine()._macIndex != mac2->_macIndex) {
							mac2->inWires[i].ConnectSource(inWire1[i].GetSrcMachine(), 0
								,inWire1[i].GetSrcWireIndex()
								,&inWire1[i].GetMapping());
							mac2->inWires[i].SetVolume(inWire1[i].GetVolume());
						}
						else {
							mac1->inWires[i].ConnectSource(*mac2,0
								,inWire1[i].GetSrcWireIndex()
								,&inWire1[i].GetMapping());
						}
					}
					if (inWire2[i].Enabled()) {
						if (inWire2[i].GetSrcMachine()._macIndex != mac1->_macIndex) {
							mac1->inWires[i].ConnectSource(inWire2[i].GetSrcMachine(), 0
								,inWire2[i].GetSrcWireIndex()
								,&inWire2[i].GetMapping());
							mac1->inWires[i].SetVolume(inWire2[i].GetVolume());
						}
						else {
							mac2->inWires[i].ConnectSource(*mac1,0
								,inWire2[i].GetSrcWireIndex()
								,&inWire2[i].GetMapping());
						}
					}
					if (outWire1[i] != &unused) {
						if (outWire1[i]->GetDstMachine()._macIndex != mac2->_macIndex) {
							outWire1[i]->ConnectSource(*mac2, 0, i, &outWire1[i]->GetMapping());
							outWire1[i]->SetVolume(outWire1[i]->GetVolume());
						}
					}
					if (outWire2[i] != &unused) {
						if (outWire2[i]->GetDstMachine()._macIndex != mac1->_macIndex) {
							outWire2[i]->ConnectSource(*mac1, 0, i, &outWire2[i]->GetMapping());
							outWire2[i]->SetVolume(outWire2[i]->GetVolume());
						}
					}
				}

				// exchange positions
				int temp = mac1->_x;
				mac1->_x = mac2->_x;
				mac2->_x = temp;
				temp = mac1->_y;
				mac1->_y = mac2->_y;
				mac2->_y = temp;

				// Exchange the Machine number.
				_pMachine[one] = mac2;
				_pMachine[two] = mac1;
				mac1->_macIndex = two;
				mac2->_macIndex = one;
				return true;
			}
			else if (mac1)
			{
				// ok we gotta swap this one for a null one
				_pMachine[one] = NULL;
				_pMachine[two] = mac1;

				mac1->_macIndex = two;
				return true;
			}
			else if (mac2)
			{
				// ok we gotta swap this one for a null one
				_pMachine[one] = mac2;
				_pMachine[two] = NULL;

				mac2->_macIndex = one;
				return true;
			}
			return false;
		}

		void Song::DestroyAllMachines()
		{
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
					DestroyMachine(c);
				}
				_pMachine[c] = 0;
			}
		}
		void Song::StopInstrument(int instrumentIdx)
		{
			for(int i=0; i< MAX_MACHINES; i++) {
				Machine* mac = _pMachine[i];
				//TODO: also for sampulse
				if(mac && mac->_type == MACH_SAMPLER) {
					Sampler& sam = *((Sampler*)mac);
					sam.StopInstrument(instrumentIdx);
				}
			}

		}

		void Song::ExchangeInstruments(int one, int two)
		{
			CExclusiveLock lock(&semaphore, 2, true);

			Instrument * tmpins;
			tmpins=_pInstrument[one];
			_pInstrument[one]=_pInstrument[two];
			_pInstrument[two]=tmpins;
			samples.ExchangeSamples(one,two);
		}

		void Song::DeleteInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) DeleteInstrument(i);
		}

		void Song::DestroyAllInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) zapObject(_pInstrument[i]);
			xminstruments.Clear();
			samples.Clear();
		}

		void Song::DeleteInstrument(int i)
		{
			_pInstrument[i]->Init();
			if (samples.IsEnabled(i)){
				samples.get(i).Init();
			}
		}

		void Song::Reset()
		{
			srand(time(NULL));
			// Cleaning pattern allocation info
			samples.Clear();
			xminstruments.Clear();
			for(int i(0) ; i < MAX_MACHINES ; ++i)
			{
				zapObject(_pMachine[i]);
			}

			for(int i(0) ; i < MAX_PATTERNS; ++i)
			{
				// All pattern reset
				patternLines[i]=defaultPatLines;
				std::sprintf(patternName[i], "Untitled"); 
				for(int j(0) ;  j < MAX_TRACKS; j++) {
					_trackNames[i][j] = "";
				}
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
			CExclusiveLock lock(&semaphore, 2, true);
			DoNew();
		}

		void Song::DoNew() {
			seqBus=0;
			shareTrackNames=true;
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
			// Clean up allocated machines.
			DestroyAllMachines();
			// Cleaning instruments
			DeleteInstruments();
			// Clear patterns
			DeleteAllPatterns();
			// Clear sequence
			Reset();
			waveSelected = 0;
			instSelected = 0;
			paramSelected = 0;
			auxcolSelected = 0;
			_saved=false;
			fileName ="Untitled.psy";
			CreateMachine(MACH_MASTER, 320, 200, 0, MASTER_INDEX);
		}

		void Song::ChangeTrackName(int patIdx, int trackidx, std::string name)
		{
			if(shareTrackNames)
			{
				for(int i(0); i < MAX_PATTERNS; i++) {
					_trackNames[i][trackidx] = name;
				}
			}
			else 
			{
				_trackNames[patIdx][trackidx] = name;
			}
		}
		void Song::SetTrackNameShareMode(bool shared)
		{
			if(shared) {
				for(int i(1); i < MAX_PATTERNS; i++) {
					for(int j(0); j < SONGTRACKS; j++) {
						_trackNames[i][j] = _trackNames[i][j];
					}
				}
			}
		}
		void Song::CopyNamesFrom(int patOrig,int patDest)
		{
			for(int i(0); i< SONGTRACKS; i++) {
				_trackNames[patDest][i] = _trackNames[patOrig][i];
			}
		}

		int Song::GetFreeMachine() const
		{
			int idx=-1;
			for(int tmac = 0; tmac < MAX_MACHINES; tmac++)
			{
				if(!_pMachine[tmac]) {
					idx=tmac;
					break;
				}
			}
			return idx;
		}
		bool Song::ValidateMixerSendCandidate(Machine& mac,bool rewiring)
		{
			// Basically, we dissallow a send comming from a generator as well as multiple-outs for sends.
			if ( mac._mode == MACHMODE_GENERATOR) return false;
			if ( mac.connectedInputs() > 1 || (mac.connectedOutputs() > 0 && !rewiring) ) return false;
			for (int i(0); i<MAX_CONNECTIONS; ++i)
			{
				if (mac.inWires[i].Enabled())
				{
					if (!ValidateMixerSendCandidate(mac.inWires[i].GetSrcMachine(),true)) //true because obviously it has one output
					{
						return false;
					}
				}
			}
			return true;
		}

		int Song::InsertConnectionNonBlocking(Machine* srcMac,Machine* dstMac, int srctype, int dsttype,float initialVol)
		{
			// Assert that we have two machines
			assert(srcMac); assert(dstMac);
			// Verify that the destination is not a generator
			if(dstMac->_mode == MACHMODE_GENERATOR) return -1;
			// Verify that src is not connected to dst already, and that destination is not connected to source.
			if (srcMac->FindOutputWire(dstMac->_macIndex) > -1 || dstMac->FindOutputWire(srcMac->_macIndex) > -1) return -1;
			// disallow mixer as a sender of another mixer
			if ( srcMac->_type == MACH_MIXER && dstMac->_type == MACH_MIXER && dsttype != 0) return -1;
			// If source is in a mixer chain, dissallow the new connection (there can only be one output and at much one input).
			if ( srcMac->_isMixerSend ) return -1;
			// If destination is in a mixer chain (or the mixer itself), validate the sender first
			if ( dstMac->_isMixerSend || (dstMac->_type == MACH_MIXER && dsttype == 1))
			{
				if (!ValidateMixerSendCandidate(*srcMac)) return -1;
			}

			// Try to get free indexes from each machine
			int freebus=srcMac->GetFreeOutputWire(srctype);
			int dfreebus=dstMac->GetFreeInputWire(dsttype);
			if(freebus == -1 || dfreebus == -1 ) return -1;

			// If everything went right, connect them.
			//Patch for Mixer. These methods should really be using Wire objects now, not indexes.
			if (dstMac->_type == MACH_MIXER && dfreebus >= MAX_CONNECTIONS) {
				dfreebus-=MAX_CONNECTIONS;
				Mixer &mixer = *static_cast<Mixer*>(dstMac);
				mixer.InsertReturn(dfreebus);
				Wire &wire = mixer.Return(dfreebus).GetWire();
				wire.ConnectSource(*srcMac,srctype,freebus);
				wire.SetVolume(initialVol);
			}
			else {
				dstMac->inWires[dfreebus].ConnectSource(*srcMac,srctype,freebus);
				dstMac->inWires[dfreebus].SetVolume(initialVol);
			}
			return dfreebus;
		}
		bool Song::ChangeWireDestMacNonBlocking(Machine* srcMac,Machine* newdstMac, int wiresrc,int newwiredest)
		{
			// Assert that we have two machines
			assert(srcMac); assert(newdstMac);
			// Verify that the destination is not a generator
			if (newdstMac->_mode == MACHMODE_GENERATOR) return false;
			// Verify that src is not connected to dst already, and that destination is not connected to source.
			if (srcMac->FindOutputWire(newdstMac->_macIndex) > -1 || newdstMac->FindOutputWire(srcMac->_macIndex) > -1) return false;
			// If source and dest are a mixer chain, dissallow connection if connecting as send.
			if ( srcMac->_type == MACH_MIXER && newdstMac->_type == MACH_MIXER && newwiredest >=MAX_CONNECTIONS) return false;
			// If destination is in a mixer chain (or the mixer itself), validate the sender first
			if ( newdstMac->_isMixerSend || (newdstMac->_type == MACH_MIXER && newwiredest >= MAX_CONNECTIONS))
			{
				///\todo: validate for the case where srcMac->_isMixerSend
				if (!ValidateMixerSendCandidate(*srcMac,true)) return false;
			}

			if (wiresrc == -1 || newwiredest == -1 || srcMac->outWires[wiresrc] == NULL || srcMac->outWires[wiresrc]->Enabled() == false)
				return false;

			//Patch for Mixer. These methods should really be using Wire objects now, not indexes.
			if (newdstMac->_type == MACH_MIXER && newwiredest >= MAX_CONNECTIONS) {
				newwiredest-=MAX_CONNECTIONS;
				Mixer &mixer = *static_cast<Mixer*>(newdstMac);
				mixer.InsertReturn(newwiredest);
				srcMac->outWires[wiresrc]->ChangeDest(mixer.Return(newwiredest).GetWire());
			}
			else {
				srcMac->outWires[wiresrc]->ChangeDest(newdstMac->inWires[newwiredest]);
			}
			return true;
		}
		bool Song::ChangeWireSourceMacNonBlocking(Machine* newsrcMac,Machine* dstMac, int newwiresrc, int wiredest)
		{
			// Assert that we have two machines
			assert(newsrcMac); assert(dstMac);
			// Verify that the destination is not a generator
			if(dstMac->_mode == MACHMODE_GENERATOR) return false;
			// Verify that src is not connected to dst already, and that destination is not connected to source.
			if (newsrcMac->FindOutputWire(dstMac->_macIndex) > -1 || dstMac->FindOutputWire(newsrcMac->_macIndex) > -1) return false;
			// disallow mixer as a sender of another mixer
			if ( newsrcMac->_type == MACH_MIXER && dstMac->_type == MACH_MIXER && wiredest >= MAX_CONNECTIONS) return false;
			// If source is in a mixer chain, dissallow the new connection.
			if ( newsrcMac->_isMixerSend ) return false;
			// If destination is in a mixer chain (or the mixer itself), validate the sender first
			if ( dstMac->_isMixerSend || (dstMac->_type == MACH_MIXER && wiredest >= MAX_CONNECTIONS))
			{
				if (!ValidateMixerSendCandidate(*newsrcMac,false)) return false;
			}

			if (newwiresrc == -1 || wiredest == -1)
				return false;

			if ( dstMac->_type == MACH_MIXER && wiredest >= MAX_CONNECTIONS)
			{
				if (((Mixer*)dstMac)->ReturnValid(wiredest-MAX_CONNECTIONS))
				{
					((Mixer*)dstMac)->Return(wiredest-MAX_CONNECTIONS).GetWire()
						.ChangeSource(*newsrcMac,0,newwiresrc, &dstMac->inWires[wiredest].GetMapping());
					return true;
				}
			}
			else if (dstMac->inWires[wiredest].Enabled()) {
				dstMac->inWires[wiredest].ChangeSource(*newsrcMac,0,newwiresrc,
					&dstMac->inWires[wiredest].GetMapping());
				return true;
			}
			return false;
		}

		void Song::DestroyMachine(int mac)
		{
			Machine *iMac = _pMachine[mac];
			if(iMac)
			{
				iMac->DeleteWires();
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
							std::memcpy(toffset + l * MULTIPLY, toffset + helpers::math::round<int,float>(l * step) * MULTIPLY,EVENT_SIZE);
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
							std::memcpy(toffset + helpers::math::round<int,float>(l * step) * MULTIPLY, toffset + l * MULTIPLY,EVENT_SIZE);
							int tz(helpers::math::round<int,float>(l * step) - 1);
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
				patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
			}
			std::sprintf(patternName[pattern], name);
			return true;
		}

		void Song::AddNewTrack(int pattern, int trackIdx) 
		{
			int first, last;
			PatternEntry blank;
			if(pattern == -1)  {
				first=0;
				last=0;
				for(int i=0;i<MAX_PATTERNS;i++) {
					if(IsPatternUsed(i)) {
						last=i;
					}
				}
			}
			else {
				first=pattern;
				last=pattern;
			}
			if(SONGTRACKS < MAX_TRACKS) {
				SONGTRACKS++;
			}
			for(int pat=first; pat <=last; pat++) {
				for(int line=0;line<patternLines[pat]; line++) {
					unsigned char *offset_source= ppPatternData[pat] + (line*MULTIPLY) + (trackIdx*EVENT_SIZE);

					memmove(offset_source+EVENT_SIZE,offset_source,(SONGTRACKS-1-trackIdx)*EVENT_SIZE);
					memcpy(offset_source,&blank,EVENT_SIZE);
				}
				for(int j(SONGTRACKS-1); j > trackIdx; j--) {
					_trackNames[pat][j] = _trackNames[pat][j-1];
				}
				_trackNames[pat][trackIdx] = "";
			}
		}
		int Song::GetHighestInstrumentIndex() const
		{
			return samples.size()-1;
		}
		int Song::GetNumInstruments() const
		{
			int used=0;
			int size = samples.size();
			for( int i=0; i<size;i++) {
				if (samples.IsEnabled(i)) {
					used++;
				}
			}
			return used;
		}

		int Song::GetHighestPatternIndexInSequence() const
		{
			int rval(0);
			for(int c(0) ; c < playLength ; ++c) if(rval < playOrder[c]) rval = playOrder[c];
			return rval;
		}

		int Song::GetNumPatterns() const {
			int used=0;
			for(int i =0; i < MAX_PATTERNS; i++) {
				if(!IsPatternEmpty(i)) used++;
			}
			return used;
		}

		int Song::GetBlankPatternUnused(int rval) const
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
					unsigned char *offset_source(ppPatternData[rval]);
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

		int Song::GetFreeBus() const
		{
			for(int c(0) ; c < MAX_BUSES ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

		int Song::GetFreeFxBus() const
		{
			for(int c(MAX_BUSES) ; c < MAX_BUSES * 2 ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

		void Song::SetWavPreview(XMInstrument::WaveData<> * wave) {
			CExclusiveLock lock(&semaphore, 2, true);
			wavprev.UseWave(wave);
		}
		void Song::SetWavPreview(int newinst)
		{
			CExclusiveLock lock(&semaphore, 2, true);
			if(samples.Exists(newinst)) {
				wavprev.UseWave(&samples.get(newinst));
			}
			else {
				wavprev.UseWave(NULL);
			}
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
		unsigned char sampRateHiByte; samples per second.  (Unsigned 16-bit quantity.)
		unsigned char sampRateLoByte;
		unsigned char numOctaves; //number of octaves of waveforms in sample. (Multisample?)
		unsigned char compressMode; // data compression (0=none, 1=Fibonacci-delta encoding).
		unsigned char volumeHiByte;
		unsigned char volumeLoByte;
		unsigned char volumeHiBytePoint;
		unsigned char volumeLoBytePoint; (FIXED) = volume.  (The number 65536 means 1.0 or full volume.)

		//A Voice holds waveform data for one or more octaves. 
		//The one-shot part is played once and the repeat part is looped.
		//The sum of oneShotHiSamples and repeatHiSamples is the full length
		// of the highest octave waveform. Each following octave waveform is twice
		// as long as the previous one. 


		char body_Id[4]		// "BODY"
		ULONGINV hlength	// of the data contained in the file. It is the sample length as well (in bytes)
		char *data			// the sample.

		*/

		int Song::IffAlloc(int instrument,const char * str)
		{
			//In big endian:
			typedef struct
			{	unsigned int	oneShotHiSamples, repeatHiSamples, samplesPerHiCycle ;
				unsigned short	samplesPerSec ;
				unsigned char	octave, compression ;
				unsigned int	volume ;
			} VHDR_CHUNK ;

			RiffFile file;
			RiffChunkHeader hd;
			ULONG data;
			ULONGINV tmp;
			int bits = 0;
			// opens the file and reads the "FORM" header.
			if(!file.Open(const_cast<char*>(str)))
			{
				return 0;
			}
			XMInstrument::WaveData<> wave;
			unsigned int Datalen=0;
			file.Read(&data,4);
			if( data == file.FourCC("16SV")) bits = 16;
			else if(data == file.FourCC("8SVX")) bits = 8;
			file.Read(&hd,sizeof hd);
			if(hd._id == file.FourCC("NAME"))
			{
				char tmp[23];
				file.Read(tmp, 22); ///\todo should be hd._size instead of "22", but it is in big endian and we read in litle endian.
				tmp[22]='\0';
				wave.WaveName(tmp);
				file.Read(&hd,sizeof hd);
			}
			if ( hd._id == file.FourCC("VHDR"))
			{
				unsigned int ls, le;
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
				if(ls != le)
				{
					wave.WaveLoopStart(ls);
					wave.WaveLoopEnd(ls + le);
					wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::NORMAL);
				}
				unsigned short srbe, sr;
				file.Read(&srbe,sizeof srbe);
				sr = (srbe&0xFF)<<8 | ((srbe&0xFF00) >> 8);
				wave.WaveSampleRate(sr);
 				file.Skip(6); // Skipping octave, compression and volume
				file.Read(&hd,sizeof hd);
			}

			if(hd._id == file.FourCC("BODY"))
			{
				std::int16_t * csamples;
				wave.AllocWaveData(Datalen,false);
				csamples = const_cast<std::int16_t*>(wave.pWaveDataL());
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
			if (instrument < PREV_WAV_INS) {
				samples.SetSample(wave,instrument);
			}
			else {
				XMInstrument::WaveData<> &prevwave = wavprev.UsePreviewWave();
				prevwave = wave;
			}
			file.Close();
			return 1;
		}

		int Song::WavAlloc(int iInstr, bool bStereo, std::uint32_t iSamplesPerChan, const char * sName)
		{
			assert(iSamplesPerChan<(1<<30)); ///< FIXME: Since in some places, signed values are used, we cannot use the whole range.
			if (iInstr < PREV_WAV_INS ) {
				XMInstrument::WaveData<> wave;
				wave.AllocWaveData(iSamplesPerChan,bStereo);
				wave.WaveName(sName);
				samples.SetSample(wave,iInstr);
			}
			else {
				XMInstrument::WaveData<> &wave = wavprev.UsePreviewWave();
				wave.Init();
				wave.AllocWaveData(iSamplesPerChan,bStereo);
			}
			return true;
		}

		int Song::WavAlloc(int instrument,const char * wavfile)
		{ 
#if !defined WINAMP_PLUGIN
			assert(wavfile != 0);
			WaveFile file;
			ExtRiffChunkHeader hd;
			// opens the file and read the format Header.
			DDCRET retcode(file.OpenForRead((char*)wavfile));
			if(retcode != DDC_SUCCESS) 
			{
				return 0; 
			}
			// sample type	
			int st_type(file.NumChannels());
			int bits(file.BitsPerSample());
			std::uint32_t Datalen(file.NumSamples());
			// Initializes the layer.
			char* filename = const_cast<char*>(strrchr(wavfile,'\\'));
			if (filename == NULL) {
				filename = const_cast<char*>(strrchr(wavfile,'//'));
				if (filename == NULL) {
					filename = const_cast<char*>(wavfile);
				} else {filename++;}
			} else { filename++;}
			WavAlloc(instrument, st_type == 2, Datalen, filename);
			// Reading of Wave data.
			// We don't use the WaveFile "ReadSamples" functions, because there are two main differences:
			// We need to convert 8bits to 16bits, and stereo channels are in different arrays.
			XMInstrument::WaveData<>& wave = (instrument < PREV_WAV_INS )?samples.get(instrument):wavprev.UsePreviewWave();
			wave.WaveSampleRate(file.SamplingRate());
			std::int16_t * sampL = wave.pWaveDataL();

			///\todo use template code for all this semi-repetitive code.

			// mono
			if(st_type == 1)
			{
				UINT8 smp8;
				switch(bits)
				{
					case 8:
						for(std::uint32_t io = 0 ; io < Datalen ; ++io)
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
						for(std::uint32_t io = 0 ; io < Datalen ; ++io)
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
				std::int16_t * sampR = wave.pWaveDataR();
				UINT8 smp8;
				switch(bits)
				{
					case 8:
						for(std::uint32_t io = 0 ; io < Datalen ; ++io)
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
						for(std::uint32_t io = 0 ; io < Datalen ; ++io)
						{
							file.ReadData(sampL, 1);
							file.ReadData(sampR, 1);
							++sampL;
							++sampR;
						}
						break;
					case 24:
						for(std::uint32_t io = 0 ; io < Datalen ; ++io)
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
						std::uint32_t ls(0);
						std::uint32_t le(0);
						file.Read(static_cast<void*>(&ls), 4);
						file.Read(static_cast<void*>(&le), 4);
						if (ls >= 0 && ls <= le && le < Datalen){
							wave.WaveLoopStart(ls);
							wave.WaveLoopEnd(le);
							wave.WaveLoopType(XMInstrument::WaveData<>::LoopType::NORMAL);
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
#endif //!defined WINAMP_PLUGIN
			return 1;
		}

		bool Song::Load(RiffFile* pFile, CProgressDialog& progress, bool fullopen)
		{
			CExclusiveLock lock(&semaphore, 2, true);
			char Header[9];
			pFile->Read(&Header, 8);
			Header[8]=0;

			if (strcmp(Header,"PSY3SONG")==0)
			{
				progress.SetWindowText("Loading new format...");
				UINT version = 0;
				UINT size = 0;
				UINT index = 0;
				int temp;
				int solo(0);
				int chunkcount=0;
				Header[4]=0;
				size_t filesize = pFile->FileSize();
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if(version > CURRENT_FILE_VERSION)
				{
#if !defined WINAMP_PLUGIN
					MessageBox(0,"This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning", MB_OK | MB_ICONERROR);
#endif //!defined WINAMP_PLUGIN
				}
				pFile->Read(&chunkcount,sizeof(chunkcount));
				int bytesread = 4;
				if (size > 4) 
				{
					// This is left here if someday, extra data is added to the file version chunk.
					// update "bytesread" accordingly.

					//file->Read(chunkversion);
					//if((chunkversion&0xFF00) ) == x) {} else if(...) {}
					pFile->Skip(size - bytesread);// Size of the current Header DATA // This ensures that any extra data is skipped.
				}

				DestroyAllMachines();
				DeleteInstruments();
				DeleteAllPatterns();
				Reset(); //added by sampler mainly to reset current pattern showed.
				while(pFile->Read(&Header, 4) && chunkcount)
				{
					progress.m_Progress.SetPos(helpers::math::round<int,float>((pFile->GetPos()*16384.0f)/filesize));
					::Sleep(1); ///< Allow screen refresh.
					// we should use the size to update the index, but for now we will skip it
					if(std::strcmp(Header,"INFO") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						size_t begins = pFile->GetPos();
						if((version&0xFF00) == VERSION_MAJOR_ZERO)
						{
							char name_[129]; char author_[65]; char comments_[65536];
							pFile->ReadString(name_, sizeof name_);
							pFile->ReadString(author_, sizeof author_);
							pFile->ReadString(comments_,sizeof comments_);
							name = name_;
							author = author_;
							comments = comments_;
							//bugfix. There were songs with incorrect size.
							if(version == 0) {
								size= (UINT)(pFile->GetPos() - begins);
							}
						}
						pFile->Seek(begins + size);
					}
					else if(std::strcmp(Header,"SNGI")==0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						size_t begins = pFile->GetPos();
						if((version&0xFF00) == VERSION_MAJOR_ZERO)
						{
							// why all these temps?  to make sure if someone changes the defs of
							// any of these members, the rest of the file reads ok.  assume 
							// everything is an int, when we write we do the same thing.

							// # of tracks for whole song
							pFile->Read(&temp, sizeof temp);
							SONGTRACKS = temp;
							// bpm
							{///\todo: This was a hack added in 1.9alpha to allow decimal BPM values
								std::int16_t temp16(0);
								pFile->Read(&temp16, sizeof temp16);
								int BPMCoarse = temp16;
								pFile->Read(&temp16, sizeof temp16);
								m_BeatsPerMin = BPMCoarse + (temp16 / 100.0f);
							}
							// linesperbeat
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
							paramSelected = temp;
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
							// fix for a bug existing in the song saver in the 1.7.x series
							if(version == 0) {
								size = (11 * sizeof(std::uint32_t)) + (SONGTRACKS * 2 * sizeof(bool));
							}
							else if(version > 0) {
								pFile->Read(shareTrackNames);
								if( shareTrackNames) {
									for(int t(0); t < SONGTRACKS; t++) {
										std::string name;
										pFile->ReadString(name);
										ChangeTrackName(0,t,name);
									}
								}
							}
							if (fullopen)
							{
								///\todo: Warning! This is done here, because the plugins, when loading, need an up-to-date information.
								/// It should be coded in some way to get this information from the loading song, since doing it here
								/// is bad for the Winamp plugin (or any other multi-document situation).
								Global::player().SetBPM(BeatsPerMin(), LinesPerBeat());
							}
						}
						pFile->Seek(begins + size);
					}
					else if(std::strcmp(Header,"SEQD")==0)
					{
						--chunkcount;
						pFile->Read(&version,sizeof version);
						pFile->Read(&size,sizeof size);
						size_t begins = pFile->GetPos();
						if((version&0xFF00) == VERSION_MAJOR_ZERO)
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
						}
						pFile->Seek(begins + size);
					}
					else if(std::strcmp(Header,"PATD") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						size_t begins = pFile->GetPos();
						if((version&0xFF00) == VERSION_MAJOR_ZERO)
						{
							// index
							pFile->Read(&index, sizeof index);
							if(index < MAX_PATTERNS)
							{
								unsigned int sizez77 = 0;
								// num lines
								pFile->Read(&temp, sizeof temp );
								// clear it out if it already exists
								RemovePattern(index);
								patternLines[index] = temp;
								// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
								pFile->Read(&temp, sizeof temp );
								pFile->ReadString(patternName[index], sizeof *patternName);
								pFile->Read(&sizez77, sizeof sizez77);
								byte* pSource = new byte[sizez77];
								pFile->Read(pSource, sizez77);
								byte* pDest;
								DataCompression::BEERZ77Decomp2(pSource, &pDest);
								zapArray(pSource,pDest);
								for(int y(0) ; y < patternLines[index] ; ++y)
								{
									unsigned char* pData(_ppattern(index) + (y * MULTIPLY));
									std::memcpy(pData, pSource, SONGTRACKS * EVENT_SIZE);
									pSource += SONGTRACKS * EVENT_SIZE;
								}
								zapArray(pDest);
							}
							//\ Fix for a bug existing in the Song Saver in the 1.7.x series
							if((version == 0x0000) &&( pFile->GetPos() == begins+size+4)) size += 4;
							if(version > 0) {
								if( !shareTrackNames) {
									for(int t(0); t < SONGTRACKS; t++) {
										std::string name;
										pFile->ReadString(name);
										ChangeTrackName(index,t,name);
									}
								}
							}
						}
						pFile->Seek(begins + size);
					}
					else if(std::strcmp(Header,"MACD") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						size_t begins = pFile->GetPos();
						if((version&0xFF00) == VERSION_MAJOR_ZERO)
						{
							pFile->Read(&index, sizeof index);
							if(index < MAX_MACHINES)
							{
								// we had better load it
								_pMachine[index] = Machine::LoadFileChunk(pFile, index, version, fullopen);
								//Bugfix.
								if (fullopen) {
									if ((_pMachine[index]->_type == MACH_VST || _pMachine[index]->_type == MACH_VSTFX)
										&& dynamic_cast<vst::Plugin*>(_pMachine[index])->ProgramIsChunk() == false) {
											size = (UINT)(pFile->GetPos() - begins);
/*									} else if ((version&0xFF) == 0) {
										size = (pFile->GetPos() - begins) 
											 + sizeof(unsigned char) + 2*sizeof(int) + _pMachine[index]->GetNumParams()*sizeof(float);
*/
									}
								}
							}
						}
						pFile->Seek(begins + size);
					}
					else if(std::strcmp(Header,"INSD") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						size_t begins = pFile->GetPos();
						if((version&0xFF00) == VERSION_MAJOR_ZERO)
						{
							pFile->Read(&index, sizeof index);
							if(index < MAX_INSTRUMENTS)
							{
								_pInstrument[index]->LoadFileChunk(pFile, version, samples, index, fullopen);
							}
						}
						pFile->Seek(begins + size);
					}
					else if(std::strcmp(Header,"EINS") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						size_t begins = pFile->GetPos();
						size_t filepos=pFile->GetPos();
						//Version zero was the development version (no format freeze). Version one is the published one.
						if((version&0xFFFF0000) == XMSampler::VERSION_ONE)
						{
							// Instrument Data Load
							int numInstruments;
							pFile->Read(numInstruments);
							int idx;
							for(int i = 0;i < numInstruments && filepos < begins+size;i++)
							{
								pFile->Read(idx);
								filepos=pFile->GetPos();
								XMInstrument inst;
								int sizeIns = inst.Load(*pFile);
								xminstruments.SetInst(inst,idx);
								if ((version&0xFFFF) > 0) {
									//Version 0 doesn't write the chunk size correctly
									//so we cannot correct it in case of error
									pFile->Seek(filepos+sizeIns);
									filepos=pFile->GetPos();
								}
							}
							int numSamples;
							pFile->Read(numSamples);
							for(int i = 0;i < numSamples && filepos < begins+size;i++)
							{
								pFile->Read(idx);
								filepos=pFile->GetPos();
								XMInstrument::WaveData<> wave;
								int sizeSamp = wave.Load(*pFile);
								samples.SetSample(wave, idx);
								if ((version&0xFFFF) > 0) {
									//Version 0 doesn't write the chunk size correctly
									//so we cannot correct it in case of error
									pFile->Seek(filepos+sizeSamp);
									filepos=pFile->GetPos();
								}
							}
						}
						pFile->Seek(begins+size);
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
				progress.m_Progress.SetPos(16384);
				::Sleep(1);

				for (int i(0); i < MAX_MACHINES;++i) if ( _pMachine[i]) {
					_pMachine[i]->PostLoad(_pMachine);
				}
				// Clean memory.
				for(int i(0) ; i < MAX_MACHINES ; ++i) if(_pMachine[i])	{
					_pMachine[i]->legacyWires.clear();
				}

				// translate any data that is required
				machineSoloed = solo;
				// Safe measures for damaged files.
				if (!_pMachine[MASTER_INDEX] )
				{
					_pMachine[MASTER_INDEX] = new Master(MASTER_INDEX);
					_pMachine[MASTER_INDEX]->Init();
				}
				
				if(chunkcount) return false;
				return true;
			}
			else if(std::strcmp(Header, "PSY2SONG") == 0)
			{
//				int i;
				progress.SetWindowText("Loading old format...");
				int num,sampR;
				bool _machineActive[128];
				unsigned char busEffect[64];
				unsigned char busMachine[64];
				DoNew();
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
					Global::player().SetBPM(BeatsPerMin(), LinesPerBeat());
	//				Global::player().bpm = m_BeatsPerMin;
	//				Global::player().lpb = m_LinesPerBeat;
	//				Global::player().SamplesPerRow(sampR * Global::configuration()._pOutputDriver->_samplesPerSec / 44100);
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
						///\todo: tweak_effect should be converted to normal tweaks!
					}
					else
					{
						patternLines[i] = 64;
						RemovePattern(i);
					}
				}
				progress.m_Progress.SetPos(2048);
				::Sleep(1); ///< ???
				// Instruments
				pFile->Read(&instSelected, sizeof instSelected);
				int pans[OLD_MAX_INSTRUMENTS];
				char names[OLD_MAX_INSTRUMENTS][32];
				for(int i=0 ; i < OLD_MAX_INSTRUMENTS ; ++i)
				{
					pFile->Read(names[i], sizeof(names[0]));
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
					pFile->Read(&pans[i], sizeof(pans[0]));
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
				
				progress.m_Progress.SetPos(4096);
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
						pFile->Read(&wltemp, sizeof(wltemp));
						if (wltemp > 0)
						{
							if ( w == 0 )
							{
								XMInstrument::WaveData<> wave;
								short tmpFineTune;
								char dummy[33];
								unsigned short volume = 0;
								bool stereo =false;
								bool doloop = false;
								unsigned int loop;
								//Old format assumed 44Khz
								wave.WaveSampleRate(44100);
								wave.PanFactor(pans[i]/256.f);
								//Old wavename, not really used anyway.
								pFile->Read(dummy, 32);
								wave.WaveName(names[i]);
								pFile->Read(volume);
								wave.WaveGlobVolume(volume*0.01f);
								pFile->Read(&tmpFineTune, sizeof(short));
								//Current sample uses 100 cents. Older used +-256
								tmpFineTune = static_cast<int>((float)tmpFineTune/2.56f);
								wave.WaveFineTune(tmpFineTune);
								pFile->Read(loop);
								wave.WaveLoopStart(loop);
								pFile->Read(loop);
								wave.WaveLoopEnd(loop);
								pFile->Read(doloop);
								wave.WaveLoopType(doloop?XMInstrument::WaveData<>::LoopType::NORMAL:XMInstrument::WaveData<>::LoopType::DO_NOT);
								pFile->Read(stereo);
								wave.AllocWaveData(wltemp,stereo);
								std::int16_t* data = wave.pWaveDataL();
								pFile->Read(data, wltemp*sizeof(short));
								if (stereo)
								{
									data = wave.pWaveDataR();
									pFile->Read(data, wltemp*sizeof(short));
								}
								samples.SetSample(wave, i);
							}
							else 
							{
								bool stereo;
								pFile->Skip(42+sizeof(bool));
								pFile->Read(&stereo,sizeof(bool));
								pFile->Skip(wltemp);
								if(stereo)pFile->Skip(wltemp);
							}
						}
					}
				}
				
				progress.m_Progress.SetPos(4096+2048);
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
						pFile->Read(&(vstL[i].numpars), sizeof(std::int32_t));
						vstL[i].pars = new float[vstL[i].numpars];

						for (int c=0; c<vstL[i].numpars; c++)
						{
							pFile->Read(&(vstL[i].pars[c]), sizeof(float));
						}
					}
				}
				
				progress.m_Progress.SetPos(8192);
				::Sleep(1);
				// Machines
				//
				pFile->Read(&_machineActive[0], sizeof(_machineActive));
				Machine* pMac[128];
				memset(pMac,0,sizeof(pMac));

				convert_internal_machines::Converter converter;

				for (int i=0; i<128; i++)
				{
					Sampler* pSampler;
					XMSampler* pXMSampler;
					Plugin* pPlugin;
					vst::Plugin * pVstPlugin(0);
					int x,y,type;
					if (_machineActive[i])
					{
						progress.m_Progress.SetPos(8192+i*(4096/128));
						::Sleep(1);

						pFile->Read(&x, sizeof(x));
						pFile->Read(&y, sizeof(y));

						pFile->Read(&type, sizeof(type));

						std::pair<int, std::string> bla(type,"");
						if(converter.plugin_names().exists(bla))
							pMac[i] = &converter.redirect(i, bla, *pFile);
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
							pSampler->DefaultC4(false);
							break;
						case MACH_XMSAMPLER:
							pMac[i] = pXMSampler = new XMSampler(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						case MACH_PLUGIN:
						{
							char sDllName[256];
							pFile->Read(sDllName, sizeof(sDllName)); // Plugin dll name
							_strlwr(sDllName);

							std::pair<int, std::string> bla2(type,sDllName);
							if(converter.plugin_names().exists(bla2))
								pMac[i] = &converter.redirect(i, bla2, *pFile);
							else {
								pMac[i] = pPlugin = new Plugin(i);
								if (pPlugin->LoadDll(sDllName)) {
									pMac[i]->Init();
									pMac[i]->Load(pFile);
								}
								else {
									pPlugin->SkipLoad(pFile);
									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(pOldMachine);
									pMac[i]->_macIndex=i;
									pMac[i]->Init();
									zapObject(pOldMachine);
									// Warning: It cannot be known if the missing plugin is a generator
									// or an effect. This will be guessed from the busMachine array.
								}
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
								vst::Plugin* pTempMac = new vst::Plugin(0);
								unsigned char program;
								int instance;
								// The trick: We need to load the information from the file in order to know the "instance" number
								// and be able to create a plugin from the corresponding dll. Later, we will set the loaded settings to
								// the newly created plugin.
								pTempMac->PreLoad(pFile,program,instance);
								assert(instance < OLD_MAX_PLUGINS);
								std::int32_t shellIdx=0;
								if((!vstL[instance].valid) || (!Global::machineload().lookupDllName(vstL[instance].dllName,temp,MACH_VST,shellIdx)))
								{
									berror=true;
									sprintf(sError,"VST plug-in missing, or erroneous data in song file \"%s\"",vstL[instance].dllName);
								}
								else
								{
									strcpy(sPath,temp.c_str());
									if (!Global::machineload().TestFilename(sPath,shellIdx))
									{
										berror=true;
										sprintf(sError,"This VST plug-in is Disabled \"%s\" - replacing with Dummy.",sPath);
									}
									else
									{
										try
										{
											pMac[i] = pVstPlugin = dynamic_cast<vst::Plugin*>(Global::vsthost().LoadPlugin(sPath,shellIdx));

											if (pVstPlugin)
											{
												pVstPlugin->LoadFromMac(pTempMac);
												pVstPlugin->SetProgram(program);
												pVstPlugin->_macIndex=i;
												const int numpars = vstL[instance].numpars;
												for (int c(0) ; c < numpars; ++c)
												{
													pVstPlugin->SetParameter(c, vstL[instance].pars[c]);
												}
											}
										}
										catch(...)
										{
											berror=true;
											sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath);
											if (pVstPlugin) { delete pVstPlugin; }
										}
									}
								}
								if (berror)
								{
#if !defined WINAMP_PLUGIN
									MessageBox(NULL,sError, "Loading Error", MB_OK);
#endif // !defined WINAMP_PLUGIN
									pMac[i] = new Dummy(pTempMac);
									pMac[i]->_macIndex=i;
									zapObject(pTempMac);
									if (type == MACH_VSTFX ) pMac[i]->_mode = MACHMODE_FX;
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
#if !defined WINAMP_PLUGIN
							{
								char buf[MAX_PATH];
								sprintf(buf,"unknown machine type: %i",type);
								MessageBox(0, buf, "Loading old song", MB_ICONERROR);
							}
#endif // !defined WINAMP_PLUGIN
							pMac[i] = new Dummy(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
						}

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

				// Validate the machine arrays. 
				for (int i=0; i<128; i++) // First, we add the output volumes to a Matrix for reference later
				{
					if (!_machineActive[i])
					{
						zapObject(pMac[i]);
					}
					else if (!pMac[i])
					{
						_machineActive[i] = false;
					}
				}

				// Patch 1: BusEffects (twf). Try to read it, and if it doesn't exist, generate it.
				progress.m_Progress.SetPos(8192+4096);
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
				progress.m_Progress.SetPos(8192+4096+1024);
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
								vst::Plugin & plugin(*static_cast<vst::Plugin*>(pMac[i]));
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
				// so what we have in inputConVol is really outputConVol
				// and we have to convert while recreating them.
				progress.m_Progress.SetPos(8192+4096+2048);
				::Sleep(1);
				for (int i=0; i<128; i++) // we go to fix this for each
				{
					if (_machineActive[i])		// valid machine (important, since we have to navigate!)
					{
						for (int c=0; c<MAX_CONNECTIONS; c++) // all for each of its input connections.
						{
							LegacyWire& wire = pMac[i]->legacyWires[c];
							if (wire._inputCon && wire._inputMachine > -1 && wire._inputMachine < 128
								&& pMac[wire._inputMachine])	// If there's a valid machine in this inputconnection,
							{
								Machine* pSourceMac = pMac[wire._inputMachine];
								int d = Machine::FindLegacyOutput(pSourceMac, i); // We get that machine and wire
								if ( d != -1 )
								{
									float val = pSourceMac->legacyWires[d]._inputConVol;
									if( val > 4.1f )
									{
										val*=0.000030517578125f; // BugFix
									}
									else if ( val < 0.00004f) 
									{
										val*=32768.0f; // BugFix
									}
									// and set the volume.
									if (wire.pinMapping.size() > 0) {
										pMac[i]->inWires[c].ConnectSource(*pSourceMac,0,d,&wire.pinMapping);
									}
									else {
										pMac[i]->inWires[c].ConnectSource(*pSourceMac,0,d);
									}
									pMac[i]->inWires[c].SetVolume(val*wire._wireMultiplier);
								}
							}
						}
					}
				}
				
				// Psycle no longer uses busMachine and busEffect, since the pMachine Array directly maps
				// to the real machine.
				// Due to this, we have to move machines to where they really are, 
				// and remap the inputs and outputs indexes again... ouch
				// At the same time, we validate each wire.
				progress.m_Progress.SetPos(8192+4096+2048+1024);
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
						Machine *cMac = _pMachine[invmach[i]] = pMac[i];
						cMac->_macIndex = invmach[i];
						_machineActive[i] = false; // mark as "converted"
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

				progress.m_Progress.SetPos(16384);
				::Sleep(1);
				if(fullopen) converter.retweak(*this);
				for (int i(0); i < MAX_MACHINES;++i) if ( _pMachine[i]) _pMachine[i]->PostLoad(_pMachine);
				seqBus=0;
				// Clean memory.
				for(int i(0) ; i < MAX_MACHINES ; ++i) if(_pMachine[i])	{
					_pMachine[i]->legacyWires.clear();
				}
				// Clean the vst loader helpers.
				for (int i=0; i<OLD_MAX_PLUGINS; i++)
				{
					if( vstL[i].valid )
					{
						zapObject(vstL[i].pars);
					}
				}
				return true;
			}

			// load did not work
			MessageBox(NULL,"Incorrect file format","Error",MB_OK);
			return false;
		}

		bool Song::Save(RiffFile* pFile,CProgressDialog& progress,bool autosave)
		{
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			// header, this has to be at the top of the file

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
				if (samples.IsEnabled(i))
				{
					chunkcount++;
				}
			}
			// Instrument Data Save
			int numInstruments = 0;	
			for(int i = 0;i < XMInstrument::MAX_INSTRUMENT;i++){
				if(xminstruments.IsEnabled(i)){
					numInstruments++;
				}
			}
			if (numInstruments >0)
			{
				chunkcount++;
			}

			if ( !autosave ) 
			{
				progress.m_Progress.SetRange(0,chunkcount);
				progress.m_Progress.SetStep(1);
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
				progress.m_Progress.StepIt();
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
			size = (UINT)(name.length() + author.length() + comments.length() + 3); // +3 for \0

			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));

			pFile->Write(name.c_str(),name.length()+1);
			pFile->Write(author.c_str(),author.length()+1);
			pFile->Write(comments.c_str(),comments.length()+1);

			if ( !autosave ) 
			{
				progress.m_Progress.StepIt();
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
			size = (11*sizeof(temp)) + (SONGTRACKS*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])))
				+ sizeof(bool); 
			if( shareTrackNames) {
				for(int t(0); t < SONGTRACKS; t++) {
					size+=_trackNames[0][t].length()+1; // +1 because of the \0 terminator.
				}
			}
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

			temp = paramSelected;
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

			pFile->Write(shareTrackNames);
			if( shareTrackNames) {
				for(int t(0); t < SONGTRACKS; t++) {
					pFile->WriteString(_trackNames[0][t]);
				}
			}


			if ( !autosave ) 
			{
				progress.m_Progress.StepIt();
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
				size = (UINT)(((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1);
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
				progress.m_Progress.StepIt();
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
					
					int sizez77 = DataCompression::BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[i]*EVENT_SIZE);
					zapArray(pSource);

					pFile->Write("PATD",4);
					version = CURRENT_FILE_VERSION_PATD;

					pFile->Write(&version,sizeof(version));
					size = (UINT)(sizez77+(4*sizeof(int))+strlen(patternName[i])+1);
					if( !shareTrackNames) {
						for(int t(0); t < SONGTRACKS; t++) {
							size+=_trackNames[index][t].length()+1; // +1 because of the \0 terminator.
						}
					}
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
					
					if( !shareTrackNames) {
						for(int t(0); t < SONGTRACKS; t++) {
							pFile->WriteString(_trackNames[index][t]);
						}
					}
					if ( !autosave ) 
					{
						progress.m_Progress.StepIt();
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
					size_t pos = pFile->GetPos();
					size = 0;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));

					_pMachine[i]->SaveFileChunk(pFile);

					size_t pos2 = pFile->GetPos(); 
					size = (UINT)(pos2-pos-sizeof(size));
					pFile->Seek(pos);
					pFile->Write(&size,sizeof(size));
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						progress.m_Progress.StepIt();
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
			///When not saving extended instruments (EINS), allow the samples to be saved in INSD. Else, they will be saved in EINS.
			bool saveEINS = (numInstruments > 0);
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (samples.IsEnabled(i))
				{
					pFile->Write("INSD",4);
					version = CURRENT_FILE_VERSION_INSD;
					pFile->Write(&version,sizeof(version));
					size_t pos = pFile->GetPos();
					size = 0;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));
					_pInstrument[i]->SaveFileChunk(pFile, samples, i, !saveEINS);

					size_t pos2 = pFile->GetPos(); 
					size = (UINT)(pos2-pos-sizeof(size));
					pFile->Seek(pos);
					pFile->Write(&size,sizeof(size));
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						progress.m_Progress.StepIt();
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
			if (saveEINS)
			{
				pFile->Write("EINS",4);
				version = XMSampler::VERSION;
				pFile->Write(&version,sizeof(version));
				size_t pos = pFile->GetPos();
				size = 0;
				pFile->Write(&size,sizeof(size));

				pFile->Write(numInstruments);

				for(int i = 0;i < XMInstrument::MAX_INSTRUMENT;i++){
					if(xminstruments.IsEnabled(i)){
						pFile->Write(i);
						xminstruments[i].Save(*pFile);
					}
				}

				// Sample Data Save
				int numSamples = 0;
				for(int i = 0;i < XMInstrument::MAX_INSTRUMENT;i++){
					if(samples.IsEnabled(i)){
						numSamples++;
					}
				}

				pFile->Write(numSamples);

				for(int i = 0;i < XMInstrument::MAX_INSTRUMENT;i++){
					if(samples.IsEnabled(i)){
						pFile->Write(i);
						samples[i].Save(*pFile);
					}
				}
				size_t pos2 = pFile->GetPos(); 
				size = (UINT)(pos2-pos-sizeof(size));
				pFile->Seek(pos);
				pFile->Write(&size,sizeof(size));
				pFile->Seek(pos2);
			}


			if ( !autosave ) 
			{
				progress.m_Progress.SetPos(chunkcount);
				::Sleep(1);
			}
			return true;
		}
		void Song::DoPreviews(int amount)
		{
#if !defined WINAMP_PLUGIN
			//todo do better.. use a vector<InstPreview*> or something instead
			float* pL(((Master*)_pMachine[MASTER_INDEX])->getLeft());
			float* pR(((Master*)_pMachine[MASTER_INDEX])->getRight());
			if(wavprev.IsEnabled())
			{
				wavprev.Work(pL, pR, amount);
			}
#endif // !defined WINAMP_PLUGIN
		}
		void Song::DeleteMachineRewiring(int macIdx)
		{
			Machine* pMac = _pMachine[macIdx];
			// move wires before deleting. Don't do inside DeleteMachine since that is used in song internally
			if( pMac->connectedInputs() > 0 && pMac->connectedOutputs() > 0) {
				//For each input connection
				for(int i = 0; i < MAX_CONNECTIONS; i++) if(pMac->inWires[i].Enabled()) {
					Machine& srcMac = pMac->inWires[i].GetSrcMachine();
					int wiresrc = pMac->inWires[i].GetSrcWireIndex();
					bool first = true;
					//Connect it to each output connection
					for(int i = 0; i < MAX_CONNECTIONS; i++) if(pMac->outWires[i] && pMac->outWires[i]->Enabled()) {
						Machine& dstMac = pMac->outWires[i]->GetDstMachine();
						//Except if already connected
						if( dstMac.FindInputWire(srcMac._macIndex) == -1) {
							int wiredst = pMac->outWires[i]->GetDstWireIndex();
							//If first wire change, it can be moved. Else it needs a new connection.
							if(first) {
								ChangeWireDestMacNonBlocking(&srcMac,&dstMac,wiresrc,wiredst);
								first = false;
							}
							else {
								float vol = pMac->outWires[i]->GetVolume();
								int dwire = InsertConnectionNonBlocking(&srcMac, &dstMac);
								dstMac.inWires[dwire].SetVolume(vol);
							}
						}
					}
				}
			}
			DestroyMachine(macIdx);
		}
		bool Song::CloneMac(int src,int dst)
		{
#if !defined WINAMP_PLUGIN
			CExclusiveLock lock(&semaphore, 2, true);
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
			PsycleGlobal::inputHandler().AddMacViewUndo();
			///\todo: Wrong song dir causes "machine cloning failed"! 
			///\todo: the process should be changed and save the data in memory.
			CString filepath = PsycleGlobal::conf().GetAbsoluteSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			OldPsyFile file;
			if (!file.Create(static_cast<LPCTSTR>(filepath), true))
			{
				return false;
			}

			file.Write("MACD",4);
			UINT version = CURRENT_FILE_VERSION_MACD;
			file.Write(&version,sizeof(version));
			size_t pos = file.GetPos();
			UINT size = 0;
			file.Write(&size,sizeof(size));

			int index = dst; // index
			file.Write(&index,sizeof(index));

			_pMachine[src]->SaveFileChunk(&file);

			size_t pos2 = file.GetPos(); 
			size = (UINT)(pos2-pos-sizeof(size));
			file.Seek(pos);
			file.Write(&size,sizeof(size));
			file.Close();

			// now load it

			if (!file.Open(static_cast<LPCTSTR>(filepath)))
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

			SMachineCoords mcoords = PsycleGlobal::conf().macView().MachineCoords;

			int xs,ys;
			if (src >= MAX_BUSES)
			{
				xs = mcoords.sEffect.width;
				ys = mcoords.sEffect.height;
			}
			else 
			{
				xs = mcoords.sGenerator.width;
				ys = mcoords.sGenerator.height;
			}

			_pMachine[dst]->_x = _pMachine[dst]->_x+32;
			_pMachine[dst]->_y = _pMachine[dst]->_y+ys+8;

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
			strncpy(_pMachine[dst]->_editName,buf,sizeof(_pMachine[dst]->_editName)-1);
#endif //!defined WINAMP_PLUGIN

			return true;
		}


		bool Song::CloneIns(int src,int dst)
		{
#if !defined WINAMP_PLUGIN

			CExclusiveLock lock(&semaphore, 2, true);
			// src has to be occupied and dst must be empty
			if (samples.IsEnabled(src) && samples.IsEnabled(dst))
			{
				return false;
			}
			// also allow dst occupid and src empty. in that case, do it backwards
			if (samples.IsEnabled(dst))
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			//If this is true, both are empty
			if (!samples.IsEnabled(src))
			{
				return false;
			}
			// ok now we get down to business
			PsycleGlobal::inputHandler().AddMacViewUndo();

			// save our file

			CString filepath = PsycleGlobal::conf().GetAbsoluteSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			OldPsyFile file;
			if (!file.Create(static_cast<LPCTSTR>(filepath), true))
			{
				return false;
			}

			file.Write("INSD",4);
			UINT version = CURRENT_FILE_VERSION_INSD;
			file.Write(&version,sizeof(version));
			size_t pos = file.GetPos();
			UINT size = 0;
			file.Write(&size,sizeof(size));

			int index = dst; // index
			file.Write(&index,sizeof(index));

			_pInstrument[src]->SaveFileChunk(&file, samples, src, true);

			size_t pos2 = file.GetPos(); 
			size = (UINT)(pos2-pos-sizeof(size));
			file.Seek(pos);
			file.Write(&size,sizeof(size));

			file.Close();

			// now load it

			if (!file.Open(static_cast<LPCTSTR>(filepath)))
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
						_pInstrument[index]->LoadFileChunk(&file,version, samples, index);
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
#endif //!defined WINAMP_PLUGIN
			return true;
		}

		bool Song::IsPatternUsed(int i) const
		{
			bool bUsed = false;
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
					bUsed = !IsPatternEmpty(i);
				}
			}
			return bUsed;
		}

		bool Song::IsPatternEmpty(int i) const {
			if (!ppPatternData[i]) {
				return true;
			}
			PatternEntry blank;
			unsigned char * pData = ppPatternData[i];
			for (int j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
			{
				if (memcmp(pData+j,&blank,EVENT_SIZE) != 0 )
				{
					return false;
				}
			}
			return true;
		}
	}
}
