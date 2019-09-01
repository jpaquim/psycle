///\file
///\brief interface file for psycle::host::Plugin
#pragma once
#include "Machine.hpp"
#include <psycle/plugin_interface.hpp>
#include "Configuration.hpp"
#include "Player.hpp"
#include "global.hpp"
#include <diversalis/compiler.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		/// \todo CPresetsDlg code sux big time concerning interface separation :-(
		class CPresetDlg;

		/// calls that the plugin side can make to the host side
		///\todo PLEASE EXTEND THIS!!!
		class PluginFxCallback : public CFxCallback
		{
			public:
				inline virtual int GetTickLength() { return Global::pPlayer->SamplesPerRow(); }
				inline virtual int GetSamplingRate() { return Global::pConfig->GetSamplesPerSec(); }
				inline virtual int GetBPM() { return Global::pPlayer->bpm; }
				inline virtual int GetTPB() { return Global::pPlayer->tpb; }

			public:
				///\todo mswindows! humpf! mswindows!
				inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { MessageBox(hWnd,ptxt,caption,type); }
			public: ///\todo private:
				///\todo mswindows! humpf! mswindows!
				HWND hWnd;
		};

		class Plugin; // forward declaration

		/// Proxy between the host and a plugin.
		class proxy
		{
			public:
				proxy(Plugin & host, CMachineInterface * plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }
				~proxy() throw() { (*this)(0); }

			///\name reference to the host side
			///\{
				private:
					Plugin const & host() const throw() { return host_; }
					Plugin       & host()       throw() { return host_; }
				private:
					Plugin       & host_;

			///\}

			///\name reference to the plugin side
			///\{
				private:
					CMachineInterface const & plugin() const throw() { return *plugin_; }
					CMachineInterface       & plugin()       throw() { return *plugin_; }
				private:
					CMachineInterface       * plugin_;

				public:
					bool const operator()() const throw() { return plugin_; }
					inline void operator()(CMachineInterface * plugin) throw(exceptions::function_error);
			///\}

			///\name protected calls from the host side to the plugin side
			///\{
				public:
					void inline callback()                                                                     throw(exceptions::function_error);
					void inline Init()                                                                         throw(exceptions::function_error);
					void inline SequencerTick()                                                                throw(exceptions::function_error);
					void inline ParameterTweak(int par, int val)                                               throw(exceptions::function_error);
					void inline Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(exceptions::function_error);
					void inline Stop()                                                                         throw(exceptions::function_error);
					void inline PutData(void * pData)                                                          throw(exceptions::function_error);
					void inline GetData(void * pData)                                                          throw(exceptions::function_error);
					int  inline GetDataSize()                                                                  throw(exceptions::function_error);
					void inline Command()                                                                      throw(exceptions::function_error);
					void inline MuteTrack(const int i)                                                         throw(exceptions::function_error);
					bool inline IsTrackMuted(const int i)                                                      throw(exceptions::function_error);
					void inline MidiNote(const int channel, const int value, const int velocity)               throw(exceptions::function_error);
					void inline Event(std::uint32_t const data)                                                throw(exceptions::function_error);
					bool inline DescribeValue(char * txt, const int param, const int value)                    throw(exceptions::function_error);
					bool inline PlayWave(const int wave, const int note, const float volume)                   throw(exceptions::function_error);
					void inline SeqTick(int channel, int note, int ins, int cmd, int val)                      throw(exceptions::function_error);
					void inline StopWave()                                                                     throw(exceptions::function_error);
					int  inline Val(int parameter)                                                             throw(exceptions::function_error);

					///\todo CPresetsDlg code sux big time concerning interface separation :-(
					///\todo deprecated: use int Val(int) instead
					friend class CPresetDlg; int const /* at least it's const! */ * Vals() { return plugin().Vals; }
			///\}
		};

		/// the host side
		class Plugin : public Machine
		{
			///\name the complicated quadristepped (!) construction
			///\{
				public:
					///\todo doc
					Plugin(int index);
					///\todo doc
					bool LoadDll(std::string psFileName);
					///\todo doc
					void Instance(std::string file_name) throw(...);
					///\todo doc
					virtual void Init();
				private:
					///\todo mswindows! humpf! mswindows!
					HINSTANCE _dll;
			///\}

			///\name two-stepped destruction
			///\{
				public:
					///\todo doc
					void Free() throw(...);
					///\todo doc
					virtual ~Plugin() throw();
			///\}

			///\name general info
			///\{
				///\todo was std::string in v1.9
				public:  virtual const char * const GetDllName() const throw() { return _psDllName.c_str(); }
				private: std::string                _psDllName;

				///\todo this was called GetBrand in in v1.9
				public:  virtual char * GetName() { return (char *)_psName.c_str(); };
				private: std::string    _psName;

				///\todo there was no ShortName in v1.9
				public:  std::string GetShortName() throw() { return _psShortName; }
				private: char        _psShortName[16];

				///\todo this was called GetVendorName in v1.9
				public:  std::string GetAuthor() throw() { return _psAuthor; }
				private: std::string _psAuthor;

				public:  bool const & IsSynth() const throw() { return _isSynth; }
				private: bool        _isSynth;

				///\todo there was also std::uint32_t GetVersion() in v1.9
			///\}

			///\name parameter info
			///\{
				public:///\todo private: move this to the proxy class
					inline CMachineInfo * GetInfo() throw() { return _pInfo; };
				private:
					///\todo move this to the proxy class
					CMachineInfo * _pInfo;
			///\}

			///\name calls to the plugin side go thru the proxy
			///\{
				/// calls to the plugin side go thru the proxy
				public:  host::proxy & proxy() throw() { return proxy_; };
				private: host::proxy   proxy_;
			///\}

			///\name calls from the plugin side go thru the PluginFxCallback
			///\{
				private:
					static PluginFxCallback _callback;
				public:
					inline static PluginFxCallback * GetCallback() throw() { return &_callback; };
			///\}

			///\name signal/event processing
			///\{
				public:
					virtual void Work(int numSamples);
					virtual float GetAudioRange(){ return 32768.0f; }
					virtual void Tick();
					virtual void Tick(int channel, PatternEntry * pEntry);
					virtual void Stop();
			///\}

			///\name parameter tweaking
			///\{
				public:
					virtual int  GetNumCols   () { return GetInfo()->numCols; };
					virtual int  GetNumParams () { return GetInfo()->numParameters; }
					virtual void GetParamName (int numparam, char * name);
					virtual void GetParamRange(int numparam, int & minval,int & maxval);
					virtual int  GetParamValue(int numparam);
					virtual void GetParamValue(int numparam, char * parval);
					virtual bool SetParameter (int numparam, int value);
			///\}

			///\name (de)serialization
			///\{
				public:
					virtual bool Load(RiffFile * pFile);
					virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
					virtual void SaveSpecificChunk(RiffFile * pFile);
			///\}
		};
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// inline implementations. we need to define body of inlined function after the class definition because of dependencies

