//
// Original Coder : Juan Antonio Arguelles Rius
//
// [C] 2000 All Rights Reserved
//
// [C] 2001-2002 Psycledelics.
//
// ChildView.cpp : implementation of the CChildView class
//
#define rdtsc __asm __emit 0fh __asm __emit 031h
#define cpuid __asm __emit 0fh __asm __emit 0a2h

#include "stdafx.h"
#include "Psycle2.h"
//#include "Registry.h"
#include "Configuration.h"
#include "FileXM.h"
//#include "FileIT.h"
#include "ChildView.h"
#include "Bitmap.cpp"

#include "Player.h"
#include "MidiInput.h"
#include "SwingFillDlg.h"
#include "Helpers.h"
#include "MasterDlg.h"
#include "GearPsychOsc.h"
#include "GearDistort.h"
#include "GearTracker.h"
#include "GearDelay.h"
#include "Gearfilter.h"
#include "GearGainer.h"
#include "GearFlanger.h"

//#include "Dsp.h"
//#include "Filter.h"

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

#include <math.h> // SwingFill

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////

//////////////////////////////////////////////////////////////////////
// Enviroment Initialization

CMainFrame		*pParentMain;

//Bitmap mv_bg;

// Enviroment variables
int smac=-1;
int smacmode=0;
int wiresource=-1;
int wiredest=-1;
int wireSX=0;
int wireSY=0;
int wireDX=0;
int wireDY=0;

unsigned idletime=0;

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
	// Set Gui Environment data

	for (int c=0; c<256; c++)	{ FLATSIZES[c]=8; }
	bmpDC = NULL;

	viewMode=VMMachine;
	MasterMachineDialog = NULL;
	PsychMachineDialog = NULL;
	DistortionMachineDialog = NULL;
	SamplerMachineDialog = NULL;
	DelayMachineDialog = NULL;
	FilterMachineDialog = NULL;
	GainerMachineDialog = NULL;
	FlangerMachineDialog = NULL;
	for (c = 0; c < MAX_WIRE_DIALOGS; c++)
	{
		WireDialog[c] = NULL;
	}

	updateMode=0;
	updatePar=0;
//	multiPattern=true; // Long way till it can be finished!

	patStep=1;
	editPosition=0;
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

//	editcur.track=0; // Not needed to initialize, since the class does it already.
//	editcur.col=0;
//	editcur.line=0;
	playpos=-1;
	newplaypos=-1; 
	selpos.bottom=0;
	newselpos.bottom=0;
	numPatternDraw=0;

//	drawTrackStart=-1;

//	scrollT=0;
//	scrollL=0;
	tOff=0;
	lOff=0;
	ntOff=0;
	nlOff=0;

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

	// Show Machine view and init MIDI
	OnMachineview();
}

