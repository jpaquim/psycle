#pragma once
#include "Machine.h"
#include "Song.h"
#include "Configuration.h"
#include "../plugin/MachineInterface.h"
#include "Player.h"
#if !defined(_WINAMP_PLUGIN_)
	#include "NewMachine.h"
#endif
///\file
///\brief interface file for psycle::host::Plugin
namespace psycle
{
	namespace host
	{
		class PluginFxCallback : public CFxCallback
		{
		public:
			HWND hWnd;
			inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { MessageBox(hWnd,ptxt,caption,type); }
			inline virtual int GetTickLength() { return Global::_pSong->SamplesPerTick; }
			inline virtual int GetSamplingRate() { return Global::pConfig->GetSamplesPerSec(); }
			inline virtual int GetBPM() { return Global::pPlayer->bpm; }
			inline virtual int GetTPB() { return Global::pPlayer->tpb; }
		};

		class Plugin; // forward declaration

		/// Proxy between the host and a plugin.
		class proxy
		{
		private:
			Plugin & host_;
			CMachineInterface * plugin_;
		private:
			inline Plugin & host() throw();
			inline const Plugin & host() const throw();
			inline CMachineInterface & plugin() throw();
			inline const CMachineInterface & plugin() const throw();
		public:
			inline proxy(Plugin & host, CMachineInterface * plugin = 0);
			inline ~proxy() throw();
			inline const bool operator()() const throw();
			inline void operator()(CMachineInterface * plugin) throw(exceptions::function_error);
			inline void Init() throw(exceptions::function_error);
			inline void SequencerTick() throw(exceptions::function_error);
			inline void ParameterTweak(int par, int val) throw(exceptions::function_error);
			inline void Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(exceptions::function_error);
			inline void Stop() throw(exceptions::function_error);
			inline void PutData(byte * pData) throw(exceptions::function_error);
			inline void GetData(byte * pData) throw(exceptions::function_error);
			inline int GetDataSize() throw(exceptions::function_error);
			inline void Command() throw(exceptions::function_error);
			inline void MuteTrack(const int i) throw(exceptions::function_error);
			inline bool IsTrackMuted(const int i) throw(exceptions::function_error);
			inline void MidiNote(const int channel, const int value, const int velocity) throw(exceptions::function_error);
			inline void Event(const dword data) throw(exceptions::function_error);
			inline bool DescribeValue(char * txt, const int param, const int value) throw(exceptions::function_error);
			inline bool PlayWave(const int wave, const int note, const float volume) throw(exceptions::function_error);
			inline void SeqTick(int channel, int note, int ins, int cmd, int val) throw(exceptions::function_error);
			inline void StopWave() throw(exceptions::function_error);
			inline int * Vals() throw(exceptions::function_error);
			inline void callback() throw(exceptions::function_error);
		};

		class Plugin : public Machine
		{
		private:
			static PluginFxCallback _callback;
		public:
			inline static PluginFxCallback * GetCallback() throw() { return &_callback; };
		public:
			Plugin(int index);
			virtual ~Plugin() throw();
			virtual void Init();
			virtual void Work(int numSamples);
			virtual void Tick();
			virtual void Tick(int channel, PatternEntry * pEntry);
			virtual void Stop();
			inline virtual const char * const GetDllName() const throw() { return _psDllName; }
			virtual char * GetName() { return _psName; };
			virtual int GetNumParams() { return GetInfo()->numParameters; }
			virtual void GetParamName(int numparam, char * name);
			virtual int GetParamValue(int numparam);
			virtual void GetParamValue(int numparam,char* parval);
			virtual bool SetParameter(int numparam,int value);
			virtual bool Load(RiffFile * pFile);
			virtual bool LoadSpecificFileChunk(RiffFile * pFile, int version);
			#if !defined _WINAMP_PLUGIN_
				virtual void SaveSpecificChunk(RiffFile * pFile);
				virtual void SaveDllName(RiffFile * pFile);
			#endif

