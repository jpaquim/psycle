/** @file 
 *  @brief implementation of the CPsycleWTLView class
 *  @author Juan Antonio Arguelles Rius
 *  [C] 2000 All Rights Reserved
 *  [C] 2001-2002 Psycledelics.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.10 $
 *
 */
#include "stdafx.h"
// STL
#include <algorithm>
#include <cctype>
#include <boost/format.hpp>
#include <sstream>

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
#include "resource.h"
#define rdtsc __asm __emit 0fh __asm __emit 031h
#define cpuid __asm __emit 0fh __asm __emit 0a2h
#include <memory>
#include "sfhelper.h"
//#include "Psycle2.h"
#include "PsycleWTLView.h"
#include "Registry.h"
#include "Configuration.h"
#include "FileXM.h"
//#include "FileIT.h"
//#include "ChildView.h"
#include "Bitmap.cpp"

#include "Player.h"
#include "MidiInput.h"
#include "SwingFillDlg.h"
#include "Helpers.h"
#include "MasterDlg.h"
#include "GearTracker.h"
#ifndef CONVERT_INTERNAL_MACHINES
#include "GearPsychOsc.h"
#include "GearDistort.h"
#include "GearDelay.h"
#include "Gearfilter.h"
#include "GearGainer.h"
#include "GearFlanger.h"
#endif

//#include "Dsp.h"
//#include "Filter.h"

#include "MenuXP.h"
#include "MainFrm.h"
#include "WireDlg.h"
#include "MacProp.h"
#include "NewMachine.h"
#include "PatDlg.h"
#include "GreetDialog.h"
#include "ConfigDlg.h"
#include "SongpDlg.h"
#include "inputhandler.h"
#include "VstEditorDlg.h"
#include "masterdlg.h"
#include "SaveWavDlg.h"

#include <math.h> // SwingFill

#include "keybhandler.cpp"
#include "mouseHandler.cpp"

// 
#include "Psy3SongLoader.h"
#include "Psy2SongLoader.h"
#include ".\psysongloader.h"
#include "PsyFSongSaver.h"
#include "XMInstrument.h"
#include "XMSampler.h"
#include "XMSongLoader.h"

/////////////////////////////
#pragma unmanaged

//////////////////////////////////////////////////////////////////////
// Enviroment Initialization

//CMainFrame		*pParentMain;

unsigned idletime = 0;

/////////////////////////////////////////////////////////////////////////////
// CPsycleWTLView


CPsycleWTLView::CPsycleWTLView() : m_FmtPan(IDS_MSG0014),m_FmtPan1(IDS_MSG0015)

{
	m_bPrepare = false;
};
void CPsycleWTLView::Initialize()
{
	// Set Gui Environment data

	// Enviroment variables
	smac = -1;
	smacmode = 0;
	wiresource = -1;
	wiredest = -1;
	wiremove = -1;
	wireSX = 0;
	wireSY = 0;
	wireDX = 0;
	wireDY = 0;

	for (int c = 0; c < 256; c++)
	{ 
		FLATSIZES[c] = 8;
	}
	
	bmpDC = NULL;

	viewMode = VMMachine;
	MasterMachineDialog = NULL;
	SamplerMachineDialog = NULL;
	XMSamplerMachineDialog = NULL;

#ifndef CONVERT_INTERNAL_MACHINES

	PsychMachineDialog = NULL;
	DistortionMachineDialog = NULL;
	
	DelayMachineDialog = NULL;
	FilterMachineDialog = NULL;
	GainerMachineDialog = NULL;
	FlangerMachineDialog = NULL;
#endif
	for (c = 0; c < MAX_WIRE_DIALOGS; c++)
	{
		WireDialog[c] = NULL;
	}

	updateMode=0;
	updatePar=0;
//	multiPattern=true; // Long way till it can be finished!

	patStep=1;
	editPosition=0;
	prevEditPosition=0;
	bEditMode = true;

	blockSelected=false;
	isBlockCopied=false;
	patBufferCopy=false;
	blockNTracks=0;
	blockNLines=0;
	bScrollDetatch=false;
	pUndoList=NULL;
	pRedoList=NULL;

	UndoCounter=0;
	UndoSaved=0;

	UndoMacCounter=0;
	UndoMacSaved=0;

//	editcur.track=0; // Not needed to initialize, since the class does it already.
//	editcur.col=0;
//	editcur.line=0;
	playpos = -1;
	newplaypos = -1; 
	selpos.bottom = 0;
	newselpos.bottom = 0;
	numPatternDraw = 0;

//	scrollT=0;
//	scrollL=0;
	tOff=0;
	lOff=0;
	ntOff=0;
	nlOff=0;

	ChordModeOffs = 0;

	Global::pInputHandler->SetChildView(this);
	Global::pResampler->SetQuality(RESAMPLE_LINEAR);
	_outputActive = false;

	// just give arbitrary values so OnSize doesn't give /0 error
	// they will be filled in correctly when we switch to pattern view
	VISLINES = 2;
	VISTRACKS = 8;

//	_getcwd(m_appdir,_MAX_PATH);
	
// Creates a new song object. The application Song.
//	Global::_pSong->Reset(); It's already called in _pSong->New();
	Global::_pSong->New();
	
	// Referencing the childView song pointer to the
	// Main Global::_pSong object [The application Global::_pSong]
	_pSong = Global::_pSong;

	m_SongLoaderMap[_T("psy")] = new SF::PsySongLoader();
	m_SongLoaderMap[_T("psf")] = new SF::PsySongLoader();
	m_SongLoaderMap[_T("xm")] = new SF::XMSongLoader();

	// Show Machine view and init MIDI
	pMainFrame->OnMachineview();
}

CPsycleWTLView::~CPsycleWTLView()
{
	Global::pInputHandler->SetChildView(NULL);
	KillRedo();
	KillUndo();

	if ( bmpDC != NULL )
	{
#ifdef _DEBUG_PATVIEW
		TCHAR buf[100];
		_stprintf(buf,SF::CResourceString(IDS_TRACE_MSG0009),(int)bmpDC);
		ATLTRACE(buf);
#endif
		bmpDC->DeleteObject();
		delete bmpDC;
	}
	if(!patternheader.IsNull())
		patternheader.DeleteObject();
	//DeleteObject(hbmPatHeader);
	if(!machineskin.IsNull())
		machineskin.DeleteObject();
	//DeleteObject(hbmMachineSkin);
	if(!patternheadermask.IsNull())
		patternheadermask.DeleteObject();
	if(!machineskinmask.IsNull())
		machineskinmask.DeleteObject();
	if(!machinebkg.IsNull())
		machinebkg.DeleteObject();
	
	//DeleteObject(hbmMachineBkg);
	for(SongLoaderMap::iterator it = m_SongLoaderMap.begin();it != m_SongLoaderMap.end();it++)
	{
		delete it->second;
	}
}



/////////////////////////////////////////////////////////////////////////////
// CPsycleWTLView message handlers
/*
BOOL CPsycleWTLView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
	
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
//		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);
		::LoadCursor(NULL, IDC_ARROW), 0, NULL);
	
	return TRUE;
}
*/
//////////////////////////////////////////////////////////////////////
// This function gives to the pParentMain the pointer to a CMainFrm
// object. Call this function from the CMainframe side object to
// allow CPsycleWTLView call functions of the CMainFrm parent object
// Call this function after creating both the CPsycleWTLView object and
// the cmainfrm object

void CPsycleWTLView::ValidateParent()
{
	//pParentMain = (CMainFrame *)rMainFrame;
	pMainFrame->_pSong = Global::_pSong;
}

//////////////////////////////////////////////////////////////////////
// Timer initialization

void CPsycleWTLView::InitTimer()
{
	KillTimer(31);
	KillTimer(159);
	
	if (!SetTimer(31,20,NULL)) // GUI update. 
	{
		MessageBox(RES_STR(IDS_COULDNT_INITIALIZE_TIMER),_T(""), MB_ICONERROR);
	}
	if ( Global::pConfig->autosaveSong )
	{
		if (!SetTimer(159,Global::pConfig->autosaveSongTime * 60000,NULL)) // Autosave Song
		{
			MessageBox(RES_STR(IDS_COULDNT_INITIALIZE_TIMER),_T(""), MB_ICONERROR);
		}
	}
}
//////////////////////////////////////////////////////////////////////
// Timer handler

