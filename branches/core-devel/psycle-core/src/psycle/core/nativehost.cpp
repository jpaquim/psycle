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

static NativeHost* instance_ = 0;

NativeHost::NativeHost(MachineCallbacks*calls)
:MachineHost(calls){
}
NativeHost::~NativeHost()
{
	if ( instance_ )
		delete instance_;
}

NativeHost& NativeHost::getInstance(MachineCallbacks* callb) {
	if ( !instance__ )
		instance__ = new NativeHost(callb);
	return instance_;
}

Machine* NativeHost::CreateMachine(PluginFinder* finder, MachineKey key,Machine::id_type id) const 
{
	std::string fullPath = finder->lookupDllName(key);
	if (fullpath.empty()){ 
		return 0;
	}
	void* hInstance = LoadDll(plugin_path, dllName);
	if(!hInstance) {
		return 0;
	}

	Plugin * p = new Plugin(callbacks, id);
	if(!p->Instance(hInstance))
	{
		//Since we don't allow to delete machines via "delete" keyword,
		//we centralize the dll unloading here too unlike in psyclemfc.
		UnloadDll(hInstance);
		delete p;
		return 0;
	}
	p->Init();
	return p;
}


void NativeHost::DeleteMachine(Machine* mac) const {
	UnloadDll(mac->GethInstance());
	delete mac;
}

void NativeHost::FillFinderData(PluginFinder* finder, bool clearfirst) const
{
	if ( !finder.hasHost(Hosts::NATIVE) ) {
		//Finder stores one map for each host, so we ensure that it knows this host.
		finder.addHost(Hosts::NATIVE);
	}
	std::map<MachineKey,PluginInfo> infoMap = finder.getMap(Hosts::NATIVE);

	if (clearfirst) {
		infoMap.clear();
	}

	std::vector<std::string> fileList;
	try {
		fileList = File::fileList(psycle_path_, File::list_modes::files);
	} catch ( std::exception& e ) {
		std::cout << "Warning: Unable to scan your native plugin directory. Please make sure the directory listed in your config file exists." << std::endl;
		return;
	}

	std::vector<std::string>::iterator it = fileList.begin();
	for ( ; it < fileList.end(); ++it ) {
		LoadNativeInfo(*it);
	}
}

void NativeHost::LoadNativeInfo(std::string fileName, std::map<MachineKey,PluginInfo>& infoMap)
{
	#if defined __unix__ || defined __APPLE__
		///\todo problem of so.x.y.z .. .so all three times todo
	#else
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#endif

	void* hInstance = LoadDll(plugin_path, dllName);
	if(!hInstance) {
		return;
	}
	Plugin p(callbacks, id);
	if(p->Instance(hInstance,false)) {
		//we centralize the dll unloading here too unlike in psyclemfc.
		UnloadDll(hInstance);
		delete p;
		return;
	}

	PluginInfo info;
	MachineKey key( Hosts::NATIVE, fileName, 0 );
	info.setKey(key);
	info.setName( plugin.GetName() );
//	info.setRole( plugin.mode() );
	info.setLibName(fileName);
	std::ostringstream o;
	std::string version;
	if (!(o << plugin.GetInfo().Version )) version = o.str();
	info.setVersion( version );
	info.setAuthor( plugin.GetInfo().Author );
	///\todo .. path should here stored and not evaluated in plugin
	infoMap[key] = info;
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


