/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.6 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "configuration.h"
#include "Registry.h"
#include "MidiInput.h"
#include "Song.h"
#include "NewMachine.h"

namespace configuration {
configuration::configuration(iController *pController)
{

	_RecordMouseTweaksSmooth = FALSE;
	_RecordUnarmed = true;
	_midiMachineViewSeqMode = false;
	autoStopMachines = false;
	useDoubleBuffer = true;

	_showAboutAtStart = true;
		


	// If you change the initial colour values, change it also in "CSkinDlg::OnResetcolours()"
	vu1 = 0x00f1c992;
	vu2 = 0x00403731;
	vu3 = 0x00262bd7;

	_outputDriverIndex = 1;

	bShowSongInfoOnLoad = true;
	m_bFileSaveReminders = true;

//
//	_stprintf(szBmpBkgFilename,_T("psycle.bmp"));

}

configuration::~configuration()
{

//	seqFont.DeleteObject();
//	generatorFont.DeleteObject();
//	effectFont.DeleteObject();

}

//void configuration::Error(const TCHAR* psMsg)
//{
//	MessageBox(NULL,psMsg, _T("Psycle"), MB_OK);
//}

//const bool configuration::CreatePsyFont(CFont &f, const TCHAR * sFontFace, const int HeightPx, const bool bBold, const bool bItalic)
//{
//	if(!f.IsNull())
//		f.DeleteObject();
//	CString sFace(sFontFace);
//				
//	LOGFONT lf;
//	memset(&lf, 0, sizeof(LOGFONT));       // clear out structure.
//
//	if(bBold)
//		lf.lfWeight = FW_BOLD;
//	if(bItalic)
//		lf.lfItalic = true;
//	lf.lfHeight = HeightPx;
//	lf.lfQuality = DEFAULT_QUALITY;
//	lf.lfCharSet = 0;
//	_tcsncpy(lf.lfFaceName,(LPCTSTR)sFace,32);
//
//	if(!f.CreatePointFontIndirect(&lf))
//	{			
//		CString sFaceLowerCase = sFace;
//		sFaceLowerCase.MakeLower();
//		_tcsncpy(lf.lfFaceName,(LPCTSTR)sFaceLowerCase,32);
//		
//		if(!f.CreatePointFontIndirect(&lf))
//			return false;
//	}
//
//	return true;
//}


//void configuration::CreateFonts()
//{	
//	bool bBold = pattern_font_flags&1?true:false;
//	bool bItalic = pattern_font_flags&2?true:false;
//	if (!CreatePsyFont(seqFont,pattern_fontface,pattern_font_point,bBold,bItalic))
//	{
//		MessageBox(NULL,pattern_fontface,SF::CResourceString(IDS_FONT_DO_NOT_FOUND),0);
//		if (!CreatePsyFont(seqFont,_T("Tahoma"),pattern_font_point,bBold,bItalic))
//			if (!CreatePsyFont(seqFont,_T("Verdana"),pattern_font_point,bBold,bItalic))
//				CreatePsyFont(seqFont,_T("Arial"),pattern_font_point,bBold,bItalic);
//	}
//
//	bBold = generator_font_flags&1;
//	bItalic = generator_font_flags&2?true:false;
//	if (!CreatePsyFont(generatorFont,generator_fontface,generator_font_point,bBold,bItalic))
//	{
//		MessageBox(NULL,generator_fontface,SF::CResourceString(IDS_FONT_DO_NOT_FOUND),0);
//		if (!CreatePsyFont(seqFont,_T("Tahoma"),generator_font_point,bBold,bItalic))
//			if (!CreatePsyFont(seqFont,_T("Verdana"),generator_font_point,bBold,bItalic))
//				CreatePsyFont(seqFont,_T("Arial"),generator_font_point,bBold,bItalic);
//	}
//
//
//	bBold = effect_font_flags&1;
//	bItalic = effect_font_flags&2?true:false;
//	if (!CreatePsyFont(effectFont,generator_fontface,effect_font_point,bBold,bItalic))
//	{
//		MessageBox(NULL,effect_fontface,SF::CResourceString(IDS_FONT_DO_NOT_FOUND),0);
//		if (!CreatePsyFont(seqFont,_T("Tahoma"),effect_font_point,bBold,bItalic))
//			if (!CreatePsyFont(seqFont,_T("Verdana"),effect_font_point,bBold,bItalic))
//				CreatePsyFont(seqFont,_T("Arial"),effect_font_point,bBold,bItalic);
//	}
//
//}
}