void CPsycleWTLView::OnTimer( UINT nIDEvent,TIMERPROC pTimerProc )
{
	if (nIDEvent == 31)
	{
		
		CComCritSecLock<CComAutoCriticalSection> lock(_pSong->Door(),TRUE);

		if (Global::_pSong->pMachine(MASTER_INDEX))
		{

			pMainFrame->UpdateVumeters(
	//			((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_LMAX,
	//			((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_RMAX,
				((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_lMax,
				((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_rMax,
				Global::pConfig->vu1,
				Global::pConfig->vu2,
				Global::pConfig->vu3,
				((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_clip);

			if ( MasterMachineDialog )
			{
				if (!--((Master*)Global::_pSong->pMachine(MASTER_INDEX))->peaktime) 
				{
					TCHAR peak[10];
					float _current_peak = ((Master *)Global::_pSong->pMachine(MASTER_INDEX))->currentpeak == 0.0f?0.0f:log10f(((Master *)Global::_pSong->pMachine(MASTER_INDEX))->currentpeak);
					_stprintf(peak,_T("%.2fdB"),20.0f * _current_peak - 90.0f);
					MasterMachineDialog->Masterpeak().SetWindowText(peak);
	//				MasterMachineDialog->m_slidermaster.SetPos(256-((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_outDry);

					float val = sqrtf(((Master*)Global::_pSong->pMachine(MASTER_INDEX))->_outDry * 64.0f);
					MasterMachineDialog->m_slidermaster.SetPos(256 - f2i(val));
					
					((Master*)Global::_pSong->pMachine(MASTER_INDEX))->peaktime = 25;
					((Master*)Global::_pSong->pMachine(MASTER_INDEX))->currentpeak = 0.0f;
				}
			}
			((Master*)Global::_pSong->pMachine(MASTER_INDEX))->vuupdated = true;
		}
		
		if (viewMode == VMMachine)
		{
			CClientDC dc(*this);
			DrawAllMachineVumeters(&dc);
		}

		if (Global::_pSong->IsTweaking())
		{
			for(int c = 0; c < MAX_MACHINES; c++)
			{
				if (_pSong->pMachine(c))
				{
					if (pMainFrame->isguiopen[c])
					{
						if ( _pSong->pMachine(c)->_type == MACH_PLUGIN )
						{
							pMainFrame->m_pWndMac[c]->Invalidate(false);
						}
						else if ( _pSong->pMachine(c)->_type == MACH_VST ||
								_pSong->pMachine(c)->_type == MACH_VSTFX )
						{
							((CVstEditorDlg*)pMainFrame->m_pWndMac[c])->Refresh(-1,0);
						}
	/*					else
						{
						}
	*/
					}
				}
			}
			Global::_pSong->IsTweaking(false);
		}

		if (Global::pPlayer->_playing)
		{
			if (Global::pPlayer->_lineChanged)
			{
				Global::pPlayer->_lineChanged = false;
				pMainFrame->SetAppSongBpm(0);
				pMainFrame->SetAppSongTpb(0);

				if (Global::pConfig->_followSong)
				{
					CListBox _seqlist((HWND)pMainFrame->m_wndSeq.GetDlgItem(IDC_SEQLIST));
					editcur.line=Global::pPlayer->_lineCounter;
					
					if (editPosition != Global::pPlayer->_playPosition)
//					if (_seqlist->GetCurSel() != Global::pPlayer->_playPosition)
					{
						_seqlist.SelItemRange(false,0,_seqlist.GetCount());
						_seqlist.SetSel(Global::pPlayer->_playPosition,true);
						editPosition=Global::pPlayer->_playPosition;
						if ( viewMode == VMPattern ) Repaint(DMPattern);//DMPlaybackChange);  // Until this mode is coded there is no point in calling it since it just makes patterns not refresh correctly currently
					}
					else if( viewMode == VMPattern ) Repaint(DMPlayback);
				}
				else if ( viewMode == VMPattern ) Repaint(DMPlayback);
			}
		}
	}
	if (nIDEvent == 159 && !Global::pPlayer->_recording)
	{
//		MessageBox("Saving Disabled");
//		return;
		CString filepath = Global::pConfig->GetInitialSongDir();
		filepath += _T("\\autosave.psy");
		OldPsyFile file;
		if (!file.Create(filepath.GetBuffer(1), true))
		{
			return;
		}
		std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
		_saver->Save(file,*_pSong);
//		_pSong->Save(&file);
//		file.Close(); <- save now handles this
	}
}


void CPsycleWTLView::EnableSound()
{
	if (_outputActive)
	{
		_outputActive = false;
		if (!Global::pConfig->_pOutputDriver->Initialized())
		{
			Global::pConfig->_pOutputDriver->Initialize(m_hWnd, Global::pPlayer->Work, Global::pPlayer);
		}
		if (!Global::pConfig->_pOutputDriver->Configured())
		{
			Global::pConfig->_pOutputDriver->Configure();
			if (Global::pPlayer->_playing)
			{
				Global::_pSong->SamplesPerTick(
					(Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)
					/(Global::pPlayer->bpm*Global::pPlayer->tpb));
			}
			else
			{
				Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin(), 
					Global::_pSong->TicksPerBeat(), 
					Global::pConfig->_pOutputDriver->_samplesPerSec);
			}
			_outputActive = true;
		}
		if (Global::pConfig->_pOutputDriver->Enable(true))
		{
			_outputActive = true;
		}
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Open();

		// set midi input mode to real-time or step
		if(Global::pConfig->_midiMachineViewSeqMode)
			CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
		else
			CMidiInput::Instance()->m_midiMode = MODE_STEP;
	}
	
}


//////////////////////////////////////////////////////////////////////
// Put exit destroying code here...

void CPsycleWTLView::OnDestroy()
{
	if (Global::pConfig->_pOutputDriver->Initialized())
	{
		Global::pConfig->_pOutputDriver->Reset();
	}
	KillTimer(31);
	KillTimer(159);

}



//////////////////////////////////////////////////////////////////////
// Include GDI sources

#include "machineview.cpp"
#include "patviewnew.cpp"


void CPsycleWTLView::Repaint(int drawMode)
{
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("Repaint()\n"));
#endif
	if ( viewMode == VMMachine )
	{
		if ( drawMode <= DMMacRefresh )
		{
			updateMode = drawMode;
			Invalidate(false);
		}
	}
	else if ( viewMode == VMPattern )
	{
		if (drawMode >= DMPattern || drawMode == DMAll )	
		{
			PreparePatternRefresh(drawMode);
		}
	}
}

void CPsycleWTLView::OnSize(UINT nType, CSize & size) 
{
	//CWnd ::OnSize(nType, cx, cy);
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("OnSize()\n"));
#endif
	CW = size.cx;
	CH = size.cy;
	_pSong->ViewSizeX(size.cx); // Hack to move machines boxes inside of the visible area.
	_pSong->ViewSizeY(size.cy);
	
	if ( bmpDC != NULL && Global::pConfig->useDoubleBuffer ) // remove old buffer to force recreating it with new size
	{
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("CPsycleWTLView::OnResize(). Delete bmpDC"));
#endif
		bmpDC->DeleteObject();
		delete bmpDC;
		bmpDC = NULL;
	}
	if (viewMode == VMPattern)
	{
		RecalcMetrics();
	}

	if(!m_bPrepare)
		Repaint();
	
	SetMsgHandled(FALSE);
}

//////////////////////////////////////////////////////////////////////
// "Save Song" Function

BOOL CPsycleWTLView::OnFileSave() 
{
//	MessageBox("Saving Disabled");
//	return false;
	BOOL bResult = TRUE;
	if ( Global::_pSong->IsSaved() )
	{
		if (MessageBox(SF::CResourceString(IDS_MSG0016),SF::CResourceString(IDS_MSG0017),MB_YESNO) == IDYES)
		{
			SF::string filepath = Global::pConfig->GetSongDir();
			filepath.append(_T("\\"));
			filepath.append(Global::_pSong->FileName());
			
			OldPsyFile file;
			
			if (!file.Create(filepath.data(), true))
			{
				MessageBox(
					SF::CResourceString(IDS_ERR_MSG0062),
					SF::CResourceString(IDS_ERR_MSG0043),
					MB_OK);
				return FALSE;
			}
			std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
			try {
				_saver->Save(file,*_pSong);
			} catch (std::exception e) {
				MessageBox(CA2T(e.what()), SF::CResourceString(IDS_ERR_MSG0043) /* Error*/, MB_OK);
				bResult = FALSE;
				return FALSE;
			}
	
			_pSong->IsSaved(true);
			
			if (pUndoList)
			{
				UndoSaved = pUndoList->counter;
			}
			else
			{
				UndoSaved = 0;
			}

			UndoMacSaved = UndoMacCounter;
			SetTitleBarText();
//			file.Close();  <- save handles this 
		}
		else 
		{
			return FALSE;
		}
	}
	else 
	{
		return OnFileSaveAs();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////
// "Save Song As" Function
BOOL CPsycleWTLView::OnFileSaveAs() 
{
//	MessageBox("Saving Disabled");
//	return false;

	OPENFILENAME ofn;       // common dialog box structure
	SF::string ifile(Global::_pSong->FileName());
	
	//CString if2 = ifile.SpanExcluding(_T("\\/:*\"<>|"));
	SF::string delm = _T("\\/:*\"<>|");
	
	int index = std::string::npos;
	
	for(int i = 0;i < static_cast<int>(ifile.length());i++)
	{
		if(delm.find(ifile[i]) != std::string::npos)
		{
			index = i;		
			break;
		}
	}

	SF::string if2;
	
	if(index != std::string::npos)
	{
		if2 = ifile.substr(0,index);
	} else {
		if2 = ifile;
	}
		
	
	TCHAR szFile[_MAX_PATH];

	_tcscpy(szFile,if2.c_str()); 
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	SF::CResourceString _filterstr(IDS_MSG0021);
	boost::scoped_array<TCHAR> _pfilter(new TCHAR[_filterstr.Length() + 2 ]);
	memcpy(_pfilter.get(),_filterstr,_filterstr.Length() * sizeof(TCHAR));
	_pfilter[_filterstr.Length() - 1] = 0;
	_pfilter[_filterstr.Length()] = 0;
	ofn.lpstrFilter = _pfilter.get();
	CString _filter(MAKEINTRESOURCE(IDS_MSG0021));
	_filter.Append(_T("\0\0\0\0"));
	ofn.lpstrFilter = _filter;

	//_T("Songs (*.psf)\0*.psf\0Psycle Pattern (*.psb)\0*.psb\0All (*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Global::pConfig->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	BOOL bResult = TRUE;
	
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn) == TRUE)
	{
		SF::string str = ofn.lpstrFile;
		SF::string str2 = str.substr(str.length() - 4,4);// Right(4);

		_tcslwr(const_cast<TCHAR *>(str2.c_str()));

		if ( ofn.nFilterIndex == 2) 
		{

			if (str2.compare(_T(".psb")) != 0 )
			{
				str += _T(".psb");
			}
			
			_stprintf(szFile,str.c_str());
			
			FILE* hFile = _tfopen(szFile,_T("wb"));
			SaveBlock(hFile);
			fflush(hFile);
			fclose(hFile);
		}
		else 
		{ 
			if ( str2.compare(_T(".psf")) != 0 )
				str += _T(".psf");

			SetSongDirAndFileName(str.c_str(),_T(""));
			

			OldPsyFile file;

			if (!file.Create(str.c_str(), true))
			{
				MessageBox(
					SF::CResourceString(IDS_ERR_MSG0062),
					SF::CResourceString(IDS_ERR_MSG0043), MB_OK);
				return FALSE;
			}
			
			std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
			try {
				_saver->Save(file,*_pSong);
			} catch (std::exception e ){
				MessageBox(CA2T(e.what()), SF::CResourceString(IDS_ERR_MSG0043), MB_OK);
				return FALSE;

			}

			_pSong->IsSaved(true);
			AppendToRecent(str.c_str());
			
			if (pUndoList)
			{
				UndoSaved = pUndoList->counter;
			}
			else
			{
				UndoSaved = 0;
			}
			UndoMacSaved = UndoMacCounter;
			SetTitleBarText();
		}
	}
	else
	{
		return FALSE;
	}
	return bResult;
}

void CPsycleWTLView::OnFileLoadsong()
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]=_T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	SF::CResourceString _filter(IDS_MSG0022);
	boost::scoped_array<TCHAR> _pfilter(new TCHAR[ _filter.Length() + 2 ]);
	memcpy(_pfilter.get(),_filter,_filter.Length() * sizeof(TCHAR));
	
	_pfilter[_filter.Length() - 1] = 0;
	_pfilter[_filter.Length()] = 0;

	ofn.lpstrFilter = _pfilter.get();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Global::pConfig->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
	}
	pMainFrame->StatusBarIdle();
}

void CPsycleWTLView::OnFileNew() 
{
	SF::CResourceString _newFile(IDS_MSG0023);
	if (CheckUnsavedSong(_newFile))
	{
		KillUndo();
		KillRedo();
		pMainFrame->CloseAllMacGuis();
		Global::pPlayer->Stop();
		Sleep(LOCK_LATENCY);
		_outputActive = false;
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();
		Sleep(LOCK_LATENCY);

		Global::_pSong->New();
		_outputActive = true;
		if (!Global::pConfig->_pOutputDriver->Enable(true))
		{
			_outputActive = false;
		}
		else
		{
			// MIDI IMPLEMENTATION
			Global::pConfig->_pMidiInput->Open();
		}
		SetTitleBarText();
		editPosition=0;
		Global::_pSong->SeqBus(0);
		pMainFrame->PsybarsUpdate(); // Updates all values of the bars
		pMainFrame->WaveEditorBackUpdate();
		pMainFrame->m_wndInst.WaveUpdate();
		pMainFrame->RedrawGearRackList();
		pMainFrame->UpdateSequencer();
		pMainFrame->UpdatePlayOrder(false); // should be done always after updatesequencer
//		pMainFrame->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
		RecalculateColourGrid();
		Repaint();
	}
	pMainFrame->StatusBarIdle();
}


