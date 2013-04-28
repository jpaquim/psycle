///\file
///\brief implementation file for psycle::host::CSkinDlg.

#include <psycle/host/detail/project.private.hpp>
#include "luahost.hpp"
#include "lua.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <psycle/helpers/math.hpp>
#include "PsycleConfig.hpp"
#include "Configuration.hpp"
#include "PsycleGlobal.hpp"
#include <algorithm>



namespace psycle { namespace host {
 	
   std::map<lua_State*, LuaPlugin*> LuaHost::host;

		//////////////////////////////////////////////////////////////////////////
		// Lua

		LuaPlugin::LuaPlugin(lua_State* state, int index)
		{
			_macIndex = index;
			_type = MACH_LUA;
			_mode = MACHMODE_FX;
			std::sprintf(_editName, "native plugin");
			productName = "unkown script";
			authorName = "unkown author";
			InitializeSamplesVector();
			L = state;
		}

		LuaPlugin::~LuaPlugin() {
			try {
				Free();
			} catch(...){}
		}
		void LuaPlugin::Free() {
			LuaHost::freestate(L);
		}

		int LuaPlugin::GenerateAudioInTicks(int /*startSample*/, int numSamples)
		{
			if(_mode == MACHMODE_GENERATOR) {
				Standby(false);
			}
			if (!_mute) 
			{
				if ((_mode == MACHMODE_GENERATOR) || (!Bypass() && !Standby()))
				{
					lua_getglobal(L, "Work");
					lua_pushnumber (L, numSamples );
					int status = lua_pcall(L, 1, 0 ,0);    // call Lua Work method with 1 param and 0 results   			
					if (status) {
						CString msg(lua_tostring(L, -1));
						AfxMessageBox(msg);
					}
					UpdateVuAndStanbyFlag(numSamples);
				}
			}
			else Standby(true);
			return numSamples;
		}

		bool LuaPlugin::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			//TODO
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Skip(size);
			return true;
		}
		void LuaPlugin::SaveSpecificChunk(RiffFile * pFile)
		{
			//TODO
		}


		LuaPlugin* LuaHost::LoadPlugin(const char * sName, int macIdx)
		{
			lua_State* L = luaL_newstate();
	        luaL_openlibs(L);
			std::string path = sName;
			/// This is needed to prevent loading problems
			std::replace(path.begin(), path.end(), '\\', '/');
			int status = luaL_dofile(L, path.c_str());
			if (status) {
				const char* msg =lua_tostring(L, -1);
				std::ostringstream s; s
					<< "Failed to open lua script: " << sName << msg << std::endl;
					throw psycle::host::exceptions::library_errors::loading_error(s.str());
			}
			LuaPlugin *plug = new LuaPlugin(L, macIdx);
			host[L]=plug;
			lua_register(L, "getsample", LuaHost::getsample);
			lua_register(L, "setsample", LuaHost::setsample);
			return plug;
		}

		void LuaHost::freestate(lua_State* L)
		{
			std::map<lua_State*, LuaPlugin*>::iterator it = host.find(L);
			lua_close (L);
			host.erase(it);
		}


		int LuaHost::getsample(lua_State* L) {
			int channel = (int) lua_tonumber(L, 1)-1;
			int index = (int) lua_tonumber(L, 2)-1;
			LuaPlugin* h = host.find(L)->second;
			lua_pushnumber (L, h->samplesV[channel][index]);
			return 1;
		}		

		int LuaHost::setsample(lua_State* L) {
			int channel = lua_tonumber(L, 1)-1;
			int index = (int) lua_tonumber(L, 2)-1;
			float value = lua_tonumber(L, 3);
			LuaPlugin* h = host.find(L)->second;
			h->samplesV[channel][index]=value;
			return 0;
		}
}   // namespace
}   // namespace
