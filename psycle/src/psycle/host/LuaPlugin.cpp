///\file
///\brief implementation file for psycle::host::CSkinDlg.

#include <psycle/host/detail/project.private.hpp>
#include "luaplugin.hpp"
#include "lua.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <psycle/helpers/math.hpp>
#include "PsycleConfig.hpp"
#include "Configuration.hpp"
#include "PsycleGlobal.hpp"
#include <algorithm>
#include <psycle/host/Song.hpp>
#include "Zap.hpp"



namespace psycle { namespace host {
 	
		//////////////////////////////////////////////////////////////////////////
		// Lua

		LuaPlugin::LuaPlugin(lua_State* state, int index, bool full)
			: proxy_(this, state)
		{		
			_macIndex = index;
			_type = MACH_LUA;
			_mode = MACHMODE_FX;
			std::sprintf(_editName, "native plugin");		
			InitializeSamplesVector();
			try {
			  proxy_.call_run();
			  if (full) {
			    proxy_.call_init();
			  }
			} catch(std::exception &e) {} //do nothing.
		}

		LuaPlugin::~LuaPlugin() {
			Free();
		}

		void LuaPlugin::Free() {
		  try {
			proxy_.free_state();
		  } catch(std::exception &e) {} //do nothing.
		}

		void LuaPlugin::OnReload()
		{
			proxy_.reload();
			/*PluginInfo info = CallPluginInfo();
	        _mode = info.mode;*/
		}

		int LuaPlugin::GenerateAudioInTicks(int /*startSample*/, int numSamples) throw(psycle::host::exception)
		{
			if (crashed()) {
				return numSamples;
			}
			if(_mode == MACHMODE_GENERATOR) {
				Standby(false);
			}
			if (!_mute) 
			{
				if ((_mode == MACHMODE_GENERATOR) || (!Bypass() && !Standby()))
				{										
					try {
					   proxy_.call_work(numSamples);					
					}catch(std::exception &e) {} //do nothing.
					UpdateVuAndStanbyFlag(numSamples);
				}
			}
			else Standby(true);
			return numSamples;
		}