CChildView::~CChildView()
{
	Global::pInputHandler->SetChildView(NULL);
	KillRedo();
	KillUndo();

	if ( bmpDC != NULL )
	{
		char buf[100];
		sprintf(buf,"CChildView::~CChildView(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
		TRACE(buf);
		bmpDC->DeleteObject();
		delete bmpDC;
	}
	patternheader.DeleteObject();
	DeleteObject(hbmPatHeader);
	machineskin.DeleteObject();
	DeleteObject(hbmMachineSkin);
	patternheadermask.DeleteObject();
	machineskinmask.DeleteObject();
}

BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_CPUPERFORMANCE, OnHelpPsycleenviromentinfo)
	ON_COMMAND(ID_MIDI_MONITOR, OnMidiMonitorDlg)
	ON_WM_DESTROY()
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_MACHINEVIEW, OnMachineview)
	ON_COMMAND(ID_PATTERNVIEW, OnPatternView)	
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_BARPLAY, OnBarplay)
	ON_COMMAND(ID_BARSTOP, OnBarstop)
	ON_COMMAND(ID_RECORDB, OnRecordb)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_RECORDB, OnUpdateRecordb)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVESONG, OnFileSavesong)
	ON_COMMAND(ID_FILE_LOADSONG, OnFileLoadsong)
	ON_COMMAND(ID_HELP_SALUDOS, OnHelpSaludos)
	ON_UPDATE_COMMAND_UI(ID_PATTERNVIEW, OnUpdatePatternView)
	ON_UPDATE_COMMAND_UI(ID_MACHINEVIEW, OnUpdateMachineview)
	ON_UPDATE_COMMAND_UI(ID_BARPLAY, OnUpdateBarplay)
	ON_COMMAND(ID_FILE_SONGPROPERTIES, OnFileSongproperties)
	ON_COMMAND(ID_VIEW_INSTRUMENTEDITOR, OnViewInstrumenteditor)
	ON_COMMAND(ID_NEWMACHINE, OnNewmachine)
	ON_COMMAND(ID_BUTTONPLAYSEQBLOCK, OnButtonplayseqblock)
	ON_UPDATE_COMMAND_UI(ID_BUTTONPLAYSEQBLOCK, OnUpdateButtonplayseqblock)
	ON_COMMAND(ID_POP_CUT, OnPopCut)
	ON_UPDATE_COMMAND_UI(ID_POP_CUT, OnUpdatePopCut)
	ON_COMMAND(ID_POP_COPY, OnPopCopy)
	ON_UPDATE_COMMAND_UI(ID_POP_COPY, OnUpdatePopCopy)
	ON_COMMAND(ID_POP_PASTE, OnPopPaste)
	ON_UPDATE_COMMAND_UI(ID_POP_PASTE, OnUpdatePopPaste)
	ON_COMMAND(ID_POP_MIXPASTE, OnPopMixpaste)
	ON_UPDATE_COMMAND_UI(ID_POP_MIXPASTE, OnUpdatePopMixpaste)
	ON_COMMAND(ID_POP_INTERPOLATE, OnPopInterpolate)
	ON_UPDATE_COMMAND_UI(ID_POP_INTERPOLATE, OnUpdatePopInterpolate)
	ON_COMMAND(ID_POP_CHANGEGENERATOR, OnPopChangegenerator)
	ON_UPDATE_COMMAND_UI(ID_POP_CHANGEGENERATOR, OnUpdatePopChangegenerator)
	ON_COMMAND(ID_POP_CHANGEINSTRUMENT, OnPopChangeinstrument)
	ON_UPDATE_COMMAND_UI(ID_POP_CHANGEINSTRUMENT, OnUpdatePopChangeinstrument)
	ON_COMMAND(ID_POP_TRANSPOSE1, OnPopTranspose1)
	ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE1, OnUpdatePopTranspose1)
	ON_COMMAND(ID_POP_TRANSPOSE12, OnPopTranspose12)
	ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE12, OnUpdatePopTranspose12)
	ON_COMMAND(ID_POP_TRANSPOSE_1, OnPopTranspose_1)
	ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_1, OnUpdatePopTranspose_1)
	ON_COMMAND(ID_POP_TRANSPOSE_12, OnPopTranspose_12)
	ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_12, OnUpdatePopTranspose_12)
	ON_COMMAND(ID_AUTOSTOP, OnAutostop)
	ON_UPDATE_COMMAND_UI(ID_AUTOSTOP, OnUpdateAutostop)
	ON_COMMAND(ID_POP_PATTENPROPERTIES, OnPopPattenproperties)
	ON_COMMAND(ID_POP_BLOCK_SWINGFILL, OnPopBlockSwingfill)
	ON_UPDATE_COMMAND_UI(ID_POP_BLOCK_SWINGFILL, OnUpdatePopBlockSwingfill)
	ON_COMMAND(ID_POP_TRACK_SWINGFILL, OnPopTrackSwingfill)
	ON_WM_SIZE()
	ON_COMMAND(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
	ON_WM_CONTEXTMENU()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FILE_IMPORT_XMFILE, OnFileImportXmfile)
	ON_COMMAND(ID_FILE_RECENT_01, OnFileRecent_01)
	ON_COMMAND(ID_FILE_RECENT_02, OnFileRecent_02)
	ON_COMMAND(ID_FILE_RECENT_03, OnFileRecent_03)
	ON_COMMAND(ID_FILE_RECENT_04, OnFileRecent_04)
	ON_COMMAND(ID_FILE_IMPORT_ITFILE, OnFileImportItfile)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateRedo)
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdatePatternCutCopyPaste)
	ON_COMMAND(ID_FILE_SAVEAUDIO, OnFileSaveaudio)
	ON_COMMAND(ID_EDIT_CUT, patCut)
	ON_COMMAND(ID_EDIT_COPY, patCopy)
	ON_COMMAND(ID_EDIT_PASTE, patPaste)
	ON_COMMAND(ID_EDIT_MIXPASTE, patMixPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdatePatternCutCopyPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePatternCutCopyPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MIXPASTE, OnUpdatePatternCutCopyPaste)
	ON_COMMAND(ID_HELP_KEYBTXT, OnHelpKeybtxt)
	ON_COMMAND(ID_HELP_README, OnHelpReadme)
	ON_COMMAND(ID_HELP_TWEAKING, OnHelpTweaking)
	ON_COMMAND(ID_HELP_WHATSNEW, OnHelpWhatsnew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
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
//////////////////////////////////////////////////////////////////////
// This function gives to the pParentMain the pointer to a CMainFrm
// object. Call this function from the CMainframe side object to
// allow CCHildView call functions of the CMainFrm parent object
// Call this function after creating both the CCHildView object and
// the cmainfrm object

void CChildView::ValidateParent()
{
	pParentMain=(CMainFrame *)pParentFrame;
	pParentMain->_pSong=Global::_pSong;
}

//////////////////////////////////////////////////////////////////////
// Timer initialization

void CChildView::InitTimer()
{
	if (!SetTimer(31,20,NULL) || !SetTimer(159,600000,NULL)) // 1st Timer: GUI update. 2nd Timer: AutoSave
	{
		AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
	}
}
//////////////////////////////////////////////////////////////////////
// Timer handler

void CChildView::OnTimer( UINT nIDEvent )
{
	if (nIDEvent == 31)
	{
		CSingleLock lock(&_pSong->door,TRUE);

		pParentMain->UpdateVumeters(
//			((Master*)Global::_pSong->_pMachines[0])->_LMAX,
//			((Master*)Global::_pSong->_pMachines[0])->_RMAX,
			((Master*)Global::_pSong->_pMachines[0])->_lMax,
			((Master*)Global::_pSong->_pMachines[0])->_rMax,
			Global::pConfig->vu1,
			Global::pConfig->vu2,
			Global::pConfig->vu3,
			((Master*)Global::_pSong->_pMachines[0])->_clip);

		if ( MasterMachineDialog )
		{
			if (!--((Master*)Global::_pSong->_pMachines[0])->peaktime) 
			{
				char peak[10];
				sprintf(peak,"%.2fdB",20*log10f(((Master*)Global::_pSong->_pMachines[0])->currentpeak)-90);
				MasterMachineDialog->m_masterpeak.SetWindowText(peak);
//				MasterMachineDialog->m_slidermaster.SetPos(256-((Master*)Global::_pSong->_pMachines[0])->_outDry);

				float val = sqrtf(((Master*)Global::_pSong->_pMachines[0])->_outDry*64.0f);
				MasterMachineDialog->m_slidermaster.SetPos(256-f2i(val));
				
				((Master*)Global::_pSong->_pMachines[0])->peaktime=25;
				((Master*)Global::_pSong->_pMachines[0])->currentpeak=0.0f;
			}
		}
		((Master*)Global::_pSong->_pMachines[0])->vuupdated = true;
		
		if (viewMode == VMMachine)
		{
			CClientDC dc(this);
			DrawMachineVumeters(&dc);
		}
		if (Global::_pSong->Tweaker)
		{
			for(int c=0; c<MAX_MACHINES; c++)
			{
				if (pParentMain->isguiopen[c])
				{
					if ( _pSong->_pMachines[c]->_type == MACH_PLUGIN )
					{
						pParentMain->m_pWndMac[c]->Invalidate(false);
					}
					else if ( _pSong->_pMachines[c]->_type == MACH_VST ||
							_pSong->_pMachines[c]->_type == MACH_VSTFX )
					{
						((CVstEditorDlg*)pParentMain->m_pWndMac[c])->Refresh(-1,0);
					}
/*					else
					{
					}
*/
				}
			}
			Global::_pSong->Tweaker = false;
		}

		if (Global::pPlayer->_playing)
		{
			if (Global::pPlayer->_lineChanged)
			{
				Global::pPlayer->_lineChanged = false;
				pParentMain->SetAppSongBpm(0);
				pParentMain->SetAppSongTpb(0);

				if (Global::pConfig->_followSong)
				{
					CListBox* pSeqList = (CListBox*)pParentMain->m_wndSeq.GetDlgItem(IDC_SEQLIST);
					editcur.line=Global::pPlayer->_lineCounter;
					
					if (pSeqList->GetCurSel() != Global::pPlayer->_playPosition)
					{
						pSeqList->SelItemRange(false,0,pSeqList->GetCount());
						pSeqList->SetSel(Global::pPlayer->_playPosition,true);
						editPosition=Global::pPlayer->_playPosition;
						if ( viewMode == VMPattern ) Repaint(DMPattern);//DMPlaybackChange);  // Until this mode is coded there is no point in calling it since it just makes patterns not refresh correctly currently
					}
					else if( viewMode == VMPattern ) Repaint(DMPlayback);
				}
				else if ( viewMode == VMPattern ) Repaint(DMPlayback);
			}
		}
	}
	if (nIDEvent == 159 )
	{
		CString filepath = Global::pConfig->GetInitialSongDir();
		filepath += "\\autosave.psy";
		OldPsyFile file;
		if (!file.Create(filepath.GetBuffer(1), true))
		{
			return;
		}
		_pSong->Save(&file);
		file.Close();
	}
}


void CChildView::EnableSound()
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

void CChildView::OnDestroy()
{
	if (Global::pConfig->_pOutputDriver->Initialized())
	{
		Global::pConfig->_pOutputDriver->Reset();
	}
	KillTimer(31);
	KillTimer(159);
}

void CChildView::OnAppExit() 
{
	pParentMain->ClosePsycle();
}

//////////////////////////////////////////////////////////////////////
// Include GDI sources

#include "machineview.cpp"
#include "patviewnew.cpp"
/*
void CChildView::OnPaint() 
{
	if (!GetUpdateRect(NULL) ) return; // If no area to update, exit.

	CPaintDC dc(this);
	CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	bufDC.DeleteDC();
}
*/

void CChildView::OnPaint() 
{
	if (!GetUpdateRect(NULL) ) return; // If no area to update, exit.
	CPaintDC dc(this);

	if ( bmpDC == NULL && Global::pConfig->useDoubleBuffer ) // buffer creation
	{
		CRect rc;
		GetClientRect(&rc);
		bmpDC = new CBitmap;
		bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
		char buf[100];
		sprintf(buf,"CChildView::OnPaint(). Initialized bmpDC to 0x%.8X\n",(int)bmpDC);
		TRACE(buf);
	}
	else if ( bmpDC != NULL && !Global::pConfig->useDoubleBuffer ) // buffer deletion
	{
		char buf[100];
		sprintf(buf,"CChildView::OnPaint(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
		TRACE(buf);
		bmpDC->DeleteObject();
		delete bmpDC;
		bmpDC=NULL;
	}
	if ( Global::pConfig->useDoubleBuffer )
	{
		CDC bufDC;
		bufDC.CreateCompatibleDC(&dc);
		CBitmap* oldbmp;
		oldbmp = bufDC.SelectObject(bmpDC);
		if (viewMode==VMMachine)	// Machine view paint handler
		{
			switch (updateMode)
			{
			case DMAll:
				DrawMachineEditor(&bufDC);
				break;
			case DMMacRefresh:
				DrawMachine(Global::_pSong->_pMachines[updatePar], updatePar, &bufDC);
				DrawMachineVumeters(&bufDC);
				updateMode=0;
				break;
			case DMAllMacsRefresh:
				for (int i=0;i<MAX_MACHINES;i++)
				{
					if (_pSong->_machineActive[i])
					{
						DrawMachine(Global::_pSong->_pMachines[i], i, &bufDC);
						DrawMachineVumeters(&bufDC);
					}
				}
				break;
			}
		}
		else if (viewMode == VMPattern)	// Pattern view paint handler
		{
			DrawPatEditor(&bufDC);
		}

		CRect rc;
		GetClientRect(&rc);
		dc.BitBlt(0,0,rc.right-rc.left,rc.bottom-rc.top,&bufDC,0,0,SRCCOPY);
		bufDC.SelectObject(oldbmp);
		bufDC.DeleteDC();
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
				DrawMachine(Global::_pSong->_pMachines[updatePar], updatePar, &dc);
				DrawMachineVumeters(&dc);
				updateMode=0;
				break;
			case DMAllMacsRefresh:
				for (int i=0;i<MAX_MACHINES;i++)
				{
					if (_pSong->_machineActive[i]) 
					{
						DrawMachine(Global::_pSong->_pMachines[i], i, &dc);
						DrawMachineVumeters(&dc);
					}
				}
				break;
			}
		}
		else if (viewMode == VMPattern)	// Pattern view paint handler
		{
			DrawPatEditor(&dc);
		}
	}
}

