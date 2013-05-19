///\file
///\brief interface file for psycle::host::CSkinDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "Machine.hpp"
#include "LuaArray.hpp"
#include "LuaHost.hpp"

struct lua_State;

namespace psycle { namespace host {

class LuaPlugin : public Machine
{
public:
	LuaPlugin(lua_State* state, int index);
	virtual ~LuaPlugin();
	void Free();

	virtual int GenerateAudioInTicks( int startSample, int numSamples );
	virtual float GetAudioRange() const { return 1.0f; }
	virtual const char* const GetName(void) const { return proxy_.info().name.c_str(); }
	// todo
	virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);
	virtual const char * const GetDllName() const throw() { return dll_path_.c_str(); }
	//PluginInfo
	virtual const std::string GetAuthor() { return proxy_.info().vendor; }
	virtual const std::uint32_t GetAPIVersion() { return proxy_.info().APIversion; }
	virtual const std::uint32_t GetPlugVersion() { return atoi(proxy_.info().version.c_str()); }
	bool IsSynth() const throw() { return (proxy_.info().mode == MACHMODE_GENERATOR); }

	//TODO: implement
	virtual void NewLine() { Machine::NewLine(); } // compatibility to psy core ?
	virtual void Tick(int track, PatternEntry * pData);
	virtual void Stop();

	//TODO: testing
	virtual void GetParamRange(int numparam,int &minval, int &maxval) {minval=0; maxval=0xFFFF;}
	virtual int GetNumParams() { return proxy_.num_parameter(); }
	virtual int GetParamType(int numparam) { return 2; }
	virtual void GetParamName(int numparam, char * parval);
	virtual void GetParamValue(int numparam, char * parval);
	virtual int GetParamValue(int numparam);
	virtual bool SetParameter(int numparam, int value); //{ return false;}
	virtual bool DescribeValue(int parameter, char * psTxt);

	void ReloadScript();

	std::string dll_path_;
protected:
	LuaProxy proxy_;
};


	}   // namespace
}   // namespace