void CPsycleWTLView::OnFileSaveaudio() 
{
	OnBarstop();
	KillTimer(31);
	KillTimer(159);
	OnTimer(159); // Autosave
	
	CSaveWavDlg dlg;
	dlg.DoModal();

	InitTimer();
}

BOOL CPsycleWTLView::CheckUnsavedSong(TCHAR* szTitle)
{
	// that method does not take machine changes into account
	BOOL bChecked = TRUE;
	if (pUndoList)
	{
		if (UndoSaved != pUndoList->counter)
		{
			bChecked = FALSE;
		}
	}
	else if (UndoMacSaved != UndoMacCounter)
	{
		bChecked = FALSE;
	}
	else
	{
		if (UndoSaved != 0)
		{
			bChecked = FALSE;
		}
	}
	
	if (!bChecked)
	{
		if (Global::pConfig->bFileSaveReminders)
		{
			//std::stringstream _message;
			SF::string filepath = Global::pConfig->GetSongDir();
			
			filepath += _T("\\");
			filepath += Global::_pSong->FileName();
			
			OldPsyFile file;
			
			//_message << SF::tformat(_T("\'%1%\' への変更を保存しますか?")) % Global::_pSong->FileName();

			//_stprintf(szText,_T("\'%s\' への変更を保存しますか?"),Global::_pSong->FileName().c_str());
			
			int result = MessageBox(
				(SF::tformat(SF::CResourceString(IDS_MSG0018)) % Global::_pSong->FileName()).str().data(),szTitle,MB_YESNOCANCEL | MB_ICONEXCLAMATION);
			
			switch (result)
			{
			case IDYES:
				{
				if (!file.Create(filepath.c_str(), true))
				{
					MessageBox(
						(SF::tformat(SF::CResourceString(IDS_ERR_MSG0051)) % filepath.c_str()).str().c_str(),
						szTitle,MB_ICONEXCLAMATION);
					return FALSE;
				}

					std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
					_saver->Save(file,*_pSong);
				}
				return TRUE;
				break;
			case IDNO:
				return TRUE;
				break;
			case IDCANCEL:
				return FALSE;
				break;
			}
		}
	}
	return TRUE;
}

void CPsycleWTLView::OnFileRevert()
{
	if (MessageBox(SF::CResourceString(IDS_MSG0019),SF::CResourceString(IDS_MSG0020),MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
	{
		if (Global::_pSong->IsSaved())
		{
			FileLoadsongNamed(
				(SF::tformat(_T("%s\\%s")) % Global::pConfig->GetSongDir() % Global::_pSong->FileName()).str().data()
			);
		}
	}
	pMainFrame->StatusBarIdle();
}


void CPsycleWTLView::OnBarplay() 
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	prevEditPosition=editPosition;
	Global::pPlayer->Start(editPosition,0);
	pMainFrame->StatusBarIdle();
}

void CPsycleWTLView::OnBarplayFromStart() 
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	prevEditPosition=editPosition;
	Global::pPlayer->Start(0,0);
	pMainFrame->StatusBarIdle();
}

void CPsycleWTLView::OnBarrec() 
{
	if (Global::pConfig->_followSong && bEditMode)
	{
		bEditMode = FALSE;
	}
	else
	{
		Global::pConfig->_followSong = TRUE;
		bEditMode = TRUE;
		CButton cb(pMainFrame->m_wndSeq.GetDlgItem(IDC_FOLLOW));
		cb.SetCheck(1);
	}
	pMainFrame->StatusBarIdle();
}

void CPsycleWTLView::OnButtonplayseqblock() 
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}

	prevEditPosition=editPosition;
	int i=0;
	while ( Global::_pSong->PlayOrderSel(i) == false ) i++;
	
	if(!Global::pPlayer->_playing)
		Global::pPlayer->Start(i,0);

	Global::pPlayer->_playBlock=!Global::pPlayer->_playBlock;

	pMainFrame->StatusBarIdle();
	if ( viewMode == VMPattern ) Repaint(DMPattern);
}

void CPsycleWTLView::OnBarstop()
{
	bool pl = Global::pPlayer->_playing;
	bool blk = Global::pPlayer->_playBlock;
	Global::pPlayer->Stop();
	pMainFrame->SetAppSongBpm(0);
	pMainFrame->SetAppSongTpb(0);

	if (pl)
	{
		if ( Global::pConfig->_followSong && blk)
		{
			editPosition=prevEditPosition;
			pMainFrame->UpdatePlayOrder(false); // <- This restores the selected block
			Repaint(DMPattern);
		}
		else
		{
			for(int i = 0; i < MAX_SONG_POSITIONS;i++){
				Global::_pSong->PlayOrderSel(i,false);
			}
			
			Global::_pSong->PlayOrderSel(editPosition,true);
			Repaint(DMCursor); 
		}
	}
}

void CPsycleWTLView::OnRecordWav() 
{
	if (!Global::pPlayer->_recording)
	{
		CFileDialog dlg(false,_T("wav"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,RES_STR(IDS_FILTER_WAV));
		if ( dlg.DoModal() == IDOK ) 
		{
			Global::pPlayer->StartRecording(dlg.m_szFileName);
		}
		if ( Global::pConfig->autoStopMachines ) 
		{
			OnAutostop();
		}
	}
	else
	{
		Global::pPlayer->StopRecording();
	}
}

void CPsycleWTLView::OnAutostop() 
{
	if ( Global::pConfig->autoStopMachines )
	{
		Global::pConfig->autoStopMachines = false;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if (Global::_pSong->pMachine(c))
			{
				Global::_pSong->pMachine(c)->_stopped=false;
			}
		}
	}
	else Global::pConfig->autoStopMachines = true;
}
/////////////////////
//

void CPsycleWTLView::OnFileSongproperties() 
{	
	CSongpDlg dlg;
	dlg._pSong=Global::_pSong;
	dlg.DoModal();
	pMainFrame->StatusBarIdle();
//	Repaint();
}

void CPsycleWTLView::OnViewInstrumenteditor()
{
	pMainFrame->ShowInstrumentEditor();
}


void CPsycleWTLView::ShowPatternDlg(void)
{
	CPatDlg dlg;
	int patNum = _pSong->PlayOrder(editPosition);
	int nlines = _pSong->PatternLines(patNum);
	TCHAR name[32];
	_tcscpy(name,_pSong->PatternName(patNum));

	dlg.patLines= nlines;
	_tcscpy(dlg.patName,name);
	
	if (dlg.DoModal() == IDOK)
	{
		if ( nlines != dlg.patLines )
		{
			AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
			AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
			_pSong->AllocNewPattern(patNum,dlg.patName,dlg.patLines,dlg.m_adaptsize?true:false);
			if ( _tcscmp(name,dlg.patName) != 0 )
			{
				_tcscpy(_pSong->PatternName(patNum),dlg.patName);
				pMainFrame->StatusBarIdle();
			}
			Repaint();
		}
		else if ( _tcscmp(name,dlg.patName) != 0 )
		{
			_tcscpy(_pSong->PatternName(patNum),dlg.patName);
			pMainFrame->StatusBarIdle();
//			Repaint(DMPatternHeader);
		}
	}

}

void CPsycleWTLView::OnNewmachine() 
{
	NewMachine();
}

void CPsycleWTLView::NewMachine(int x, int y, int mac) 
{
	// Show new machine dialog
	CNewMachine dlg;

	if (mac >= 0)
	{
		if (mac < MAX_BUSES)
		{
			dlg.LastType1 = 0;
		}
		else
		{
			dlg.LastType1 = 1;
		}
	}
	
	if ((dlg.DoModal() == IDOK) && (dlg.Outputmachine >= 0))
	{
		int fb,xs,ys;
		if (mac < 0)
		{
			if (dlg.OutBus) 
			{
				fb = Global::_pSong->GetFreeBus();
				xs = MachineCoords.sGenerator.width;
				ys = MachineCoords.sGenerator.height;
			}
			else 
			{
				fb = Global::_pSong->GetFreeFxBus();
				xs = MachineCoords.sEffect.width;
				ys = MachineCoords.sEffect.height;
			}
		}
		else
		{
			if ((mac >= MAX_BUSES) && !(dlg.OutBus))
			{
				fb = mac;
				xs = MachineCoords.sEffect.width;
				ys = MachineCoords.sEffect.height;
				// delete machine if it already exists
				if (Global::_pSong->pMachine(fb))
				{
					x = Global::_pSong->pMachine(fb)->_x;
					y = Global::_pSong->pMachine(fb)->_y;
					pMainFrame->CloseMacGui(fb);
					Global::_pSong->DestroyMachine(fb);
				}
			}
			else if ((mac < MAX_BUSES) && (dlg.OutBus))
			{
				fb = mac;
				xs = MachineCoords.sGenerator.width;
				ys = MachineCoords.sGenerator.height;
				// delete machine if it already exists
				if (Global::_pSong->pMachine(fb))
				{
					x = Global::_pSong->pMachine(fb)->_x;
					y = Global::_pSong->pMachine(fb)->_y;
					pMainFrame->CloseMacGui(fb);
					Global::_pSong->DestroyMachine(fb);
				}
			}
			else
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0068));
				return;
			}
		}


		AddMacViewUndo();

		// random position
		if ((x < 0) || (y < 0))
		{
			bool bCovered = TRUE;
			while (bCovered)
			{
				x = (rand())%(CW-xs);
				y = (rand())%(CH-ys);
				bCovered = FALSE;
				for (int i=0; i < MAX_MACHINES; i++)
				{
					if (Global::_pSong->pMachine(i))
					{
						if ((abs(Global::_pSong->pMachine(i)->_x - x) < 32) &&
							(abs(Global::_pSong->pMachine(i)->_y - y) < 32))
						{
							bCovered = TRUE;
							i = MAX_MACHINES;
						}
					}
				}
			}
		}
		// Stop driver to handle possible conflicts between threads.
		// should be no conflicts because last thing create machine does is set active machine flag.
		// busses are set last, so no messages will be sent until after machine is created anyway
		/*
		_outputActive = false;
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();
		*/

		if ( fb == -1 || !Global::_pSong->CreateMachine((MachineType)dlg.Outputmachine, x, y, dlg.psOutputDll,fb))
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0069),SF::CResourceString(IDS_ERR_MSG0043),MB_OK);
		}
		else
		{
			if ( dlg.OutBus)
			{
				Global::_pSong->SeqBus(fb);
			}

			// make sure that no 2 machines have the same name, because that is irritating

			int number = 1;
			TCHAR buf[sizeof(_pSong->pMachine(fb)->_editName) + 4];
			_tcscpy(buf,_pSong->pMachine(fb)->_editName);

			for (int i = 0; i < MAX_MACHINES-1; i++)
			{
				if (i!=fb)
				{
					if (_pSong->pMachine(i))
					{
						if (_tcscmp(_pSong->pMachine(i)->_editName,buf)==0)
						{
							number++;
							_stprintf(buf,_T("%s %d"),_pSong->pMachine(fb)->_editName,number);
							i = -1;
						}
					}
				}
			}

			buf[sizeof(_pSong->pMachine(fb)->_editName)-1] = 0;
			_tcscpy(_pSong->pMachine(fb)->_editName,buf);

			pMainFrame->UpdateComboGen();
			Repaint(DMAllMacsRefresh);
//			Repaint(DMMacRefresh); // Seems that this doesn't always work (multiple calls to Repaint?)
		}
		
		/*
		// Restarting the driver...
		pMainFrame->UpdateEnvInfo();
		_outputActive = true;
		if (!Global::pConfig->_pOutputDriver->Enable(true))
		{
			_outputActive = false;
		}
		else
		{
			// MIDI IMPLEMENTATION
			Global::pConfig->_pMidiInput->Open();
		}
		*/
	}
	//	Repaint();
	pMainFrame->RedrawGearRackList();
}

