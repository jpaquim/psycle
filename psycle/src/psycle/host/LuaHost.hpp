///\file
///\brief interface file for psycle::host::CSkinDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "Machine.hpp"

struct lua_State;

namespace psycle { namespace host {

class LuaPlugin : public Machine
{
public:
	LuaPlugin(lua_State* state, int index);
	virtual ~LuaPlugin();
	void Free();

	virtual int GenerateAudioInTicks( int startSample, int numSamples );
	virtual float GetAudioRange() const { return 32768.0f; }
	virtual const char* const GetName(void) const { return productName.c_str(); }
	virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);
	virtual const char * const GetDllName() const throw() { return luaName.c_str(); }

	//TODO: implement
	virtual const std::string GetAuthor() { return authorName; }
	virtual const std::uint32_t GetAPIVersion() { return 0; }
	virtual const std::uint32_t GetPlugVersion() { return 0; }
	bool const & IsSynth() const throw() { return _isSynth; }

	//TODO: implement
	virtual void NewLine() { Machine::NewLine(); }
	virtual void Tick(int track, PatternEntry * pData){}
	virtual void Stop(){}

	//TODO: implement
	virtual void GetParamRange(int numparam,int &minval, int &maxval) {minval=0; maxval=1;}
	virtual int GetNumParams() {return 0; }
	virtual int GetParamType(int numparam) { return 2; }
	virtual void GetParamName(int numparam, char * parval){parval[0]='\0';}
	virtual void GetParamValue(int numparam, char * parval){parval[0]='\0';}
	virtual int GetParamValue(int numparam) {return 0;}
	virtual bool SetParameter(int numparam, int value){ return false;}
	virtual void SetParameter(int numparam, float value){}
	virtual bool DescribeValue(int parameter, char * psTxt){psTxt[0]='\0'; return false; }


protected:
	lua_State * L;


	/// Indicates the lua script name
	std::string luaName;
	/// Indicates the name (product) of this script
	std::string productName;
	/// Indicates the author of this script
	std::string authorName;
	/// content of the script.
	std::string scriptText;

	bool        _isSynth;
};


class LuaHost
{
public:
	LuaHost();
	~LuaHost();

	static LuaPlugin* LoadPlugin(const char * sName, int macIdx);
	static void freestate(lua_State* L);
	static int getsample(lua_State* L);
	static int setsample(lua_State* L);

public:
	static std::map<lua_State*, LuaPlugin*> host;
};

	}   // namespace
}   // namespace
