//
// Designed / Lead Coder / Programming : Juan Antonio Arguelles Rius
//
// [C] 2000 All Rights Reserved
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

//#include "Dsp.h"
//#include "Filter.h"

#include "MainFrm.h"
#include "WireDlg.h"
#include "MacProp.h"
#include "NewMachine.h"
#include "PatDlg.h"
#include "SaveDlg.h"
#include "GreetDialog.h"
#include "ConfigDlg.h"
#include "SongpDlg.h"
#include "inputhandler.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////

//////////////////////////////////////////////////////////////////////
// Enviroment Initialization

CMainFrame		*pParentMain;

Bitmap mv_bg;
Bitmap pv_bg;

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
	seqFont.CreatePointFont(80,"Tahoma");
	for (int c=0; c<256; c++)	{ FLATSIZES[c]=8; }
	bmpDC = NULL;
	
	viewMode=VMMachine;
	updateMode=0;
	updatePar=0;
	multiPattern=true; // Long way till it can be finished!

	patStep=1;
	editPosition=0;
	bEditMode = false;

	_followSong = false;
	_previousTicks=0;
	
	blockSelected=false;
	isBlockCopied=false;
	patBufferCopy=false;
	blockNTracks=0;
	blockNLines=0;

	editcur.track=0;
	editcur.col=0;
	editcur.line=0;
	playpos.bottom=0;
	newplaypos.bottom=0; newplaypos.left=0; newplaypos.right=XOFFSET-2;
	guipos.bottom=0;	guipos.top=0;	guipos.left=0;	guipos.right=0;
	selpos.bottom=0;
	newselpos.bottom=0;

	drawTrackStart=-1;

	scrollT=0;
	scrollL=0;
	tOff=0;
	lOff=0;
	ntOff=0;
	nlOff=0;

	Global::pInputHandler->SetChildView(this);
	Global::pResampler->SetQuality(RESAMPLE_LINEAR);
	_outputActive = false;

//	_getcwd(m_appdir,_MAX_PATH);
	
	stuffbmp.LoadBitmap(IDB_STUFF);

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
	
	Global::pConfig->Write();
	Global::pPlayer->Stop();
	_outputActive = false;
	Global::pConfig->_pOutputDriver->Enable(false);
	// MIDI IMPLEMENTATION
	Global::pConfig->_pMidiInput->Close();
	Sleep(LOCK_LATENCY);
	CNewMachine::DestroyPluginInfo();
	Global::pInputHandler->SetChildView(NULL);
	seqFont.DeleteObject();
	stuffbmp.DeleteObject();

	if ( bmpDC != NULL )
	{
		bmpDC->DeleteObject();
		delete bmpDC;
	}
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
	if (!SetTimer(31,10,NULL) || !SetTimer(159,600000,NULL)) // 1st Timer: GUI update. 2nd Timer: AutoSave
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
			((Master*)Global::_pSong->_pMachines[0])->_LMAX,
			((Master*)Global::_pSong->_pMachines[0])->_RMAX,
			Global::pConfig->vu1,
			Global::pConfig->vu2,
			Global::pConfig->vu3,
			((Master*)Global::_pSong->_pMachines[0])->_clip);

		if (viewMode == VMMachine)
		{
			CClientDC dc(this);
			DrawMachineVumeters(&dc);
		}
		if (Global::_pSong->Tweaker)
		{
			for(int c=0; c<MAX_MACHINES; c++)
			{
				if (pParentMain->isguiopen[c] && _pSong->_pMachines[c]->_type != MACH_VST &&
					_pSong->_pMachines[c]->_type != MACH_VSTFX) // VST do auto refresh.
					pParentMain->m_pWndMac[c]->Invalidate(false);
			}
			Global::_pSong->Tweaker = false;
		}

		if (Global::pPlayer->_playing)
		{
			if (Global::pPlayer->_lineChanged)
			{
				Global::pPlayer->_lineChanged = false;
				char buf[80];
				sprintf(buf,"[Playing] Pos: %.2X   Pat: %.2X   Lin: %.2d ,   [Elapsed Time]: %.2d : %.2d : %.2f",
					Global::pPlayer->_playPosition,
					Global::pPlayer->_playPattern,
					Global::pPlayer->_lineCounter,
					Global::pPlayer->_playTimem / 60,
					Global::pPlayer->_playTimem % 60,
					Global::pPlayer->_playTime);
				
				pParentMain->StatusBarText(buf);
				pParentMain->SetAppSongBpm(0);
				pParentMain->SetAppSongTpb(0);

				if (_followSong)
				{
					CListBox* pSeqList = (CListBox*)pParentMain->m_wndSeq.GetDlgItem(IDC_SEQLIST);
					editcur.line=Global::pPlayer->_lineCounter;

/*
					// this was for the old messy multi track hack method

					if ( _previousTicks-1 > editcur.track )
					{
						_previousTicks-=editcur.track+1;
						editcur.track = _pSong->SONGTRACKS-1;
					}
					if (_previousTicks>1) 
						editcur.track-= _previousTicks-1;
*/
 					if (pSeqList->GetCurSel() != Global::pPlayer->_playPosition)
					{
						pSeqList->SelItemRange(false,0,pSeqList->GetCount());
						pSeqList->SetSel(Global::pPlayer->_playPosition,true);
						editPosition=Global::pPlayer->_playPosition;
						if ( viewMode == VMPattern ) Repaint(DMPatternChange);
					}
					else if( viewMode == VMPattern ) Repaint(DMPlayback);
				}
				else if ( viewMode == VMPattern ) Repaint(DMPlayback);

				_previousTicks=0;
			}
		}
	}
	if (nIDEvent == 159 )
	{
		if ( Global::_pSong->_saved ) 
		{
			CString filepath = Global::pConfig->GetInitialSongDir();
			filepath += "\\autosave.psy";
			CSaveDlg dlg;
			dlg._pSong = Global::_pSong;
			sprintf(dlg.szFile, filepath);
			dlg.SaveSong();
		}
	}
}