void CPsycleWTLView::OnConfigurationSettings() 
{

	SF::CResourceString _title(IDS_MSG0024);

	CConfigDlg dlg(ATL::_U_STRINGorID((TCHAR*)_title));

	_outputActive = false;
	
	dlg.Init(Global::pConfig);

	if (dlg.DoModal() == IDOK)
	{
		KillTimer(159);
		if ( Global::pConfig->autosaveSong )
		{
			SetTimer(159,Global::pConfig->autosaveSongTime*60000,NULL);
		}
		_outputActive = true;
		EnableSound();
	}
//	Repaint();
}

void CPsycleWTLView::OnHelpSaludos() 
{
	CGreetDialog dlg;
	dlg.DoModal();
//	Repaint();
}

int CPsycleWTLView::SongIncBpm(int x)
{
	AddMacViewUndo();
	return Controller::Instance().Song().IncBpm(x);
}


#define TWOPI_F (2.0f*3.141592665f)

void CPsycleWTLView::ShowSwingFillDlg(bool bTrackMode)
{
	int st = Global::_pSong->BeatsPerMin();
	static int sw = 2;
	static float sv = 50.0f;
	static float sp = 90.0f;
	static BOOL of = true;
	CSwingFillDlg dlg;
	dlg.tempo = st;
	dlg.width = sw;
	dlg.variance = sv;
	dlg.phase = sp;
	dlg.offset = true;

	dlg.DoModal();
	if (dlg.bGo)
	{
		st = dlg.tempo;
		sw = dlg.width;
		sv = dlg.variance;
		sp = dlg.phase;
		of = dlg.offset;
		float var = (sv/100.0f);

		// time to do our fill
		// first some math
		// our range has to go from spd+var to spd-var and back in width+1 lines
		float step = TWOPI_F/(sw);
		float index = sp*TWOPI_F/360;

		int l;
		int x;
		int y;
		int ny;
		if (bTrackMode)
		{
			x = editcur.track;
			y = 0;
			ny = _pSong->PatternLines(_ps());
		}
		else
		{
			x = blockSel.start.track;
			y = blockSel.start.line;
			ny = 1+blockSel.end.line-blockSel.start.line;
		}

		// remember we are at each speed for the length of time it takes to do one tick
		// this approximately calculates the offset
		float dcoffs = 0;
		if (of)
		{
			float swing=0;
			for (l=0;l<sw;l++)
			{
				float val = ((sinf(index)*var*st)+st);
				swing += (val/st)*(val/st);
				index+=step;
			}
			dcoffs = ((swing-sw)*st)/sw;
		}

		// now fill the pattern
		unsigned char *base = _ppattern();
		if (base)
		{
			AddUndo(_ps(),x,y,1,ny,editcur.track,editcur.line,editcur.col,editPosition);
			for (l=y;l<y+ny;l++)
			{
				int const displace=x*EVENT_SIZE+l*MULTIPLY;
				
				unsigned char *offset=base+displace;
				
				PatternEntry *entry = (PatternEntry*) offset;
				entry->_cmd = 0xff;
				int val = f2i(((sinf(index)*var*st)+st)+dcoffs);//-0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
				if (val < 1)
				{
					val = 1;
				}
				else if (val > 255)
				{
					val = 255;
				}
				entry->_parameter = unsigned char (val);
				index+=step;
			}
			NewPatternDraw(x,x,y,y+ny);	
			Repaint(DMData);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Right Click Popup Menu

void CPsycleWTLView::OnPopCut() { CopyBlock(true); }
void CPsycleWTLView::OnPopCopy() { CopyBlock(false); }
void CPsycleWTLView::OnPopPaste() { PasteBlock(editcur.track,editcur.line,false); }
void CPsycleWTLView::OnPopMixpaste() { PasteBlock(editcur.track,editcur.line,true); }

void CPsycleWTLView::OnPopDelete() { DeleteBlock(); }

void CPsycleWTLView::OnPopInterpolate() { BlockParamInterpolate(); }

void CPsycleWTLView::OnPopChangegenerator() { BlockGenChange(_pSong->SeqBus()); }

void CPsycleWTLView::OnPopChangeinstrument() { BlockInsChange(_pSong->AuxcolSelected()); }

void CPsycleWTLView::OnPopTranspose1() { BlockTranspose(1); }

void CPsycleWTLView::OnPopTranspose12() { BlockTranspose(12); }

void CPsycleWTLView::OnPopTranspose_1() { BlockTranspose(-1); }

void CPsycleWTLView::OnPopTranspose_12() { BlockTranspose(-12); }

void CPsycleWTLView::OnPopPattenproperties() 
{
	ShowPatternDlg();
}

void CPsycleWTLView::OnPopBlockSwingfill()
{
	// fill block
	ShowSwingFillDlg(FALSE);
}

void CPsycleWTLView::OnPopTrackSwingfill()
{
	// fill track
	ShowSwingFillDlg(TRUE);
}



//
void CPsycleWTLView::SetSongDirAndFileName(const TCHAR *  pFilePath,const TCHAR * extention)
{	
	SF::string str = pFilePath;
	int index = str.find_last_of(_T('\\'));
	if (index != std::string::npos)
	{
		Global::pConfig->SetSongDir(str.substr(0,index + 1).c_str());
		str = str.substr(index + 1,str.length() - index - 1) ;
		str += extention;
		Global::_pSong->FileName(str);
	}
	else
	{
		Global::_pSong->FileName(str + extention);
	}
}

void CPsycleWTLView::AppendToRecent(const TCHAR* fName)
{
	int iCount;
	TCHAR* nameBuff;
	UINT nameSize;

	HMENU hFileMenu, hRootMenuBar;
		
	UINT ids[] = {ID_FILE_RECENT_01,
				ID_FILE_RECENT_02,
				ID_FILE_RECENT_03,
				ID_FILE_RECENT_04};

	MENUITEMINFO hNewItemInfo, hTempItemInfo;

	hRootMenuBar = CMainFrame::GetInstance().m_CmdBar.GetMenu();
//	::GetMenu(GetParent());
//	pRootMenuBar = this->GetParent()->GetMenu();
//	hRootMenuBar = HMENU (*pRootMenuBar);
	hFileMenu = GetSubMenu(hRootMenuBar, 0);
	hRecentMenu = GetSubMenu(hFileMenu, 9);

	//Remove initial empty element, if present.
	if (GetMenuItemID(hRecentMenu, 0) == ID_FILE_RECENT_NONE)
	{
		DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
	}

	//Check for duplicates and eventually remove.
	for (iCount = 0; iCount<GetMenuItemCount(hRecentMenu);iCount++)
	{
		nameSize = GetMenuString(hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
		nameBuff = new TCHAR[nameSize];
		GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
		if ( !_tcscmp(nameBuff, fName) )
		{
			DeleteMenu(hRecentMenu, iCount, MF_BYPOSITION);
		}
		delete nameBuff;
	}
	
	//Ensure menu size doesn't exceed 4 positions.
	if (GetMenuItemCount(hRecentMenu) == 4)
	{
		DeleteMenu(hRecentMenu, 4-1, MF_BYPOSITION);
	}

	hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
	hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
	hNewItemInfo.fType		= MFT_STRING;
	hNewItemInfo.wID		= ids[0];
	hNewItemInfo.cch		= _tcslen(fName);
	hNewItemInfo.dwTypeData = const_cast<TCHAR *>(fName);
	
	
	BOOL _result = InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);
	
	ATLASSERT(_result == TRUE);

	//Update identifiers.
	for (iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
	{
		hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
		hTempItemInfo.fMask		= MIIM_ID;
		hTempItemInfo.wID		= ids[iCount];

		SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
	}

}

void CPsycleWTLView::OnFileRecent_01()
{
	CallOpenRecent(0);
}
void CPsycleWTLView::OnFileRecent_02()
{
	CallOpenRecent(1);
}
void CPsycleWTLView::OnFileRecent_03()
{
	CallOpenRecent(2);
}
void CPsycleWTLView::OnFileRecent_04()
{
	CallOpenRecent(3);
}

void CPsycleWTLView::OnFileLoadsongNamed(TCHAR* fName, int fType)
{
	if ( fType == 3 )
	{
		FILE* hFile = _tfopen(fName,_T("rb"));
		LoadBlock(hFile);
		fclose(hFile);
	}
	else
	{
		if (CheckUnsavedSong(SF::CResourceString(IDS_MSG0025)))
		{
			FileLoadsongNamed(fName);
		}
	}
}

// ファイルロード前の初期化処理
void CPsycleWTLView::InitializeBeforeLoading()
{
	KillUndo();
	KillRedo();

	pMainFrame->CloseAllMacGuis();
	Global::pPlayer->Stop();
	Sleep(LOCK_LATENCY);
	_outputActive = false;
	Global::pConfig->_pOutputDriver->Enable(false);
	// MIDI IMPLEMENTATION
	Global::pConfig->_pMidiInput->Close();
	Sleep(LOCK_LATENCY);
}

void CPsycleWTLView::ProcessAfterLoading()
{

	Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin(),
		Global::_pSong->TicksPerBeat(),
		Global::pConfig->_pOutputDriver->_samplesPerSec);

	_outputActive = true;
	
	if (!Global::pConfig->_pOutputDriver->Enable(true))
	{
		_outputActive = false;
	}
	else
	{
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Open();
	}

	editPosition = 0;
	
	pMainFrame->PsybarsUpdate();
	pMainFrame->WaveEditorBackUpdate();
	pMainFrame->m_wndInst.WaveUpdate();
	pMainFrame->RedrawGearRackList();
	pMainFrame->UpdateSequencer();
	pMainFrame->UpdatePlayOrder(false);
//			pMainFrame->UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
	RecalculateColourGrid();
	Repaint();
	SetTitleBarText();

}

void CPsycleWTLView::FileLoadsongNamed(const TCHAR* fName)
{
	
	WTL::CFindFile file;
	if(!file.FindFile(fName)){
		MessageBox(SF::CResourceString(IDS_ERR_MSG0063),
			SF::CResourceString(IDS_ERR_MSG0064), MB_OK);
	}

	SF::string f = fName;
	SF::string ext = f.substr(f.find_last_of(_T(".")) + 1,f.length() - (f.find_last_of(_T(".")) + 1));
	_tcslwr(const_cast<TCHAR *>(ext.c_str()));
	SongLoaderMap::iterator it = m_SongLoaderMap.find(ext);
	if(it == m_SongLoaderMap.end())
	{
		MessageBox(
			(SF::tformat(SF::CResourceString(IDS_ERR_MSG0065)) % fName).str().c_str());
		return;
	}
	
	InitializeBeforeLoading();

	try {
		it->second->Load(SF::string(fName),*_pSong);
	} catch(std::exception e) {
		MessageBox((SF::tformat(SF::CResourceString(IDS_ERR_MSG0066)) % CA2T(e.what())).str().c_str());
		Global::_pSong->New();
		return;
	}

	//!Fidelooop!!//
	AppendToRecent(fName);
	
	if(ext == _T("psf")){
		ext = _T("");
	} else {
		ext = _T(".psf");
	}

	SetSongDirAndFileName(fName,ext.c_str());

	ProcessAfterLoading();

	if (Global::pConfig->bShowSongInfoOnLoad)
	{
		TCHAR buffer[512];
		_stprintf(buffer,_T("'%s'\n\n%s\n\n%s")
			,_pSong->Name().c_str()
			,_pSong->Author().c_str()
			,_pSong->Comment().c_str());
		
		MessageBox(buffer,SF::CResourceString(IDS_ERR_MSG0067),MB_OK);
	}

}

void CPsycleWTLView::CallOpenRecent(int pos)
{
	TCHAR* nameBuff;
	UINT nameSize;

	nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
	nameBuff = new TCHAR[nameSize];
	GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
	
	OnFileLoadsongNamed(nameBuff, 1);
	delete nameBuff;
}

void CPsycleWTLView::SetTitleBarText()
{
	SF::string titlename(_T("["));
	titlename += Global::_pSong->FileName();

/*	if (!(Global::_pSong->_saved))
	{
		titlename+=_T(" *");
	}
	else */ 
	if (pUndoList)
	{
		if (UndoSaved != pUndoList->counter)
		{
			titlename.append(_T(" *"));
		}
	}
	else if (UndoMacSaved != UndoMacCounter)
	{
		titlename.append(_T(" *"));
	}
	else
	{
		if (UndoSaved != 0)
		{
			titlename.append(_T(" *"));
		}
	}
	// I don't know how to access to the
	titlename.append(SF::CResourceString(IDS_MSG0026));	
	pMainFrame->SetWindowText(titlename.c_str());				// IDR_MAINFRAME String Title.
}


void CPsycleWTLView::OnHelpKeybtxt() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPKEYB),Global::pConfig->appPath);
	::ShellExecute(pMainFrame->m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CPsycleWTLView::OnHelpReadme() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPREADME),Global::pConfig->appPath);
	::ShellExecute(pMainFrame->m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CPsycleWTLView::OnHelpTweaking() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPTWEAK),Global::pConfig->appPath);
	::ShellExecute(pMainFrame->m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CPsycleWTLView::OnHelpWhatsnew() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPWHATSNEW),Global::pConfig->appPath);
	::ShellExecute(pMainFrame->m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

//#define _UGLY_DEFAULT_SKIN_

void CPsycleWTLView::LoadMachineSkin()
{
	static TCHAR szOld[64] = _T("");
	if (_tcscmp(szOld, Global::pConfig->machine_skin))
	{
		_tcscpy(szOld, Global::pConfig->machine_skin);
		// ok so...
		if (_tcscmp(szOld, DEFAULT_MACHINE_SKIN))
		{
			BOOL result = FALSE;
			FindMachineSkin(Global::pConfig->GetInitialSkinDir(),Global::pConfig->machine_skin, &result);
			if (result)
			{
				return;
			}
		}
		// load defaults
		_tcscpy(szOld, DEFAULT_MACHINE_SKIN);
		// and coords
#ifdef _UGLY_DEFAULT_SKIN_
		MachineCoords.sMaster.x = 0;
		MachineCoords.sMaster.y = 0;
		MachineCoords.sMaster.width = 148;
		MachineCoords.sMaster.height = 48;

		MachineCoords.sGenerator.x = 0;
		MachineCoords.sGenerator.y = 48;
		MachineCoords.sGenerator.width = 148;
		MachineCoords.sGenerator.height = 48;
		MachineCoords.sGeneratorVu0.x = 0;
		MachineCoords.sGeneratorVu0.y = 144;
		MachineCoords.sGeneratorVu0.width = 6;
		MachineCoords.sGeneratorVu0.height = 5;
		MachineCoords.sGeneratorVuPeak.x = 96;
		MachineCoords.sGeneratorVuPeak.y = 144;
		MachineCoords.sGeneratorVuPeak.width = 6;
		MachineCoords.sGeneratorVuPeak.height = 5;
		MachineCoords.sGeneratorPan.x = 21;
		MachineCoords.sGeneratorPan.y = 149;
		MachineCoords.sGeneratorPan.width = 24;
		MachineCoords.sGeneratorPan.height = 9;
		MachineCoords.sGeneratorMute.x = 7;
		MachineCoords.sGeneratorMute.y = 149;
		MachineCoords.sGeneratorMute.width = 7;
		MachineCoords.sGeneratorMute.height = 7;
		MachineCoords.sGeneratorSolo.x = 14;
		MachineCoords.sGeneratorSolo.y = 149;
		MachineCoords.sGeneratorSolo.width = 7;
		MachineCoords.sGeneratorSolo.height = 7;

		MachineCoords.sEffect.x = 0;
		MachineCoords.sEffect.y = 96;
		MachineCoords.sEffect.width = 148;
		MachineCoords.sEffect.height = 48;
		MachineCoords.sEffectVu0.x = 0;
		MachineCoords.sEffectVu0.y = 144;
		MachineCoords.sEffectVu0.width = 6;
		MachineCoords.sEffectVu0.height = 5;
		MachineCoords.sEffectVuPeak.x = 96;
		MachineCoords.sEffectVuPeak.y = 144;
		MachineCoords.sEffectVuPeak.width = 6;
		MachineCoords.sEffectVuPeak.height = 5;
		MachineCoords.sEffectPan.x = 21;
		MachineCoords.sEffectPan.y = 149;
		MachineCoords.sEffectPan.width = 24;
		MachineCoords.sEffectPan.height = 9;
		MachineCoords.sEffectMute.x = 7;
		MachineCoords.sEffectMute.y = 149;
		MachineCoords.sEffectMute.width = 7;
		MachineCoords.sEffectMute.height = 7;
		MachineCoords.sEffectBypass.x = 0;
		MachineCoords.sEffectBypass.y = 149;
		MachineCoords.sEffectBypass.width = 7;
		MachineCoords.sEffectBypass.height = 12;

		MachineCoords.dGeneratorVu.x = 8;
		MachineCoords.dGeneratorVu.y = 3;
		MachineCoords.dGeneratorVu.width = 96;
		MachineCoords.dGeneratorVu.height = 0;
		MachineCoords.dGeneratorPan.x = 3;
		MachineCoords.dGeneratorPan.y = 35;
		MachineCoords.dGeneratorPan.width = 117;
		MachineCoords.dGeneratorPan.height = 0;
		MachineCoords.dGeneratorMute.x = 137;
		MachineCoords.dGeneratorMute.y = 4;
		MachineCoords.dGeneratorSolo.x = 137;
		MachineCoords.dGeneratorSolo.y = 17;
		MachineCoords.dGeneratorName.x = 10;
		MachineCoords.dGeneratorName.y = 12;

		MachineCoords.dEffectVu.x = 8;
		MachineCoords.dEffectVu.y = 3;
		MachineCoords.dEffectVu.width = 96;
		MachineCoords.dEffectVu.height = 0;
		MachineCoords.dEffectPan.x = 3;
		MachineCoords.dEffectPan.y = 35;
		MachineCoords.dEffectPan.width = 117;
		MachineCoords.dEffectPan.height = 0;
		MachineCoords.dEffectMute.x = 137;
		MachineCoords.dEffectMute.y = 4;
		MachineCoords.dEffectBypass.x = 137;
		MachineCoords.dEffectBypass.y = 15;
		MachineCoords.dEffectName.x = 10;
		MachineCoords.dEffectName.y = 12;
		MachineCoords.bHasTransparency = FALSE;
#else
		MachineCoords.sMaster.x = 0;
		MachineCoords.sMaster.y = 0;
		MachineCoords.sMaster.width = 148;
		MachineCoords.sMaster.height = 47;//48;

		MachineCoords.sGenerator.x = 0;
		MachineCoords.sGenerator.y = 47;//48;
		MachineCoords.sGenerator.width = 148;
		MachineCoords.sGenerator.height = 47;//48;
		MachineCoords.sGeneratorVu0.x = 0;
		MachineCoords.sGeneratorVu0.y = 141;//144;
		MachineCoords.sGeneratorVu0.width = 2;//6;
		MachineCoords.sGeneratorVu0.height = 4;//5;
		MachineCoords.sGeneratorVuPeak.x = 128;//96;
		MachineCoords.sGeneratorVuPeak.y = 141;//144;
		MachineCoords.sGeneratorVuPeak.width = 2;//6;
		MachineCoords.sGeneratorVuPeak.height = 4;//5;
		MachineCoords.sGeneratorPan.x = 45;//102;
		MachineCoords.sGeneratorPan.y = 145;//144;
		MachineCoords.sGeneratorPan.width = 16;//24;
		MachineCoords.sGeneratorPan.height = 5;//9;
		MachineCoords.sGeneratorMute.x = 0;//133;
		MachineCoords.sGeneratorMute.y = 145;//144;
		MachineCoords.sGeneratorMute.width = 15;//7;
		MachineCoords.sGeneratorMute.height = 14;//7;
		MachineCoords.sGeneratorSolo.x = 15;//140;
		MachineCoords.sGeneratorSolo.y = 145;//144;
		MachineCoords.sGeneratorSolo.width = 15;//7;
		MachineCoords.sGeneratorSolo.height = 14;//7;

		MachineCoords.sEffect.x = 0;
		MachineCoords.sEffect.y = 94;//96;
		MachineCoords.sEffect.width = 148;
		MachineCoords.sEffect.height = 47;//48;
		MachineCoords.sEffectVu0.x = 0;
		MachineCoords.sEffectVu0.y = 141;//144;
		MachineCoords.sEffectVu0.width = 2;//6;
		MachineCoords.sEffectVu0.height = 4;//5;
		MachineCoords.sEffectVuPeak.x = 128;//96;
		MachineCoords.sEffectVuPeak.y = 141;//144;
		MachineCoords.sEffectVuPeak.width = 2;//6;
		MachineCoords.sEffectVuPeak.height = 4;//5;
		MachineCoords.sEffectPan.x = 45;//102;
		MachineCoords.sEffectPan.y = 145;//144;
		MachineCoords.sEffectPan.width = 16;//24;
		MachineCoords.sEffectPan.height = 5;//9;
		MachineCoords.sEffectMute.x = 0;//133;
		MachineCoords.sEffectMute.y = 145;//144;
		MachineCoords.sEffectMute.width = 15;//7;
		MachineCoords.sEffectMute.height = 14;//7;
		MachineCoords.sEffectBypass.x = 30;//126;
		MachineCoords.sEffectBypass.y = 145;//144;
		MachineCoords.sEffectBypass.width = 15;//7;
		MachineCoords.sEffectBypass.height = 14;//13;

		MachineCoords.dGeneratorVu.x = 10;//8;
		MachineCoords.dGeneratorVu.y = 35;//3;
		MachineCoords.dGeneratorVu.width = 130;//96;
		MachineCoords.dGeneratorVu.height = 0;
		MachineCoords.dGeneratorPan.x = 39;//3;
		MachineCoords.dGeneratorPan.y = 26;//35;
		MachineCoords.dGeneratorPan.width = 91;//117;
		MachineCoords.dGeneratorPan.height = 0;
		MachineCoords.dGeneratorMute.x = 11;//137;
		MachineCoords.dGeneratorMute.y = 5;//4;
		MachineCoords.dGeneratorSolo.x = 26;//137;
		MachineCoords.dGeneratorSolo.y = 5;//17;
		MachineCoords.dGeneratorName.x = 49;//10;
		MachineCoords.dGeneratorName.y = 7;//12;

		MachineCoords.dEffectVu.x = 10;//8;
		MachineCoords.dEffectVu.y = 35;//3;
		MachineCoords.dEffectVu.width = 130;//96;
		MachineCoords.dEffectVu.height = 0;
		MachineCoords.dEffectPan.x = 39;//3;
		MachineCoords.dEffectPan.y = 26;//35;
		MachineCoords.dEffectPan.width = 91;//117;
		MachineCoords.dEffectPan.height = 0;
		MachineCoords.dEffectMute.x = 11;//137;
		MachineCoords.dEffectMute.y = 5;//4;
		MachineCoords.dEffectBypass.x = 26;//137;
		MachineCoords.dEffectBypass.y = 5;//15;
		MachineCoords.dEffectName.x = 49;//10;
		MachineCoords.dEffectName.y = 7;//12;
		MachineCoords.bHasTransparency = FALSE;
#endif
		if(!machineskin.IsNull())
			machineskin.DeleteObject();
		
		DeleteObject(hbmMachineSkin);
		if(!machineskinmask.IsNull())
			machineskinmask.DeleteObject();
		machineskin.LoadBitmap(IDB_MACHINE_SKIN);
	}
}

void CPsycleWTLView::FindMachineSkin(CString findDir, CString findName, BOOL *result)
{
	CFindFile finder;

	int loop = finder.FindFile(findDir + _T("\\*"));	// check for subfolders.
	while (loop) 
	{								
		loop = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindMachineSkin(finder.GetFilePath(),findName,result);
		}
	}
	finder.Close();

	loop = finder.FindFile(findDir + _T("\\") + findName + _T(".psm")); // check if the directory is empty
	while (loop)
	{
		loop = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			CString sName, tmpPath;
			sName = finder.GetFileName();
			// ok so we have a .psm, does it have a valid matching .bmp?
			TCHAR* pExt = const_cast<TCHAR *>(_tcschr(sName,46));// last .
			pExt[0] = 0;
			TCHAR szOpenName[MAX_PATH];
			_stprintf(szOpenName,_T("%s\\%s.bmp"),findDir,sName);

			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			machineskinmask.DeleteObject();
			hbmMachineSkin = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hbmMachineSkin)
			{
				machineskin.Attach(hbmMachineSkin);
				if (machineskin.IsNull())
				{	
					memset(&MachineCoords,0,sizeof(MachineCoords));
					// load settings
					FILE* hfile;
					_stprintf(szOpenName,_T("%s\\%s.psm"),findDir,sName);
					if ((hfile=_tfopen(szOpenName,_T("rw"))) == NULL )
					{
						MessageBox(
							SF::CResourceString(IDS_ERR_MSG0070),
							SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
						return;
					}
					TCHAR buf[512];
					while (_fgetts(buf, 512, hfile))
					{
						if (_tcsstr(buf,_T("\"master_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sMaster.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sMaster.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sMaster.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sMaster.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sGenerator.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGenerator.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGenerator.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGenerator.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_vu0_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorVu0.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorVu0.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorVu0.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVu0.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_vu_peak_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorVuPeak.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorVuPeak.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorVuPeak.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVuPeak.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_pan_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorPan.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorPan.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorPan.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorPan.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_mute_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorMute.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorMute.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorMute.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorMute.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_solo_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorSolo.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorSolo.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorSolo.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorSolo.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffect.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffect.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffect.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffect.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_vu0_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectVu0.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectVu0.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectVu0.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVu0.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_vu_peak_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectVuPeak.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectVuPeak.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectVuPeak.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVuPeak.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_pan_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectPan.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectPan.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectPan.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectPan.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_mute_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectMute.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectMute.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectMute.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectMute.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_bypass_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectBypass.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectBypass.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectBypass.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectBypass.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_vu_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorVu.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorVu.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dGeneratorVu.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorVu.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_pan_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorPan.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorPan.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dGeneratorPan.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorPan.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_mute_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorMute.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorMute.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_solo_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorSolo.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorSolo.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"generator_name_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorName.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorName.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_vu_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectVu.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectVu.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dEffectVu.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectVu.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_pan_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectPan.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectPan.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dEffectPan.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectPan.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_mute_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectMute.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectMute.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_bypass_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectBypass.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectBypass.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"effect_name_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectName.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectName.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"transparency\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								MachineCoords.cTransparency = _httoi(q+1);
								MachineCoords.bHasTransparency = TRUE;
							}
						}
					}
					if (MachineCoords.bHasTransparency)
					{
						PrepareMask(&machineskin,&machineskinmask,MachineCoords.cTransparency);
					}
					fclose(hfile);
					*result = TRUE;
					break;
				}
			}
		}
	}
	finder.Close();
}

