/***************************************************************************
*   Copyright (C) 2007 Psycledelics   *
*   psycle.sf.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02?111-1307, USA.             *
***************************************************************************/
#ifndef PLUGIN_H
#define PLUGIN_H

#include "machine.h"
#include "plugin_interface.h"

/**
@author  Psycledelics
*/

namespace psy { namespace core {

class PluginFxCallback : public CFxCallback
{
	public:
		virtual void MessBox(char const* ptxt,char const* caption,unsigned int type);
			virtual int GetTickLength();
		virtual int GetSamplingRate();
		virtual int GetBPM();
		virtual int GetTPB();
};

class Plugin; // forward declaration

/// Proxy between the host and a plugin.
class Proxy
{
	private:
		Plugin & host_;
		CMachineInterface * plugin_;
	private:
		Plugin & host() throw();
		Plugin const & host() const throw();
		CMachineInterface & plugin() throw();
		CMachineInterface const & plugin() const throw();
	public:
		Proxy(Plugin & host, CMachineInterface * plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }
		~Proxy() throw ()
		{ 
			// (*this)(0);  ///\todo this segfaults under windows .. investigate 
		}
		
		const bool operator()() const throw();
		void operator()(CMachineInterface * plugin) throw(); //exceptions::function_error);
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
		void Event(const dword data) throw(); //exceptions::function_error);
		bool DescribeValue(char * txt, const int param, const int value) const throw(); //exceptions::function_error);
		bool PlayWave(const int wave, const int note, const float volume) throw(); //exceptions::function_error);
		void SeqTick(int channel, int note, int ins, int cmd, int val) throw(); //exceptions::function_error);
		void StopWave() throw(); //exceptions::function_error);
		int const * const Vals() const throw(); //exceptions::function_error);
		int * const Vals() throw(); //exceptions::function_error);
		void callback() throw(); //exceptions::function_error);
};

class Plugin : public Machine
{
	private:
		static PluginFxCallback _callback;
	public:
		inline static PluginFxCallback * GetCallback() throw() { return &_callback; }
	public:
		Plugin(MachineCallbacks* callbacks, Machine::id_type index, CoreSong* song);

		virtual ~Plugin() throw();

		virtual void Init();
		virtual int GenerateAudioInTicks( int startSample, int numSamples );
		virtual void Tick( );
		virtual void Tick(int channel, const PatternEvent & pEntry );
		virtual void Stop();
		inline virtual std::string GetDllName() const throw() { return _psDllName; }
		virtual std::string GetName() const { return _psName; }

		virtual int GetNumParams() const { return GetInfo().numParameters; }
		virtual int GetNumCols() const { return GetInfo().numCols; }
		virtual void GetParamName(int numparam, char * name) const;
		virtual void GetParamRange(int numparam,int &minval, int &maxval) const;
		virtual int GetParamValue(int numparam) const;
		virtual void GetParamValue(int numparam,char* parval) const;
		virtual bool SetParameter(int numparam,int value);

		inline Proxy const & proxy() const throw() { return proxy_; }
		inline Proxy & proxy() throw() { return proxy_; }

		bool Instance(const std::string & file_name);
		bool LoadDll (std::string const & path, std::string const & file_name);

		///\name (de)serialization
		///\{
			public:
				/// Loader for psycle fileformat version 2.
				virtual bool LoadPsy2FileFormat(std::string const & plugin_path, RiffFile* pFile);
				virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
				virtual void SaveSpecificChunk(RiffFile * pFile) const;
				virtual void SaveDllName      (RiffFile * pFile) const;
		///\}

		CMachineInfo const & GetInfo() const throw() { return *info_; }
		
	private:
		void* _dll;
		char _psShortName[16];
		std::string _psAuthor;
		std::string _psDllName;
		std::string _psName;
		bool _isSynth;
		CMachineInfo * info_;
		Proxy proxy_;
};

inline void Proxy::Init() throw() { assert((*this)()); plugin().Init(); }
inline CMachineInterface & Proxy::plugin() throw() { return *plugin_; }
inline CMachineInterface const & Proxy::plugin() const throw() { return *plugin_; }
inline void Proxy::SequencerTick() throw() { plugin().SequencerTick(); }
inline void Proxy::ParameterTweak(int par, int val) throw() { assert((*this)()); plugin().ParameterTweak(par, val);  }
inline Plugin & Proxy::host() throw() { return host_; }
inline Plugin const & Proxy::host() const throw() { return host_; }
inline void Proxy::callback() throw() { assert((*this)()); plugin().pCB = host().GetCallback(); }
inline const bool Proxy::operator()() const throw() { return !!plugin_; }
inline void Proxy::operator()(CMachineInterface * plugin) throw()//exceptions::function_error)
{
	delete this->plugin_; this->plugin_ = plugin;
	if(plugin)
	{
		callback();
		//Init(); // [bohan] i can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
	}
}
inline void Proxy::SeqTick(int channel, int note, int ins, int cmd, int val) throw() { assert((*this)()); plugin().SeqTick(channel, note, ins, cmd, val); }
inline void Proxy::StopWave() throw() { assert((*this)());plugin().StopWave(); }
inline void Proxy::Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw() { assert((*this)()); fflush(stdout); plugin().Work(psamplesleft, psamplesright, numsamples, tracks);  }
inline int * const Proxy::Vals() throw() { assert((*this)()); return plugin().Vals; }
inline int const * const Proxy::Vals() const throw() { assert((*this)()); return plugin().Vals; }
inline void Proxy::Stop() throw() { assert((*this)()); plugin().Stop();  }
inline bool Proxy::DescribeValue(char * txt, const int param, const int value) const throw() { assert((*this)()); return const_cast<Proxy*>(this)->plugin().DescribeValue(txt, param, value); }
inline void Proxy::PutData(void * pData) throw() { assert((*this)()); plugin().PutData(pData);  }
inline void Proxy::GetData(void * pData) const throw() { assert((*this)()); const_cast<Proxy*>(this)->plugin().GetData(pData); }
inline int Proxy::GetDataSize() const throw() { assert((*this)()); return const_cast<Proxy*>(this)->plugin().GetDataSize(); }

}}
#endif
