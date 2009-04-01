/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
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
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
/*
*<@JosepMa> the so-called seib host (which is mine, but based on his), is composed of two classes:
*<@JosepMa> CVstHost and CEffect.
*<@JosepMa> the former maps all the AudioMaster calls, provides a way to create CEffects, and helps
*           in getting time/position information.
*<@JosepMa> CEffect is a C++ wrapper for the AEffect class, for a host (AudioEffect is a C++ wrapper
*           for a plugin)
*<@JosepMa> as such it maps all the dispatch calls to functions with parameter validation, and helps
*           in the construction and destruction processes. Tries to help on other simpler tasks, and
*           in the handling of parameter windows (VstEffectWnd.cpp/.hpp)
*<@JosepMa> vst::AudioMaster and vst::plugin are subclasses of the aforementioned classes, which both: extend
*           the functionality of the base classes, and adapts them to its usage inside psycle
*<@JosepMa> the host one doesn't provide much more (since the base class is good enough), and the
*           plugin one wraps the CEffect into a Machine class
*/
#ifndef VSTHOST_HPP
#define VSTHOST_HPP

#if !defined _WIN64 && !defined _WIN32
	#if defined DIVERSALIS__COMPILER__GNU
		#warning ###########################- UNIMPLEMENTED ###################
	#endif
#else

#include <psycle/core/machinehost.hpp>
#include <seib/vst/CVSTHost.Seib.hpp>
#include <map>

using namespace seib::vst;

namespace psy{ namespace core{ namespace vst {

// Dialog max ticks for parameters.
const int quantize = 65535;

class AudioMaster;

class host : public MachineHost
{
protected:
	host(MachineCallbacks*);
public:
	virtual ~host();
	static host& getInstance(MachineCallbacks*);

	virtual Machine* CreateMachine(PluginFinder&, MachineKey, Machine::id_type);

	virtual const Hosts::type hostCode() const { return Hosts::VST; }
	virtual const std::string hostName() const { return "VST2"; }
	virtual std::string const & getPluginPath(int) const { return plugin_path_; }
	virtual int getNumPluginPaths() const { return 1; }
	virtual void setPluginPath(std::string path) { plugin_path_ = path; }

protected:
	virtual void FillPluginInfo(const std::string&, const std::string&, PluginFinder&);
	void* LoadDll( const std::string& );
	void UnloadDll( void*  );
	std::string plugin_path_;
	AudioMaster* master;
};



class AudioMaster : public CVSTHost
{
public:
	friend class host;
	AudioMaster(MachineCallbacks* callbacks)
		: pCallbacks(callbacks)
		, currentKey(MachineKey::invalid())
	{
		quantization = quantize;
		SetBlockSize(STREAM_SIZE); SetTimeSignature(4,4);
		vstTimeInfo.smpteFrameRate = kVstSmpte25fps;
	}
	virtual ~AudioMaster(){;}

	///< Helper class for Machine Creation.
	virtual CEffect * CreateEffect(LoadedAEffect &loadstruct);
	virtual CEffect * CreateWrapper(AEffect *effect); 
	virtual void Log(std::string message);

	///> Plugin gets Info from the host
	virtual bool OnGetProductString(char *text) const { strcpy(text, "psycle-core"); return true; }
	virtual long OnGetHostVendorVersion() const { return 1000; }
	virtual bool OnCanDo(CEffect &pEffect,const char *ptr) const ;
	virtual long OnGetHostLanguage() const { return kVstLangEnglish; }
	virtual void CalcTimeInfo(long lMask = -1);
	virtual long DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos) const;
	virtual long DECLARE_VST_DEPRECATED(OnGetNumAutomatableParameters)(CEffect &pEffect) const { return 0xFF; } // the size of the aux column.
	virtual long OnGetAutomationState(CEffect &pEffect) const ;
	virtual long OnGetInputLatency(CEffect &pEffect) const;
	virtual long OnGetOutputLatency(CEffect &pEffect) const;
	//\todo : how can this function be implemented? :o
	virtual long OnGetCurrentProcessLevel(CEffect &pEffect) const { return 0; }
	virtual bool OnWillProcessReplacing(CEffect &pEffect) const;

	///> Plugin sends actions to the host
	virtual bool OnProcessEvents(CEffect &pEffect, VstEvents* events) { return false; }
private:
	MachineCallbacks* pCallbacks;
	MachineKey currentKey;
	Machine::id_type currentId;
};
}}}
#endif // VSTHOST_HPP
#endif