void CChildView::OnActivate()
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

void CChildView::OnPaint() 
{
	CPaintDC dc(this);

	if ( bmpDC == NULL && Global::pConfig->useDoubleBuffer ) // buffer creation
	{
		CRect rc;
		GetClientRect(&rc);
		bmpDC = new CBitmap;
		bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);

	}
	else if ( bmpDC != NULL && !Global::pConfig->useDoubleBuffer ) // buffer deletion
	{
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
			case 0:
				DrawMachineEditor(&bufDC);
				break;
			case 1:
				DrawMachine(Global::_pSong->_pMachines[updatePar], updatePar, &bufDC);
				updateMode=0;
				break;
			}
		}
		else if (viewMode == VMPattern)	// Pattern view paint handler
		{
			if ( updateMode != DMNone ) DrawPatEditor(&bufDC);
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
			case 0:
				DrawMachineEditor(&dc);
				break;
			case 1:
				DrawMachine(Global::_pSong->_pMachines[updatePar], updatePar, &dc);
				updateMode=0;
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
	if ( viewMode == VMPattern ) PreparePatternRefresh(drawMode);
	else if ( viewMode == VMMachine )
	{
		updateMode = drawMode;
		Invalidate(false);
	}
}

void CChildView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
//	CRect rClient;
//	GetClientRect(&rClient);
//	CW = rClient.Width();
//	CH = rClient.Height();
	CW = cx;
	CH = cy;
	_pSong->viewSize.x=cx-148;
	_pSong->viewSize.y=cy-48;

	newplaypos.right=CW;
	VISLINES = (CH-YOFFSET)/ROWHEIGHT;
	VISTRACKS = (CW-XOFFSET)/ROWWIDTH;

	if (VISLINES < 1) { VISLINES = 1; }
	if (VISTRACKS < 1) { VISTRACKS = 1; }

	if ( bmpDC != NULL && Global::pConfig->useDoubleBuffer ) // buffer size change
	{
		bmpDC->DeleteObject();
		delete bmpDC;
		bmpDC=NULL;
	}
	Repaint();
}

//////////////////////////////////////////////////////////////////////
// User/Mouse Responses

#include "keybhandler.cpp"
#include "mouseHandler.cpp"



//////////////////////////////////////////////////////////////////////
// "Save Song" Function

void CChildView::OnFileSave() 
{
	if ( Global::_pSong->_saved )
	{
		if (MessageBox("Proceed with Saving?","Song Save",MB_YESNO) == IDYES)
		{
			CString filepath = Global::pConfig->GetSongDir();
			filepath += "\\";
			filepath += Global::_pSong->fileName;
			CSaveDlg dlg;
			dlg._pSong = Global::_pSong;
			sprintf(dlg.szFile, filepath);
			dlg.SaveSong();
	//		AfxMessageBox(IDS_SONG_SAVED,MB_ICONINFORMATION); <- Needed or not?
		}

	}
	else OnFileSavesong();
	
}

//////////////////////////////////////////////////////////////////////
// "Save Song As" Function