			void Instance(const char file_name[]) throw(...);
			void Free() throw(...);
			bool LoadDll(char* psFileName);
			inline char * GetShortName() throw() { return _psShortName; }
			inline char * GetAuthor() throw() { return _psAuthor; }
			inline const bool & IsSynth() const throw() { return _isSynth; }
			inline CMachineInfo * GetInfo() throw() { return _pInfo; };
			inline proxy & proxy() throw() { return proxy_; };
		private:
			HINSTANCE _dll;
			char _psShortName[16];
			char* _psAuthor;
			char* _psDllName;
			char* _psName;
			bool _isSynth;
			CMachineInfo * _pInfo;
			host::proxy proxy_;
		};

		inline Plugin & proxy::host() throw() { return host_; }
		inline const Plugin & proxy::host() const throw() { return host_; }
		inline CMachineInterface & proxy::plugin() throw() { return *plugin_; }
		inline const CMachineInterface & proxy::plugin() const throw() { return *plugin_; }
		inline proxy::proxy(Plugin & host, CMachineInterface * plugin) : host_(host), plugin_(0) { (*this)(plugin); }
		inline proxy::~proxy() throw() { (*this)(0); }
		inline const bool proxy::operator()() const throw() { return plugin_; }
		inline void proxy::operator()(CMachineInterface * plugin) throw(exceptions::function_error)
		{
			delete this->plugin_;
			this->plugin_ = plugin;
			//if((*this)())
			if(plugin)
			{
				callback();
				//Init();
			}
		}
		#pragma warning(push)
		#pragma warning(disable:4702) // unreachable code
		inline void proxy::Init() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "Init";
			try
			{
				plugin().Init();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::SequencerTick() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "SequencerTick";
			try
			{
				plugin().SequencerTick();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::ParameterTweak(int par, int val) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "ParameterTweak";
			try
			{
				plugin().ParameterTweak(par, val);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "Work";
			try
			{
				plugin().Work(psamplesleft, psamplesright, numsamples, tracks);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::Stop() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "Stop";
			try
			{
				plugin().Stop();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::PutData(byte * pData) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "PutData";
			try
			{
				plugin().PutData(pData);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::GetData(byte * pData) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "GetData";
			try
			{
				plugin().GetData(pData);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline int proxy::GetDataSize() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "GetDataSize";
			try
			{
				return plugin().GetDataSize();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
			return 0; // dummy return to avoid warning
		}
		inline void proxy::Command() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "Command";
			try
			{
				plugin().Command();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::MuteTrack(const int i) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "MuteTrack";
			try
			{
				plugin().MuteTrack(i);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline bool proxy::IsTrackMuted(const int i) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "IsTrackMuted";
			try
			{
				return const_cast<const CMachineInterface &>(plugin()).IsTrackMuted(i);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
			return false; // dummy return to avoid warning
		}
		inline void proxy::MidiNote(const int channel, const int value, const int velocity) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "MidiNote";
			try
			{
				plugin().MidiNote(channel, value, velocity);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::Event(const dword data) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "Event";
			try
			{
				plugin().Event(data);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline bool proxy::DescribeValue(char * txt, const int param, const int value) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "DescribeValue";
			try
			{
				return plugin().DescribeValue(txt, param, value);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
			return false; // dummy return to avoid warning
		}
		inline bool proxy::PlayWave(const int wave, const int note, const float volume) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "PlayWave";
			try
			{
				plugin().PlayWave(wave, note, volume);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
			return false; // dummy return to avoid warning
		}
		inline void proxy::SeqTick(int channel, int note, int ins, int cmd, int val) throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "SeqTick";
			try
			{
				plugin().SeqTick(channel, note, ins, cmd, val);
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline void proxy::StopWave() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "StopWave";
			try
			{
				plugin().StopWave();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		inline int * proxy::Vals() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "Vals";
			try
			{
				return plugin().Vals;
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
			return 0; // dummy return to avoid warning
		}
		inline void proxy::callback() throw(exceptions::function_error)
		{
			assert((*this)());
			static const char function[] = "callback";
			try
			{
				plugin().pCB = host().GetCallback();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const char e[]) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(host(), function, &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(host(), function); }
		}
		#pragma warning(pop)
	}
}