void CChildView::Repaint(int drawMode)
{
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

void CChildView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);

	CW = cx;
	CH = cy;
	_pSong->viewSize.x=cx; // Hack to move machines boxes inside of the visible area.
	_pSong->viewSize.y=cy;
	
	if ( bmpDC != NULL && Global::pConfig->useDoubleBuffer ) // remove old buffer to force recreating it with new size
	{
		TRACE("CChildView::OnResize(). Deleted bmpDC");
		bmpDC->DeleteObject();
		delete bmpDC;
		bmpDC=NULL;
	}
	if (viewMode == VMPattern)
	{
		RecalcMetrics();
	}
	Repaint();
}

//////////////////////////////////////////////////////////////////////
// User/Mouse Responses

#include "keybhandler.cpp"
#include "mouseHandler.cpp"



//////////////////////////////////////////////////////////////////////
// "Save Song" Function

BOOL CChildView::OnFileSave() 
{
	BOOL bResult = TRUE;
	if ( Global::_pSong->_saved )
	{
		if (MessageBox("Proceed with Saving?","Song Save",MB_YESNO) == IDYES)
		{
			CString filepath = Global::pConfig->GetSongDir();
			filepath += "\\";
			filepath += Global::_pSong->fileName;
			
			OldPsyFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				MessageBox("Error creating file", "Error", MB_OK);
				return FALSE;
			}
			if (!_pSong->Save(&file))
			{
				MessageBox("Error saving file", "Error", MB_OK);
				bResult = FALSE;
			}
			else _pSong->_saved=true;
			file.Close();

			if (pUndoList)
			{
				UndoSaved = pUndoList->counter;
			}
			else
			{
				UndoSaved = 0;
			}
			SetTitleBarText();
		}
		else 
		{
			return FALSE;
		}
	}
	else 
	{
		return OnFileSavesong();
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////
// "Save Song As" Function

BOOL CChildView::OnFileSavesong() 
{
	OPENFILENAME ofn;       // common dialog box structure
	CString ifile = Global::_pSong->fileName;
	CString if2 = ifile.SpanExcluding("\\/:*\"<>|");
	
	char szFile[_MAX_PATH];

	strcpy(szFile,(LPCTSTR)if2);
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Songs\0*.psy\0Psycle Block\0*.psb\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Global::pConfig->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST;
	BOOL bResult = TRUE;
	
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn) == TRUE)
	{
		CString str = ofn.lpstrFile;
		if ( ofn.nFilterIndex == 2 ) 
		{
			CString str2 = str.Right(4);
			if ( str2.CompareNoCase(".psb") != 0 ) str.Insert(str.GetLength(),".psb");
			sprintf(szFile,str);
			FILE* hFile=fopen(szFile,"wb");
			SaveBlock(hFile);
			fflush(hFile);
			fclose(hFile);
		}
		else 
		{ 
			CString str2 = str.Right(4);
			if ( str2.CompareNoCase(".psy") != 0 ) str.Insert(str.GetLength(),".psy");
			int index = str.ReverseFind('\\');
			if (index != -1)
			{
				Global::pConfig->SetSongDir(str.Left(index));
				Global::_pSong->fileName = str.Mid(index+1);
			}
			else
			{
				Global::_pSong->fileName = str;
			}
			
			OldPsyFile file;
			if (!file.Create(str.GetBuffer(1), true))
			{
				MessageBox("Error creating file", "Error", MB_OK);
				return FALSE;
			}
			if (!_pSong->Save(&file))
			{
				MessageBox("Error saving file", "Error", MB_OK);
				bResult = FALSE;
			}
			else 
			{
				_pSong->_saved=true;
			}
			file.Close();

			AppendToRecent(str.GetBuffer(1));
			
			if (pUndoList)
			{
				UndoSaved = pUndoList->counter;
			}
			else
			{
				UndoSaved = 0;
			}
		}
		SetTitleBarText();
	}
	else
	{
		return FALSE;
	}
	return bResult;
}