void CPsycleWTLView::LoadPatternHeaderSkin()
{
	static TCHAR szOld[64] = _T("");
	if (_tcscmp(szOld, Global::pConfig->pattern_header_skin))
	{
		_tcscpy(szOld, Global::pConfig->pattern_header_skin);
		// ok so...
		if (_tcscmp(szOld, DEFAULT_PATTERN_HEADER_SKIN))
		{
			BOOL result = FALSE;
			FindPatternHeaderSkin(Global::pConfig->GetInitialSkinDir(),Global::pConfig->pattern_header_skin, &result);
			if (result)
			{
				return;
			}
		}
		// load defaults
		_tcscpy(szOld, DEFAULT_PATTERN_HEADER_SKIN);
		// and coords
#ifdef _UGLY_DEFAULT_SKIN_
		PatHeaderCoords.sBackground.x=0;
		PatHeaderCoords.sBackground.y=0;
		PatHeaderCoords.sBackground.width=109;
		PatHeaderCoords.sBackground.height=16;
		PatHeaderCoords.sNumber0.x = 0;
		PatHeaderCoords.sNumber0.y = 16;
		PatHeaderCoords.sNumber0.width = 7;
		PatHeaderCoords.sNumber0.height = 12;
		PatHeaderCoords.sRecordOn.x = 70;
		PatHeaderCoords.sRecordOn.y = 16;
		PatHeaderCoords.sRecordOn.width = 7;
		PatHeaderCoords.sRecordOn.height = 7;
		PatHeaderCoords.sMuteOn.x = 77;
		PatHeaderCoords.sMuteOn.y = 16;
		PatHeaderCoords.sMuteOn.width = 7;
		PatHeaderCoords.sMuteOn.height = 7;
		PatHeaderCoords.sSoloOn.x = 84;
		PatHeaderCoords.sSoloOn.y = 16;
		PatHeaderCoords.sSoloOn.width = 7;
		PatHeaderCoords.sSoloOn.height = 7;
		PatHeaderCoords.dDigitX0.x = 23;
		PatHeaderCoords.dDigitX0.y = 2;
		PatHeaderCoords.dDigit0X.x = 30;
		PatHeaderCoords.dDigit0X.y = 2;
		PatHeaderCoords.dRecordOn.x = 52;
		PatHeaderCoords.dRecordOn.y = 5;
		PatHeaderCoords.dMuteOn.x = 75;
		PatHeaderCoords.dMuteOn.y = 5;
		PatHeaderCoords.dSoloOn.x = 96;
		PatHeaderCoords.dSoloOn.y = 5;
		PatHeaderCoords.bHasTransparency = FALSE;
#else
		PatHeaderCoords.sBackground.x=0;
		PatHeaderCoords.sBackground.y=0;
		PatHeaderCoords.sBackground.width=109;
		PatHeaderCoords.sBackground.height=18;//16
		PatHeaderCoords.sNumber0.x = 0;
		PatHeaderCoords.sNumber0.y = 18;//16
		PatHeaderCoords.sNumber0.width = 7;
		PatHeaderCoords.sNumber0.height = 12;
		PatHeaderCoords.sRecordOn.x = 70;
		PatHeaderCoords.sRecordOn.y = 18;//16
		PatHeaderCoords.sRecordOn.width = 11;//7;
		PatHeaderCoords.sRecordOn.height = 11;//7;
		PatHeaderCoords.sMuteOn.x = 81;//77;
		PatHeaderCoords.sMuteOn.y = 18;//16;
		PatHeaderCoords.sMuteOn.width = 11;//7;
		PatHeaderCoords.sMuteOn.height = 11;//7;
		PatHeaderCoords.sSoloOn.x = 92;//84;
		PatHeaderCoords.sSoloOn.y = 18;//16;
		PatHeaderCoords.sSoloOn.width = 11;//7;
		PatHeaderCoords.sSoloOn.height = 11;//7;
		PatHeaderCoords.dDigitX0.x = 24;//22;
		PatHeaderCoords.dDigitX0.y = 3;//2;
		PatHeaderCoords.dDigit0X.x = 31;//29;
		PatHeaderCoords.dDigit0X.y = 3;//2;
		PatHeaderCoords.dRecordOn.x = 52;
		PatHeaderCoords.dRecordOn.y = 3;//5;
		PatHeaderCoords.dMuteOn.x = 75;
		PatHeaderCoords.dMuteOn.y = 3;//5;
		PatHeaderCoords.dSoloOn.x = 97;//96;
		PatHeaderCoords.dSoloOn.y = 3;//5;
		PatHeaderCoords.bHasTransparency = FALSE;
#endif
		if(!patternheader.IsNull())
			patternheader.DeleteObject();
		//DeleteObject(hbmPatHeader);
		if(!patternheadermask.IsNull())
			patternheadermask.DeleteObject();
		patternheader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
	}
}

