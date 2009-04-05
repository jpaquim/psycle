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


#include <psycle/core/vsthost.h>
#if !defined _WIN64 && !defined _WIN32
	#if defined DIVERSALIS__COMPILER__GNU
		#warning ###########################- UNIMPLEMENTED ###################
	#endif
#else

#include <psycle/core/pluginfinder.h>
#include <psycle/core/vstplugin.h>
#include <psycle/core/file.h>
#include <psycle/core/playertimeinfo.h>

#include <iostream>
#include <sstream>
#if defined __unix__ || defined __APPLE__
	#include <dlfcn.h>
#elif defined _WIN32
	#include <windows.h>
#endif
namespace psy {namespace core { namespace vst {

host::host(MachineCallbacks*calls)
:MachineHost(calls){
	master = new AudioMaster(calls);
}
host::~host()
{
	delete master;
}

host& host::getInstance(MachineCallbacks* callb)
{
	static host instance(callb);
	return instance;
}

Machine* host::CreateMachine(PluginFinder& finder, MachineKey key,Machine::id_type id) 
{
	if (key == MachineKey::wrapperVst() ) {
		return static_cast<vst::plugin*>(master->CreateWrapper(0));
	}
	//FIXME: This is a good place where to use exceptions. (task for a later date)
	std::string fullPath = finder.lookupDllName(key);
	if (fullPath.empty()) return 0;

	master->currentKey = key;
	master->currentId = id;
	vst::plugin* plug = (vst::plugin*)master->LoadPlugin(fullPath.c_str(),key.index());
	plug->Init();
	return plug;
}


void host::FillPluginInfo(const std::string& fullName, const std::string& fileName, PluginFinder& finder)
{
	#if defined __unix__ || defined __APPLE__
		if ( fileName.find( "lib-xpsycle.") == std::string::npos ) return;
	#else
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#endif
	
	void* hInstance = LoadDll(fullName);
	if (!hInstance) return;
	
	UnloadDll(hInstance);
}

void* host::LoadDll( std::string const & file_name )
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



void host::UnloadDll( void* hInstance )
{
	assert(hInstance);
	#if defined __unix__ || defined __APPLE__
		::dlclose(hInstance);
	#else
		::FreeLibrary((HINSTANCE)hInstance);
	#endif
}



//==============================================================
//==============================================================

CEffect * AudioMaster::CreateEffect(LoadedAEffect &loadstruct)
{
	return new plugin(pCallbacks,currentKey,currentId,loadstruct);
}

CEffect * AudioMaster::CreateWrapper(AEffect *effect)
{
	return new plugin(pCallbacks,currentId,effect);
}


void AudioMaster::CalcTimeInfo(long lMask)
{
	///\todo: cycleactive and recording to a "Start()" function.
	// automationwriting and automationreading.
	//
	/*
	kVstTransportCycleActive	= 1 << 2,
	kVstTransportRecording		= 1 << 3,

	kVstAutomationWriting		= 1 << 6,
	kVstAutomationReading		= 1 << 7,
	*/

	//kVstCyclePosValid			= 1 << 12,	// start and end
	//	cyclestart // locator positions in quarter notes.
	//	cycleend   // locator positions in quarter notes.

	CVSTHost::CalcTimeInfo(lMask);
}


bool AudioMaster::OnCanDo(CEffect &pEffect, const char *ptr) const
{
	using namespace seib::vst::HostCanDos;
	bool value =  CVSTHost::OnCanDo(pEffect,ptr);
	if (value) return value;
	else if (
		//||	(!strcmp(ptr, canDoReceiveVstEvents))	// "receiveVstEvents",
		//||	(!strcmp(ptr, canDoReceiveVstMidiEvent ))// "receiveVstMidiEvent",
		//||	(!strcmp(ptr, "receiveVstTimeInfo" ))// DEPRECATED

		(!strcmp(ptr, canDoReportConnectionChanges )) // "reportConnectionChanges",
		//||	(!strcmp(ptr, canDoAcceptIOChanges ))	// "acceptIOChanges",
		||(!strcmp(ptr, canDoSizeWindow ))		// "sizeWindow",

		//||	(!strcmp(ptr, canDoAsyncProcessing ))	// DEPRECATED
		//||	(!strcmp(ptr, canDoOffline ))			// "offline",
		//||	(!strcmp(ptr, "supportShell" ))		// DEPRECATED
		//||	(!strcmp(ptr, canDoEditFile ))			// "editFile",
		//||	(!strcmp(ptr, canDoSendVstMidiEventFlagIsRealtime ))
		)
		return true;
	return false;                           /* per default, no.                  */
}

long AudioMaster::DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos) const
{
	//\todo: return the real tempo in the future, not always the current one
	// pos in Sample frames, return bpm* 10000
	return vstTimeInfo.tempo * 10000;
}
long AudioMaster::OnGetOutputLatency(CEffect &pEffect) const
{
	return pCallbacks->timeInfo().outputLatency();
}
long AudioMaster::OnGetInputLatency(CEffect &pEffect) const
{
	return pCallbacks->timeInfo().inputLatency();
}
void AudioMaster::Log(std::string message)
{
	//todo
}
bool AudioMaster::OnWillProcessReplacing(CEffect &pEffect) const {
	return ((plugin*)&pEffect)->WillProcessReplace();
}


///\todo: Get information about this function
long AudioMaster::OnGetAutomationState(CEffect &pEffect) const { return kVstAutomationUnsupported; }


}}}

#endif
