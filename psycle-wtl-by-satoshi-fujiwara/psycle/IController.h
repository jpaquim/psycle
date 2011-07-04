#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.1 $
 */
#include "iPlayer.h"
#include "iConfigurable.h"
#include "IConfiguration.h"
#include "iSong.h"
#include "iMainFrame.h"
#include "iResampler.h"
#include "iAudioDriver.h"
#include "IMidiInput.h"

namespace model {

__interface iController : iConfigurable
{
	__declspec(property(get=GetSong)) iSong& Song;
	__declspec(property(get=GetPlayer)) iPlayer& Player;
	__declspec(property(get=GetResampler)) iResampler& Resampler;
	__declspec(property(get=GetMainFrame)) view::iMainFrame& MainFrame;
	__declspec(property(get=GetMainFrame)) view::iMainFrame& MainFrame;

	iSong& GetSong();
	iPlayer& GetPlayer();
	iResampler& GetResampler();
	view::iMainFrame& GetMainFrame() ; 

	void UpdatePlayOrder(const bool mode) ;
	void UpdateSequencer() ;

	void SelectFile(string& inputFileName) ;
	void InitializeBeforeLoading() ;
	void ProcessAfterLoading() ;
	void AddBpm(const int addValue) ;
	void AddTpb(const int addValue) ;
	// File Method 
	void NewFile() ;
	void SaveFile(const bool bFileSaveAs) ;

	const bool IsOutputActive() const ;
	void IsOutputActive(const bool bActive) ;

	// TODO ÉRÅ[ÉhÇí«â¡Ç∑ÇÈ
	const int SamplesPerSec() const ;
	const ULONG CpuHz() const ;

	audiodriver::iAudioDriver* const OutputDriver() const ;
	IMidiInput*  const MidiInput() const ;	// MIDI IMPLEMENTATION

	HRESULT Run(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow) ;

	const string& CurrentDirectory() const ;

};

}