void CPsycleWTLView::FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result)
{
	CFindFile finder;

	int loop = finder.FindFile(findDir + _T("\\*"));	// check for subfolders.
	while (loop) 
	{		
		loop = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindPatternHeaderSkin(finder.GetFilePath(),findName,result);
		}
	}
	finder.Close();

	loop = finder.FindFile(findDir + _T("\\") + findName + _T(".psh")); // check if the directory is empty
	while (loop)
	{
		if (!finder.IsDirectory())
		{
			CString sName, tmpPath;
			sName = finder.GetFileName();
			// ok so we have a .psh, does it have a valid matching .bmp?
			TCHAR* pExt = const_cast<TCHAR*>(_tcschr(sName,46));// last .
			pExt[0]=0;
			TCHAR szOpenName[MAX_PATH];
			_stprintf(szOpenName,_T("%s\\%s.bmp"),findDir,sName);

			if(!patternheader.IsNull())
				patternheader.DeleteObject();
			
			// DeleteObject(hbmPatHeader);
			
			if(!patternheadermask.IsNull())
				patternheadermask.DeleteObject();

			hbmPatHeader = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			
			if (hbmPatHeader)
			{	patternheader.Attach(hbmPatHeader);
				if (patternheader.IsNull())
				{	
					memset(&PatHeaderCoords,0,sizeof(PatHeaderCoords));
					// load settings
					FILE* hfile;
					_stprintf(szOpenName,_T("%s\\%s.psh"),findDir,sName);
					if ((hfile=_tfopen(szOpenName,_T("rw"))) == NULL )
					{
						MessageBox(
							SF::CResourceString(IDS_ERR_MSG0070),
							SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
						return;
					}
					TCHAR buf[512];
					while (_fgetts(buf, 512, hfile))
					{
						if (_tcsstr(buf,_T("\"background_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sBackground.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sBackground.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sBackground.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sBackground.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"number_0_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sNumber0.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sNumber0.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sNumber0.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sNumber0.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"record_on_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sRecordOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sRecordOn.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sRecordOn.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sRecordOn.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"mute_on_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sMuteOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sMuteOn.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sMuteOn.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sMuteOn.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"solo_on_source\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sSoloOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sSoloOn.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sSoloOn.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sSoloOn.height = _tstoi(q+1);
										}
									}
								}
							}
						}
						else if (_tcsstr(buf,_T("\"digit_x0_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dDigitX0.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dDigitX0.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"digit_0x_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dDigit0X.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dDigit0X.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"record_on_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dRecordOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dRecordOn.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"mute_on_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dMuteOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dMuteOn.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"solo_on_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dSoloOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dSoloOn.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"transparency\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.cTransparency = _httoi(q+1);
								PatHeaderCoords.bHasTransparency = TRUE;
							}
						}
					}
					if (PatHeaderCoords.bHasTransparency)
					{
						PrepareMask(&patternheader,&patternheadermask,PatHeaderCoords.cTransparency);
					}
					fclose(hfile);
					*result = TRUE;
					break;
				}
			}
		}
		loop = finder.FindNextFile();

	}
	finder.Close();
}


