// -*- mode:c++; indent-tabs-mode:t -*-
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
#include <psycle/core/psycleCorePch.hpp>

#include "nativehost.hpp"
#include "pluginfinder.h"
#include "plugin.h"

namespace psy {namespace core {


typedef CMachineInfo * (* GETINFO) ();
typedef CMachineInterface * (* CREATEMACHINE) ();

NativeHost::NativeHost(MachineCallbacks*calls)
:MachineHost(calls){
}
NativeHost::~NativeHost()
{
}

NativeHost& NativeHost::getInstance(MachineCallbacks* callb)
{
	static NativeHost instance(callb);
	return instance;
}

Machine* NativeHost::CreateMachine(PluginFinder* finder, MachineKey key,Machine::id_type id) 
{
	//FIXME: This is a good place where to use exceptions. (task for a later date)
	std::string fullPath = finder->lookupDllName(key);
	if (fullPath.empty()) return 0;
	void* hInstance = LoadDll(fullPath);
	if (!hInstance) return 0;
	CMachineInfo* info = LoadDescriptor(hInstance,key.index());
	if (!info) {
		UnloadDll(hInstance);
		return 0;
	}
	CMachineInterface* maciface = Instantiate(hInstance,key.index());
	if (!maciface) {
		UnloadDll(hInstance);
		return 0;
	}
	Plugin * p = new Plugin(callbacks, key, id, info, maciface );
	p->Init();
	return p;
}


void NativeHost::FillPluginInfo(const std::string& currentPath, const std::string& fileName, std::map<MachineKey,PluginInfo>& infoMap)
{
	#if defined __unix__ || defined __APPLE__
		///\todo problem of so.x.y.z .. .so all three times todo
	#else
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#endif
	
	void* hInstance = LoadDll(fileName);
	if (!hInstance) return 0;
	
	CMachineInfo* minfo = LoadDescriptor(hInstance);
	if (minfo) {
		PluginInfo pinfo;
		MachineKey key( hostCode() , fileName, 0 );
		pinfo.setName( minfo->Name );
		//pinfo.setRole( plugin.mode() );
		pinfo.setLibName(currentPath + fileName);
		std::ostringstream o;
		std::string version;
		if (!(o << minfo->Version )) version = o.str();
		pinfo.setVersion( version );
		pinfo.setAuthor( minfo->Author );
		infoMap[key] = pinfo;
	
		minfo = LoadDescriptor(hInstance,++index);
	}
	UnloadDll(hInstance);
}

void* NativeHost::LoadDll( std::string const & psFileName )
{
	void* hInstance;
///FIXME: The disabled code was introduced in psyclemfc in order to load plugins that depend on external dll's.
///       In such cases, the dll needs to be placed in the same dir than the .exe, not the one of the .dll, and this
///       code allowed to place it with the dlls.
#if 0
	char const static path_env_var_name[] =
	{
		#if defined __unix__ || defined __APPLE__
			"LD_LIBRARY_PATH"
		#elif defined _WIN64 || or defined _WIN32
			"PATH"
		#else
			#error unknown dynamic linker
		#endif
	};
	// save the original path env var
	std::string old_path;
	{
		char const * const env(std::getenv(path_env_var_name));
		if(env) old_path = env;
	}
	// append the plugin dir to the path env var
	std::string new_path(old_path);
	if(new_path.length()) {
		new_path +=
			#if defined __unix__ || defined __APPLE__
				":"
			#elif defined _WIN64 || or defined _WIN32
				";"
			#else
				#error unknown dynamic linker
			#endif
	}
	///\todo new_path += dir_name(file_name), using boost::filesystem::path for portability
	// append the plugin dir to the path env var
	if(::putenv((path_env_var_name + ("=" + new_path)).c_str())) {
		std::cerr << "psycle: plugin: warning: could not alter " << path_env_var_name << " env var.\n";
	}
#endif // 0

#if defined __unix__ || defined __APPLE__
	hInstance = ::dlopen(file_name.c_str(), RTLD_LAZY /*RTLD_NOW*/);
#else
	// Set error mode to disable system error pop-ups (for LoadLibrary)
	UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	hInstance = LoadLibraryA( file_name.c_str() );
	// Restore previous error mode
	SetErrorMode( uOldErrorMode );
#endif

#if 0
	// set the path env var back to its original value
	if(::putenv((path_env_var_name + ("=" + old_path)).c_str())) {
		std::cerr << "psycle: plugin: warning: could not set " << path_env_var_name << " env var back to its original value.\n";
	}
#endif // 0

	if (!hInstance) {
	#if defined __unix__ || defined __APPLE__
		std::cerr << "Cannot load library: " << dlerror() << '\n';
	#else
		///\todo
	#endif
	}
	return hInstance;
}

CMachineInfo* NativeHost::LoadDescriptor(void* hInstance)
{
	try {
	#if defined __unix__ || defined __APPLE__
		GETINFO GetInfo = (GETINFO) dlsym( hInstance, "GetInfo");
		if (!GetInfo) {
			std::cerr << "Cannot load symbols: " << dlerror() << '\n';
			return 0;
		}
	#else
		GETINFO GetInfo = (GETINFO) GetProcAddress( static_cast<HINSTANCE>( hInstance ), "GetInfo" );
		if (!GetInfo) {
			///\todo readd the original code here!
			return 0;
		}
	#endif
		CMachineInfo* info_ = GetInfo();
		if(info_->Version < MI_VERSION) {
			std::cerr << "plugin format is too old" << info_->Version << GetDllName() << "\n";
			info_ = 0;
		}
		return info;
	} catch (...) {
		std::cerr << "exception while getting plugin info handler\n";
		return 0;
	}
}

CMachineInterface* NativeHost::Instantiate(void * hInstance)
{      
	try {
	#if defined __unix__ || defined __APPLE__
		CREATEMACHINE GetInterface =  (CREATEMACHINE) dlsym(hInstance, "CreateMachine");
	#else
		CREATEMACHINE GetInterface = (CREATEMACHINE) GetProcAddress( (HINSTANCE)hInstance, "CreateMachine" );
	#endif
		if(!GetInterface) {
		#if defined __unix__ || defined __APPLE__
			std::cerr << "Cannot load symbol: " << dlerror() << "\n";
		#else
			///\todo
		#endif
			return 0;
		} else {
			return GetInterface();
		}
	} catch (...) {
		throw InstanceException()
		std::cerr << "exception while creating interface instance\n";
		return 0;
	}
}


void NativeHost::UnloadDll( void* hInstance )
{
	assert(_hInstance);
	#if defined __unix__ || defined __APPLE__
		::dlclose(_dll);
	#else
		::FreeLibrary((HINSTANCE)_dll);
	#endif
}


}}


