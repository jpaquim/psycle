///\file
///\brief interface file for psycle::host::Plugin
#pragma once
#include "Machine.hpp"
#include "../plugin_interface.hpp"
//#include "Song.hpp"
#include "Configuration.hpp"
#include "Player.hpp"
//#include "NewMachine.hpp"
namespace psycle
{
	namespace host
	{
		class PluginFxCallback : public CFxCallback
		{
		public:
			HWND hWnd;
			inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { MessageBox(hWnd,ptxt,caption,type); }
			inline virtual int GetTickLength() { return Global::pPlayer->SamplesPerRow(); }
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
			inline virtual const char * const GetDllName() const throw() { return _psDllName.c_str(); }
			virtual char * GetName() { return (char *)_psName.c_str(); };
			virtual int GetNumParams() { return GetInfo()->numParameters; }
			virtual int  GetNumCols   () { return GetInfo()->numCols; };
			virtual void GetParamName(int numparam, char * name);
			virtual void GetParamRange(int numparam,int &minval,int &maxval);
			virtual int GetParamValue(int numparam);
			virtual void GetParamValue(int numparam,char* parval);
			virtual bool SetParameter(int numparam,int value);
			virtual bool Load(RiffFile * pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

			void Instance(std::string file_name) throw(...);
			void Free() throw(...);
			bool LoadDll(std::string psFileName);
			inline std::string GetShortName() throw() { return _psShortName; }
			inline std::string GetAuthor() throw() { return _psAuthor; }
			inline const bool & IsSynth() const throw() { return _isSynth; }
			inline CMachineInfo * GetInfo() throw() { return _pInfo; };
			inline proxy & proxy() throw() { return proxy_; };
		private:
			HINSTANCE _dll;
			char _psShortName[16];
			std::string _psAuthor;
			std::string _psDllName;
			std::string _psName;
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
			zapObject(this->plugin_,plugin);
			//if((*this)())
			if(plugin)
			{
				callback();
				//Init(); // [bohan] i can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
			}
		}

		#if defined $catch$
			#error "macro clash"
		#endif
		#define $catch$(function) \
			catch(        std::exception const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(                  char const e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(  signed long long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(  signed      long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(  signed           int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(  signed     short int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(  signed          char const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(unsigned long long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(unsigned      long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(unsigned           int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(unsigned     short int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(unsigned          char const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(            void const * const e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
			catch(               ...              ) { host::exceptions::function_errors::rethrow<void*>(host(), function); }

		#pragma warning(push)
		#pragma warning(disable:4702) // unreachable code
		inline void proxy::Init() throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().Init(); } $catch$("Init") }
		inline void proxy::SequencerTick() throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().SequencerTick(); } $catch$("SequencerTick") }
		inline void proxy::ParameterTweak(int par, int val) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().ParameterTweak(par, val); } $catch$("ParameterTweak") }
		inline void proxy::Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().Work(psamplesleft, psamplesright, numsamples, tracks); } $catch$("Work") }
		inline void proxy::Stop() throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().Stop(); } $catch$("Stop") }
		inline void proxy::PutData(byte * pData) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().PutData(pData); } $catch$("PutData") }
		inline void proxy::GetData(byte * pData) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().GetData(pData); } $catch$("GetData") }
		inline int proxy::GetDataSize() throw(exceptions::function_error)
		{ assert((*this)()); try { return plugin().GetDataSize(); } $catch$("GetDataSize") return 0; /* dummy return to avoid warning */ }
		inline void proxy::Command() throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().Command(); } $catch$("Command") }
		inline void proxy::MuteTrack(const int i) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().MuteTrack(i); } $catch$("MuteTrack") }
		inline bool proxy::IsTrackMuted(const int i) throw(exceptions::function_error)
		{ assert((*this)()); try { return const_cast<const CMachineInterface &>(plugin()).IsTrackMuted(i); } $catch$("IsTrackMuted") return false; /* dummy return to avoid warning */ }
		inline void proxy::MidiNote(const int channel, const int value, const int velocity) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().MidiNote(channel, value, velocity); } $catch$("MidiNote") }
		inline void proxy::Event(const dword data) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().Event(data); } $catch$("Event") }
		inline bool proxy::DescribeValue(char * txt, const int param, const int value) throw(exceptions::function_error)
		{ assert((*this)()); try { return plugin().DescribeValue(txt, param, value); } $catch$("DescribeValue") return false; /* dummy return to avoid warning */ }
		inline bool proxy::PlayWave(const int wave, const int note, const float volume) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().PlayWave(wave, note, volume); } $catch$("PlayWave") return false; /* dummy return to avoid warning */ }
		inline void proxy::SeqTick(int channel, int note, int ins, int cmd, int val) throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().SeqTick(channel, note, ins, cmd, val); } $catch$("SeqTick") }
		inline void proxy::StopWave() throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().StopWave(); } $catch$("StopWave") }
		inline int * proxy::Vals() throw(exceptions::function_error)
		{ assert((*this)()); try { return plugin().Vals; } $catch$("Vals") return 0; /* dummy return to avoid warning */ }
		inline void proxy::callback() throw(exceptions::function_error)
		{ assert((*this)()); try { plugin().pCB = host().GetCallback(); } $catch$("callback") }
		#pragma warning(pop)

		#undef $catch$
	}
}