void CPsycleWTLView::RecalcMetrics()
{
	if (Global::pConfig->pattern_draw_empty_data)
	{
		_stprintf(szBlankParam,_T("."));
		_stprintf(szBlankNote,_T("---"));
	}
	else
	{
		_stprintf(szBlankParam,_T(" "));
		_stprintf(szBlankNote,_T("   "));
	}
	TEXTHEIGHT = Global::pConfig->pattern_font_y;
	ROWHEIGHT = TEXTHEIGHT+1;

	TEXTWIDTH = Global::pConfig->pattern_font_x;
	for (int c=0; c<256; c++)	
	{ 
		FLATSIZES[c]=Global::pConfig->pattern_font_x; 
	}
	COLX[0] = 0;
	COLX[1] = (TEXTWIDTH*3)+2;
	COLX[2] = COLX[1]+TEXTWIDTH;
	COLX[3] = COLX[2]+TEXTWIDTH+1;
	COLX[4] = COLX[3]+TEXTWIDTH;
	COLX[5] = COLX[4]+TEXTWIDTH+1;
	COLX[6] = COLX[5]+TEXTWIDTH;
	COLX[7] = COLX[6]+TEXTWIDTH;
	COLX[8] = COLX[7]+TEXTWIDTH;
	COLX[9] = COLX[8]+TEXTWIDTH+1;
	COLX[10] = COLX[9]+TEXTWIDTH;
	COLX[11] = COLX[10]+TEXTWIDTH;
	COLX[12] = COLX[11]+TEXTWIDTH;
	COLX[13] = COLX[12]+TEXTWIDTH + 1;
	ROWWIDTH = COLX[13];
	HEADER_ROWWIDTH = PatHeaderCoords.sBackground.width+1;
	HEADER_HEIGHT = PatHeaderCoords.sBackground.height+2;
	if (ROWWIDTH < HEADER_ROWWIDTH)
	{
		int temp = (HEADER_ROWWIDTH-ROWWIDTH)/2;
		ROWWIDTH = HEADER_ROWWIDTH;
		for (int i = 0; i < 10; i++)
		{
			COLX[i] += temp;
		}
	}
	HEADER_INDENT = (ROWWIDTH - HEADER_ROWWIDTH)/2;
	if (Global::pConfig->_linenumbers)
	{
		XOFFSET = (4*TEXTWIDTH);
		YOFFSET = TEXTHEIGHT+2;
		if (YOFFSET < HEADER_HEIGHT)
		{
			YOFFSET = HEADER_HEIGHT;
		}
	}
	else
	{
		XOFFSET = 1;
		YOFFSET = HEADER_HEIGHT;
	}
	VISTRACKS = (CW-XOFFSET)/ROWWIDTH;
	VISLINES = (CH-YOFFSET)/ROWHEIGHT;
	if (VISLINES < 1) 
	{ 
		VISLINES = 1; 
	}
	if (VISTRACKS < 1) 
	{ 
		VISTRACKS = 1; 
	}

	triangle_size_tall = Global::pConfig->mv_triangle_size+((46*Global::pConfig->mv_wirewidth) / 32);
	triangle_size_center = triangle_size_tall / 2;
	triangle_size_wide = (triangle_size_tall * 5) / 8;

}


void CPsycleWTLView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
{
   BITMAP bm;

   // Get the dimensions of the source bitmap
   pBmpSource->GetBitmap(&bm);

   // Create the mask bitmap
   pBmpMask->DeleteObject();
   pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);

   // We will need two DCs to work with. One to hold the Image
   // (the source), and one to hold the mask (destination).
   // When blitting onto a monochrome bitmap from a color, pixels
   // in the source color bitmap that are equal to the background
   // color are blitted as white. All the remaining pixels are
   // blitted as black.

   WTL::CDC hdcSrc, hdcDst;

   hdcSrc.CreateCompatibleDC(NULL);
   hdcDst.CreateCompatibleDC(NULL);

   // Load the bitmaps into memory DC
   CBitmapHandle hbmSrcT(hdcSrc.SelectBitmap((HBITMAP)(*pBmpSource)));
   CBitmapHandle hbmDstT(hdcDst.SelectBitmap((HBITMAP)(*pBmpMask)));

   // Change the background to trans color
   hdcSrc.SetBkColor(clrTrans);

   // This call sets up the mask bitmap.
   hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, (HDC)hdcSrc,0,0,SRCCOPY);

   // Now, we need to paint onto the original image, making
   // sure that the "transparent" area is set to black. What
   // we do is AND the monochrome image onto the color Image
   // first. When blitting from mono to color, the monochrome
   // pixel is first transformed as follows:
   // if  1 (black) it is mapped to the color set by SetTextColor().
   // if  0 (white) is is mapped to the color set by SetBkColor().
   // Only then is the raster operation performed.

   hdcSrc.SetTextColor(RGB(255,255,255));
   hdcSrc.SetBkColor(RGB(0,0,0));

   hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, (HDC)hdcDst,0,0,SRCAND);

   // Clean up by deselecting any objects, and delete the
   // DC's.
   hdcSrc.SelectBitmap((HBITMAP)hbmSrcT);
   hdcDst.SelectBitmap((HBITMAP)hbmDstT);

   hdcSrc.DeleteDC();
   hdcDst.DeleteDC();
}

void CPsycleWTLView::TransparentBlt(CDC* pDC,
                                   int xStart,  int yStart,
                                   int wWidth,  int wHeight,
                                   CDC* pTmpDC,
								   CBitmap* bmpMask,
                                   int xSource, // = 0
                                   int ySource)  // = 0)
{

   // We are going to paint the two DDB's in sequence to the destination.
   // 1st the monochrome bitmap will be blitted using an AND operation to
   // cut a hole in the destination. The color image will then be ORed
   // with the destination, filling it into the hole, but leaving the
   // surrounding area untouched.

   CDC hdcMem;
   hdcMem.CreateCompatibleDC((HDC)(*pDC));

   CBitmapHandle hbmT(hdcMem.SelectBitmap((HBITMAP)(*bmpMask)));

   pDC->SetTextColor(RGB(0,0,0));
   pDC->SetBkColor(RGB(255,255,255));

   if (!pDC->BitBlt( xStart, yStart, wWidth, wHeight, (HDC)hdcMem, xSource, ySource, 
	   SRCAND))
   {
	   ATLTRACE(_T("transparent draw failed(SRCAND)"));
   }

   // Also note the use of SRCPAINT rather than SRCCOPY.

	if (!pDC->BitBlt(xStart, yStart, wWidth, wHeight, (HDC)(*pTmpDC), xSource, ySource,
		SRCPAINT))
	{
		ATLTRACE(_T("transparent draw failed(SRCPAINT)"));
	}

   // Now, clean up.
   hdcMem.SelectBitmap((HBITMAP)hbmT);
   hdcMem.DeleteDC();
}

void CPsycleWTLView::patTrackMute()
{
	if (viewMode == VMPattern)
	{
		_pSong->IsTrackMuted(editcur.track,!_pSong->IsTrackMuted(editcur.track));
		Repaint(DMTrackHeader);
	}
}

void CPsycleWTLView::patTrackSolo()
{
	if (viewMode == VMPattern)
	{
		if (_pSong->TrackSoloed() == editcur.track)
		{
			for (int i = 0; i < MAX_TRACKS; i++)
			{
				_pSong->IsTrackMuted(i,false);
			}
			_pSong->TrackSoloed(-1);
		}
		else
		{
			for (int i = 0; i < MAX_TRACKS; i++)
			{
				_pSong->IsTrackMuted(i,true);
			}
			_pSong->IsTrackMuted(editcur.track,false);
			_pSong->TrackSoloed(editcur.track);
		}
		Repaint(DMTrackHeader);
	}
}

void CPsycleWTLView::patTrackRecord()
{
	if (viewMode == VMPattern)
	{
		_pSong->IsTrackArmed(editcur.track,!_pSong->IsTrackArmed(editcur.track));
		_pSong->TrackArmedCount(0);
		for ( int i=0;i<MAX_TRACKS;i++ )
		{
			if (_pSong->IsTrackArmed(i))
			{
				_pSong->TrackArmedCount(_pSong->TrackArmedCount() + 1);
			}
		}
		Repaint(DMTrackHeader);
	}
}