void CChildView::OnFileSavesong() 
{
	OPENFILENAME ofn;       // common dialog box structure
	CString ifile = Global::_pSong->Name;
	ifile += ".psy";
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
	
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn) == TRUE)
	{
		CString str = ofn.lpstrFile;
		if ( ofn.nFilterIndex == 2 ) {
			CString str2 = str.Right(4);
			if ( str2.CompareNoCase(".psb") != 0 ) str.Insert(str.GetLength(),".psb");
			sprintf(szFile,str);
			FILE* hFile=fopen(szFile,"wb");
			SaveBlock(hFile);
			fflush(hFile);
			fclose(hFile);
		}
		else { 
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

			CSaveDlg dlg;

			dlg._pSong = Global::_pSong;
			strcpy(dlg.szFile, str);
			dlg.SaveSong();

			AppendToRecent(dlg.szFile);
		}
		CString titlename = "[";
		titlename+=Global::_pSong->fileName;
		titlename+="] ";
		titlename+="Psycle Modular Music Creation Studio";	// I don't know how to access to the
		pParentMain->SetWindowText(titlename);				// IDR_MAINFRAME String Title.
	}
	Repaint();
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
}

void CChildView::OnFileNew() 
{
	if (MessageBox("Are you sure?","New song",MB_YESNO | MB_ICONWARNING)==IDYES)
	{
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
		CString titlename = "[";
		titlename+=Global::_pSong->fileName;
		titlename+="] ";
		titlename+="Psycle Modular Music Creation Studio";	// I don't know how to access to the
		pParentMain->SetWindowText(titlename);				// IDR_MAINFRAME String Title.
		editPosition=0;
		Global::_pSong->seqBus=0;
		pParentMain->PsybarsUpdate(); // Updates all values of the bars
		pParentMain->UpdatePlayOrder(false);
		pParentMain->WaveEditorBackUpdate();
		pParentMain->UpdateSequencer();
		Repaint();
	}
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
		viewMode = VMPattern;
//		ShowScrollBar(SB_BOTH,FALSE);
		
		// set midi input mode to step insert
		CMidiInput::Instance()->m_midiMode = MODE_STEP;
		
		GetParent()->SetActiveWindow();

		if (( _followSong ) && ( editPosition  != Global::pPlayer->_playPosition )
			&& (Global::pPlayer->_playing) )
		{
			editPosition=Global::pPlayer->_playPosition;
		}
		Repaint();
	}
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
	((Master*)(Global::_pSong->_pMachines[0]))->_clip = false;

	Global::pPlayer->_playPosition = editPosition;
	Global::pPlayer->Start(0);
	_previousTicks=0;
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
	Global::pPlayer->Stop();
	Global::pPlayer->_playPosition = editPosition;
	Global::pPlayer->Start(0);
	int i=0;
	while ( Global::_pSong->playOrderSel[i] == false ) i++;
	Global::pPlayer->_playPosition = i;
	Global::pPlayer->_playTime = 0;
	Global::pPlayer->_playTimem = 0;
	Global::pPlayer->_playPattern = Global::_pSong->playOrder[i];
	Global::pPlayer->_playBlock=true;
	if ( viewMode == VMPattern ) Repaint(DMPatternChange);
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
		if ( Global::pConfig->autoStopMachines ) Global::pConfig->autoStopMachines = false;
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
	Repaint();
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
			_pSong->AllocNewPattern(patNum,dlg.patName,dlg.patLines,dlg.m_adaptsize?true:false);
			Repaint(DMPatternChange);
		}
		if ( strcmp(name,dlg.patName) != 0 )
		{
			strcpy(_pSong->patternName[patNum],dlg.patName);
			Repaint(DMPatternHeader);
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
			}
			else
			{
				Global::_pSong->busEffect[fb] = Global::_lbc;
			}
			pParentMain->UpdateComboGen();
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
	Repaint();
}

