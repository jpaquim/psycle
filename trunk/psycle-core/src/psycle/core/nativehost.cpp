// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include <diversalis/os.hpp>
#include <universalis/os/exceptions/code_description.hpp>
#include <universalis/os/loggers.hpp>
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

namespace psycle { namespace core {
	using namespace psycle::plugin_interface;

	namespace loggers = universalis::os::loggers;

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

Machine* NativeHost::CreateMachine(PluginFinder& finder, const MachineKey& key,Machine::id_type id) 
{
	if (key == MachineKey::failednative) 
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
		if ( fileName.find( "libpsycle-plugin-") == std::string::npos ) return;
	#else
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#endif
	
	void* hInstance = LoadDll(fullName);
	if (!hInstance) return;
	
	CMachineInfo* minfo = LoadDescriptor(hInstance);
	if (minfo) {
		PluginInfo pinfo;
		//todo!
		//pinfo.setFileTime();
		pinfo.setName( minfo->Name );
		bool _isSynth = (minfo->Flags == 3);
		pinfo.setRole( _isSynth?MachineRole::GENERATOR : MachineRole::EFFECT );
		pinfo.setLibName(fullName);
		{
			std::ostringstream o;
			if ( minfo->APIVersion < 0x10) {
				o << minfo->APIVersion;
			}
			else {
				o << std::hex << minfo->APIVersion;
			}
			pinfo.setApiVersion(  o.str() );
		}
		{
			std::ostringstream o;
			std::string version;
			o << std::hex << minfo->PlugVersion;
			pinfo.setPlugVersion(  o.str() );
		}
		pinfo.setAuthor( minfo->Author );
		pinfo.setAllow(true);
		MachineKey key( hostCode(), fileName, 0);
		finder.AddInfo( key, pinfo);
	}
	///\todo: Implement the bad cases, so that the plugin can be added to 
	/// the finder as bad.
	UnloadDll(hInstance);
}

void* NativeHost::LoadDll( std::string const & file_name )
{
	void* hInstance;
	
	std::string old_path = File::appendDirToEnvPath(file_name);

#if defined __unix__ || defined __APPLE__
	hInstance = ::dlopen(file_name.c_str(), RTLD_LAZY /*RTLD_NOW*/);
	if (!hInstance && loggers::exception()) {
		std::ostringstream s;
		s << "psycle: core: nativehost: LoadDll:" << std::endl
			<< "could not load library: " << file_name << std::endl
			<<  dlerror();
		loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
#else
	// Set error mode to disable system error pop-ups (for LoadLibrary)
	UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	hInstance = LoadLibraryA( file_name.c_str() );
	// Restore previous error mode
	SetErrorMode( uOldErrorMode );
	if (!hInstance && loggers::exception()) {
		std::ostringstream s;
		s << "psycle: core: nativehost: LoadDll:" << std::endl
			<< "could not load library: " << file_name
			<< universalis::os::exceptions::code_description();
		loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
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
		if (!GetInfo && loggers::exception()) {
			std::ostringstream s;
			s << "psycle: core: nativehost: LoadDescriptor:" << std::endl
				<< "Cannot load symbols: " << dlerror();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			return 0;
		}
	#else
		GETINFO GetInfo = (GETINFO) GetProcAddress( static_cast<HINSTANCE>( hInstance ), "GetInfo" );
		if (!GetInfo) {
			///\todo readd the original code here!
			if (!GetInfo && loggers::exception()) {
				std::ostringstream s;
				s << "psycle: core: nativehost: LoadDescriptor:" << std::endl
					<< "Cannot load symbols: " 
					<< universalis::os::exceptions::code_description();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				return 0;
			}
		}
	#endif
		CMachineInfo* info_ = GetInfo();
		// version 10 and 11 didn't use HEX representation.
		// Also, verify for 32 or 64bits.
		if(!(info_->APIVersion == 11 && (MI_VERSION&0xFFF0) == 0x0010)
			&& !((info_->APIVersion&0xFFF0) == (MI_VERSION&0xFFF0)) && loggers::exception()) {

			std::ostringstream s;
			s << "plugin version not supported" << info_->APIVersion;
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			info_ = 0;
		}
		return info_;
	} catch (...) {
		if(loggers::exception()) {
			std::ostringstream s;
			s << "exception while getting plugin info handler";
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
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
			if(loggers::exception()) {
				std::ostringstream s;
				s << "Cannot load symbol: " << dlerror();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		#else
			if(loggers::exception()) {
				std::ostringstream s;
				s << "Cannot load symbol: "
				<< universalis::os::exceptions::code_description();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		#endif
			return 0;
		} else {
			return GetInterface();
		}
	} catch (...) {
		if(loggers::exception()) {
			std::ostringstream s;
			s << "exception while creating interface instance";
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		return 0;
	}
}


void NativeHost::UnloadDll( void* hInstance )
{
	assert(hInstance);
	#if defined __unix__ || defined __APPLE__
		dlclose(hInstance);
	#else
		::FreeLibrary((HINSTANCE)hInstance);
	#endif
}


}}
