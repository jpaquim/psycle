#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.1 $
 */
class AudioDriver;
class CMidiInput;
#include "iCompornentConfiguration.h"


namespace model {
	__interface IConfiguration : iCompornentConfiguration
	{

		__declspec(property(get=GetInitialized)) bool IsInitialized;
		__declspec(property(get=GetSamplesPerSec)) int SamplesPerSec;
		__declspec(property(get=GetInstrumentDir,put=PutInstrumentDir)) string& InstrumentDir;
		__declspec(property(get=GetSongDir,put=PutSongDir)) string& SongDir;
		__declspec(property(get=GetSkinDir,put=PutSkinDir)) string& SkinDir;
		__declspec(property(get=GetVstDir,put=PutVstDir)) string& VstDir;
		__declspec(property(get=GetIsFileSaveRemind,put=PutIsFileSaveRemind)) bool IsFileSaveRemind;
		__declspec(property(get=GetIsShowSongInfoOnLoad,put=PutIsShowSongInfoOnLoad)) bool IsShowSongInfoOnLoad;
		__declspec(property(get=GetOutputDriver)) ::AudioDriver& OuputDriver;
		__declspec(property(get=GetMidiInput)) ::CMidiInput&  MidiInput;

		const bool GetInitialized();
		const int GetSamplesPerSec();

		PROP_DEF(string&,InstrumentDir)
		PROP_DEF(string&,SongDir)
		PROP_DEF(string&,SkinDir)
		PROP_DEF(string&,VstDir)
		PROP_DEF(bool,IsFileSaveRemind)
		PROP_DEF(bool,IsShowSongInfoOnLoad)
		
		::AudioDriver& GetOutputDriver() ;
		::CMidiInput&  GetMidiInput() ;	// MIDI IMPLEMENTATION

	};
}