namespace psycle
{
	namespace host
	{
		inline void proxy::operator()(CMachineInterface * plugin) throw(exceptions::function_error)
		{
			delete this->plugin_; this->plugin_ = plugin;
			//if((*this)())
			if(plugin)
			{
				callback();
				//Init(); // [bohan] i can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
			}
		}

		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma warning(push)
			#pragma warning(disable:4702) // unreachable code
		#endif

		void inline proxy:: Init()                                                                         throw(exceptions::function_error) { try { plugin().Init();                                                        } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: SequencerTick()                                                                throw(exceptions::function_error) { try { plugin().SequencerTick();                                               } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: ParameterTweak(int par, int val)                                               throw(exceptions::function_error) { try { plugin().ParameterTweak(par, val);                                      } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(exceptions::function_error) { try { plugin().Work(psamplesleft, psamplesright, numsamples, tracks);         } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: Stop()                                                                         throw(exceptions::function_error) { try { plugin().Stop();                                                        } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: PutData(void * pData)                                                          throw(exceptions::function_error) { try { plugin().PutData(pData);                                                } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: GetData(void * pData)                                                          throw(exceptions::function_error) { try { plugin().GetData(pData);                                                } PSYCLE__HOST__CATCH_ALL(host()) }
		int  inline proxy:: GetDataSize()                                                                  throw(exceptions::function_error) { try { return plugin().GetDataSize();                                          } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
		void inline proxy:: Command()                                                                      throw(exceptions::function_error) { try { plugin().Command();                                                     } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: MuteTrack(const int i)                                                         throw(exceptions::function_error) { try { plugin().MuteTrack(i);                                                  } PSYCLE__HOST__CATCH_ALL(host()) }
		bool inline proxy:: IsTrackMuted(const int i)                                                      throw(exceptions::function_error) { try { return const_cast<const CMachineInterface &>(plugin()).IsTrackMuted(i); } PSYCLE__HOST__CATCH_ALL(host()) return false; /* dummy return to avoid warning */ }
		void inline proxy:: MidiNote(const int channel, const int value, const int velocity)               throw(exceptions::function_error) { try { plugin().MidiNote(channel, value, velocity);                            } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: Event(std::uint32_t const data)                                                throw(exceptions::function_error) { try { plugin().Event(data);                                                   } PSYCLE__HOST__CATCH_ALL(host()) }
		bool inline proxy:: DescribeValue(char * txt, const int param, const int value)                    throw(exceptions::function_error) { try { return plugin().DescribeValue(txt, param, value);                       } PSYCLE__HOST__CATCH_ALL(host()) return false; /* dummy return to avoid warning */ }
		bool inline proxy:: PlayWave(const int wave, const int note, const float volume)                   throw(exceptions::function_error) { try { plugin().PlayWave(wave, note, volume);                                  } PSYCLE__HOST__CATCH_ALL(host()) return false; /* dummy return to avoid warning */ }
		void inline proxy:: SeqTick(int channel, int note, int ins, int cmd, int val)                      throw(exceptions::function_error) { try { plugin().SeqTick(channel, note, ins, cmd, val);                         } PSYCLE__HOST__CATCH_ALL(host()) }
		void inline proxy:: StopWave()                                                                     throw(exceptions::function_error) { try { plugin().StopWave();                                                    } PSYCLE__HOST__CATCH_ALL(host()) }
		int  inline proxy::Val(int parameter)                                                              throw(exceptions::function_error) { try { return plugin().Vals[parameter];                                        } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
		void inline proxy:: callback()                                                                     throw(exceptions::function_error) { try { plugin().pCB = host().GetCallback();                                    } PSYCLE__HOST__CATCH_ALL(host()) }

		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma warning(pop)
		#endif
	}
}
