///\file
///\brief interface file for psycle::host::Plugin
#pragma once
#include "machine.hpp"
#include "player.hpp"
#include <psycle/engine/configuration.hpp>
#include <psycle/plugin_interface.hpp>
#include <psycle/plugin_gui.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		/// \todo CPresetsDlg code sux big time concerning interface separation :-(
		class CPresetDlg;

		/// calls that the plugin side can make to the host side
		///\todo PLEASE EXTEND THIS!!!
		class PluginFxCallback : public plugin_interface::CFxCallback
		{
			public:
				inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { ::MessageBox(hWnd,ptxt,caption,type); }
				inline virtual int GetTickLength()   { return Global::player().SamplesPerRow(); }
				inline virtual int GetSamplingRate() { return Global::player().SampleRate(); }
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
					plugin_interface::CMachineInterface const inline & plugin() const throw() { assert(plugin_); return *plugin_; }
					plugin_interface::CMachineInterface       inline & plugin()       throw() { assert(plugin_); return *plugin_; }
				private:
					plugin_interface::CMachineInterface              * plugin_;
				public:
					inline bool operator()() const throw() { return plugin_; }
					void operator()(plugin_interface::CMachineInterface * plugin) throw(exceptions::function_error);
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

					/// \todo CPresetsDlg code sux big time concerning interface separation :-(
					friend class CPresetDlg; int const /* at least it's const! */ * Vals() { return plugin().Vals; }
			///\}
		};

		/// the host side
		class Plugin : public Machine
		{
			///\name the complicated quadristepped (@#$%§!) construction
			///\{
				public:
					///< Helper class for Machine Creation.
					static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
					///\todo doc
					Plugin(Machine::id_type id);
					///\todo doc
					bool LoadDll (std::string const & file_name);
					///\todo doc
					void Instance(std::string const & file_name);
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
				public:  virtual const std::string GetDllName() { return _psDllName; }
				private: std::string                _psDllName;

				public:  virtual const std::string GetBrand() { return _psName; }
				private: std::string                _psName;

				public:  virtual const std::string GetVendorName() { return _psAuthor; }
				private: std::string _psAuthor;

				public:	virtual const std::uint32_t GetVersion() { return 0; }
				public: virtual const std::uint32_t GetCategory() { return 0; }

				public:  bool const inline & IsSynth() const throw() { return _isSynth; }
				private: bool               _isSynth;
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
				public:
					virtual /*plugin_interface::*/CMachineGuiParameter* GetParam(int num); // dw00t
				private:
					/*plugin_interface::*/CMachineGuiParameter ** _pParams; //dw00t
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
				///\todo move this to the proxy class
				plugin_interface::CMachineInfo        *  _pInfo;

		};
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// inline implementations. we need to define body of inlined function after the class definition because of dependencies

namespace psycle
{
	namespace host
	{
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