void CPsycleWTLView::DoMacPropDialog(int propMac)
{
	if ((propMac < 0 ) || (propMac >= MAX_MACHINES-1))
	{
		return;
	}
	CMacProp dlg;
	dlg.m_view=this;
	dlg.pMachine = Global::_pSong->pMachine(propMac);
	dlg.pSong = Global::_pSong;
	dlg.thisMac = propMac;
	
	if (dlg.DoModal() == IDOK)
	{
		_stprintf(dlg.pMachine->_editName, dlg.txt);
		pMainFrame->StatusBarText(dlg.txt);
		pMainFrame->UpdateEnvInfo();
		pMainFrame->UpdateComboGen();
		if (pMainFrame->pGearRackDialog)
		{
			pMainFrame->RedrawGearRackList();
		}
	}
	if (dlg.deleted)
	{
		pMainFrame->CloseMacGui(propMac);
		Global::_pSong->DestroyMachine(propMac);
		pMainFrame->UpdateEnvInfo();
		pMainFrame->UpdateComboGen();
		if (pMainFrame->pGearRackDialog)
		{
			pMainFrame->RedrawGearRackList();
		}
	}
}



void CPsycleWTLView::OnConfigurationLoopplayback() 
{
	Global::pPlayer->_loopSong = !Global::pPlayer->_loopSong;
}

void CPsycleWTLView::LoadMachineBackground()
{	if(!machinebkg.IsNull())
		machinebkg.DeleteObject();
	DeleteObject(hbmMachineBkg);
	if (Global::pConfig->bBmpBkg)
	{
		Global::pConfig->bBmpBkg=FALSE;
		hbmMachineBkg = (HBITMAP)LoadImage(NULL, Global::pConfig->szBmpBkgFilename, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
		if (hbmMachineBkg)
		{
			machinebkg.Attach(hbmMachineBkg);
			if (!machinebkg.IsNull())
			{	
				BITMAP bm;
				::GetObject(hbmMachineBkg,sizeof(BITMAP),&bm);

				bkgx = bm.bmWidth;
				bkgy = bm.bmHeight;

				if ((bkgx > 0) && (bkgy > 0))
				{
					Global::pConfig->bBmpBkg = TRUE;
				}
			}
		}
	}
}

BOOL CPsycleWTLView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT CPsycleWTLView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("OnPaint()\n"));
#endif
	if (!GetUpdateRect(NULL)) return 0; // If no area to update, exit.

	WTL::CPaintDC dc(m_hWnd);
/*	dc.SaveDC();
	dc.SetBkColor(0x00404040);
	dc.SetTextColor(0x00f0f0f0);
	CFont font;
	font.CreateFont(8,8,0,0,0,0,0,0,128,OUT_TT_ONLY_PRECIS,0,0,0,_T(_T("ＭＳ 明朝")));
	HFONT _old = dc.SelectFont(Global::pConfig->seqFont);
	RECT rc;
	RECT rc1 = {0,0,12,12};
	
	GetClientRect(&rc);
	//TODO: Add your drawing code here
//	dc.ExtTextOut(100,100,ETO_OPAQUE | ETO_CLIPPED,&rc,_T("こ"),-1,NULL);
	for(int i = 0;i < rc.right - rc.left;i += 24){
		dc.SetBkColor(0x00404040 + i);
			dc.SetTextColor(0x00f0f0f0);

		for(int j = 0;j < rc.bottom - rc.top ;j += 10){
			rc1.left = i + rc.left;
			rc1.top = j+ rc.top;
			rc1.right = rc1.left + 8 * 23;
			rc1.bottom = rc1.bottom + 8;
			dc.ExtTextOut( i + rc.left,j + rc.top,ETO_OPAQUE | ETO_CLIPPED,&rc1,_T("ABC"),3,FLATSIZES);
//			dc.TextOut( i + rc.left,j + rc.top,L"こ",-1);
		}
	}
	dc.SelectFont(_old);
	dc.RestoreDC(-1);
	
	return 0;
*/


	if ( bmpDC == NULL && Global::pConfig->useDoubleBuffer ) // buffer creation
	{
		CRect rc;
		GetClientRect(&rc);
		bmpDC = new CBitmap();
		bmpDC->CreateCompatibleBitmap(dc.m_hDC,rc.right - rc.left,rc.bottom - rc.top);
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("CPsycleWTLView::OnPaint(). bmpDC  0x%.8X initialize\n"),(int)bmpDC);
#endif
	}
	else if ( bmpDC != NULL && !Global::pConfig->useDoubleBuffer ) // buffer deletion
	{
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("CPsycleWTLView::OnPaint(). bmpDC (0x%.8X) deleted\n"),(int)bmpDC);
#endif
		delete bmpDC;
		bmpDC = NULL;
	}
	if ( Global::pConfig->useDoubleBuffer )
	{
		CDC bufDC;
		bufDC.CreateCompatibleDC(dc);
		CBitmapHandle oldbmp(bufDC.SelectBitmap(*bmpDC));
		if (viewMode == VMMachine)	// Machine view paint handler
		{
			switch (updateMode)
			{
			case DMAll:
				DrawMachineEditor(&bufDC);
				break;
			case DMMacRefresh:
//				ClearMachineSpace(Global::_pSong->_pMachines[updatePar], updatePar, &bufDC);
				DrawMachine(updatePar, &bufDC);
				DrawMachineVumeters(updatePar, &bufDC);
				updateMode=0;
				break;
			case DMAllMacsRefresh:
				for (int i=0;i<MAX_MACHINES;i++)
				{
					if (_pSong->pMachine(i))
					{
						DrawMachine(i, &bufDC);
					}
				}
				DrawAllMachineVumeters(&bufDC);
				break;
			}
		}
		else if (viewMode == VMPattern)	// Pattern view paint handler
		{
			DrawPatEditor(&bufDC);
		}

		CRect rc;
		GetClientRect(&rc);
		dc.BitBlt(0,0,rc.right - rc.left,rc.bottom - rc.top,bufDC,0,0,SRCCOPY);
		bufDC.SelectBitmap(oldbmp);
	}
	else
	{
		if (viewMode==VMMachine)	// Machine view paint handler
		{
			switch (updateMode)
			{
			case DMAll:
				DrawMachineEditor(&dc);
				break;
			case DMMacRefresh:
//				ClearMachineSpace(Global::_pSong->_pMachines[updatePar], updatePar, &dc);
				DrawMachine(updatePar, &dc);
				DrawMachineVumeters(updatePar, &dc);
				updateMode = 0;
				break;
			case DMAllMacsRefresh:
				for (int i = 0;i < MAX_MACHINES;i++)
				{
					if (_pSong->pMachine(i)) 
					{
						DrawMachine(i, &dc);
					}
				}
				DrawAllMachineVumeters(&dc);
				break;
			}
		}
		else if (viewMode == VMPattern)	// Pattern view paint handler
		{
			DrawPatEditor(&dc);
		}
	}
	
	bHandled = FALSE;
	SetMsgHandled(FALSE);
	return 0;
}


void CPsycleWTLView::UpdateUI()
{
	// メニューバー更新 //
	// 録音ボタン
	pMainFrame->UISetCheck(ID_RECORDB,(Global::pPlayer->_recording)?1:0);
	// パターンビュー表示
	pMainFrame->UISetCheck(ID_PATTERNVIEW,(viewMode == VMPattern)?1:0);
	// マシンビュー表示
	pMainFrame->UISetCheck(ID_MACHINEVIEW,(viewMode == VMMachine)?1:0);
	// 
	pMainFrame->UISetCheck(ID_AUTOSTOP,(Global::pConfig->autoStopMachines)?1:0);
	pMainFrame->UISetCheck(ID_BARPLAY,(Global::pPlayer->_playing)?1:0);
	pMainFrame->UISetCheck(ID_BARPLAYFROMSTART,0);
	pMainFrame->UISetCheck(ID_BARREC,(Global::pConfig->_followSong && bEditMode)?1:0);
	pMainFrame->UISetCheck(ID_BUTTONPLAYSEQBLOCK,Global::pPlayer->_playBlock?TRUE:FALSE);
	// UNDO
	if(pUndoList) 
	{
		switch (pUndoList->type)
		{
		case UNDO_SEQUENCE:
			pMainFrame->UIEnable(ID_EDIT_UNDO,TRUE);
			pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
			break;
		default:
			if(viewMode == VMPattern)// && bEditMode)
			{
				pMainFrame->UIEnable(ID_EDIT_UNDO,TRUE);
				pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
			}
			else
			{
				pMainFrame->UIEnable(ID_EDIT_UNDO,FALSE);
				pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO1));
			}
			break;
		}
	}
	else
	{
		pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
		pMainFrame->UIEnable(ID_EDIT_UNDO,FALSE);
	}
	// REDO
	if(pRedoList) 
	{
		switch (pRedoList->type)
		{
		case UNDO_SEQUENCE:
			pMainFrame->UIEnable(ID_EDIT_REDO,TRUE);
			pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO));
			break;
		default:
			if(viewMode == VMPattern)// && bEditMode)
			{
				pMainFrame->UIEnable(ID_EDIT_REDO,TRUE);
				pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO));
			}
			else
			{
				pMainFrame->UIEnable(ID_EDIT_REDO,FALSE);
				pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO1));
			}
			break;
		}
	}
	else
	{
		pMainFrame->UIEnable(ID_EDIT_REDO,FALSE);
		pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO));
	}

	pMainFrame->UIEnable(ID_EDIT_CUT,(viewMode == VMPattern)?TRUE:FALSE);
	pMainFrame->UIEnable(ID_EDIT_COPY,(viewMode == VMPattern)?TRUE:FALSE);
	pMainFrame->UIEnable(ID_EDIT_DELETE,(viewMode == VMPattern)?TRUE:FALSE);
	pMainFrame->UIEnable(ID_EDIT_MIXPASTE,(patBufferCopy&&(viewMode == VMPattern))?TRUE:FALSE);
	pMainFrame->UIEnable(ID_EDIT_PASTE,(patBufferCopy&&(viewMode == VMPattern))?TRUE:FALSE);

	pMainFrame->UISetCheck(ID_CONFIGURATION_LOOPPLAYBACK,(Global::pPlayer->_loopSong)?1:0);
	
	BOOL bCutCopy =  blockSelected && (viewMode == VMPattern);

	pMainFrame->UIEnable(ID_POP_CUT,bCutCopy);
	pMainFrame->UIEnable(ID_POP_COPY,bCutCopy);
	pMainFrame->UIEnable(ID_POP_PASTE,(isBlockCopied && (viewMode == VMPattern))?TRUE:FALSE);
	pMainFrame->UIEnable(ID_POP_DELETE, bCutCopy);
	pMainFrame->UIEnable(ID_POP_INTERPOLATE, bCutCopy);
	pMainFrame->UIEnable(ID_POP_CHANGEGENERATOR, bCutCopy);
	pMainFrame->UIEnable(ID_POP_CHANGEINSTRUMENT, bCutCopy);
	pMainFrame->UIEnable(ID_POP_TRANSPOSE1, bCutCopy);
	pMainFrame->UIEnable(ID_POP_TRANSPOSE12, bCutCopy);
	pMainFrame->UIEnable(ID_POP_TRANSPOSE_1, bCutCopy);
	pMainFrame->UIEnable(ID_POP_TRANSPOSE_12, bCutCopy);
	pMainFrame->UIEnable(ID_POP_BLOCK_SWINGFILL, bCutCopy);
	pMainFrame->UIEnable(ID_POP_MIXPASTE,(isBlockCopied && (viewMode == VMPattern))?TRUE:FALSE);

}

