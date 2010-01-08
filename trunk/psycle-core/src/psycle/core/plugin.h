// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PLUGIN__INCLUDED
#define PSYCLE__CORE__PLUGIN__INCLUDED
#pragma once

#include "machine.h"
#include <psycle/plugin_interface.hpp>

namespace psycle { namespace core {
	typedef psycle::plugin_interface::CMachineInfo * (* GETINFO) ();
	typedef psycle::plugin_interface::CMachineInterface * (* CREATEMACHINE) ();
	typedef void (* DELETEMACHINE) (psycle::plugin_interface::CMachineInterface &);

	class PluginFxCallback : public psycle::plugin_interface::CFxCallback {
	public:
		/* implement */ void MessBox(char const * ptxt, char const * caption, unsigned int type) const;
		/* implement */ int GetTickLength() const;
		/* implement */ int GetSamplingRate() const;
		/* implement */ int GetBPM() const;
		/* implement */ int GetTPB() const;
		/* implement */ int CallbackFunc(int, int, int, int) const;
		/* implement */ float * unused0(int, int) const;
		/* implement */ float * unused1(int, int) const;
};

class Plugin; // forward declaration

/// Proxy between the host and a plugin.
class Proxy {
	private:
		Plugin & host_;
		psycle::plugin_interface::CMachineInterface * plugin_;
	private:
		Plugin & host() throw();
		Plugin const & host() const throw();
		psycle::plugin_interface::CMachineInterface & plugin() throw();
		psycle::plugin_interface::CMachineInterface const & plugin() const throw();
	public:
		Proxy(Plugin & host, psycle::plugin_interface::CMachineInterface * plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }
		~Proxy() throw ()
		{
			// Proxy cannot free the interface, since its destructor happens after the destructor of Plugin,
			// which has already freed the dll.
			// (*this)(0); 
		}
		
		const bool operator()() const throw();
		void operator()(psycle::plugin_interface::CMachineInterface * plugin) throw(); //exceptions::function_error);
		void Init() throw(); //std::exceptions::function_error);
		void SequencerTick() throw(); //exceptions::function_error);
		void ParameterTweak(int par, int val) throw(); //exceptions::function_error);
		void Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(); //exceptions::function_error);
		void Stop() throw(); //exceptions::function_error);
		void PutData(void * pData) throw(); //exceptions::function_error);
		void GetData(void * pData) const throw(); //exceptions::function_error);
		int GetDataSize() const throw(); //exceptions::function_error);
		void Command() throw(); //exceptions::function_error);
		void MuteTrack(const int i) throw(); //exceptions::function_error);
		bool IsTrackMuted(const int i) throw(); //exceptions::function_error);
		void MidiNote(const int channel, const int value, const int velocity) throw(); //exceptions::function_error);
		void Event(const std::uint32_t data) throw(); //exceptions::function_error);
		bool DescribeValue(char * txt, const int param, const int value) const throw(); //exceptions::function_error);
		bool PlayWave(const int wave, const int note, const float volume) throw(); //exceptions::function_error);
		void SeqTick(int channel, int note, int ins, int cmd, int val) throw(); //exceptions::function_error);
		void StopWave() throw(); //exceptions::function_error);
		int const * const Vals() const throw(); //exceptions::function_error);
		int * const Vals() throw(); //exceptions::function_error);
		void callback() throw(); //exceptions::function_error);
};

class NativeHost;

class PSYCLE__CORE__DECL Plugin : public Machine {
	friend class NativeHost;
	private:
		static PluginFxCallback _callback;
	public:
		inline static PluginFxCallback * GetCallback() throw() { return &_callback; }
	protected:
		Plugin(MachineCallbacks*, MachineKey, Machine::id_type, void* hInstance,
			psycle::plugin_interface::CMachineInfo*, psycle::plugin_interface::CMachineInterface*);
	public:
		virtual ~Plugin() throw();
		virtual void Init();
		virtual int GenerateAudioInTicks( int startSample, int numSamples );
		virtual void Tick( );
		virtual void Tick(int channel, const PatternEvent & pEntry );
		virtual void Stop();
		///\name (de)serialization
		///\{
			/// Loader for psycle fileformat version 2.
			virtual bool LoadPsy2FileFormat(RiffFile* pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile) const;
		///\}
		inline virtual std::string GetDllName() const { return key_.dllName(); }
		virtual MachineKey getMachineKey() const { return key_; }
		virtual std::string GetName() const { return _psName; }
		virtual int GetNumParams() const { return GetInfo().numParameters; }
		virtual int GetNumCols() const { return GetInfo().numCols; }
		virtual void GetParamName(int numparam, char * name) const;
		virtual void GetParamRange(int numparam,int &minval, int &maxval) const;
		virtual int GetParamValue(int numparam) const;
		virtual void GetParamValue(int numparam,char* parval) const;
		virtual bool SetParameter(int numparam,int value);