		bool LuaPlugin::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size,sizeof(size)); // size of whole structure
			if(size)
			{
				UINT count;
				pFile->Read(&count,sizeof(count));  // size of vars
				/*
				if (count)
				{
					pFile->Read(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
				}
				*/
				std::vector<int> vals;
				for (UINT i = 0; i < count; i++)
				{
					int temp;
					pFile->Read(&temp,sizeof(temp));
					vals.push_back(temp);
					//SetParameter(i, temp);
				}
				std::map<std::string, int> ids;
				for (UINT i = 0; i < count; i++)
				{
					std::string id;
					bool res = pFile->ReadString(id); // (&temp,sizeof(temp));
					ids[id] = i;
				}
				int num = GetNumParams();
				for (int i = 0; i < num; ++i) {
				  std::string id = proxy_.get_parameter_id(i);
				  std::map<std::string, int>::iterator it = ids.find(id);
				  if (it != ids.end()) {
				    int idx = ids[id];
				    SetParameter(i, vals[idx]);
				  } else {
					  // parameter not found
				  }
				}

				size = 0;
				//size -= sizeof(count) + sizeof(int)*count;
				if(size)
				{
					byte* pData = new byte[size];
					pFile->Read(pData, size); // Number of parameters
					try
					{
						// proxy().PutData(pData); // Internal load
					}
					catch(const std::exception &e)
					{
#ifndef NDEBUG 
						delete[] pData;
						throw e;
						return false;
#else
						e;
						delete[] pData;
						return false;
#endif
					}
					delete[] pData;
					return true;
				}
			}
			return true;
		}

		void LuaPlugin::SaveSpecificChunk(RiffFile * pFile)
		{
			UINT count = GetNumParams();
			UINT size2(0);
			try
			{
				// size2 = proxy().GetDataSize();
			}
			catch(const std::exception &e)
			{
				e;
#ifndef NDEBUG 
					throw e;
#endif
				// data won't be saved
			}
			UINT size = size2 + sizeof(count) + sizeof(int)*count;
			std::vector<std::string> ids;
			for (UINT i = 0; i < count; i++) {
			  std::string id = proxy_.get_parameter_id(i);
			  ids.push_back(id);
			  size += id.length()+1;
			}
			pFile->Write(&size,sizeof(size));
			pFile->Write(&count,sizeof(count));
			//pFile->Write(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
			for (UINT i = 0; i < count; i++)
			{
				int temp = GetParamValue(i);
				pFile->Write(&temp,sizeof temp);
			}
			// ids
			for (UINT i = 0; i < count; i++) {
			  pFile->WriteString(ids[i]);
			}
			if(size2)
			{
				byte * pData = new byte[size2];
				try
				{
					// proxy().GetData(pData); // Internal save
				}
				catch(const std::exception &e)
				{
					e;
					// this sucks because we already wrote the size,
					// so now we have to write the data, even if they are corrupted.
#ifndef NDEBUG 
					throw e;
#endif

				}
				pFile->Write(pData, size2); // Number of parameters
				zapArray(pData);
			}
		}

		bool LuaPlugin::SetParameter(int numparam, int value) {
			if (crashed()) {
				return false;
			}
			try {
			  int minval; int maxval;
			  proxy_.get_parameter_range(numparam, minval, maxval);
			  int quantization = (maxval-minval);
			  proxy_.call_parameter(numparam,float(value)/float(quantization));
			  return true;
			} catch(std::exception &e) {} //do nothing.
			return false;
		}

		void LuaPlugin::GetParamRange(int numparam,int &minval, int &maxval) { 
			if (crashed()) {
			  minval = 0; maxval = 0xFFFF;
			  return;
			}
			try {
			  if( numparam < GetNumParams() ) {
			     proxy_.get_parameter_range(numparam, minval, maxval);
			  }
			}catch(std::exception &e) {}
		}

		int LuaPlugin::GetParamType(int numparam) { 
			if (crashed()) {			  
			  return 0;
			}
            int mpf = 0 ;
			try {
			  if( numparam < GetNumParams() ) {
			     mpf = proxy_.get_parameter_type(numparam);
			  }
			}catch(std::exception &e) {}
			return mpf;
		}

		void LuaPlugin::GetParamName(int numparam, char * parval) {
			if (crashed()) {
				std::strcpy(parval, "");
				return;
			}
			try {
			  if( numparam < GetNumParams() ) {
				std::string name = proxy_.get_parameter_name(numparam);
				std::strcpy(parval, name.c_str());
			  } else std::strcpy(parval, "Out of Range");
			}catch(std::exception &e) { std::strcpy(parval, ""); }
		}

		int LuaPlugin::GetParamValue(int numparam){
			if (crashed()) {
				return 0;
			}
			if(numparam < GetNumParams()) {
			  int minval; int maxval;			  
			  try {
				proxy_.get_parameter_range(numparam, minval, maxval);
			    int quantization = (maxval-minval);
				return proxy_.get_parameter_value(numparam) * quantization;
  			  } catch(std::exception &e) {} //do nothing.
			} else {
				// out of range
			}
			return 0;
		}

		bool LuaPlugin::DescribeValue(int numparam, char * psTxt){			
			if (crashed()) {
				return false;
			}

				if(numparam >= 0 && numparam < GetNumParams()) {
					try {
					  std::string par_display = proxy_.get_parameter_display(numparam);
					  std::string par_label = proxy_.get_parameter_label(numparam);
					  std::sprintf(psTxt, "%s(%s)", par_display.c_str(), par_label.c_str());
					  return true;
					} catch(std::exception &e) {
                      std::string par_display("Out of range");
  				      std::sprintf(psTxt, "%s", par_display);
					  return true;
					} //do nothing.
				}
			return false;
		}

		void LuaPlugin::GetParamValue(int numparam, char * parval) {
			if (crashed()) {
				return;
			}
			if(numparam < GetNumParams()) {
				try {
					if(!DescribeValue(numparam, parval)) {
						std::sprintf(parval,"%.0f",GetParamValue(numparam) * 1); // 1 = Plugin::quantizationVal())
					}					
				}
				catch(const std::exception &e) {
#ifndef NDEBUG 
					throw e;
					return;
#else
					e;
					return;
#endif
				}
			}
			else std::strcpy(parval,"Out of Range");
		}

		std::string LuaPlugin::help() {
			if (crashed()) {
				return "saucer section missing";
			}		
			try {
				return proxy_.call_help();
			} catch(const std::exception &e) {}
			return "";
		}

		void LuaPlugin::NewLine() {
			if (crashed()) {
				return;
			}		
			try {
				proxy_.call_newline();
			} catch(const std::exception &e) {}
		}

		void LuaPlugin::Tick(int track, PatternEntry * pData){
			if (crashed()) {
				return;
			}
			try {
		      proxy_.call_seqtick(track, pData->_note, pData->_inst, pData->_cmd, pData->_parameter);
			} catch(const std::exception &e) {}
		}

		void LuaPlugin::Stop(){
			if (crashed()) {
				return;
			}
			try { proxy_.call_stop(); } catch(const std::exception &e) {}
		}
	  
		
		

}   // namespace
}   // namespace
