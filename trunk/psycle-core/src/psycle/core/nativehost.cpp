// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "nativehost.hpp"

#include "pluginfinder.h"
#include "plugin.h"
#include "file.h"

#include <iostream>
#include <sstream>

#if defined __unix__ || defined __APPLE__
	#include <dlfcn.h>
#elif defined _WIN32
	#include <windows.h>
#endif

namespace psy { namespace core {
	using namespace psycle::plugin_interface;

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

Machine* NativeHost::CreateMachine(PluginFinder& finder, MachineKey key,Machine::id_type id) 
{
	if (key == MachineKey::failednative()) 
	{
		Plugin *p = new Plugin(mcallback_, key, id, 0, 0, 0);
		return p;
	}

	//FIXME: This is a good place where to use exceptions. (task for a later date)
	std::string fullPath = finder.lookupDllName(key);
	if (fullPath.empty()) return 0;
	void* hInstance = LoadDll(fullPath);
	if (!hInstance) return 0;
	CMachineInfo* info = LoadDescriptor(hInstance);
	if (!info) {
		UnloadDll(hInstance);
		return 0;
	}
	CMachineInterface* maciface = Instantiate(hInstance);
	if (!maciface) {
		UnloadDll(hInstance);
		return 0;
	}
	Plugin * p = new Plugin(mcallback_, key, id, hInstance, info, maciface );
	p->Init();
	return p;
}


void NativeHost::FillPluginInfo(const std::string& fullName, const std::string& fileName, PluginFinder& finder)
{
	#if defined __unix__ || defined __APPLE__
		if ( fileName.find( "lib-xpsycle.") == std::string::npos ) return;
	#else
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#endif
	
	void* hInstance = LoadDll(fullName);
	if (!hInstance) return;
	
	CMachineInfo* minfo = LoadDescriptor(hInstance);
	if (minfo) {
		PluginInfo pinfo;
		pinfo.setName( minfo->Name );
		bool _isSynth = (minfo->Flags == 3);
		pinfo.setRole( _isSynth?MachineRole::GENERATOR : MachineRole::EFFECT );
		pinfo.setLibName(fullName);
		std::ostringstream o;
		std::string version;
		if (!(o << minfo->Version )) version = o.str();
		pinfo.setVersion( version );
		pinfo.setAuthor( minfo->Author );
		MachineKey key( hostCode(), fileName, 0);
		finder.AddInfo( key, pinfo);
	}
	UnloadDll(hInstance);
}

void* NativeHost::LoadDll( std::string const & file_name )
{
	void* hInstance;
	
	std::string old_path = File::appendDirToEnvPath(file_name);

#if defined __unix__ || defined __APPLE__
	hInstance = ::dlopen(file_name.c_str(), RTLD_LAZY /*RTLD_NOW*/);
	if (!hInstance) {
		std::cerr << "Cannot load library: " << dlerror() << '\n';
	}
#else
	// Set error mode to disable system error pop-ups (for LoadLibrary)
	UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	hInstance = LoadLibraryA( file_name.c_str() );
	// Restore previous error mode
	SetErrorMode( uOldErrorMode );
	if (!hInstance) {
		///\todo
	}
#endif

	File::setEnvPath(old_path);
	
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
			std::cerr << "plugin format is too old" << info_->Version << "\n";
			info_ = 0;
		}
		return info_;
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
		std::cerr << "exception while creating interface instance\n";
		return 0;
	}
}


void NativeHost::UnloadDll( void* hInstance )
{
	assert(hInstance);
	#if defined __unix__ || defined __APPLE__
		::dlclose(hInstance);
	#else
		::FreeLibrary((HINSTANCE)hInstance);
	#endif
}


}}