void CChildView::OnConfigurationSettings() 
{
	CConfigDlg dlg("Psycle Configuration");

	dlg.Init(Global::pConfig);
	if (dlg.DoModal() == IDOK)
	{
		_outputActive = true;
		Global::pConfig->_pOutputDriver->Enable(true);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Open();
	}
	Repaint();
	
}
void CChildView::OnHelpSaludos() 
{
	CGreetDialog dlg;
	dlg.DoModal();
	Repaint();
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

void CChildView::SetPatStep(int stp)
{
	patStep=stp;
	if (viewMode == VMPattern)
	{
		Repaint();
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

void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( viewMode == VMPattern )
	{
		switch(nSBCode)
		{
			case SB_LINEDOWN:
				if ( lOff<_pSong->patternLines[_ps()]-VISLINES)
				{
					nlOff=lOff+1;
					AdvanceLine(1,false,false);
					Repaint(DMScroll);
				}
				else AdvanceLine(1,false);
				break;
			case SB_LINEUP:
				if ( lOff>0 )
				{
					nlOff=lOff-1;
					PrevLine(1,false,false);
					Repaint(DMScroll);
				}
				else PrevLine(1,false);
				break;
			case SB_PAGEDOWN:
				AdvanceLine(16,false);
				break;
			case SB_PAGEUP:
				PrevLine(16,false);
				break;
			case SB_THUMBPOSITION:
				nlOff=nPos;
				if ((int)nPos != lOff )
				{
					AdvanceLine(nPos-lOff,false,false); // AdvanceLine does not accept Negative values
					Repaint(DMScroll);					// in general, but here we can make an exception.
				}						
				break;
			case SB_THUMBTRACK:
				nlOff=nPos;
				if ((int)nPos != lOff )
				{
					AdvanceLine(nPos-lOff,false,false); // AdvanceLine does not accept Negative values
					Repaint(DMScroll);					// in general, but here we can make an exception.
				}						
				break;
			default: break;
		}
	}
	
	CWnd ::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( viewMode == VMPattern )
	{
		switch(nSBCode)
		{
			case SB_LINERIGHT:
				if ( tOff<_pSong->SONGTRACKS-VISTRACKS)
				{
					ntOff=tOff+1;
					AdvanceTrack(1,false,false);
					Repaint(DMScroll);
				}
				else AdvanceTrack(1,false);
				break;
			case SB_LINELEFT:
				if ( tOff>0 )
				{
					ntOff=tOff-1;
					PrevTrack(1,false,false);
					Repaint(DMScroll);
				}
				else PrevTrack(1,false);
				break;
			case SB_PAGERIGHT:
				AdvanceTrack(VISTRACKS,false);
				break;
			case SB_PAGELEFT:
				PrevTrack(VISTRACKS,false);
				break;
			case SB_THUMBPOSITION:
				ntOff=nPos;
				if ((int)nPos != lOff )
				{
					AdvanceTrack(nPos-tOff,false,false);// AdvanceTrack does not accept Negative values
					Repaint(DMScroll);					// in general, but here we can make an exception.
				}
				break;
			case SB_THUMBTRACK:
				ntOff=nPos;
				if ((int)nPos != lOff )
				{
					AdvanceTrack(nPos-tOff,false,false);// AdvanceTrack does not accept Negative values
					Repaint(DMScroll);					// in general, but here we can make an exception.
				}
				break;
			default: break;
		}
	}
	
	CWnd ::OnVScroll(nSBCode, nPos, pScrollBar);
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
		Repaint();
		pParentMain->PsybarsUpdate();
		pParentMain->WaveEditorBackUpdate();
		pParentMain->UpdateSequencer();
		pParentMain->UpdatePlayOrder(false);
	}
	CString titlename = "[";
	titlename+=Global::_pSong->fileName;
	titlename+="] ";
	titlename+="Psycle Modular Music Creation Studio";	// I don't know how to access to the
	pParentMain->SetWindowText(titlename);				// IDR_MAINFRAME String Title.	
}

void CChildView::OnFileImportItfile() 
{
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
		pParentMain->UpdateSequencer();
		pParentMain->UpdatePlayOrder(false);
	}
	CString titlename = "[";
	titlename+=Global::_pSong->fileName;
	titlename+="] ";
	titlename+="Psycle Modular Music Creation Studio";	// I don't know how to access to the
	pParentMain->SetWindowText(titlename);				// IDR_MAINFRAME String Title.	
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
	hRecentMenu = GetSubMenu(hFileMenu, 8);

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
		pParentMain->CloseAllMacGuis();
		Global::pPlayer->Stop();
		Sleep(LOCK_LATENCY);
		_outputActive = false;
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();
		Sleep(LOCK_LATENCY);
		
		CSaveDlg dlg;
		dlg._pSong = Global::_pSong;
		sprintf(dlg.szFile,fName);
		dlg.LoadSong();
		
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
		pParentMain->UpdateSequencer();
		pParentMain->UpdatePlayOrder(false);
		Repaint();
	}
	CString titlename = "[";
	titlename+=Global::_pSong->fileName;
	titlename+="] ";
	titlename+="Psycle Modular Music Creation Studio";	// I don't know how to access to the
	pParentMain->SetWindowText(titlename);				// IDR_MAINFRAME String Title.
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

