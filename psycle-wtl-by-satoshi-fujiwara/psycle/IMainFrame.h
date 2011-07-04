#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */

#include "iConfigurable.h"
namespace view {
__interface iMainFrame : model::iConfigurable
{
	/** Change BPM Event */
	void ChangeBpm(const int bpm);
	/** Change TPB Event */
	void ChangeTpb(const int tpb);
	/** Check Unsaved Song */
	const bool CheckUnsavedSong();
	const bool CheckUnsavedSong(const TCHAR * szTitle);
	void NewSong();
	void SongSaved();
	void SongLoaded();

	const UINT MessageDialog(const string& text,const string& caption,const UINT type);
	const bool SelectFile(TCHAR * fileName);

	__declspec(property(get=GetHwnd)) ::HWND WindowHandle;

	const HWND GetHwnd();

	void InitializeBeforeLoading();
};

}