		inline Proxy const & proxy() const { return proxy_; }
		inline Proxy & proxy() { return proxy_; }
		psycle::plugin_interface::CMachineInfo const & GetInfo() const throw() { return *info_; }
		void DeleteMachine(psycle::plugin_interface::CMachineInterface &plugin);
		
	private:
		char _psShortName[16];
		std::string _psAuthor;
		std::string _psName;
		MachineKey key_;
		void* libHandle_;
		bool _isSynth;
		psycle::plugin_interface::CMachineInfo * info_;
		Proxy proxy_;
};

inline void Proxy::Init() throw() { assert((*this)()); plugin().Init(); }
inline psycle::plugin_interface::CMachineInterface & Proxy::plugin() throw() { return *plugin_; }
inline psycle::plugin_interface::CMachineInterface const & Proxy::plugin() const throw() { return *plugin_; }
inline void Proxy::SequencerTick() throw() { plugin().SequencerTick(); }
inline void Proxy::ParameterTweak(int par, int val) throw() { assert((*this)()); plugin().ParameterTweak(par, val);  }
inline Plugin & Proxy::host() throw() { return host_; }
inline Plugin const & Proxy::host() const throw() { return host_; }
inline void Proxy::callback() throw() { assert((*this)()); plugin().pCB = host().GetCallback(); }
inline const bool Proxy::operator()() const throw() { return !!plugin_; }
inline void Proxy::operator()(psycle::plugin_interface::CMachineInterface * plugin) throw()//exceptions::function_error)
{
	if (this->plugin_) {
		host().DeleteMachine(*this->plugin_);
	}
	this->plugin_ = plugin;
	if(plugin)
	{
		callback();
		// Check if this could be done here. 
		// Init(); 
	}
}
inline void Proxy::SeqTick(int channel, int note, int ins, int cmd, int val) throw() { assert((*this)()); plugin().SeqTick(channel, note, ins, cmd, val); }
inline void Proxy::StopWave() throw() { assert((*this)());plugin().StopWave(); }
inline void Proxy::Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw() { assert((*this)()); plugin().Work(psamplesleft, psamplesright, numsamples, tracks);  }
inline int * const Proxy::Vals() throw() { assert((*this)()); return plugin().Vals; }
inline int const * const Proxy::Vals() const throw() { assert((*this)()); return plugin().Vals; }
inline void Proxy::Stop() throw() { assert((*this)()); plugin().Stop();  }
inline bool Proxy::DescribeValue(char * txt, const int param, const int value) const throw() { assert((*this)()); return const_cast<Proxy*>(this)->plugin().DescribeValue(txt, param, value); }
inline void Proxy::PutData(void * pData) throw() { assert((*this)()); plugin().PutData(pData);  }
inline void Proxy::GetData(void * pData) const throw() { assert((*this)()); const_cast<Proxy*>(this)->plugin().GetData(pData); }
inline int Proxy::GetDataSize() const throw() { assert((*this)()); return const_cast<Proxy*>(this)->plugin().GetDataSize(); }
inline void Proxy::Command()  throw() { assert((*this)()); plugin().Command(); }

}}
#endif