void CChildView::OnFileLoadsong()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Songs\0*.psy\0Psycle Block\0*.psb\0All\0*.*\0";
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
	pParentMain->StatusBarIdle();
}

void CChildView::OnFileNew() 
{
	if (CheckUnsavedSong("New Song"))
	{
		KillUndo();
		KillRedo();
		pParentMain->CloseAllMacGuis();
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
		Global::_pSong->seqBus=0;
		pParentMain->PsybarsUpdate(); // Updates all values of the bars
		pParentMain->UpdatePlayOrder(false);
		pParentMain->WaveEditorBackUpdate();
		pParentMain->m_wndInst.WaveUpdate();
		pParentMain->UpdateSequencer();
		pParentMain->UpdateComboIns();
		RecalculateColourGrid();
		Repaint();
	}
	pParentMain->StatusBarIdle();
}


void CChildView::OnFileSaveaudio() 
{
	MessageBox("Option not developed yet","Save to wav file",MB_OK);
	// TODO: Add your command handler code here
	
}

BOOL CChildView::CheckUnsavedSong(char* szTitle)
{
	// that method does not take machine changes into account
	/*
	BOOL bChecked = TRUE;
	if (pUndoList)
	{
		if (UndoSaved != pUndoList->counter)
		{
			bChecked = FALSE;
		}
	}
	else
	{
		if (UndoSaved != 0)
		{
			bChecked = FALSE;
		}
	}

	if (!bChecked)
	*/
	{
		char szText[128];
		sprintf(szText,"Save changes to %s?",Global::_pSong->fileName);
		int result = MessageBox(szText,szTitle,MB_YESNOCANCEL | MB_ICONEXCLAMATION);
		switch (result)
		{
		case IDYES:
			return OnFileSave();
			break;
		case IDNO:
			return TRUE;
			break;
		case IDCANCEL:
			return FALSE;
			break;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Tool bar buttons and View Commands
void CChildView::OnMachineview() 
{
	if (viewMode != VMMachine)
	{
		viewMode = VMMachine;
		ShowScrollBar(SB_BOTH,FALSE);

		// set midi input mode to real-time or Step
		if(Global::pConfig->_midiMachineViewSeqMode)
			CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
		else
			CMidiInput::Instance()->m_midiMode = MODE_STEP;

		Repaint();
		pParentMain->StatusBarIdle();
	}
}
void CChildView::OnUpdateMachineview(CCmdUI* pCmdUI) 
{
	if (viewMode==VMMachine)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CChildView::OnPatternView() 
{
	if (viewMode != VMPattern)
	{
		RecalcMetrics();

		viewMode = VMPattern;
//		ShowScrollBar(SB_BOTH,FALSE);
		
		// set midi input mode to step insert
		CMidiInput::Instance()->m_midiMode = MODE_STEP;
		
		GetParent()->SetActiveWindow();

		if (( Global::pConfig->_followSong ) && ( editPosition  != Global::pPlayer->_playPosition )
			&& (Global::pPlayer->_playing) )
		{
			editPosition=Global::pPlayer->_playPosition;
		}
		Repaint();
		pParentMain->StatusBarIdle();
	}
	SetFocus();
}

void CChildView::OnUpdatePatternView(CCmdUI* pCmdUI) 
{
	if (viewMode == VMPattern)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CChildView::OnBarplay() 
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	((Master*)(Global::_pSong->_pMachines[0]))->_clip = false;

	Global::pPlayer->Start(editPosition,0);
	pParentMain->StatusBarIdle();
}
void CChildView::OnUpdateBarplay(CCmdUI* pCmdUI) 
{
	if (Global::pPlayer->_playing)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CChildView::OnButtonplayseqblock() 
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	((Master*)(Global::_pSong->_pMachines[0]))->_clip = false;

	int i=0;
	while ( Global::_pSong->playOrderSel[i] == false ) i++;
	Global::pPlayer->Start(i,0);
	Global::pPlayer->_playBlock=true;
	pParentMain->StatusBarIdle();
	if ( viewMode == VMPattern ) Repaint(DMPattern);
}
void CChildView::OnUpdateButtonplayseqblock(CCmdUI* pCmdUI) 
{
	if ( Global::pPlayer->_playBlock == true ) pCmdUI->SetCheck(TRUE);
	else pCmdUI->SetCheck(FALSE);
}

void CChildView::OnBarstop()
{
	Global::pInputHandler->Stop();
}

void CChildView::OnRecordb() 
{
	if (!Global::pPlayer->_recording)
	{
		static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
		
		CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
		if ( dlg.DoModal() == IDOK ) Global::pPlayer->StartRecording(dlg.GetFileName().GetBuffer(4));
		if ( Global::pConfig->autoStopMachines ) OnAutostop();
	}
	else
	{
		Global::pPlayer->StopRecording();
	}
}
void CChildView::OnUpdateRecordb(CCmdUI* pCmdUI) 
{
	if (Global::pPlayer->_recording)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CChildView::OnAutostop() 
{
	if ( Global::pConfig->autoStopMachines )
	{
		Global::pConfig->autoStopMachines = false;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if (Global::_pSong->_machineActive[c])
			{
				Global::_pSong->_pMachines[c]->_stopped=false;
			}
		}
	}
	else Global::pConfig->autoStopMachines = true;
}
void CChildView::OnUpdateAutostop(CCmdUI* pCmdUI) 
{
	if (Global::pConfig->autoStopMachines == true ) pCmdUI->SetCheck(TRUE);
	else pCmdUI->SetCheck(FALSE);
}

/////////////////////
//

void CChildView::OnFileSongproperties() 
{	CSongpDlg dlg;
	dlg._pSong=Global::_pSong;
	dlg.DoModal();
	pParentMain->StatusBarIdle();
//	Repaint();
}

void CChildView::OnViewInstrumenteditor()
{
	pParentMain->ShowInstrumentEditor();
}

//////////////////////////////////////////////////////////////////////
// Show the CPU Performance dialog

void CChildView::OnHelpPsycleenviromentinfo() 
{
	pParentMain->ShowPerformanceDlg();
}

//////////////////////////////////////////////////////////////////////
// Show the MIDI monitor dialog

void CChildView::OnMidiMonitorDlg() 
{
	pParentMain->ShowMidiMonitorDlg();
}

void CChildView::ShowPatternDlg(void)
{
	CPatDlg dlg;
	int patNum = _pSong->playOrder[editPosition];
	int nlines = _pSong->patternLines[patNum];
	char name[32];
	strcpy(name,_pSong->patternName[patNum]);

	dlg.patLines= nlines;
	strcpy(dlg.patName,name);
	
	if (dlg.DoModal() == IDOK)
	{
		if ( nlines != dlg.patLines )
		{
			AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
			AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
			_pSong->AllocNewPattern(patNum,dlg.patName,dlg.patLines,dlg.m_adaptsize?true:false);
			if ( strcmp(name,dlg.patName) != 0 )
			{
				strcpy(_pSong->patternName[patNum],dlg.patName);
				pParentMain->StatusBarIdle();
			}
			Repaint();
		}
		else if ( strcmp(name,dlg.patName) != 0 )
		{
			strcpy(_pSong->patternName[patNum],dlg.patName);
			pParentMain->StatusBarIdle();
//			Repaint(DMPatternHeader);
		}
	}

}

void CChildView::OnNewmachine() 
{
	// Show new machine dialog
	CNewMachine dlg;
	
	if (dlg.DoModal() == IDOK)
	{
		// Stop driver to handle possible conflicts between threads.
		_outputActive = false;
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();

		int fb;
		if (dlg.OutBus) fb = Global::_pSong->GetFreeBus();
		else fb = Global::_pSong->GetFreeFxBus();

		if ( fb == -1 || !Global::_pSong->CreateMachine((MachineType)dlg.Outputmachine, rand()/64, rand()/80, dlg.psOutputDll))
		{
			MessageBox("Machine Creation Failed","Error!",MB_OK);
		}
		else
		{
			if ( dlg.OutBus)
			{
				Global::_pSong->seqBus = fb;
				Global::_pSong->busMachine[fb] = Global::_lbc;

				if ( _pSong->_pMachines[Global::_lbc]->_type == MACH_VST ||
					_pSong->_pMachines[Global::_lbc]->_type == MACH_VSTFX )
				{
					((VSTPlugin*)(_pSong->_pMachines[Global::_lbc]))->macindex = fb;
				}
			}
			else
			{
				Global::_pSong->busEffect[fb] = Global::_lbc;

				if ( _pSong->_pMachines[Global::_lbc]->_type == MACH_VST ||
					_pSong->_pMachines[Global::_lbc]->_type == MACH_VSTFX )
				{
					((VSTPlugin*)(_pSong->_pMachines[Global::_lbc]))->macindex = fb+MAX_BUSES;
				}
			}
			
			pParentMain->UpdateComboGen();
			updatePar = Global::_lbc;
			Repaint(DMMacRefresh);
		}
		
		// Restarting the driver...
		pParentMain->UpdateEnvInfo();
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
	}
//	Repaint();
}

void CChildView::OnConfigurationSettings() 
{
	CConfigDlg dlg("Psycle Configuration");

	_outputActive = false;
	dlg.Init(Global::pConfig);
	if (dlg.DoModal() == IDOK)
	{
		_outputActive = true;
		EnableSound();
	}
//	Repaint();
	
}
void CChildView::OnHelpSaludos() 
{
	CGreetDialog dlg;
	dlg.DoModal();
//	Repaint();
}

int CChildView::SongIncBpm(int x)
{
	Global::_pSong->BeatsPerMin+=x;
	
	if (Global::_pSong->BeatsPerMin < 33)
	{
		Global::_pSong->BeatsPerMin = 33;
	}
	if (Global::_pSong->BeatsPerMin > 999)
	{
		Global::_pSong->BeatsPerMin=999;
	}
	Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin, Global::_pSong->_ticksPerBeat, Global::pConfig->_pOutputDriver->_samplesPerSec);
	
	return Global::_pSong->BeatsPerMin;
}


#define TWOPI_F (2.0f*3.141592665f)

void CChildView::ShowSwingFillDlg(bool bTrackMode)
{
	int st = Global::_pSong->BeatsPerMin;
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
			ny = _pSong->patternLines[_ps()];
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
		AddUndo(_ps(),x,y,1,ny,editcur.track,editcur.line,editcur.col,editPosition);
		int displace=_ps()*MULTIPLY2;
		for (l=y;l<y+ny;l++)
		{
			int const displace2=x*5+l*MULTIPLY;
			
			unsigned char *offset=_pSong->pPatternData+displace+displace2;
			
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
//		drawTrackStart=x;
//		drawTrackEnd=x;
//		drawLineStart=y;
//		drawLineEnd=y+ny;
		NewPatternDraw(x,x,y,y+ny);	
		Repaint(DMData);
	}
}


///////////////////////////////////////////////////////////////////////
// Right Click Popup Menu

void CChildView::OnPopCut() { CopyBlock(true); }
void CChildView::OnUpdatePopCut(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopCopy() { CopyBlock(false); }
void CChildView::OnUpdatePopCopy(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopPaste() { PasteBlock(editcur.track,editcur.line,false); }
void CChildView::OnUpdatePopPaste(CCmdUI* pCmdUI) 
{
	if (isBlockCopied) pCmdUI->Enable(TRUE);
	else  pCmdUI->Enable(FALSE);
}

void CChildView::OnPopMixpaste() { PasteBlock(editcur.track,editcur.line,true); }
void CChildView::OnUpdatePopMixpaste(CCmdUI* pCmdUI) 
{
	if (isBlockCopied) pCmdUI->Enable(TRUE);
	else  pCmdUI->Enable(FALSE);
}

void CChildView::OnPopInterpolate() { BlockParamInterpolate(); }
void CChildView::OnUpdatePopInterpolate(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopChangegenerator() { BlockGenChange(_pSong->seqBus); }
void CChildView::OnUpdatePopChangegenerator(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopChangeinstrument() { BlockInsChange(_pSong->auxcolSelected); }
void CChildView::OnUpdatePopChangeinstrument(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopTranspose1() { BlockTranspose(1); }
void CChildView::OnUpdatePopTranspose1(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopTranspose12() { BlockTranspose(12); }
void CChildView::OnUpdatePopTranspose12(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopTranspose_1() { BlockTranspose(-1); }
void CChildView::OnUpdatePopTranspose_1(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopTranspose_12() { BlockTranspose(-12); }
void CChildView::OnUpdatePopTranspose_12(CCmdUI* pCmdUI) 
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopPattenproperties() 
{
	ShowPatternDlg();
}

void CChildView::OnPopBlockSwingfill()
{
	// fill block
	ShowSwingFillDlg(FALSE);
}

void CChildView::OnUpdatePopBlockSwingfill(CCmdUI* pCmdUI)
{
	if ( blockSelected ) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnPopTrackSwingfill()
{
	// fill track
	ShowSwingFillDlg(TRUE);
}

void CChildView::OnUpdateUndo(CCmdUI* pCmdUI)
{
	if(pUndoList) 
	{
		switch (pUndoList->type)
		{
		case UNDO_SEQUENCE:
			pCmdUI->Enable(TRUE);
			pCmdUI->SetText("Undo");
			break;
		default:
			if(viewMode == VMPattern)// && bEditMode)
			{
				pCmdUI->Enable(TRUE);
				pCmdUI->SetText("Undo");
			}
			else
			{
				pCmdUI->Enable(FALSE);
				pCmdUI->SetText("Undo in Pattern View");
			}
			break;
		}
	}
	else
	{
		pCmdUI->SetText("Undo");
		pCmdUI->Enable(FALSE);
	}
}

void CChildView::OnUpdateRedo(CCmdUI* pCmdUI)
{
	if(pRedoList) 
	{
		switch (pRedoList->type)
		{
		case UNDO_SEQUENCE:
			pCmdUI->Enable(TRUE);
			pCmdUI->SetText("Redo");
			break;
		default:
			if(viewMode == VMPattern)// && bEditMode)
			{
				pCmdUI->Enable(TRUE);
				pCmdUI->SetText("Redo");
			}
			else
			{
				pCmdUI->Enable(FALSE);
				pCmdUI->SetText("Redo in Pattern View");
			}
			break;
		}
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText("Redo");
	}
}

void CChildView::OnUpdatePatternCutCopyPaste(CCmdUI* pCmdUI) 
{
	if(viewMode == VMPattern) pCmdUI->Enable(TRUE);
	else pCmdUI->Enable(FALSE);
}

void CChildView::OnFileImportXmfile() 
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "XM Songs\0*.xm\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Global::pConfig->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		KillUndo();
		KillRedo();

		pParentMain->CloseAllMacGuis();
		Global::pPlayer->Stop();
		Sleep(LOCK_LATENCY);
		_outputActive = false;
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();
		Sleep(LOCK_LATENCY);
		
		CFileXM XM;
		XM.Open(ofn.lpstrFile);

		//CXM XM;
		char buffer[512];		
		Global::_pSong->New();
		
 		if(!XM.Import(_pSong))
		{			
			MessageBox("Load failed");
			Global::_pSong->New();
			return;
		}

		// build sampler
		_pSong->CreateMachine(MACH_SAMPLER, rand()/64, rand()/80, "");
		_pSong->InsertConnection(1,0);
		_pSong->seqBus = _pSong->GetFreeBus();
		_pSong->busMachine[_pSong->seqBus] = Global::_lbc;

		sprintf(buffer,"%s\n\n%s\n\n%s"
			,Global::_pSong->Name
			,Global::_pSong->Author
			,Global::_pSong->Comment);
		MessageBox(buffer,"XM file imported",MB_OK);
		Global::_pSong->_saved=true;

		CString str = ofn.lpstrFile;
		int index = str.ReverseFind('\\');
		if (index != -1)
		{
			Global::pConfig->SetSongDir(str.Left(index));
			Global::_pSong->fileName = str.Mid(index+1)+".psy";
		}
		else
		{
			Global::_pSong->fileName = str+".psy";
		}
		
		Global::_pSong->fileName =str+".psy";
		//Global::_pSong->SetBPM(XM.default_BPM, XM.default_tempo, Global::pConfig->_pOutputDriver->_samplesPerSec);

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
		pParentMain->PsybarsUpdate();
		pParentMain->WaveEditorBackUpdate();
		pParentMain->m_wndInst.WaveUpdate();
		pParentMain->UpdateSequencer();
		pParentMain->UpdatePlayOrder(false);
		RecalculateColourGrid();
		Repaint();
	}
	SetTitleBarText();
}

void CChildView::OnFileImportItfile() 
{
	MessageBox("Option not developed yet","Import IT file",MB_OK);
	
/*	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Impulse Tracker Songs\0*.it\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Global::pConfig->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{

		pParentMain->CloseAllMacGuis();
		Global::pPlayer->Stop();
		Sleep(LOCK_LATENCY);
		_outputActive = false;
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();
		Sleep(LOCK_LATENCY);
		
		CFileIT it;
		it.Open(ofn.lpstrFile);

		//CXM XM;
		char buffer[512];		
		Global::_pSong->New();
		
 		if(!it.Import(_pSong))
		{			
			MessageBox("Load failed");
			Global::_pSong->New();
			return;
		}

		// build sampler
		_pSong->CreateMachine(MACH_SAMPLER, rand()/64, rand()/80, "");
		_pSong->InsertConnection(1,0);
		_pSong->seqBus = _pSong->GetFreeBus();
		_pSong->busMachine[_pSong->seqBus] = Global::_lbc;

		sprintf(buffer,"%s\n\n%s\n\n%s"
			,Global::_pSong->Name
			,Global::_pSong->Author
			,Global::_pSong->Comment);
		MessageBox(buffer,"IT file imported",MB_OK);
		Global::_pSong->_saved=true;

		CString str = ofn.lpstrFile;
		int index = str.ReverseFind('\\');
		if (index != -1)
		{
			Global::pConfig->SetSongDir(str.Left(index));
			Global::_pSong->fileName = str.Mid(index+1)+".psy";
		}
		else
		{
			Global::_pSong->fileName = str+".psy";
		}
		
		Global::_pSong->fileName =str+".psy";
		//Global::_pSong->SetBPM(XM.default_BPM, XM.default_tempo, Global::pConfig->_pOutputDriver->_samplesPerSec);
		
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
		Repaint();
		pParentMain->PsybarsUpdate();
		pParentMain->WaveEditorBackUpdate();
		pParentMain->m_wndInst.WaveUpdate();
		pParentMain->UpdateSequencer();
		pParentMain->UpdatePlayOrder(false);
	}
	SetTitleBarText();
*/	
}

void CChildView::AppendToRecent(char* fName)
{
	int iCount;
	char* nameBuff;
	UINT nameSize;

	HMENU hFileMenu, hRootMenuBar;
		
	UINT ids[] = {ID_FILE_RECENT_01,
				ID_FILE_RECENT_02,
				ID_FILE_RECENT_03,
				ID_FILE_RECENT_04};

	MENUITEMINFO hNewItemInfo, hTempItemInfo;

	hRootMenuBar = ::GetMenu(this->GetParent()->m_hWnd);
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
		nameBuff = new char[nameSize];
		GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
		if ( !strcmp(nameBuff, fName) )
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
	hNewItemInfo.cch		= strlen(fName);
	hNewItemInfo.dwTypeData = fName;
	
	InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);

	//Update identifiers.
	for (iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
	{
		hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
		hTempItemInfo.fMask		= MIIM_ID;
		hTempItemInfo.wID		= ids[iCount];

		SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
	}

}

void CChildView::OnFileRecent_01()
{
	CallOpenRecent(0);
}
void CChildView::OnFileRecent_02()
{
	CallOpenRecent(1);
}
void CChildView::OnFileRecent_03()
{
	CallOpenRecent(2);
}
void CChildView::OnFileRecent_04()
{
	CallOpenRecent(3);
}

void CChildView::OnFileLoadsongNamed(char* fName, int fType)
{
	if ( fType == 2 )
	{
		FILE* hFile=fopen(fName,"rb");
		LoadBlock(hFile);
		fclose(hFile);
	}
	else
	{
		if (CheckUnsavedSong("Load Song"))
		{
			pParentMain->CloseAllMacGuis();
			Global::pPlayer->Stop();
			Sleep(LOCK_LATENCY);
			_outputActive = false;
			Global::pConfig->_pOutputDriver->Enable(false);
			// MIDI IMPLEMENTATION
			Global::pConfig->_pMidiInput->Close();
			Sleep(LOCK_LATENCY);
			
			OldPsyFile file;
			if (!file.Open(fName))
			{
				MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
				return;
			}
			_pSong->Load(&file);
			file.Close();
			
			_pSong->_saved=true;

			//!Fidelooop!!//
			AppendToRecent(fName);
			CString str = fName;
			int index = str.ReverseFind('\\');
			if (index != -1)
			{
				Global::pConfig->SetSongDir(str.Left(index));
				Global::_pSong->fileName = str.Mid(index+1);
			}
			else
			{
				Global::_pSong->fileName = str;
			}
			
			Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin, Global::_pSong->_ticksPerBeat, Global::pConfig->_pOutputDriver->_samplesPerSec);

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
			editPosition=0;
			Global::_pSong->seqBus=0;
			pParentMain->PsybarsUpdate();
			pParentMain->WaveEditorBackUpdate();
			pParentMain->m_wndInst.WaveUpdate();
			pParentMain->UpdateSequencer();
			pParentMain->UpdatePlayOrder(false);
			pParentMain->UpdateComboIns();
			RecalculateColourGrid();
			Repaint();
			KillUndo();
			KillRedo();
			SetTitleBarText();

			char buffer[512];
			sprintf(buffer,"'%s'\n\n%s\n\n%s"
				,_pSong->Name
				,_pSong->Author
				,_pSong->Comment);
			
			MessageBox(buffer,"Psycle song loaded",MB_OK);
		}
	}
}

void CChildView::CallOpenRecent(int pos)
{
	char* nameBuff;
	UINT nameSize;

	nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
	nameBuff = new char[nameSize];
	GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
	
	OnFileLoadsongNamed(nameBuff, 1);
	delete nameBuff;
}

void CChildView::SetTitleBarText()
{
	CString titlename = "[";
	titlename+=Global::_pSong->fileName;
	if (pUndoList)
	{
		if (UndoSaved != pUndoList->counter)
		{
			titlename+=" *";
		}
	}
	else
	{
		if (UndoSaved != 0)
		{
			titlename+=" *";
		}
	}
	titlename+="] ";
	titlename+="Psycle Modular Music Creation Studio";	// I don't know how to access to the
	pParentMain->SetWindowText(titlename);				// IDR_MAINFRAME String Title.
}


void CChildView::OnHelpKeybtxt() 
{
	ShellExecute(pParentMain->m_hWnd,"open","Docs\\keys.txt",NULL,"",SW_SHOW);
}

void CChildView::OnHelpReadme() 
{
	ShellExecute(pParentMain->m_hWnd,"open","Docs\\readme.txt",NULL,"",SW_SHOW);
}

void CChildView::OnHelpTweaking() 
{
	ShellExecute(pParentMain->m_hWnd,"open","Docs\\tweaking.txt",NULL,"",SW_SHOW);
}

void CChildView::OnHelpWhatsnew() 
{
	ShellExecute(pParentMain->m_hWnd,"open","Docs\\whatsnew.txt",NULL,"",SW_SHOW);
}

void CChildView::LoadMachineSkin()
{
	static char szOld[64] = "";
	if (strcmp(szOld, Global::pConfig->machine_skin))
	{
		strcpy(szOld, Global::pConfig->machine_skin);
		// ok so...
		if (strcmp(szOld, DEFAULT_MACHINE_SKIN))
		{
			BOOL result = FALSE;
			FindMachineSkin(Global::pConfig->GetInitialSkinDir(),Global::pConfig->machine_skin, &result);
			if (result)
			{
				return;
			}
		}
		// load defaults
		strcpy(szOld, DEFAULT_MACHINE_SKIN);
		// and coords
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
		MachineCoords.sGeneratorPan.x = 102;
		MachineCoords.sGeneratorPan.y = 144;
		MachineCoords.sGeneratorPan.width = 24;
		MachineCoords.sGeneratorPan.height = 9;
		MachineCoords.sGeneratorMute.x = 133;
		MachineCoords.sGeneratorMute.y = 144;
		MachineCoords.sGeneratorMute.width = 7;
		MachineCoords.sGeneratorMute.height = 7;
		MachineCoords.sGeneratorSolo.x = 140;
		MachineCoords.sGeneratorSolo.y = 144;
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
		MachineCoords.sEffectPan.x = 102;
		MachineCoords.sEffectPan.y = 144;
		MachineCoords.sEffectPan.width = 24;
		MachineCoords.sEffectPan.height = 9;
		MachineCoords.sEffectMute.x = 133;
		MachineCoords.sEffectMute.y = 144;
		MachineCoords.sEffectMute.width = 7;
		MachineCoords.sEffectMute.height = 7;
		MachineCoords.sEffectBypass.x = 126;
		MachineCoords.sEffectBypass.y = 144;
		MachineCoords.sEffectBypass.width = 7;
		MachineCoords.sEffectBypass.height = 13;

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

		machineskin.DeleteObject();
		DeleteObject(hbmMachineSkin);
		machineskinmask.DeleteObject();
		machineskin.LoadBitmap(IDB_MACHINE_SKIN);
	}
}

void CChildView::FindMachineSkin(CString findDir, CString findName, BOOL *result)
{
	CFileFind finder;

	int loop = finder.FindFile(findDir + "\\*.");	// check for subfolders.
	while (loop) 
	{								// Note: Subfolders with dots won't work.
		loop = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindMachineSkin(finder.GetFilePath(),findName,result);
		}
	}
	finder.Close();

	loop = finder.FindFile(findDir + "\\" + findName + ".psm"); // check if the directory is empty
	while (loop)
	{
		loop = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			CString sName, tmpPath;
			sName = finder.GetFileName();
			// ok so we have a .psm, does it have a valid matching .bmp?
			char* pExt = strrchr(sName,46);// last .
			pExt[0]=0;
			char szOpenName[MAX_PATH];
			sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);

			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			machineskinmask.DeleteObject();
			hbmMachineSkin = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hbmMachineSkin)
			{
				if (machineskin.Attach(hbmMachineSkin))
				{	
					memset(&MachineCoords,0,sizeof(MachineCoords));
					// load settings
					FILE* hfile;
					sprintf(szOpenName,"%s\\%s.psm",findDir,sName);
					if ((hfile=fopen(szOpenName,"rw")) == NULL )
					{
						MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
						return;
					}
					char buf[512];
					while (fgets(buf, 512, hfile))
					{
						if (strstr(buf,"\"master_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sMaster.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sMaster.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sMaster.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sMaster.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sGenerator.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGenerator.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGenerator.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGenerator.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_vu0_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorVu0.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorVu0.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorVu0.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVu0.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_vu_peak_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorVuPeak.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorVuPeak.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorVuPeak.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVuPeak.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_pan_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorPan.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorPan.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorPan.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorPan.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_mute_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorMute.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorMute.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorMute.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorMute.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_solo_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sGeneratorSolo.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sGeneratorSolo.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sGeneratorSolo.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorSolo.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffect.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffect.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffect.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffect.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_vu0_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectVu0.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectVu0.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectVu0.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVu0.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_vu_peak_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectVuPeak.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectVuPeak.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectVuPeak.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVuPeak.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_pan_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectPan.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectPan.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectPan.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectPan.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_mute_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectMute.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectMute.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectMute.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectMute.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_bypass_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.sEffectBypass.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.sEffectBypass.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.sEffectBypass.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectBypass.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_vu_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorVu.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorVu.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dGeneratorVu.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorVu.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_pan_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorPan.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorPan.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dGeneratorPan.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorPan.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"generator_mute_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorMute.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorMute.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"generator_solo_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorSolo.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorSolo.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"generator_name_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dGeneratorName.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dGeneratorName.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"effect_vu_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectVu.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectVu.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dEffectVu.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectVu.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_pan_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectPan.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectPan.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										MachineCoords.dEffectPan.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectPan.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"effect_mute_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectMute.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectMute.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"effect_bypass_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectBypass.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectBypass.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"effect_name_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								MachineCoords.dEffectName.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									MachineCoords.dEffectName.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"transparency\"="))
						{
							char *q = strchr(buf,61); // =
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
					*result = TRUE;
					break;
				}
			}
		}
	}
	finder.Close();
}

void CChildView::LoadPatternHeaderSkin()
{
	static char szOld[64] = "";
	if (strcmp(szOld, Global::pConfig->pattern_header_skin))
	{
		strcpy(szOld, Global::pConfig->pattern_header_skin);
		// ok so...
		if (strcmp(szOld, DEFAULT_PATTERN_HEADER_SKIN))
		{
			BOOL result = FALSE;
			FindPatternHeaderSkin(Global::pConfig->GetInitialSkinDir(),Global::pConfig->pattern_header_skin, &result);
			if (result)
			{
				return;
			}
		}
		// load defaults
		strcpy(szOld, DEFAULT_PATTERN_HEADER_SKIN);
		// and coords
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
		PatHeaderCoords.dDigitX0.x = 22;
		PatHeaderCoords.dDigitX0.y = 2;
		PatHeaderCoords.dDigit0X.x = 29;
		PatHeaderCoords.dDigit0X.y = 2;
		PatHeaderCoords.dRecordOn.x = 52;
		PatHeaderCoords.dRecordOn.y = 5;
		PatHeaderCoords.dMuteOn.x = 75;
		PatHeaderCoords.dMuteOn.y = 5;
		PatHeaderCoords.dSoloOn.x = 96;
		PatHeaderCoords.dSoloOn.y = 5;
		PatHeaderCoords.bHasTransparency = FALSE;

		patternheader.DeleteObject();
		DeleteObject(hbmPatHeader);
		patternheadermask.DeleteObject();
		patternheader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
	}
}

void CChildView::FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result)
{
	CFileFind finder;

	int loop = finder.FindFile(findDir + "\\*.");	// check for subfolders.
	while (loop) 
	{								// Note: Subfolders with dots won't work.
		loop = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindPatternHeaderSkin(finder.GetFilePath(),findName,result);
		}
	}
	finder.Close();

	loop = finder.FindFile(findDir + "\\" + findName + ".psh"); // check if the directory is empty
	while (loop)
	{
		loop = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			CString sName, tmpPath;
			sName = finder.GetFileName();
			// ok so we have a .psh, does it have a valid matching .bmp?
			char* pExt = strrchr(sName,46);// last .
			pExt[0]=0;
			char szOpenName[MAX_PATH];
			sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);

			patternheader.DeleteObject();
			DeleteObject(hbmPatHeader);
			patternheadermask.DeleteObject();
			hbmPatHeader = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hbmPatHeader)
			{
				if (patternheader.Attach(hbmPatHeader))
				{	
					memset(&PatHeaderCoords,0,sizeof(PatHeaderCoords));
					// load settings
					FILE* hfile;
					sprintf(szOpenName,"%s\\%s.psh",findDir,sName);
					if ((hfile=fopen(szOpenName,"rw")) == NULL )
					{
						MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
						return;
					}
					char buf[512];
					while (fgets(buf, 512, hfile))
					{
						if (strstr(buf,"\"background_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sBackground.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sBackground.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sBackground.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sBackground.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"number_0_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sNumber0.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sNumber0.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sNumber0.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sNumber0.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"record_on_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sRecordOn.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sRecordOn.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sRecordOn.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sRecordOn.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"mute_on_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sMuteOn.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sMuteOn.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sMuteOn.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sMuteOn.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"solo_on_source\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.sSoloOn.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.sSoloOn.y = atoi(q+1);
									q = strchr(q+1,44); // ,
									if (q)
									{
										PatHeaderCoords.sSoloOn.width = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sSoloOn.height = atoi(q+1);
										}
									}
								}
							}
						}
						else if (strstr(buf,"\"digit_x0_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dDigitX0.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dDigitX0.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"digit_0x_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dDigit0X.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dDigit0X.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"record_on_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dRecordOn.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dRecordOn.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"mute_on_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dMuteOn.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dMuteOn.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"solo_on_dest\"="))
						{
							char *q = strchr(buf,61); // =
							if (q)
							{
								PatHeaderCoords.dSoloOn.x = atoi(q+1);
								q = strchr(q+1,44); // ,
								if (q)
								{
									PatHeaderCoords.dSoloOn.y = atoi(q+1);
								}
							}
						}
						else if (strstr(buf,"\"transparency\"="))
						{
							char *q = strchr(buf,61); // =
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
					*result = TRUE;
					break;
				}
			}
		}
	}
	finder.Close();
}


void CChildView::RecalcMetrics()
{
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
	ROWWIDTH = COLX[9];
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
}


void CChildView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
{
   BITMAP bm;

   // Get the dimensions of the source bitmap
   pBmpSource->GetObject(sizeof(BITMAP), &bm);

   // Create the mask bitmap
   pBmpMask->DeleteObject();
   pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);

   // We will need two DCs to work with. One to hold the Image
   // (the source), and one to hold the mask (destination).
   // When blitting onto a monochrome bitmap from a color, pixels
   // in the source color bitmap that are equal to the background
   // color are blitted as white. All the remaining pixels are
   // blitted as black.

   CDC hdcSrc, hdcDst;

   hdcSrc.CreateCompatibleDC(NULL);
   hdcDst.CreateCompatibleDC(NULL);

   // Load the bitmaps into memory DC
   CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
   CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);

   // Change the background to trans color
   hdcSrc.SetBkColor(clrTrans);

   // This call sets up the mask bitmap.
   hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);

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

   hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);

   // Clean up by deselecting any objects, and delete the
   // DC's.
   hdcSrc.SelectObject(hbmSrcT);
   hdcDst.SelectObject(hbmDstT);

   hdcSrc.DeleteDC();
   hdcDst.DeleteDC();
}

void CChildView::TransparentBlt(CDC* pDC,
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
   hdcMem.CreateCompatibleDC(pDC);

   CBitmap* hbmT = hdcMem.SelectObject(bmpMask);

   pDC->SetTextColor(RGB(0,0,0));
   pDC->SetBkColor(RGB(255,255,255));

   pDC->BitBlt( xStart, yStart, wWidth, wHeight, &hdcMem, xSource, ySource, 
	   SRCAND);

   // Also note the use of SRCPAINT rather than SRCCOPY.

   pDC->BitBlt(xStart, yStart, wWidth, wHeight, pTmpDC, xSource, ySource,
		SRCPAINT);

   // Now, clean up.
   hdcMem.SelectObject(hbmT);
   hdcMem.DeleteDC();
}

