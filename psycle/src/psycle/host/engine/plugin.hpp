///\file
///\brief interface file for psycle::host::Plugin
#pragma once
#include "machine.hpp"
#include "player.hpp"
#include <psycle/host/configuration.hpp>
#include <psycle/plugin_interface.hpp>
#include <psycle/plugin_gui.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		/// calls that the plugin side can make to the host side
		///\todo PLEASE EXTEND THIS!!!
		class PluginFxCallback : public plugin_interface::CFxCallback
		{
			public:
				inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { ::MessageBox(hWnd,ptxt,caption,type); }
				inline virtual int GetTickLength()   { return Global::player().SamplesPerRow(); }
				inline virtual int GetSamplingRate() { return Global::configuration().GetSamplesPerSec(); }
				inline virtual int GetBPM() { return Global::player().bpm; }
				inline virtual int GetTPB() { return Global::player().tpb; }
			PSYCLE__PRIVATE:
				/// mswindows! humpf! mswindows!
				HWND hWnd;
		};

		class Plugin; // forward declaration

		/// proxy between the host and a plugin
		class proxy
		{
			public:
				inline  proxy(Plugin & host, plugin_interface::CMachineInterface * plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }
				inline ~proxy() throw() { (*this)(0); }

			///\name reference to the host side
			///\{
				private:
					Plugin const inline & host()      const throw() { return host_; }
					Plugin       inline & host()            throw() { return host_; }
				private:
					Plugin              & host_;
			///\}

			///\name reference to the plugin side
			///\{
				private:
					plugin_interface::CMachineInterface const inline & plugin() const throw() { return *plugin_; }
					plugin_interface::CMachineInterface       inline & plugin()       throw() { return *plugin_; }
				private:
					plugin_interface::CMachineInterface              * plugin_;
				public:
					inline bool operator()() const throw() { return plugin_; }
					inline void operator()(plugin_interface::CMachineInterface * plugin) throw(exceptions::function_error)
					{
						delete this->plugin_; ///\todo call the plugin's exported deletion function instead
						this->plugin_ = plugin;
						if(plugin)
						{
							callback();
							//Init(); // can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
						}
					}
			///\}

			///\name protected calls from the host side to the plugin side
			///\{
				public:
					#if defined DIVERSALIS__COMPILER__MICROSOFT
						#pragma warning(push)
						#pragma warning(disable:4702) // unreachable code
					#endif

					void inline Init()                                                                         throw(exceptions::function_error) { assert((*this)()); try { plugin().Init();                                                        } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline SequencerTick()                                                                throw(exceptions::function_error) { assert((*this)()); try { plugin().SequencerTick();                                               } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline ParameterTweak(int par, int val)                                               throw(exceptions::function_error) { assert((*this)()); try { plugin().ParameterTweak(par, val);                                      } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(exceptions::function_error) { assert((*this)()); try { plugin().Work(psamplesleft, psamplesright, numsamples, tracks);         } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline Stop()                                                                         throw(exceptions::function_error) { assert((*this)()); try { plugin().Stop();                                                        } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline PutData(void * pData)                                                          throw(exceptions::function_error) { assert((*this)()); try { plugin().PutData(pData);                                                } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline GetData(void * pData)                                                          throw(exceptions::function_error) { assert((*this)()); try { plugin().GetData(pData);                                                } PSYCLE__HOST__CATCH_ALL(host()) }
					int  inline GetDataSize()                                                                  throw(exceptions::function_error) { assert((*this)()); try { return plugin().GetDataSize();                                          } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
					void inline Command()                                                                      throw(exceptions::function_error) { assert((*this)()); try { plugin().Command();                                                     } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline MuteTrack(const int i)                                                         throw(exceptions::function_error) { assert((*this)()); try { plugin().MuteTrack(i);                                                  } PSYCLE__HOST__CATCH_ALL(host()) }
					bool inline IsTrackMuted(const int i)                                                      throw(exceptions::function_error) { assert((*this)()); try { return const_cast<const CMachineInterface &>(plugin()).IsTrackMuted(i); } PSYCLE__HOST__CATCH_ALL(host()) return false; /* dummy return to avoid warning */ }
					void inline MidiNote(const int channel, const int value, const int velocity)               throw(exceptions::function_error) { assert((*this)()); try { plugin().MidiNote(channel, value, velocity);                            } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline Event(std::uint32_t const data)                                                throw(exceptions::function_error) { assert((*this)()); try { plugin().Event(data);                                                   } PSYCLE__HOST__CATCH_ALL(host()) }
					bool inline DescribeValue(char * txt, const int param, const int value)                    throw(exceptions::function_error) { assert((*this)()); try { return plugin().DescribeValue(txt, param, value);                       } PSYCLE__HOST__CATCH_ALL(host()) return false; /* dummy return to avoid warning */ }
					bool inline PlayWave(const int wave, const int note, const float volume)                   throw(exceptions::function_error) { assert((*this)()); try { plugin().PlayWave(wave, note, volume);                                  } PSYCLE__HOST__CATCH_ALL(host()) return false; /* dummy return to avoid warning */ }
					void inline SeqTick(int channel, int note, int ins, int cmd, int val)                      throw(exceptions::function_error) { assert((*this)()); try { plugin().SeqTick(channel, note, ins, cmd, val);                         } PSYCLE__HOST__CATCH_ALL(host()) }
					void inline StopWave()                                                                     throw(exceptions::function_error) { assert((*this)()); try { plugin().StopWave();                                                    } PSYCLE__HOST__CATCH_ALL(host()) }
					int  inline * Vals()                                                                       throw(exceptions::function_error) { assert((*this)()); try { return plugin().Vals;                                                   } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
					void inline callback()                                                                     throw(exceptions::function_error) { assert((*this)()); try { plugin().pCB = host().GetCallback();                                    } PSYCLE__HOST__CATCH_ALL(host()) }

					#if defined DIVERSALIS__COMPILER__MICROSOFT
						#pragma warning(pop)
					#endif
			///\}
		};

		/// the host side
		class Plugin : public Machine
		{
			///\name the complicated quadristepped (@#$%§!) construction
			///\{
				public:
					///\todo doc
					Plugin(Machine::id_type id);
					///\todo doc
					void Instance(std::string const & file_name);
					///\todo doc
					bool LoadDll (std::string const & file_name);
					///\todo doc
					virtual void Init();
				private:
					/// mswindows! humpf! mswindows!
					HINSTANCE _dll;
			///\}

			///\name two-stepped destruction
			///\{
				public:
					///\todo doc
					void Free();
					///\todo doc
					virtual ~Plugin() throw();
			///\}

			///\name general info
			///\{
				public:  std::string const inline & GetDllName  () const throw() { return _psDllName; }
				private: std::string _psDllName;
				
				public:  std::string const inline & GetName     () const throw() { return _psName; };
				private: std::string _psName;

				public:  std::string const inline & GetShortName() const throw() { return _psShortName; }
				private: char _psShortName[16];

				public:  std::string const inline & GetAuthor   () const throw() { return _psAuthor; }
				private: std::string _psAuthor;

				public:  bool        const inline & IsSynth     () const throw() { return _isSynth; }
				private: bool                      _isSynth;

				PSYCLE__PRIVATE:
					PSYCLE__DEPRECATED("use the std::string const & overload") virtual char*GetName(){return/*static*/reinterpret_cast<char*>_psName.c_str();}
					PSYCLE__DEPRECATED("use the std::string const & overload") virtual char*GetDllName(){return _psDllName.c_str();}
			///\}

			///\name calls to the plugin side go thru the proxy
			///\{
				public:
					/// calls to the plugin side go thru the proxy
					host::proxy inline & proxy() throw() { return proxy_; };
				private:
					host::proxy          proxy_;
			///\}

			///\name calls from the plugin side go thru the PluginFxCallback
			///\{
				public:
					PluginFxCallback static inline * GetCallback() throw() { return &callback_; }
					PluginFxCallback static inline &    callback() throw() { return  callback_; }
				private:
					PluginFxCallback static             callback_;
			///\}

			///\name signal/event processing
			///\{
				public:
					virtual void Work(int numSamples);
					virtual void Tick();
					virtual void Tick(int channel, PatternEntry * pEntry);
					virtual void Stop();
			///\}

			///\name parameter tweaking
			///\{
				public:
					virtual int  GetNumCols   () { return GetInfo()->numCols; };
					virtual int  GetNumParams () { return GetInfo()->numParameters; };
					virtual void GetParamName (int numparam, char * name);
					virtual void GetParamRange(int numparam, int & minval, int & maxval);
					virtual int  GetParamValue(int numparam);
					virtual void GetParamValue(int numparam, char* parval);
					virtual bool SetParameter (int numparam, int value);
					virtual plugin_interface::CMachineGuiParameter* GetParam(int num); // dw00t
				private:
					CMachineGuiParameter ** _pParams; //dw00t
			///\}

			///\name (de)serialization
			///\{
				public:
					virtual bool LoadOldFileFormat(RiffFile * pFile);
					virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
					virtual void SaveSpecificChunk(RiffFile * pFile);
					virtual void SaveDllName      (RiffFile * pFile);
			///\}

			PSYCLE__PRIVATE:
				PSYCLE__DEPRECATED("use member functions")
				plugin_interface::CMachineInfo inline * GetInfo() throw() { return _pInfo; };
			private:
				plugin_interface::CMachineInfo        * _pInfo;
		};
	}
}
