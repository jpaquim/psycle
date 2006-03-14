/** @file 
 *  @brief implementation of the DefaultPatternView class
 *  @author Juan Antonio Arguelles Rius
 *  [C] 2000 All Rights Reserved
 *  [C] 2001-2002 Psycledelics.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.2 $
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
#include "UndoRedo.h"
#include "PatternUndo.h"
#include "Registry.h"
#include "configuration.h"
#include "FileXM.h"
//#include "FileIT.h"
//#include "ChildView.h"
#include "Bitmap.cpp"

#include "Player.h"
#include "MidiInput.h"
#include "SwingFillDlg.h"
#include "Helpers.h"

//#include "Dsp.h"
//#include "Filter.h"
#include "MenuXP.h"
#include "MainFrm.h"
#include "PatDlg.h"
#include "GreetDialog.h"
#include "ConfigDlg.h"
#include "inputhandler.h"

#include <math.h> // SwingFill

#include "keybhandler.cpp"
#include "mouseHandler.cpp"

//#include "patviewnew.cpp"
// 


/////////////////////////////


//////////////////////////////////////////////////////////////////////
// Enviroment Initialization

//CMainFrame		*pParentMain;

unsigned idletime = 0;

/////////////////////////////////////////////////////////////////////////////
// DefaultPatternView

namespace configuration {

	DefaultPatternView::DefaultPatternView()
	{
		m_separator  = 0x00400000;
		m_separator2  = 0x00004000;
		m_background  = 0x009a887c;
		m_background2  = 0x00aa786c;
		m_row4beat  = 0x00d5ccc6;
		m_row4beat2 = 0x00fdfcf6;
		m_rowbeat  = 0x00c9beb8;
		m_rowbeat2 = 0x00f9eee8;
		m_row  = 0x00c1b5aa;
		m_row2 = 0x00f1e5da;
		m_font  = 0x00000000;
		m_font2  = 0x00000000;
		m_fontPlay  = 0x00ffffff;
		m_fontPlay2  = 0x00ffffff;
		m_fontCur  = 0x00ffffff;
		m_fontCur2  = 0x00ffffff;
		m_fontSel  = 0x00ffffff;
		m_fontSel2  = 0x00ffffff;
		m_selection  = 0x00e00000;
		m_selection2 = 0x00ff5050;
		m_playbar  = 0x0000e000;
		m_playbar2 = 0x005050e0;
		m_cursor  = 0x000000e0;
		m_cursor2 = 0x005050ff;

		_tcscpy(m_fontface,_T("MS UI Gothic"));
		_tcscpy(m_header_skin,DEFAULT_PATTERN_HEADER_SKIN);
	
		m_font_point = 85;
		m_font_x = 9;
		m_font_y = 12;
		m_draw_empty_data = TRUE;

		_initialized = false;
		_wrapAround = true;
		_centerCursor = false;
		_cursorAlwaysDown = false;
		_RecordNoteoff = false;
		_RecordTweaks = false;
		_notesToEffects = false;

		_linenumbers = true;
		_linenumbersHex = false;
		_linenumbersCursor = false;
		_followSong = false;
	}
}

DefaultPatternView::DefaultPatternView() : m_FmtPan(IDS_MSG0014),m_FmtPan1(IDS_MSG0015)

{
	m_bPrepare = false;
};

void DefaultPatternView::Initialize()
{
	// Set Gui Environment data

	for (int c = 0; c < 256; c++)
	{ 
		m_FLATSIZES[c] = 8;
	}
	
	m_BmpDC = NULL;

	m_UpdateMode = 0;
	m_UpdatePar = 0;
//	multiPattern=true; // Long way till it can be finished!

	m_PatStep = 1;
	m_EditPosition = 0;
	m_PrevEditPosition = 0;
	m_bEditMode = true;

	m_BlockSelected = false;
	m_bBlockCopied = false;
	m_PatBufferCopy = false;
	m_BlockNTracks = 0;
	m_BlockNLines = 0;
	m_bScrollDetach = false;

	m_PlayPosition = -1;
	m_newm_playpos = -1; 
	m_SelectionPosition.bottom = 0;
	m_newm_Selpos.bottom = 0;
	m_NumPatternDraw = 0;

	m_TrackOffset = 0;
	m_LineOffset = 0;
	m_NTrackOffset = 0;
	m_NLineOffset = 0;

	m_ChordModeOffs = 0;
	Controller& _controller(Controller::Instance()); 
	_controller.InputHandler().SetChildView(this);
	_controller.Resampler().SetQuality(RESAMPLE_LINEAR);

	// just give arbitrary values so OnSize doesn't give /0 error
	// they will be filled in correctly when we switch to pattern view
	m_VISLINES = 2;
	m_VISTRACKS = 8;

//	_getcwd(m_appdir,_MAX_PATH);
	
// Creates a new song object. The application Song.
//	Global::m_pSong->Reset(); It's already called in m_pSong->New();
	_controller.Song().New();
	
	// Referencing the childView song pointer to the
	// Main Global::m_pSong object [The application Global::m_pSong]
	m_pSong = Controller::Instance().SongPtr();

}

DefaultPatternView::~DefaultPatternView()
{
	Global::pInputHandler->SetChildView(NULL);
	KillRedo();
	KillUndo();

	if ( m_BmpDC != NULL )
	{
#ifdef _DEBUG_PATVIEW
		TCHAR buf[100];
		_stprintf(buf,SF::CResourceString(IDS_TRACE_MSG0009),(int)m_BmpDC);
		ATLTRACE(buf);
#endif
		m_BmpDC->DeleteObject();
		delete m_BmpDC;
	}
	if(!m_PatternHeader.IsNull())
		m_PatternHeader.DeleteObject();
	//DeleteObject(m_hbmPatHeader);
	if(!machineskin.IsNull())
		machineskin.DeleteObject();
	//DeleteObject(hbmMachineSkin);
	if(!m_PatternHeaderMask.IsNull())
		m_PatternHeaderMask.DeleteObject();
	if(!machineskinmask.IsNull())
		machineskinmask.DeleteObject();
	if(!machinebkg.IsNull())
		machinebkg.DeleteObject();
	

}



/////////////////////////////////////////////////////////////////////////////
// DefaultPatternView message handlers
/*
BOOL DefaultPatternView::PreCreateWindow(CREATESTRUCT& cs) 
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
// allow DefaultPatternView call functions of the CMainFrm parent object
// Call this function after creating both the DefaultPatternView object and
// the cmainfrm object

void DefaultPatternView::ValidateParent()
{
	//pParentMain = (CMainFrame *)rMainFrame;
	m_pMainFrame->m_pSong = Global::m_pSong;
}



void DefaultPatternView::EnableSound()
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
				Global::m_pSong->SamplesPerTick(
					(Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)
					/(Global::pPlayer->bpm*Global::pPlayer->tpb));
			}
			else
			{
				Global::m_pSong->SetBPM(Global::m_pSong->BeatsPerMin(), 
					Global::m_pSong->TicksPerBeat(), 
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

void DefaultPatternView::OnDestroy()
{
	if (Global::pConfig->_pOutputDriver->Initialized())
	{
		Global::pConfig->_pOutputDriver->Reset();
	}
	KillTimer(31);
	KillTimer(159);

}






void DefaultPatternView::Repaint(int drawMode)
{
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("Repaint()\n"));
#endif
	PreparePatternRefresh(drawMode);
}

void DefaultPatternView::OnSize(UINT nType, CSize & size) 
{
	//CWnd ::OnSize(nType, cx, cy);
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("OnSize()\n"));
#endif
	m_CW = size.cx;
	m_CH = size.cy;
	m_pSong->ViewSizeX(size.cx); // Hack to move machines boxes inside of the visible area.
	m_pSong->ViewSizeY(size.cy);
	
	if ( m_BmpDC != NULL && Global::pConfig->m_bUseDoubleBuffer ) // remove old buffer to force recreating it with new size
	{
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("DefaultPatternView::OnResize(). Delete m_BmpDC"));
#endif
		m_BmpDC->DeleteObject();
		delete m_BmpDC;
		m_BmpDC = NULL;
	}
	if (viewMode == VMPattern)
	{
		RecalcMetrics();
	}

	if(!m_bPrepare)
		Repaint();
	
	SetMsgHandled(FALSE);
}



void DefaultPatternView::ShowPatternDlg(void)
{
	CPatDlg dlg;
	int patNum = m_pSong->PlayOrder(m_EditPosition);
	int nlines = m_pSong->PatternLines(patNum);
	TCHAR name[32];
	_tcscpy(name,m_pSong->PatternName(patNum));

	dlg.patLines= nlines;
	_tcscpy(dlg.patName,name);
	
	if (dlg.DoModal() == IDOK)
	{
		if ( nlines != dlg.patLines )
		{
			AddUndo(patNum,0,0,MAX_TRACKS,nlines,m_EditCursor.track,m_EditCursor.line,m_EditCursor.col,m_EditPosition);
			AddUndoLength(patNum,nlines,m_EditCursor.track,m_EditCursor.line,m_EditCursor.col,m_EditPosition);
			m_pSong->AllocNewPattern(patNum,dlg.patName,dlg.patLines,dlg.m_adaptsize?true:false);
			if ( _tcscmp(name,dlg.patName) != 0 )
			{
				_tcscpy(m_pSong->PatternName(patNum),dlg.patName);
				m_pMainFrame->StatusBarIdle();
			}
			Repaint();
		}
		else if ( _tcscmp(name,dlg.patName) != 0 )
		{
			_tcscpy(m_pSong->PatternName(patNum),dlg.patName);
			m_pMainFrame->StatusBarIdle();
//			Repaint(DMPatternHeader);
		}
	}

}

void DefaultPatternView::NewMachine(int x, int y, int mac) 
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
				fb = Global::m_pSong->GetFreeBus();
				xs = MachineCoords.sGenerator.width;
				ys = MachineCoords.sGenerator.height;
			}
			else 
			{
				fb = Global::m_pSong->GetFreeFxBus();
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
				if (Global::m_pSong->pMachine(fb))
				{
					x = Global::m_pSong->pMachine(fb)->_x;
					y = Global::m_pSong->pMachine(fb)->_y;
					m_pMainFrame->CloseMacGui(fb);
					Global::m_pSong->DestroyMachine(fb);
				}
			}
			else if ((mac < MAX_BUSES) && (dlg.OutBus))
			{
				fb = mac;
				xs = MachineCoords.sGenerator.width;
				ys = MachineCoords.sGenerator.height;
				// delete machine if it already exists
				if (Global::m_pSong->pMachine(fb))
				{
					x = Global::m_pSong->pMachine(fb)->_x;
					y = Global::m_pSong->pMachine(fb)->_y;
					m_pMainFrame->CloseMacGui(fb);
					Global::m_pSong->DestroyMachine(fb);
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
				x = (rand())%(m_CW-xs);
				y = (rand())%(m_CH-ys);
				bCovered = FALSE;
				for (int i=0; i < MAX_MACHINES; i++)
				{
					if (Global::m_pSong->pMachine(i))
					{
						if ((abs(Global::m_pSong->pMachine(i)->_x - x) < 32) &&
							(abs(Global::m_pSong->pMachine(i)->_y - y) < 32))
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

		if ( fb == -1 || !Global::m_pSong->CreateMachine((MachineType)dlg.Outputmachine, x, y, dlg.psOutputDll,fb))
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0069),SF::CResourceString(IDS_ERR_MSG0043),MB_OK);
		}
		else
		{
			if ( dlg.OutBus)
			{
				Global::m_pSong->SeqBus(fb);
			}

			// make sure that no 2 machines have the same name, because that is irritating

			int number = 1;
			TCHAR buf[sizeof(m_pSong->pMachine(fb)->_editName) + 4];
			_tcscpy(buf,m_pSong->pMachine(fb)->_editName);

			for (int i = 0; i < MAX_MACHINES-1; i++)
			{
				if (i!=fb)
				{
					if (m_pSong->pMachine(i))
					{
						if (_tcscmp(m_pSong->pMachine(i)->_editName,buf)==0)
						{
							number++;
							_stprintf(buf,_T("%s %d"),m_pSong->pMachine(fb)->_editName,number);
							i = -1;
						}
					}
				}
			}

			buf[sizeof(m_pSong->pMachine(fb)->_editName)-1] = 0;
			_tcscpy(m_pSong->pMachine(fb)->_editName,buf);

			m_pMainFrame->UpdateComboGen();
			Repaint(DMAllMacsRefresh);
//			Repaint(DMMacRefresh); // Seems that this doesn't always work (multiple calls to Repaint?)
		}
		
		/*
		// Restarting the driver...
		m_pMainFrame->UpdateEnvInfo();
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
	m_pMainFrame->RedrawGearRackList();
}



int DefaultPatternView::SongIncBpm(int x)
{
	AddMacViewUndo();

	Global::m_pSong->BeatsPerMin(Global::m_pSong->BeatsPerMin() + x);
	
	if (Global::m_pSong->BeatsPerMin() < 33)
	{
		Global::m_pSong->BeatsPerMin(33);
	}
	if (Global::m_pSong->BeatsPerMin() > 999)
	{
		Global::m_pSong->BeatsPerMin(999);
	}

 	Global::m_pSong->SetBPM(Global::m_pSong->BeatsPerMin(),
		Global::m_pSong->TicksPerBeat(),
		Global::pConfig->_pOutputDriver->_samplesPerSec);
	
	return Global::m_pSong->BeatsPerMin();
}


#define TWOPI_F (2.0f*3.141592665f)

void DefaultPatternView::ShowSwingFillDlg(bool bTrackMode)
{
	int st = Global::m_pSong->BeatsPerMin();
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
			x = m_EditCursor.track;
			y = 0;
			ny = m_pSong->PatternLines(_ps());
		}
		else
		{
			x = m_BlockSelection.start.track;
			y = m_BlockSelection.start.line;
			ny = 1+m_BlockSelection.end.line-m_BlockSelection.start.line;
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
			AddUndo(_ps(),x,y,1,ny,m_EditCursor.track,m_EditCursor.line,m_EditCursor.col,m_EditPosition);
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

void DefaultPatternView::OnPopCut() { CopyBlock(true); }
void DefaultPatternView::OnPopCopy() { CopyBlock(false); }
void DefaultPatternView::OnPopPaste() { PasteBlock(m_EditCursor.track,m_EditCursor.line,false); }
void DefaultPatternView::OnPopMixpaste() { PasteBlock(m_EditCursor.track,m_EditCursor.line,true); }

void DefaultPatternView::OnPopDelete() { DeleteBlock(); }

void DefaultPatternView::OnPopInterpolate() { BlockParamInterpolate(); }

void DefaultPatternView::OnPopChangegenerator() { BlockGenChange(m_pSong->SeqBus()); }

void DefaultPatternView::OnPopChangeinstrument() { BlockInsChange(m_pSong->AuxcolSelected()); }

void DefaultPatternView::OnPopTranspose1() { BlockTranspose(1); }

void DefaultPatternView::OnPopTranspose12() { BlockTranspose(12); }

void DefaultPatternView::OnPopTranspose_1() { BlockTranspose(-1); }

void DefaultPatternView::OnPopTranspose_12() { BlockTranspose(-12); }

void DefaultPatternView::OnPopPattenproperties() 
{
	ShowPatternDlg();
}

void DefaultPatternView::OnPopBlockSwingfill()
{
	// fill block
	ShowSwingFillDlg(FALSE);
}

void DefaultPatternView::OnPopTrackSwingfill()
{
	// fill track
	ShowSwingFillDlg(TRUE);
}



//



//#define _UGLY_DEFAULT_SKIN_

void DefaultPatternView::LoadMachineSkin()
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
void DefaultPatternView::LoadPatternHeaderSkin()
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
		m_PatHeaderCoords.sBackground.x=0;
		m_PatHeaderCoords.sBackground.y=0;
		m_PatHeaderCoords.sBackground.width=109;
		m_PatHeaderCoords.sBackground.height=16;
		m_PatHeaderCoords.sNumber0.x = 0;
		m_PatHeaderCoords.sNumber0.y = 16;
		m_PatHeaderCoords.sNumber0.width = 7;
		m_PatHeaderCoords.sNumber0.height = 12;
		m_PatHeaderCoords.sRecordOn.x = 70;
		m_PatHeaderCoords.sRecordOn.y = 16;
		m_PatHeaderCoords.sRecordOn.width = 7;
		m_PatHeaderCoords.sRecordOn.height = 7;
		m_PatHeaderCoords.sMuteOn.x = 77;
		m_PatHeaderCoords.sMuteOn.y = 16;
		m_PatHeaderCoords.sMuteOn.width = 7;
		m_PatHeaderCoords.sMuteOn.height = 7;
		m_PatHeaderCoords.sSoloOn.x = 84;
		m_PatHeaderCoords.sSoloOn.y = 16;
		m_PatHeaderCoords.sSoloOn.width = 7;
		m_PatHeaderCoords.sSoloOn.height = 7;
		m_PatHeaderCoords.dDigitX0.x = 23;
		m_PatHeaderCoords.dDigitX0.y = 2;
		m_PatHeaderCoords.dDigit0X.x = 30;
		m_PatHeaderCoords.dDigit0X.y = 2;
		m_PatHeaderCoords.dRecordOn.x = 52;
		m_PatHeaderCoords.dRecordOn.y = 5;
		m_PatHeaderCoords.dMuteOn.x = 75;
		m_PatHeaderCoords.dMuteOn.y = 5;
		m_PatHeaderCoords.dSoloOn.x = 96;
		m_PatHeaderCoords.dSoloOn.y = 5;
		m_PatHeaderCoords.bHasTransparency = FALSE;
#else
		m_PatHeaderCoords.sBackground.x=0;
		m_PatHeaderCoords.sBackground.y=0;
		m_PatHeaderCoords.sBackground.width=109;
		m_PatHeaderCoords.sBackground.height=18;//16
		m_PatHeaderCoords.sNumber0.x = 0;
		m_PatHeaderCoords.sNumber0.y = 18;//16
		m_PatHeaderCoords.sNumber0.width = 7;
		m_PatHeaderCoords.sNumber0.height = 12;
		m_PatHeaderCoords.sRecordOn.x = 70;
		m_PatHeaderCoords.sRecordOn.y = 18;//16
		m_PatHeaderCoords.sRecordOn.width = 11;//7;
		m_PatHeaderCoords.sRecordOn.height = 11;//7;
		m_PatHeaderCoords.sMuteOn.x = 81;//77;
		m_PatHeaderCoords.sMuteOn.y = 18;//16;
		m_PatHeaderCoords.sMuteOn.width = 11;//7;
		m_PatHeaderCoords.sMuteOn.height = 11;//7;
		m_PatHeaderCoords.sSoloOn.x = 92;//84;
		m_PatHeaderCoords.sSoloOn.y = 18;//16;
		m_PatHeaderCoords.sSoloOn.width = 11;//7;
		m_PatHeaderCoords.sSoloOn.height = 11;//7;
		m_PatHeaderCoords.dDigitX0.x = 24;//22;
		m_PatHeaderCoords.dDigitX0.y = 3;//2;
		m_PatHeaderCoords.dDigit0X.x = 31;//29;
		m_PatHeaderCoords.dDigit0X.y = 3;//2;
		m_PatHeaderCoords.dRecordOn.x = 52;
		m_PatHeaderCoords.dRecordOn.y = 3;//5;
		m_PatHeaderCoords.dMuteOn.x = 75;
		m_PatHeaderCoords.dMuteOn.y = 3;//5;
		m_PatHeaderCoords.dSoloOn.x = 97;//96;
		m_PatHeaderCoords.dSoloOn.y = 3;//5;
		m_PatHeaderCoords.bHasTransparency = FALSE;
#endif
		if(!m_PatternHeader.IsNull())
			m_PatternHeader.DeleteObject();
		//DeleteObject(m_hbmPatHeader);
		if(!m_PatternHeaderMask.IsNull())
			m_PatternHeaderMask.DeleteObject();
		m_PatternHeader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
	}
}

void DefaultPatternView::FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result)
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

			if(!m_PatternHeader.IsNull())
				m_PatternHeader.DeleteObject();
			
			// DeleteObject(m_hbmPatHeader);
			
			if(!m_PatternHeaderMask.IsNull())
				m_PatternHeaderMask.DeleteObject();

			m_hbmPatHeader = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			
			if (m_hbmPatHeader)
			{	m_PatternHeader.Attach(m_hbmPatHeader);
				if (m_PatternHeader.IsNull())
				{	
					memset(&m_PatHeaderCoords,0,sizeof(m_PatHeaderCoords));
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
								m_PatHeaderCoords.sBackground.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.sBackground.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										m_PatHeaderCoords.sBackground.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											m_PatHeaderCoords.sBackground.height = _tstoi(q+1);
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
								m_PatHeaderCoords.sNumber0.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.sNumber0.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										m_PatHeaderCoords.sNumber0.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											m_PatHeaderCoords.sNumber0.height = _tstoi(q+1);
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
								m_PatHeaderCoords.sRecordOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.sRecordOn.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										m_PatHeaderCoords.sRecordOn.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											m_PatHeaderCoords.sRecordOn.height = _tstoi(q+1);
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
								m_PatHeaderCoords.sMuteOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.sMuteOn.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										m_PatHeaderCoords.sMuteOn.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											m_PatHeaderCoords.sMuteOn.height = _tstoi(q+1);
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
								m_PatHeaderCoords.sSoloOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.sSoloOn.y = _tstoi(q+1);
									q = _tcschr(q+1,44); // ,
									if (q)
									{
										m_PatHeaderCoords.sSoloOn.width = _tstoi(q+1);
										q = _tcschr(q+1,44); // ,
										if (q)
										{
											m_PatHeaderCoords.sSoloOn.height = _tstoi(q+1);
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
								m_PatHeaderCoords.dDigitX0.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.dDigitX0.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"digit_0x_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								m_PatHeaderCoords.dDigit0X.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.dDigit0X.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"record_on_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								m_PatHeaderCoords.dRecordOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.dRecordOn.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"mute_on_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								m_PatHeaderCoords.dMuteOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.dMuteOn.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"solo_on_dest\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								m_PatHeaderCoords.dSoloOn.x = _tstoi(q+1);
								q = _tcschr(q+1,44); // ,
								if (q)
								{
									m_PatHeaderCoords.dSoloOn.y = _tstoi(q+1);
								}
							}
						}
						else if (_tcsstr(buf,_T("\"transparency\"=")))
						{
							TCHAR *q = _tcschr(buf,61); // =
							if (q)
							{
								m_PatHeaderCoords.cTransparency = _httoi(q+1);
								m_PatHeaderCoords.bHasTransparency = TRUE;
							}
						}
					}
					if (m_PatHeaderCoords.bHasTransparency)
					{
						PrepareMask(&m_PatternHeader,&m_PatternHeaderMask,m_PatHeaderCoords.cTransparency);
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


void DefaultPatternView::RecalcMetrics()
{
	if (Global::pConfig->pattern_draw_empty_data)
	{
		_stprintf(m_BlankParam,_T("."));
		_stprintf(m_BlankNote,_T("---"));
	}
	else
	{
		_stprintf(m_BlankParam,_T(" "));
		_stprintf(m_BlankNote,_T("   "));
	}
	m_TEXTHEIGHT = Global::pConfig->pattern_font_y;
	m_ROWHEIGHT = m_TEXTHEIGHT+1;

	m_TEXTWIDTH = Global::pConfig->pattern_font_x;
	for (int c=0; c<256; c++)	
	{ 
		m_FLATSIZES[c]=Global::pConfig->pattern_font_x; 
	}
	m_COLX[0] = 0;
	m_COLX[1] = (m_TEXTWIDTH*3)+2;
	m_COLX[2] = m_COLX[1]+m_TEXTWIDTH;
	m_COLX[3] = m_COLX[2]+m_TEXTWIDTH+1;
	m_COLX[4] = m_COLX[3]+m_TEXTWIDTH;
	m_COLX[5] = m_COLX[4]+m_TEXTWIDTH+1;
	m_COLX[6] = m_COLX[5]+m_TEXTWIDTH;
	m_COLX[7] = m_COLX[6]+m_TEXTWIDTH;
	m_COLX[8] = m_COLX[7]+m_TEXTWIDTH;
	m_COLX[9] = m_COLX[8]+m_TEXTWIDTH+1;
	m_COLX[10] = m_COLX[9]+m_TEXTWIDTH;
	m_COLX[11] = m_COLX[10]+m_TEXTWIDTH;
	m_COLX[12] = m_COLX[11]+m_TEXTWIDTH;
	m_COLX[13] = m_COLX[12]+m_TEXTWIDTH + 1;
	m_ROWWIDTH = m_COLX[13];
	m_HEADER_ROWWIDTH = m_PatHeaderCoords.sBackground.width+1;
	m_HEADER_HEIGHT = m_PatHeaderCoords.sBackground.height+2;
	if (m_ROWWIDTH < m_HEADER_ROWWIDTH)
	{
		int temp = (m_HEADER_ROWWIDTH-m_ROWWIDTH)/2;
		m_ROWWIDTH = m_HEADER_ROWWIDTH;
		for (int i = 0; i < 10; i++)
		{
			m_COLX[i] += temp;
		}
	}
	m_HEADER_INDENT = (m_ROWWIDTH - m_HEADER_ROWWIDTH)/2;
	if (Global::pConfig->_linenumbers)
	{
		m_XOFFSET = (4*m_TEXTWIDTH);
		m_YOFFSET = m_TEXTHEIGHT+2;
		if (m_YOFFSET < m_HEADER_HEIGHT)
		{
			m_YOFFSET = m_HEADER_HEIGHT;
		}
	}
	else
	{
		m_XOFFSET = 1;
		m_YOFFSET = m_HEADER_HEIGHT;
	}
	m_VISTRACKS = (m_CW-m_XOFFSET)/m_ROWWIDTH;
	m_VISLINES = (m_CH-m_YOFFSET)/m_ROWHEIGHT;
	if (m_VISLINES < 1) 
	{ 
		m_VISLINES = 1; 
	}
	if (m_VISTRACKS < 1) 
	{ 
		m_VISTRACKS = 1; 
	}

	triangle_size_tall = Global::pConfig->mv_triangle_size+((46*Global::pConfig->mv_wirewidth) / 32);
	triangle_size_center = triangle_size_tall / 2;
	triangle_size_wide = (triangle_size_tall * 5) / 8;

}


void DefaultPatternView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
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

void DefaultPatternView::TransparentBlt(CDC* pDC,
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

void DefaultPatternView::patTrackMute()
{
	if (viewMode == VMPattern)
	{
		m_pSong->IsTrackMuted(m_EditCursor.track,!m_pSong->IsTrackMuted(m_EditCursor.track));
		Repaint(DMTrackHeader);
	}
}

void DefaultPatternView::patTrackSolo()
{
	if (viewMode == VMPattern)
	{
		if (m_pSong->TrackSoloed() == m_EditCursor.track)
		{
			for (int i = 0; i < MAX_TRACKS; i++)
			{
				m_pSong->IsTrackMuted(i,false);
			}
			m_pSong->TrackSoloed(-1);
		}
		else
		{
			for (int i = 0; i < MAX_TRACKS; i++)
			{
				m_pSong->IsTrackMuted(i,true);
			}
			m_pSong->IsTrackMuted(m_EditCursor.track,false);
			m_pSong->TrackSoloed(m_EditCursor.track);
		}
		Repaint(DMTrackHeader);
	}
}

void DefaultPatternView::patTrackRecord()
{
	if (viewMode == VMPattern)
	{
		m_pSong->IsTrackArmed(m_EditCursor.track,!m_pSong->IsTrackArmed(m_EditCursor.track));
		m_pSong->TrackArmedCount(0);
		for ( int i=0;i<MAX_TRACKS;i++ )
		{
			if (m_pSong->IsTrackArmed(i))
			{
				m_pSong->TrackArmedCount(m_pSong->TrackArmedCount() + 1);
			}
		}
		Repaint(DMTrackHeader);
	}
}

void DefaultPatternView::DoMacPropDialog(int propMac)
{
	if ((propMac < 0 ) || (propMac >= MAX_MACHINES-1))
	{
		return;
	}
	CMacProp dlg;
	dlg.m_view=this;
	dlg.pMachine = Global::m_pSong->pMachine(propMac);
	dlg.pSong = Global::m_pSong;
	dlg.thisMac = propMac;
	
	if (dlg.DoModal() == IDOK)
	{
		_stprintf(dlg.pMachine->_editName, dlg.txt);
		m_pMainFrame->StatusBarText(dlg.txt);
		m_pMainFrame->UpdateEnvInfo();
		m_pMainFrame->UpdateComboGen();
		if (m_pMainFrame->pGearRackDialog)
		{
			m_pMainFrame->RedrawGearRackList();
		}
	}
	if (dlg.deleted)
	{
		m_pMainFrame->CloseMacGui(propMac);
		Global::m_pSong->DestroyMachine(propMac);
		m_pMainFrame->UpdateEnvInfo();
		m_pMainFrame->UpdateComboGen();
		if (m_pMainFrame->pGearRackDialog)
		{
			m_pMainFrame->RedrawGearRackList();
		}
	}
}




void DefaultPatternView::LoadMachineBackground()
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

				m_BkgX = bm.bmWidth;
				m_BkgY = bm.bmHeight;

				if ((m_BkgX > 0) && (m_BkgY > 0))
				{
					Global::pConfig->bBmpBkg = TRUE;
				}
			}
		}
	}
}

BOOL DefaultPatternView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT DefaultPatternView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
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
			dc.ExtTextOut( i + rc.left,j + rc.top,ETO_OPAQUE | ETO_CLIPPED,&rc1,_T("ABC"),3,m_FLATSIZES);
//			dc.TextOut( i + rc.left,j + rc.top,L"こ",-1);
		}
	}
	dc.SelectFont(_old);
	dc.RestoreDC(-1);
	
	return 0;
*/


	if ( m_BmpDC == NULL && Global::pConfig->m_bUseDoubleBuffer ) // buffer creation
	{
		CRect rc;
		GetClientRect(&rc);
		m_BmpDC = new CBitmap();
		m_BmpDC->CreateCompatibleBitmap(dc.m_hDC,rc.right - rc.left,rc.bottom - rc.top);
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("DefaultPatternView::OnPaint(). m_BmpDC  0x%.8X initialize\n"),(int)m_BmpDC);
#endif
	}
	else if ( m_BmpDC != NULL && !Global::pConfig->m_bUseDoubleBuffer ) // buffer deletion
	{
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("DefaultPatternView::OnPaint(). m_BmpDC (0x%.8X) deleted\n"),(int)m_BmpDC);
#endif
		delete m_BmpDC;
		m_BmpDC = NULL;
	}
	if ( Global::pConfig->m_bUseDoubleBuffer )
	{
		CDC bufDC;
		bufDC.CreateCompatibleDC(dc);
		CBitmapHandle oldbmp(bufDC.SelectBitmap(*m_BmpDC));
		if (viewMode == VMMachine)	// Machine view paint handler
		{
			switch (m_UpdateMode)
			{
			case DMAll:
				DrawMachineEditor(&bufDC);
				break;
			case DMMacRefresh:
//				ClearMachineSpace(Global::m_pSong->_pMachines[m_UpdatePar], m_UpdatePar, &bufDC);
				DrawMachine(m_UpdatePar, &bufDC);
				DrawMachineVumeters(m_UpdatePar, &bufDC);
				m_UpdateMode=0;
				break;
			case DMAllMacsRefresh:
				for (int i=0;i<MAX_MACHINES;i++)
				{
					if (m_pSong->pMachine(i))
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
			switch (m_UpdateMode)
			{
			case DMAll:
				DrawMachineEditor(&dc);
				break;
			case DMMacRefresh:
//				ClearMachineSpace(Global::m_pSong->_pMachines[m_UpdatePar], m_UpdatePar, &dc);
				DrawMachine(m_UpdatePar, &dc);
				DrawMachineVumeters(m_UpdatePar, &dc);
				m_UpdateMode = 0;
				break;
			case DMAllMacsRefresh:
				for (int i = 0;i < MAX_MACHINES;i++)
				{
					if (m_pSong->pMachine(i)) 
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


void DefaultPatternView::UpdateUI()
{
	// メニューバー更新 //
	// 録音ボタン
	m_pMainFrame->UISetCheck(ID_RECORDB,(Global::pPlayer->_recording)?1:0);
	// パターンビュー表示
	m_pMainFrame->UISetCheck(ID_PATTERNVIEW,(viewMode == VMPattern)?1:0);
	// マシンビュー表示
	m_pMainFrame->UISetCheck(ID_MACHINEVIEW,(viewMode == VMMachine)?1:0);
	// 
	m_pMainFrame->UISetCheck(ID_AUTOSTOP,(Global::pConfig->autoStopMachines)?1:0);
	m_pMainFrame->UISetCheck(ID_BARPLAY,(Global::pPlayer->_playing)?1:0);
	m_pMainFrame->UISetCheck(ID_BARPLAYFROMSTART,0);
	m_pMainFrame->UISetCheck(ID_BARREC,(Global::pConfig->_followSong && m_bEditMode)?1:0);
	m_pMainFrame->UISetCheck(ID_BUTTONPLAYSEQBLOCK,Global::pPlayer->_playBlock?TRUE:FALSE);
	// UNDO
	if(m_pUndoList) 
	{
		switch (m_pUndoList->type)
		{
		case UNDO_SEQUENCE:
			m_pMainFrame->UIEnable(ID_EDIT_UNDO,TRUE);
			m_pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
			break;
		default:
			if(viewMode == VMPattern)// && m_bEditMode)
			{
				m_pMainFrame->UIEnable(ID_EDIT_UNDO,TRUE);
				m_pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
			}
			else
			{
				m_pMainFrame->UIEnable(ID_EDIT_UNDO,FALSE);
				m_pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO1));
			}
			break;
		}
	}
	else
	{
		m_pMainFrame->UISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
		m_pMainFrame->UIEnable(ID_EDIT_UNDO,FALSE);
	}
	// REDO
	if(pRedoList) 
	{
		switch (pRedoList->type)
		{
		case UNDO_SEQUENCE:
			m_pMainFrame->UIEnable(ID_EDIT_REDO,TRUE);
			m_pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO));
			break;
		default:
			if(viewMode == VMPattern)// && m_bEditMode)
			{
				m_pMainFrame->UIEnable(ID_EDIT_REDO,TRUE);
				m_pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO));
			}
			else
			{
				m_pMainFrame->UIEnable(ID_EDIT_REDO,FALSE);
				m_pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO1));
			}
			break;
		}
	}
	else
	{
		m_pMainFrame->UIEnable(ID_EDIT_REDO,FALSE);
		m_pMainFrame->UISetText(ID_EDIT_REDO,RES_STR(IDS_REDO));
	}

	m_pMainFrame->UIEnable(ID_EDIT_CUT,(viewMode == VMPattern)?TRUE:FALSE);
	m_pMainFrame->UIEnable(ID_EDIT_COPY,(viewMode == VMPattern)?TRUE:FALSE);
	m_pMainFrame->UIEnable(ID_EDIT_DELETE,(viewMode == VMPattern)?TRUE:FALSE);
	m_pMainFrame->UIEnable(ID_EDIT_MIXPASTE,(m_PatBufferCopy&&(viewMode == VMPattern))?TRUE:FALSE);
	m_pMainFrame->UIEnable(ID_EDIT_PASTE,(m_PatBufferCopy&&(viewMode == VMPattern))?TRUE:FALSE);

	m_pMainFrame->UISetCheck(ID_CONFIGURATION_LOOPPLAYBACK,(Global::pPlayer->_loopSong)?1:0);
	
	BOOL bCutCopy =  m_BlockSelected && (viewMode == VMPattern);

	m_pMainFrame->UIEnable(ID_POP_CUT,bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_COPY,bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_PASTE,(m_bBlockCopied && (viewMode == VMPattern))?TRUE:FALSE);
	m_pMainFrame->UIEnable(ID_POP_DELETE, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_INTERPOLATE, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_CHANGEGENERATOR, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_CHANGEINSTRUMENT, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_TRANSPOSE1, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_TRANSPOSE12, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_TRANSPOSE_1, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_TRANSPOSE_12, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_BLOCK_SWINGFILL, bCutCopy);
	m_pMainFrame->UIEnable(ID_POP_MIXPASTE,(m_bBlockCopied && (viewMode == VMPattern))?TRUE:FALSE);

}

#define DRAW_DATA		1
#define DRAW_HSCROLL	2
#define DRAW_VSCROLL	4
#define DRAW_TRHEADER	8
#define DRAW_FULL_DATA	16

void DefaultPatternView::PreparePatternRefresh(int drawMode)
{
	m_bPrepare = false;
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("PreparePatternRefresh\n"));
	m_elapsedTime = timeGetTime();
#endif

	CRect rect;	
	updateMode = drawMode;					// this is ununsed for patterns
	const int snt = m_pSong->SongTracks();
	const int plines = m_pSong->PatternLines(m_pSong->PlayOrder(editPosition));
	if ( m_EditCursor.track >= snt ) // This should only happen when changing the song tracks.
	{							// Else, there is a problem.
		ATLTRACE(SF::CResourceString(IDS_TRACE_MSG0010));
		m_EditCursor.track = snt-1;
	}
	if ( m_EditCursor.line >= plines ) // This should only happen when changing the pattern lines
	{							  // or changing to a pattern with less lines.
		ATLTRACE(SF::CResourceString(IDS_TRACE_MSG0011));
		m_EditCursor.line = plines-1;
	}
	//////////////////////////////////////////////////////////////////////
	// Set the offsets and positions of data on the screen.

	// Track Offset
	if ( snt <= VISTRACKS)	
	{ 
		m_MaxTracks = snt; 
		m_RNTrackOffset = 0; 
	}
	else
	{
		if (bScrollDetatch)
		{
			if ( drawMode == DMHScroll )
			{
				m_RNTrackOffset = ntOff;
				if ( m_RNTrackOffset >= snt-VISTRACKS ) 
					m_MaxTracks = VISTRACKS;
				else 
					m_MaxTracks = VISTRACKS+1;
			}
			else
			{
				if ( m_TrackOffset+VISTRACKS > snt )
				{
					m_RNTrackOffset = snt-VISTRACKS;
					m_MaxTracks=VISTRACKS;
				}
				else if (detatchpoint.track < m_TrackOffset ) 
				{ 
					m_RNTrackOffset = detatchpoint.track; 
					m_MaxTracks = VISTRACKS+1; 
				}
				else
				{
					if (detatchpoint.track >= m_TrackOffset+VISTRACKS ) 
						m_RNTrackOffset =detatchpoint.track-VISTRACKS+1;
					else 
						m_RNTrackOffset = m_TrackOffset;
				
					if ( m_RNTrackOffset >= snt-VISTRACKS ) 
						m_MaxTracks = VISTRACKS;
					else 
						m_MaxTracks = VISTRACKS+1;
				}
			}
		}
		else if (Global::pConfig->_centerCursor)
		{
			if ( drawMode == DMHScroll ) 
				m_RNTrackOffset = ntOff;
			else 
				m_RNTrackOffset = m_EditCursor.track - (VISTRACKS/2);

			if (m_RNTrackOffset >= snt-VISTRACKS)	
			{	
				m_RNTrackOffset = snt-VISTRACKS;	
				m_MaxTracks = VISTRACKS;	
			}
			else 
			{
				if ( m_RNTrackOffset < 0 ) 
				{ 
					m_RNTrackOffset = 0; 
				}
				m_MaxTracks = VISTRACKS+1;
			}
		}
		else
		{
			if ( drawMode == DMHScroll )
			{
				m_RNTrackOffset = ntOff;
				if ( m_RNTrackOffset >= snt-VISTRACKS ) 
					m_MaxTracks = VISTRACKS;
				else 
					m_MaxTracks = VISTRACKS+1;
			}
			else
			{
				if ( m_TrackOffset+VISTRACKS > snt )
				{
					m_RNTrackOffset = snt-VISTRACKS;
					m_MaxTracks=VISTRACKS;
				}
				else if ( m_EditCursor.track < m_TrackOffset ) 
				{ 
					m_RNTrackOffset = m_EditCursor.track; 
					m_MaxTracks = VISTRACKS+1; 
				}
				else
				{
					if ( m_EditCursor.track >= m_TrackOffset+VISTRACKS ) 
						m_RNTrackOffset =m_EditCursor.track-VISTRACKS+1;
					else 
						m_RNTrackOffset = m_TrackOffset;
				
					if ( m_RNTrackOffset >= snt-VISTRACKS ) 
						m_MaxTracks = VISTRACKS;
					else 
						m_MaxTracks = VISTRACKS+1;
				}
			}
		}
	}
	// Line Offset
	if ( plines <=  VISLINES)	
	{ 
		m_MaxLines = plines; 
		m_RNLineOffset = 0; 
	}
	else 
	{
		if (bScrollDetatch)
		{
			if ( drawMode == DMVScroll )
			{
				m_RNLineOffset = nlOff;
				if ( m_RNLineOffset >= plines-VISLINES ) 
					m_MaxLines = VISLINES;
				else 
					m_MaxLines = VISLINES+1;
			}
			else 
			{
				if ( m_LineOffset+VISLINES > plines )
				{
					m_RNLineOffset = plines - VISLINES;
					m_MaxLines = VISLINES;
				}
				else if ( detatchpoint.line < m_LineOffset+1 ) 
				{ 
					m_RNLineOffset = detatchpoint.line-1; 
					if (m_RNLineOffset < 0)
					{
						m_RNLineOffset = 0;
					}
					m_MaxLines = VISLINES+1; 
				}
				else 
				{
					if ( detatchpoint.line >= m_LineOffset+VISLINES ) 
						m_RNLineOffset =detatchpoint.line-VISLINES+1;
					else 
						m_RNLineOffset = m_LineOffset;

					if ( m_RNLineOffset >= plines-VISLINES ) 
						m_MaxLines = VISLINES;
					else 
						m_MaxLines = VISLINES+1;
				}
			}
		}
		else if (Global::pConfig->_centerCursor)
		{
			if ( drawMode == DMVScroll ) 
				m_RNLineOffset = nlOff;
			else 
				m_RNLineOffset = m_EditCursor.line - (VISLINES/2);

			if (m_RNLineOffset >= plines-VISLINES) 
			{ 
				m_RNLineOffset = plines-VISLINES; 
				m_MaxLines = VISLINES; 
			}
			else 
			{
				if ( m_RNLineOffset < 0 ) 
				{ 
					m_RNLineOffset = 0; 
				}
				m_MaxLines = VISLINES+1;
			}
		}
		else
		{
			if ( drawMode == DMVScroll )
			{
				m_RNLineOffset = nlOff;
				if ( m_RNLineOffset >= plines-VISLINES ) 
					m_MaxLines = VISLINES;
				else 
					m_MaxLines = VISLINES+1;
			}
			else 
			{
				if ( m_LineOffset+VISLINES > plines )
				{
					m_RNLineOffset = plines-VISLINES;
					m_MaxLines=VISLINES;
				}
				else if ( m_EditCursor.line < m_LineOffset+1 ) 
				{ 
					m_RNLineOffset = m_EditCursor.line-1; 
					if (m_RNLineOffset < 0)
					{
						m_RNLineOffset = 0;
					}
					m_MaxLines = VISLINES+1; 
				}
				else 
				{
					if ( m_EditCursor.line >= m_LineOffset+VISLINES ) 
						m_RNLineOffset =m_EditCursor.line-VISLINES+1;
					else 
						m_RNLineOffset = m_LineOffset;

					if ( m_RNLineOffset >= plines-VISLINES ) 
						m_MaxLines = VISLINES;
					else 
						m_MaxLines = VISLINES+1;
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////
	// Determines if background Scroll is needed or not.

	if (drawMode != DMAll && drawMode != DMPattern)
	{
		if ( m_RNLineOffset != m_LineOffset )
		{
			rect.top = m_YOFFSET;	
			rect.left = 0;
			rect.bottom = m_CH;		
			rect.right = m_CW;
			updatePar |= DRAW_VSCROLL;
			InvalidateRect(rect,FALSE);
		}
		if ( m_RNTrackOffset != m_TrackOffset )
		{
			rect.top=0;		
			rect.left=m_XOFFSET;
			rect.bottom=m_CH;
			rect.right=m_CW;
			updatePar |= DRAW_HSCROLL;
			InvalidateRect(rect,FALSE);
		}
	}
	
	switch (drawMode)
	{
	case DMAll: 
		// header
		rect.top=0; 
		rect.left=0;
		rect.bottom=m_CH;	
		rect.right=m_CW;
		updatePar |= DRAW_TRHEADER | DRAW_FULL_DATA;
		InvalidateRect(rect,FALSE);
		if ( snt > VISTRACKS )
		{	
			ShowScrollBar(SB_HORZ,TRUE);
			SetScrollRange(SB_HORZ,0,snt - VISTRACKS);
		}
		else
		{	
			ShowScrollBar(SB_HORZ,FALSE); 
		}

		if ( plines > VISLINES )
		{	
			ShowScrollBar(SB_VERT,TRUE);
			SetScrollRange(SB_VERT,0,plines - VISLINES);
		}
		else
		{	
			ShowScrollBar(SB_VERT,FALSE); 
		}
		break;
	case DMPattern: 
		// all data
		rect.top=m_YOFFSET;		
		rect.left=0;
		rect.bottom=m_CH;
		rect.right=m_CW;
		updatePar |= DRAW_FULL_DATA;
		InvalidateRect(rect,FALSE);
		if ( snt > VISTRACKS )
		{	
			ShowScrollBar(SB_HORZ,TRUE);
			SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
		}
		else
		{	
			ShowScrollBar(SB_HORZ,FALSE); 
		}

		if ( plines > VISLINES )
		{	
			ShowScrollBar(SB_VERT,TRUE);
			SetScrollRange(SB_VERT,0,plines-VISLINES);
		}
		else
		{	
			ShowScrollBar(SB_VERT,FALSE); 
		}
		break;
	case DMPlayback: 
		{
			int pos = Global::pPlayer->_lineCounter;
			if (( pos-m_RNLineOffset >= 0 ) &&  ( pos-m_RNLineOffset <m_MaxLines ) &&
				(m_pSong->PlayOrder(editPosition) == m_pSong->PlayOrder(Global::pPlayer->_playPosition)))
			{
				if (pos != playpos)
				{
					newplaypos = pos;

					rect.top= m_YOFFSET+ ((pos-m_RNLineOffset)*m_ROWHEIGHT);
					rect.bottom=rect.top+m_ROWHEIGHT;	// left never changes and is set at ChildView init.
					rect.left = 0;
					rect.right=m_CW;
					NewPatternDraw(0, m_pSong->SongTracks(), pos, pos);
					updatePar |= DRAW_DATA;
					InvalidateRect(rect,FALSE);
					if ((playpos >= 0) && (playpos != newplaypos))
					{
						rect.top = m_YOFFSET+ ((playpos-m_RNLineOffset)*m_ROWHEIGHT);
						rect.bottom = rect.top+m_ROWHEIGHT;
						rect.left = 0;
						rect.right = m_CW;
						NewPatternDraw(0, m_pSong->SongTracks(), playpos, playpos);
						updatePar |= DRAW_DATA;
						playpos =-1;
						InvalidateRect(rect,FALSE);
					}
				}
			}
			else 
			{
				newplaypos=-1;
				if (playpos >= 0) 
				{
					rect.top = m_YOFFSET+ ((playpos-m_RNLineOffset)*m_ROWHEIGHT);
					rect.bottom = rect.top+m_ROWHEIGHT;
					rect.left = 0;
					rect.right = m_CW;
					NewPatternDraw(0, m_pSong->SongTracks(), playpos, playpos);
					updatePar |= DRAW_DATA;
					playpos = -1;
					InvalidateRect(rect,FALSE);
				}
			}
		}
		break;
	case DMPlaybackChange: 
		if (m_pSong->PlayOrder(editPosition) == m_pSong->PlayOrder(Global::pPlayer->_playPosition))
		{
			newplaypos= Global::pPlayer->_lineCounter;
		}
		else 
		{
			newplaypos=-1;
		}
		playpos=-1;
		rect.top=m_YOFFSET;		
		rect.left=0;
		rect.bottom=m_CH;
		rect.right=m_CW;
		updatePar |= DRAW_FULL_DATA;
		InvalidateRect(rect,FALSE);
		if ( snt > VISTRACKS )
		{	
			ShowScrollBar(SB_HORZ,TRUE);
			SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
		}
		else
		{	
			ShowScrollBar(SB_HORZ,FALSE); 
		}

		if ( plines > VISLINES )
		{	
			ShowScrollBar(SB_VERT,TRUE);
			SetScrollRange(SB_VERT,0,plines-VISLINES);
		}
		else
		{	
			ShowScrollBar(SB_VERT,FALSE); 
		}
		break;
	case DMSelection: 
		// could optimize to only draw the changes
		if (blockSelected)
		{
			if ((m_BlockSelection.end.track<m_RNTrackOffset) || (m_BlockSelection.end.line<m_RNLineOffset) ||
				(m_BlockSelection.start.track>=m_RNTrackOffset+VISTRACKS) ||
				(m_BlockSelection.start.line>=m_RNLineOffset+VISLINES))
			{
				newselpos.bottom = 0; // This marks as _T("don't show selection") (because out of range)
			}
			else 
			{
//				if (m_BlockSelection.start.line <= m_BlockSelection.end.line)
//				{
					newselpos.top=m_BlockSelection.start.line;
					newselpos.bottom=m_BlockSelection.end.line+1;
//				}
//				else
//				{
//					newselpos.top=m_BlockSelection.end.line;
//					newselpos.bottom=m_BlockSelection.start.line+1;
//				}

//				if (m_BlockSelection.start.track <= m_BlockSelection.end.track)
//				{
					newselpos.left=m_BlockSelection.start.track;
					newselpos.right=m_BlockSelection.end.track+1;
//				}
//				else
//				{
//					newselpos.right=m_BlockSelection.start.track;
//					newselpos.left=m_BlockSelection.end.track+1;
//				}

				if (selpos.bottom == 0)
				{
					//if(m_BlockSelection.start.track<m_RNTrackOffset) 
					//	rect.left=m_XOFFSET;
					//else 
						rect.left=m_XOFFSET+(m_BlockSelection.start.track-m_RNTrackOffset)*m_ROWWIDTH;
					
					//if(m_BlockSelection.start.line<=m_RNLineOffset) 
					//	rect.top=m_YOFFSET;
					//else 
						rect.top=m_YOFFSET+(m_BlockSelection.start.line-m_RNLineOffset)*m_ROWHEIGHT;
					
					//if(m_BlockSelection.end.track>=m_RNTrackOffset+VISTRACKS) 
					//	rect.right=m_CW;
					//else 
						rect.right=m_XOFFSET+(m_BlockSelection.end.track-m_RNTrackOffset+1)*m_ROWWIDTH;

					//if(m_BlockSelection.end.line>=m_RNLineOffset+VISLINES ) 
					//	rect.bottom=m_CH;
					//else 
						rect.bottom=m_YOFFSET+(m_BlockSelection.end.line-m_RNLineOffset+1)*m_ROWHEIGHT;
					
					NewPatternDraw(m_BlockSelection.start.track, m_BlockSelection.end.track, m_BlockSelection.start.line, m_BlockSelection.end.line);
					updatePar |= DRAW_DATA;
					InvalidateRect(rect,FALSE);
				}
				else if (newselpos != selpos)
				{
					if (newselpos.left < selpos.left)
					{
						rect.left = newselpos.left;
						if (newselpos.right > selpos.right)
						{
							rect.right = newselpos.right;
						}
						else if (newselpos.right < selpos.right)
						{
							rect.right = selpos.right;
						}
						else 
						{
							rect.right = selpos.left;
						}

						if (newselpos.top <= selpos.top)
						{
							rect.top = newselpos.top;
						}
						else 
						{
							rect.top = selpos.top;
						}

						if (newselpos.bottom >= selpos.bottom)
						{
							rect.bottom = newselpos.bottom;
						}
						else 
						{
							rect.bottom = selpos.bottom;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}
					else if (newselpos.left > selpos.left)
					{
						rect.left = selpos.left;
						if (newselpos.right > selpos.right)
						{
							rect.right = newselpos.right;
						}
						else if (newselpos.right < selpos.right)
						{
							rect.right = selpos.right;
						}
						else 
						{
							rect.right = newselpos.left;
						}

						if (newselpos.top <= selpos.top)
						{
							rect.top = newselpos.top;
						}
						else 
						{
							rect.top = selpos.top;
						}

						if (newselpos.bottom >= selpos.bottom)
						{
							rect.bottom = newselpos.bottom;
						}
						else 
						{
							rect.bottom = selpos.bottom;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}

					if (newselpos.right < selpos.right)
					{
						rect.left = newselpos.right;
						rect.right = selpos.right;

						if (newselpos.top <= selpos.top)
						{
							rect.top = newselpos.top;
						}
						else 
						{
							rect.top = selpos.top;
						}

						if (newselpos.bottom >= selpos.bottom)
						{
							rect.bottom = newselpos.bottom;
						}
						else 
						{
							rect.bottom = selpos.bottom;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}
					else if (newselpos.right > selpos.right)
					{
						rect.left = selpos.right;
						rect.right = newselpos.right;

						if (newselpos.top <= selpos.top)
						{
							rect.top = newselpos.top;
						}
						else 
						{
							rect.top = selpos.top;
						}

						if (newselpos.bottom >= selpos.bottom)
						{
							rect.bottom = newselpos.bottom;
						}
						else 
						{
							rect.bottom = selpos.bottom;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}

					if (newselpos.top < selpos.top)
					{
						rect.top = newselpos.top;
						if (newselpos.bottom > selpos.bottom)
						{
							rect.bottom = newselpos.bottom;
						}
						else if (newselpos.bottom < selpos.bottom)
						{
							rect.bottom = selpos.bottom;
						}
						else 
						{
							rect.bottom = selpos.top;
						}

						if (newselpos.left <= selpos.left)
						{
							rect.left = newselpos.left;
						}
						else 
						{
							rect.left = selpos.left;
						}

						if (newselpos.right >= selpos.right)
						{
							rect.right = newselpos.right;
						}
						else 
						{
							rect.right = selpos.right;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}
					else if (newselpos.top > selpos.top)
					{
						rect.top = selpos.top;
						if (newselpos.bottom > selpos.bottom)
						{
							rect.bottom = newselpos.bottom;
						}
						else if (newselpos.bottom < selpos.bottom)
						{
							rect.bottom = selpos.bottom;
						}
						else 
						{
							rect.bottom = newselpos.top;
						}

						if (newselpos.left <= selpos.left)
						{
							rect.left = newselpos.left;
						}
						else 
						{
							rect.left = selpos.left;
						}

						if (newselpos.right >= selpos.right)
						{
							rect.right = newselpos.right;
						}
						else 
						{
							rect.right = selpos.right;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}

					if (newselpos.bottom < selpos.bottom)
					{
						rect.top = newselpos.bottom;
						rect.bottom = selpos.bottom;

						if (newselpos.left <= selpos.left)
						{
							rect.left = newselpos.left;
						}
						else 
						{
							rect.left = selpos.left;
						}

						if (newselpos.right >= selpos.right)
						{
							rect.right = newselpos.right;
						}
						else 
						{
							rect.right = selpos.right;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}
					else if (newselpos.bottom > selpos.bottom)
					{
						rect.top = selpos.bottom;
						rect.bottom = newselpos.bottom;

						if (newselpos.left <= selpos.left)
						{
							rect.left = newselpos.left;
						}
						else 
						{
							rect.left = selpos.left;
						}

						if (newselpos.right >= selpos.right)
						{
							rect.right = newselpos.right;
						}
						else 
						{
							rect.right = selpos.right;
						}
					
						NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
						updatePar |= DRAW_DATA;
						rect.left = m_XOFFSET+(rect.left-m_RNTrackOffset)*m_ROWWIDTH;
						rect.right = m_XOFFSET+(rect.right-m_RNTrackOffset)*m_ROWWIDTH;
						rect.top=m_YOFFSET+(rect.top-m_RNLineOffset)*m_ROWHEIGHT;
						rect.bottom=m_YOFFSET+(rect.bottom-m_RNLineOffset)*m_ROWHEIGHT;
						InvalidateRect(rect,FALSE);
					}

				}
			}
		}
		else if ( selpos.bottom != 0)
		{
			//if(selpos.left<m_RNTrackOffset) 
			//	rect.left=m_XOFFSET;
			//else 
				rect.left=m_XOFFSET+(selpos.left-m_RNTrackOffset)*m_ROWWIDTH;
			
			//if(selpos.top<=m_RNLineOffset) 
			//	rect.top=m_YOFFSET;
			//else 
				rect.top=m_YOFFSET+(selpos.top-m_RNLineOffset)*m_ROWHEIGHT;
			
			//if(selpos.right>=m_RNTrackOffset+VISTRACKS) 
			//	rect.right=m_CW;
			//else 
				rect.right=m_XOFFSET+(selpos.right-m_RNTrackOffset)*m_ROWWIDTH;

			//if(selpos.bottom>=m_RNLineOffset+VISLINES ) 
			//	rect.bottom=m_CH;
			//else 
				rect.bottom=m_YOFFSET+(selpos.bottom-m_RNLineOffset)*m_ROWHEIGHT;
			
			NewPatternDraw(selpos.left, selpos.right, selpos.top, selpos.bottom);
			updatePar |= DRAW_DATA;
			newselpos.bottom=0;
			InvalidateRect(rect,FALSE);
		}
		break;
	case DMData: 
		{
			SPatternDraw* pPD = &m_pPatternDraw[numPatternDraw-1];
			
			rect.left=m_XOFFSET+  ((pPD->drawTrackStart-m_RNTrackOffset)*m_ROWWIDTH);
			rect.right=m_XOFFSET+ ((pPD->drawTrackEnd-(m_RNTrackOffset-1))*m_ROWWIDTH);
			rect.top=m_YOFFSET+	((pPD->drawLineStart-m_RNLineOffset)*m_ROWHEIGHT);
			rect.bottom=m_YOFFSET+((pPD->drawLineEnd-(m_RNLineOffset-1))*m_ROWHEIGHT);
			updatePar |= DRAW_DATA;
			InvalidateRect(rect,FALSE);
		}
		break;
	case DMTrackHeader: 
		// header
		rect.top=0; 
		rect.left=m_XOFFSET;
		rect.bottom=m_YOFFSET-1;	
		rect.right=m_XOFFSET+m_MaxTracks*m_ROWWIDTH;
		updatePar |= DRAW_TRHEADER;
		InvalidateRect(rect,FALSE);
		break;
//	case DMCursor: 
//		break;
	case DMNone: 
		break;
	}

	if ((m_EditCursor.col != m_EditLastPosition.col) || (m_EditCursor.track != m_EditLastPosition.track) || (m_EditCursor.line != m_EditLastPosition.line))
	{
		rect.left = m_XOFFSET+(m_EditCursor.track-m_RNTrackOffset)*m_ROWWIDTH;
		rect.right = rect.left+m_ROWWIDTH;
		rect.top = m_YOFFSET+(m_EditCursor.line-m_RNLineOffset)*m_ROWHEIGHT;
		rect.bottom = rect.top+m_ROWWIDTH;
		NewPatternDraw(m_EditCursor.track, m_EditCursor.track, m_EditCursor.line, m_EditCursor.line);
		updatePar |= DRAW_DATA;
		InvalidateRect(rect,FALSE);
		if (m_EditCursor.line != m_EditLastPosition.line)
		{
			if (m_XOFFSET!=1)
			{
				rect.left = 0;
				rect.right = m_XOFFSET;
				InvalidateRect(rect,FALSE);
			}
			rect.left = m_XOFFSET+(m_EditLastPosition.track-m_RNTrackOffset)*m_ROWWIDTH;
			rect.right = rect.left+m_ROWWIDTH;
			rect.top = m_YOFFSET+(m_EditLastPosition.line-m_RNLineOffset)*m_ROWHEIGHT;
			rect.bottom = rect.top+m_ROWWIDTH;
			NewPatternDraw(m_EditLastPosition.track, m_EditLastPosition.track, m_EditLastPosition.line, m_EditLastPosition.line);
			InvalidateRect(rect,FALSE);
			if (m_XOFFSET!=1)
			{
				rect.left = 0;
				rect.right = m_XOFFSET;
				InvalidateRect(rect,FALSE);
			}
		}
		else if (m_EditCursor.track != m_EditLastPosition.track)
		{
			rect.left = m_XOFFSET+(m_EditLastPosition.track-m_RNTrackOffset)*m_ROWWIDTH;
			rect.right = rect.left+m_ROWWIDTH;
			rect.top = m_YOFFSET+(m_EditLastPosition.line-m_RNLineOffset)*m_ROWHEIGHT;
			rect.bottom = rect.top+m_ROWWIDTH;
			NewPatternDraw(m_EditLastPosition.track, m_EditLastPosition.track, m_EditLastPosition.line, m_EditLastPosition.line);
			InvalidateRect(rect,FALSE);
		}
	}

	// turn off play line if not playing
	if (playpos >= 0 && !Global::pPlayer->_playing) 
	{
		newplaypos=-1;
		rect.top = m_YOFFSET+ (playpos-m_RNLineOffset)*m_ROWHEIGHT;
		rect.bottom = rect.top+m_ROWHEIGHT;
		rect.left = 0;
		rect.right = m_XOFFSET+(m_MaxTracks)*m_ROWWIDTH;
		NewPatternDraw(0, m_pSong->SongTracks(), playpos, playpos);
		playpos =-1;
		updatePar |= DRAW_DATA;
		InvalidateRect(rect,FALSE);
	}

	////////////////////////////////////////////////////////////////////
	// Checks for specific code to update.

	SetScrollPos(SB_HORZ,m_RNTrackOffset);
	SetScrollPos(SB_VERT,m_RNLineOffset);
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("UpdateWindow %d \n"),timeGetTime() - m_elapsedTime);
#endif
	m_bPrepare = false;
	UpdateWindow();
}




#define DF_NONE			0
#define	DF_SELECTION	1
#define DF_PLAYBAR		2
#define DF_CURSOR		4
#define DF_DRAWN		15

void DefaultPatternView::DrawPatEditor(CDC * const devc)
{
/*	
	CDC& dc = *devc; 
	//WTL::CPaintDC dc(m_hWnd);
	dc.SaveDC();
	dc.SetBkColor(0x00404040);
	dc.SetTextColor(0x00f0f0f0);
	CFont font;
	font.CreateFont(8,8,0,0,0,0,0,0,128,OUT_TT_ONLY_PRECIS,0,0,0,_T("ＭＳ 明朝"));
	HFONT _old = dc.SelectFont(Global::pConfig->seqFont);
	RECT rc;
	RECT rc1 = {0,0,12,12};
	
	GetClientRect(&rc);
	//TODO: Add your drawing code here
//	dc.ExtTextOut(100,100,ETO_OPAQUE | ETO_CLIPPED,&rc,"こ",-1,NULL);
	TCHAR a[] = "ABCD";
	TCHAR* b = new char[5];
	strcpy(b,a);

	int i,j;
	for(i = 0;i < rc.right - rc.left;i += 24){

		for(j = 0;j < rc.bottom - rc.top ;j += 10){
			dc.SetBkColor(0x00404040 + i);
			dc.SetTextColor(0x00f0f0f0);
			rc1.left = i + rc.left;
			rc1.top = j+ rc.top;
			rc1.right = rc1.left + 8 * 23;
			rc1.bottom = rc1.bottom + 8;
//			TXTFLAT(&dc,&rc1,_T("AAA")) ;
//			devc->ExtTextOut(rc1.left +  2,rc1.top,ETO_OPAQUE | ETO_CLIPPED ,&rc1,a,lstrlen(a),m_FLATSIZES);
			devc->ExtTextOut(rc1.left +  2,rc1.top,ETO_OPAQUE | ETO_CLIPPED ,&rc1,b,sizeof(b) - 1,m_FLATSIZES);

//			CDC* pdc = &dc;
//			pdc->ExtTextOut(i,j,ETO_OPAQUE | ETO_CLIPPED,&rc1,"AAA",-1,m_FLATSIZES);

//			dc.TextOut( i + rc.left,j + rc.top,L"こ",-1);
		}
	}
	delete [] b;
	dc.SelectFont(_old);
	dc.RestoreDC(-1);
	return;*/

	//ATLTRACE(_T("DefaultPatternView::DrawPatEditor Start %d ms\n"),timeGetTime() - m_elapsedTime);
	///////////////////////////////////////////////////////////
	// Prepare pattern for update (Undraw guipos and playpos)
	CRect rect(0,0,0,0);

	int scrollT= m_TrackOffset - m_RNTrackOffset;
	int scrollL= m_LineOffset - m_RNLineOffset;

	m_TrackOffset = ntOff = m_RNTrackOffset; 
	m_LineOffset = nlOff = m_RNLineOffset;
//	CFont _font;
//	_font.CreateFont(8,0,0,0,0,0,0,0,ANSI_CHARSET,0,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,0,_T("Tahoma"));

	CFontHandle oldFont(devc->SelectFont(Global::pConfig->seqFont));
//	CFontHandle oldFont(devc->SelectFont(_font));

	// 1 if there is a redraw header, we do that 
	/////////////////////////////////////////////////////////////
	// Update Mute/Solo Indicators
	if ((updatePar & DRAW_TRHEADER) || (abs(scrollT) > VISTRACKS) || (scrollT && scrollL))
	{
		rect.top = 0;
		rect.bottom = m_YOFFSET;
		if (m_XOFFSET!=1)
		{
			rect.left = 0;
			rect.right = 1;
			devc->FillSolidRect(&rect,pvc_separator[0]);
			rect.left++;
			rect.right = m_XOFFSET - 1;
			devc->FillSolidRect(&rect,pvc_background[0]);
			devc->SetBkColor(Global::pConfig->pvc_background);	// This affects TXT background
			devc->SetTextColor(Global::pConfig->pvc_font);
			TXT(devc,_T("Line"),1,1,m_XOFFSET - 2,m_YOFFSET - 2);
		}
		WTL::CDC memDC;
		memDC.CreateCompatibleDC(*devc);
		CBitmapHandle oldbmp(memDC.SelectBitmap(m_PatternHeader));
		int xOffset = m_XOFFSET-1;

		if (m_PatHeaderCoords.bHasTransparency)
		{
			for(int i=m_TrackOffset;i<m_TrackOffset+m_MaxTracks;i++)
			{
				rect.left = xOffset;
				rect.right = xOffset+1;
				devc->FillSolidRect(&rect,pvc_separator[i+1]);
				rect.left++;
				rect.right+= m_ROWWIDTH-1;
				devc->FillSolidRect(&rect,pvc_background[i+1]);

				const int trackx0 = i/10;
				const int track0x = i%10;

				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				TransparentBlt(devc,
					xOffset+1+m_HEADER_INDENT,
					1,
					m_PatHeaderCoords.sBackground.width, 
					m_PatHeaderCoords.sBackground.height,
					&memDC, 
					&m_PatternHeaderMask,
					m_PatHeaderCoords.sBackground.x,
					m_PatHeaderCoords.sBackground.y);
				TransparentBlt(devc,
					xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dDigitX0.x, 
					1+m_PatHeaderCoords.dDigitX0.y, 
					m_PatHeaderCoords.sNumber0.width,	 
					m_PatHeaderCoords.sNumber0.height, 
					&memDC, 
					&m_PatternHeaderMask,
					m_PatHeaderCoords.sNumber0.x+(trackx0*m_PatHeaderCoords.sNumber0.width), 
					m_PatHeaderCoords.sNumber0.y);
				TransparentBlt(devc,
					xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dDigit0X.x, 
					1+m_PatHeaderCoords.dDigit0X.y, 
					m_PatHeaderCoords.sNumber0.width,	 
					m_PatHeaderCoords.sNumber0.height, 
					&memDC, 
					&m_PatternHeaderMask,
					m_PatHeaderCoords.sNumber0.x+(track0x*m_PatHeaderCoords.sNumber0.width), 
					m_PatHeaderCoords.sNumber0.y);
				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				if (Global::m_pSong->IsTrackMuted(i))
					TransparentBlt(devc,
						xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dMuteOn.x, 
						1+m_PatHeaderCoords.dMuteOn.y, 
						m_PatHeaderCoords.sMuteOn.width, 
						m_PatHeaderCoords.sMuteOn.height, 
						&memDC, 
						&m_PatternHeaderMask,
						m_PatHeaderCoords.sMuteOn.x, 
						m_PatHeaderCoords.sMuteOn.y);

				if (Global::m_pSong->IsTrackArmed(i))
					TransparentBlt(devc,
						xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dRecordOn.x, 
						1+m_PatHeaderCoords.dRecordOn.y, 
						m_PatHeaderCoords.sRecordOn.width, 
						m_PatHeaderCoords.sRecordOn.height, 
						&memDC, 
						&m_PatternHeaderMask,
						m_PatHeaderCoords.sRecordOn.x, 
						m_PatHeaderCoords.sRecordOn.y);

				if (Global::m_pSong->TrackSoloed() == i )
					TransparentBlt(devc,
						xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dSoloOn.x, 
						1+m_PatHeaderCoords.dSoloOn.y, 
						m_PatHeaderCoords.sSoloOn.width, 
						m_PatHeaderCoords.sSoloOn.height, 
						&memDC, 
						&m_PatternHeaderMask,
						m_PatHeaderCoords.sSoloOn.x, 
						m_PatHeaderCoords.sSoloOn.y);
				xOffset += m_ROWWIDTH;
			}
		}
		else
		{
			for(int i=m_TrackOffset;i<m_TrackOffset+m_MaxTracks;i++)
			{
				rect.left = xOffset;
				rect.right = xOffset+1;
				devc->FillSolidRect(&rect,pvc_separator[i+1]);
				rect.left++;
				rect.right+= m_ROWWIDTH-1;
				devc->FillSolidRect(&rect,pvc_background[i+1]);

				const int trackx0 = i/10;
				const int track0x = i%10;

				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				devc->BitBlt(
					xOffset+1+m_HEADER_INDENT,
					1,
					m_PatHeaderCoords.sBackground.width, 
					m_PatHeaderCoords.sBackground.height,
					memDC, 
					m_PatHeaderCoords.sBackground.x,
					m_PatHeaderCoords.sBackground.y, 
					SRCCOPY);
				devc->BitBlt(
					xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dDigitX0.x, 
					1+m_PatHeaderCoords.dDigitX0.y, 
					m_PatHeaderCoords.sNumber0.width,	 
					m_PatHeaderCoords.sNumber0.height, 
					memDC, 
					m_PatHeaderCoords.sNumber0.x+(trackx0*m_PatHeaderCoords.sNumber0.width), 
					m_PatHeaderCoords.sNumber0.y, 
					SRCCOPY);
				devc->BitBlt(
					xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dDigit0X.x, 
					1+m_PatHeaderCoords.dDigit0X.y, 
					m_PatHeaderCoords.sNumber0.width,	 
					m_PatHeaderCoords.sNumber0.height, 
					memDC, 
					m_PatHeaderCoords.sNumber0.x+(track0x*m_PatHeaderCoords.sNumber0.width), 
					m_PatHeaderCoords.sNumber0.y, 
					SRCCOPY);

				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				if (Global::m_pSong->IsTrackMuted(i))
					devc->BitBlt(
						xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dMuteOn.x, 
						1+m_PatHeaderCoords.dMuteOn.y, 
						m_PatHeaderCoords.sMuteOn.width, 
						m_PatHeaderCoords.sMuteOn.height, 
						(HDC)memDC, 
						m_PatHeaderCoords.sMuteOn.x, 
						m_PatHeaderCoords.sMuteOn.y, 
						SRCCOPY);

				if (Global::m_pSong->IsTrackArmed(i))
					devc->BitBlt(
						xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dRecordOn.x, 
						1+m_PatHeaderCoords.dRecordOn.y, 
						m_PatHeaderCoords.sRecordOn.width, 
						m_PatHeaderCoords.sRecordOn.height, 
						(HDC)memDC, 
						m_PatHeaderCoords.sRecordOn.x, 
						m_PatHeaderCoords.sRecordOn.y, 
						SRCCOPY);

				if (Global::m_pSong->TrackSoloed() == i )
					devc->BitBlt(
						xOffset+1+m_HEADER_INDENT+m_PatHeaderCoords.dSoloOn.x, 
						1+m_PatHeaderCoords.dSoloOn.y, 
						m_PatHeaderCoords.sSoloOn.width, 
						m_PatHeaderCoords.sSoloOn.height, 
						(HDC)memDC, 
						m_PatHeaderCoords.sSoloOn.x, 
						m_PatHeaderCoords.sSoloOn.y, 
						SRCCOPY);
				xOffset += m_ROWWIDTH;
			}
		}
		memDC.SelectBitmap(oldbmp);
		memDC.DeleteDC();
	}

	// 2 if there is a redraw all, we do that then exit
	if ((updatePar & DRAW_FULL_DATA) || (abs(scrollT) > VISTRACKS) || (abs(scrollL) > VISLINES) || (scrollT && scrollL))
	{
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("DRAW_FULL_DATA %d ms\n"),timeGetTime() - m_elapsedTime);
#endif
		// draw everything
		rect.top = m_YOFFSET;
		rect.bottom = m_CH;

		if (m_XOFFSET!=1)
		{
			rect.left = 0;
			rect.right = 1;
			devc->FillSolidRect(&rect,pvc_separator[0]);
			rect.left++;
			rect.right = m_XOFFSET-1;
			devc->FillSolidRect(&rect,pvc_background[0]);
		}
		int xOffset = m_XOFFSET-1;

		for (int i=m_TrackOffset;i<m_TrackOffset+m_MaxTracks;i++)
		{
			rect.left = xOffset;
			rect.right = xOffset+1;
			devc->FillSolidRect(&rect,pvc_separator[i + 1]);
			rect.left++;
			rect.right += m_ROWWIDTH-1;
			devc->FillSolidRect(&rect,pvc_background[i + 1]);

			xOffset += m_ROWWIDTH;
		}
		DrawPatternData(devc,0,VISTRACKS + 1,0,VISLINES + 1);
		// wipe todo list
		numPatternDraw = 0;
		// Fill Bottom Space with Background colour if needed
		if (m_MaxLines < VISLINES + 1)
		{
#ifdef _DEBUG_PATVIEW
			ATLTRACE(_T("DRAW_BOTTOM\n"));
#endif
			if (m_XOFFSET!=1)
			{
				rect.left = 0; 
				rect.right = m_XOFFSET; 
				rect.top = m_YOFFSET + (m_MaxLines * m_ROWHEIGHT); 
				rect.bottom = m_CH;
				devc->FillSolidRect(&rect,pvc_separator[0]);
			}

			int xOffset = m_XOFFSET;

			rect.top = m_YOFFSET + (m_MaxLines * m_ROWHEIGHT); 
			rect.bottom = m_CH;
			for(int i = m_TrackOffset; i < m_TrackOffset + m_MaxTracks; i++)
			{
				rect.left = xOffset; 
				rect.right = xOffset + m_ROWWIDTH; 
				devc->FillSolidRect(&rect,pvc_separator[i+1]);
				xOffset += m_ROWWIDTH;
			}
		}
		// Fill Right Space with Background colour if needed
		if (m_MaxTracks < VISTRACKS+1)
		{
#ifdef _DEBUG_PATVIEW
			ATLTRACE(_T("DRAW_RIGHT\n"));
#endif
			rect.top = 0; 
			rect.bottom = m_CH;  
			rect.right = m_CW;
			rect.left = m_XOFFSET + (m_MaxTracks * m_ROWWIDTH) - 1;
			devc->FillSolidRect(&rect,Global::pConfig->pvc_separator2);
		}
	}
	else
	{
		if (scrollT && scrollL)
		{

		}
		else // not scrollT + scrollL
		{
			// h scroll - remember to check the header when scrolling H so no double blits
			//			  add to draw list uncovered area
			if (scrollT)
			{
				CRgn rgn;
				if (updatePar & DRAW_TRHEADER)
				{
					const RECT patR = {m_XOFFSET,m_YOFFSET , m_CW, m_CH};
					devc->ScrollDC(scrollT * m_ROWWIDTH,0,&patR,&patR,(HRGN)rgn,&rect);
					if ( scrollT > 0 )
					{	
#ifdef _DEBUG_PATVIEW
						ATLTRACE(_T("DRAW_HSCROLL+\n"));
#endif
						rect.top = m_YOFFSET;
						rect.bottom = m_CH;

						int xOffset = m_XOFFSET-1;
						for (int i = 0; i < scrollT; i++)
						{
						rect.left = xOffset;
						rect.right = xOffset + 1;
							devc->FillSolidRect(&rect,pvc_separator[i+m_TrackOffset+1]);
							rect.left++;
							rect.right += m_ROWWIDTH-1;
							devc->FillSolidRect(&rect,pvc_background[i+m_TrackOffset+1]);

							xOffset += m_ROWWIDTH;
						}
						DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
					}
					else 
					{	
#ifdef _DEBUG_PATVIEW
						ATLTRACE(_T("DRAW_HSCROLL-\n"));
#endif
						rect.top = m_YOFFSET;
						rect.bottom = m_CH;
						int xOffset = m_XOFFSET-1+((VISTRACKS+scrollT)*m_ROWWIDTH);
						for (int i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
						{
							rect.left = xOffset;
							rect.right = xOffset+1;
							devc->FillSolidRect(&rect,pvc_separator[i+m_TrackOffset+1]);
							rect.left++;
							rect.right+= m_ROWWIDTH-1;
							devc->FillSolidRect(&rect,pvc_background[i+m_TrackOffset+1]);

							xOffset += m_ROWWIDTH;
						}
						DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
					}
				}
				else
				{
					// scroll header too
					const RECT trkR = {m_XOFFSET, 0, m_CW, m_CH};
					devc->ScrollDC(scrollT*m_ROWWIDTH,0,&trkR,&trkR,(HRGN)rgn,&rect);
					if (scrollT > 0)
					{	
#ifdef _DEBUG_PATVIEW
						ATLTRACE(_T("DRAW_HSCROLL+\n"));
#endif
						rect.top = 0;
						rect.bottom = m_CH;
						int xOffset = m_XOFFSET-1;
						for (int i = 0; i < scrollT; i++)
						{
							rect.left = xOffset;
							rect.right = xOffset+1;
							devc->FillSolidRect(&rect,pvc_separator[i+m_TrackOffset+1]);
							rect.left++;
							rect.right += m_ROWWIDTH-1;
							devc->FillSolidRect(&rect,pvc_background[i+m_TrackOffset+1]);

							xOffset += m_ROWWIDTH;
						}
						DrawPatternData(devc,0, scrollT, 0, VISLINES+1);

						CDC memDC;
						memDC.CreateCompatibleDC(*devc);
						CBitmapHandle oldbmp = memDC.SelectBitmap(m_PatternHeader);

						xOffset = m_XOFFSET-1;

						if (m_PatHeaderCoords.bHasTransparency)
						{
							for(i=m_TrackOffset;i<m_TrackOffset+scrollT;i++)
							{
								const int trackx0 = i/10;
								const int track0x = i%10;

								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								TransparentBlt(devc,
									xOffset + 1 + m_HEADER_INDENT,
									1,
									m_PatHeaderCoords.sBackground.width, 
									m_PatHeaderCoords.sBackground.height,
									&memDC, 
									&m_PatternHeaderMask,
									m_PatHeaderCoords.sBackground.x,
									m_PatHeaderCoords.sBackground.y);
								TransparentBlt(devc,
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigitX0.x, 
									1 + m_PatHeaderCoords.dDigitX0.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									&memDC, 
									&m_PatternHeaderMask,
									m_PatHeaderCoords.sNumber0.x+(trackx0*m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y);
								TransparentBlt(devc,
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigit0X.x, 
									1 + m_PatHeaderCoords.dDigit0X.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									&memDC, 
									&m_PatternHeaderMask,
									m_PatHeaderCoords.sNumber0.x + (track0x * m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y);
								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								if (Global::m_pSong->IsTrackMuted(i))
									TransparentBlt(devc,
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dMuteOn.x, 
										1 + m_PatHeaderCoords.dMuteOn.y, 
										m_PatHeaderCoords.sMuteOn.width, 
										m_PatHeaderCoords.sMuteOn.height, 
										&memDC, 
										&m_PatternHeaderMask,
										m_PatHeaderCoords.sMuteOn.x, 
										m_PatHeaderCoords.sMuteOn.y);

								if (Global::m_pSong->IsTrackArmed(i))
									TransparentBlt(devc,
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dRecordOn.x, 
										1 + m_PatHeaderCoords.dRecordOn.y, 
										m_PatHeaderCoords.sRecordOn.width, 
										m_PatHeaderCoords.sRecordOn.height, 
										&memDC, 
										&m_PatternHeaderMask,
										m_PatHeaderCoords.sRecordOn.x, 
										m_PatHeaderCoords.sRecordOn.y);

								if (Global::m_pSong->TrackSoloed() == i )
									TransparentBlt(devc,
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dSoloOn.x, 
										1 + m_PatHeaderCoords.dSoloOn.y, 
										m_PatHeaderCoords.sSoloOn.width, 
										m_PatHeaderCoords.sSoloOn.height, 
										&memDC, 
										&m_PatternHeaderMask,
										m_PatHeaderCoords.sSoloOn.x, 
										m_PatHeaderCoords.sSoloOn.y);
								xOffset += m_ROWWIDTH;
							}
						}
						else
						{
							for(i=m_TrackOffset;i<m_TrackOffset+scrollT;i++)
							{
								const int trackx0 = i/10;
								const int track0x = i%10;

								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								devc->BitBlt(
									xOffset + 1 + m_HEADER_INDENT,
									1,
									m_PatHeaderCoords.sBackground.width, 
									m_PatHeaderCoords.sBackground.height,
									(HDC)memDC, 
									m_PatHeaderCoords.sBackground.x,
									m_PatHeaderCoords.sBackground.y, 
									SRCCOPY);
								devc->BitBlt(
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigitX0.x, 
									1 + m_PatHeaderCoords.dDigitX0.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									(HDC)memDC, 
									m_PatHeaderCoords.sNumber0.x+(trackx0*m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y, 
									SRCCOPY);
								devc->BitBlt(
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigit0X.x, 
									1 + m_PatHeaderCoords.dDigit0X.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									(HDC)memDC, 
									m_PatHeaderCoords.sNumber0.x+(track0x*m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y, 
									SRCCOPY);

								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								if (Global::m_pSong->IsTrackMuted(i))
									devc->BitBlt(
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dMuteOn.x, 
										1 + m_PatHeaderCoords.dMuteOn.y, 
										m_PatHeaderCoords.sMuteOn.width, 
										m_PatHeaderCoords.sMuteOn.height, 
										(HDC)memDC, 
										m_PatHeaderCoords.sMuteOn.x, 
										m_PatHeaderCoords.sMuteOn.y, 
										SRCCOPY);

								if (Global::m_pSong->IsTrackArmed(i))
									devc->BitBlt(
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dRecordOn.x, 
										1 + m_PatHeaderCoords.dRecordOn.y, 
										m_PatHeaderCoords.sRecordOn.width, 
										m_PatHeaderCoords.sRecordOn.height, 
										(HDC)memDC, 
										m_PatHeaderCoords.sRecordOn.x, 
										m_PatHeaderCoords.sRecordOn.y, 
										SRCCOPY);

								if (Global::m_pSong->TrackSoloed() == i )
									devc->BitBlt(
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dSoloOn.x, 
										1 + m_PatHeaderCoords.dSoloOn.y, 
										m_PatHeaderCoords.sSoloOn.width, 
										m_PatHeaderCoords.sSoloOn.height, 
										(HDC)memDC, 
										m_PatHeaderCoords.sSoloOn.x, 
										m_PatHeaderCoords.sSoloOn.y, 
										SRCCOPY);
								xOffset += m_ROWWIDTH;
							}
						}
						memDC.SelectBitmap((HBITMAP)oldbmp);
						memDC.DeleteDC();
					}
					else 
					{	
#ifdef _DEBUG_PATVIEW
						ATLTRACE(_T("DRAW_HSCROLL-\n"));
#endif
						rect.top = 0;
						rect.bottom = m_CH;
						int xOffset = m_XOFFSET - 1 + ((VISTRACKS + scrollT) * m_ROWWIDTH);
						for (int i = VISTRACKS + scrollT; i < VISTRACKS + 1; i++)
						{
							rect.left = xOffset;
							rect.right = xOffset + 1;
							devc->FillSolidRect(&rect,pvc_separator[i + m_TrackOffset + 1]);
							rect.left++;
							rect.right += m_ROWWIDTH - 1;
							devc->FillSolidRect(&rect,pvc_background[i + m_TrackOffset + 1]);

							xOffset += m_ROWWIDTH;
						}
						DrawPatternData(devc,VISTRACKS + scrollT, VISTRACKS + 1, 0, VISLINES + 1);

						CDC memDC;
						memDC.CreateCompatibleDC((HDC)(*devc));
						CBitmapHandle oldbmp(memDC.SelectBitmap((HBITMAP)m_PatternHeader));

						xOffset = m_XOFFSET - 1 + ((m_MaxTracks + scrollT - 1) * m_ROWWIDTH);

						if (m_PatHeaderCoords.bHasTransparency)
						{
							for(i = m_TrackOffset + m_MaxTracks + scrollT - 1; i < m_TrackOffset + m_MaxTracks; i++)
							{
								const int trackx0 = i / 10;
								const int track0x = i % 10;

								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								TransparentBlt(devc,
									xOffset + 1 + m_HEADER_INDENT,
									1,
									m_PatHeaderCoords.sBackground.width, 
									m_PatHeaderCoords.sBackground.height,
									&memDC, 
									&m_PatternHeaderMask,
									m_PatHeaderCoords.sBackground.x,
									m_PatHeaderCoords.sBackground.y);
								TransparentBlt(devc,
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigitX0.x, 
									1 + m_PatHeaderCoords.dDigitX0.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									&memDC, 
									&m_PatternHeaderMask,
									m_PatHeaderCoords.sNumber0.x + (trackx0 * m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y);
								TransparentBlt(devc,
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigit0X.x, 
									1 + m_PatHeaderCoords.dDigit0X.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									&memDC, 
									&m_PatternHeaderMask,
									m_PatHeaderCoords.sNumber0.x+(track0x*m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y);
								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								if (Global::m_pSong->IsTrackMuted(i))
									TransparentBlt(devc,
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dMuteOn.x, 
										1 + m_PatHeaderCoords.dMuteOn.y, 
										m_PatHeaderCoords.sMuteOn.width, 
										m_PatHeaderCoords.sMuteOn.height, 
										&memDC, 
										&m_PatternHeaderMask,
										m_PatHeaderCoords.sMuteOn.x, 
										m_PatHeaderCoords.sMuteOn.y);

								if (Global::m_pSong->IsTrackArmed(i))
									TransparentBlt(devc,
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dRecordOn.x, 
										1 + m_PatHeaderCoords.dRecordOn.y, 
										m_PatHeaderCoords.sRecordOn.width, 
										m_PatHeaderCoords.sRecordOn.height, 
										&memDC, 
										&m_PatternHeaderMask,
										m_PatHeaderCoords.sRecordOn.x, 
										m_PatHeaderCoords.sRecordOn.y);

								if (Global::m_pSong->TrackSoloed() == i )
									TransparentBlt(devc,
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dSoloOn.x, 
										1 + m_PatHeaderCoords.dSoloOn.y, 
										m_PatHeaderCoords.sSoloOn.width, 
										m_PatHeaderCoords.sSoloOn.height, 
										&memDC, 
										&m_PatternHeaderMask,
										m_PatHeaderCoords.sSoloOn.x, 
										m_PatHeaderCoords.sSoloOn.y);
								xOffset += m_ROWWIDTH;
							}
						}
						else
						{
							for(i = m_TrackOffset + m_MaxTracks + scrollT - 1; i < m_TrackOffset + m_MaxTracks; i++)
							{
								const int trackx0 = i / 10;
								const int track0x = i % 10;

								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								devc->BitBlt(
									xOffset + 1 + m_HEADER_INDENT,
									1,
									m_PatHeaderCoords.sBackground.width, 
									m_PatHeaderCoords.sBackground.height,
									(HDC)memDC, 
									m_PatHeaderCoords.sBackground.x,
									m_PatHeaderCoords.sBackground.y, 
									SRCCOPY);
								devc->BitBlt(
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigitX0.x, 
									1 + m_PatHeaderCoords.dDigitX0.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									(HDC)memDC, 
									m_PatHeaderCoords.sNumber0.x + (trackx0 * m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y, 
									SRCCOPY);
								devc->BitBlt(
									xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dDigit0X.x, 
									1 + m_PatHeaderCoords.dDigit0X.y, 
									m_PatHeaderCoords.sNumber0.width,	 
									m_PatHeaderCoords.sNumber0.height, 
									(HDC)memDC, 
									m_PatHeaderCoords.sNumber0.x + (track0x * m_PatHeaderCoords.sNumber0.width), 
									m_PatHeaderCoords.sNumber0.y, 
									SRCCOPY);

								// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
								if (Global::m_pSong->IsTrackMuted(i))
									devc->BitBlt(
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dMuteOn.x, 
										1 + m_PatHeaderCoords.dMuteOn.y, 
										m_PatHeaderCoords.sMuteOn.width, 
										m_PatHeaderCoords.sMuteOn.height, 
										(HDC)memDC, 
										m_PatHeaderCoords.sMuteOn.x, 
										m_PatHeaderCoords.sMuteOn.y, 
										SRCCOPY);

								if (Global::m_pSong->IsTrackArmed(i))
									devc->BitBlt(
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dRecordOn.x, 
										1 + m_PatHeaderCoords.dRecordOn.y, 
										m_PatHeaderCoords.sRecordOn.width, 
										m_PatHeaderCoords.sRecordOn.height, 
										(HDC)memDC, 
										m_PatHeaderCoords.sRecordOn.x, 
										m_PatHeaderCoords.sRecordOn.y, 
										SRCCOPY);

								if (Global::m_pSong->TrackSoloed() == i )
									devc->BitBlt(
										xOffset + 1 + m_HEADER_INDENT + m_PatHeaderCoords.dSoloOn.x, 
										1 + m_PatHeaderCoords.dSoloOn.y, 
										m_PatHeaderCoords.sSoloOn.width, 
										m_PatHeaderCoords.sSoloOn.height, 
										(HDC)memDC, 
										m_PatHeaderCoords.sSoloOn.x, 
										m_PatHeaderCoords.sSoloOn.y, 
										SRCCOPY);
								xOffset += m_ROWWIDTH;
							}
						}
						memDC.SelectBitmap((HBITMAP)oldbmp);
						memDC.DeleteDC();
					}
				}
				// Fill Bottom Space with Background colour if needed
				if (m_MaxLines < VISLINES + 1)
				{
					int xOffset = m_XOFFSET;
					CRect rect;
					rect.top = m_YOFFSET + (m_MaxLines * m_ROWHEIGHT); 
					rect.bottom = m_CH;
					for(int i = m_TrackOffset; i < m_TrackOffset + m_MaxTracks; i++)
					{
						rect.left = xOffset; 
						rect.right = xOffset + m_ROWWIDTH; 
						devc->FillSolidRect(&rect,pvc_separator[i+1]);
						xOffset += m_ROWWIDTH;
					}
				}
				// Fill Right Space with Background colour if needed
				if (m_MaxTracks < VISTRACKS + 1)
				{
#ifdef _DEBUG_PATVIEW
					ATLTRACE(_T("DRAW_RIGHT\n"));
#endif
					CRect rect;
					rect.top = 0; 
					rect.bottom = m_CH;  
					rect.right = m_CW;
					rect.left = m_XOFFSET + (m_MaxTracks * m_ROWWIDTH) - 1;
					devc->FillSolidRect(&rect,Global::pConfig->pvc_separator2);
				}
			}

			// v scroll - 
			//			  add to draw list uncovered area
			else if (scrollL)
			{
				const RECT linR = {0, m_YOFFSET, m_CW, m_CH};
				CRgn rgn;
				devc->ScrollDC(0,scrollL * m_ROWHEIGHT,&linR,&linR,(HRGN)rgn,&rect);
				// add visible part to 
				if (scrollL > 0)
				{	
#ifdef _DEBUG_PATVIEW
					ATLTRACE(_T("DRAW_VSCROLL+\n"));
#endif
					//if(m_EditCursor.line!=0)
					DrawPatternData(devc, 0, VISTRACKS + 1, 0,scrollL);
				}
				else 
				{	
#ifdef _DEBUG_PATVIEW
					ATLTRACE(_T("DRAW_VSCROLL-\n"));
#endif
					DrawPatternData(devc, 0, VISTRACKS + 1,VISLINES + scrollL,VISLINES + 1);
				}
				// Fill Bottom Space with Background colour if needed
				if (m_MaxLines < VISLINES + 1)
				{
#ifdef _DEBUG_PATVIEW
					ATLTRACE(_T("DRAW_BOTTOM\n"));
#endif
					if (m_XOFFSET != 1)
					{
						CRect rect;
						rect.left = 0; 
						rect.right = m_XOFFSET; 
						rect.top = m_YOFFSET + (m_MaxLines * m_ROWHEIGHT); 
						rect.bottom = m_CH;
						devc->FillSolidRect(&rect,pvc_separator[0]);
					}

					int xOffset = m_XOFFSET;

					CRect rect;
					rect.top = m_YOFFSET + (m_MaxLines * m_ROWHEIGHT); 
					rect.bottom = m_CH;
					for(int i = m_TrackOffset; i < m_TrackOffset + m_MaxTracks; i++)
					{
						rect.left = xOffset; 
						rect.right = xOffset + m_ROWWIDTH; 
						devc->FillSolidRect(&rect,pvc_separator[i+1]);
						xOffset += m_ROWWIDTH;
					}
				}
			}
		}

		// then we draw any data that needs to be drawn
		// each time we draw data check for playbar or cursor, not fast but...
		// better idea is to have an array of flags, so never draw twice
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////
		// Draw Pattern data.
		if (updatePar & DRAW_DATA)
		{
#ifdef _DEBUG_PATVIEW
			ATLTRACE(_T("DRAW_DATA\n"));
#endif
			////////////////////////////////////////////////
			// Draw Data Changed (DMDataChange)
			for (int i = 0; i < numPatternDraw; i++)
			{

				int ts = m_pPatternDraw[i].drawTrackStart - m_TrackOffset;
				if ( ts < 0 ) 
					ts = 0;
				int te = m_pPatternDraw[i].drawTrackEnd - (m_TrackOffset-1);
				if ( te > m_MaxTracks ) 
					te = m_MaxTracks;

				int ls = m_pPatternDraw[i].drawLineStart - m_LineOffset;
				if ( ls < 0 ) 
					ls = 0;
				int le = m_pPatternDraw[i].drawLineEnd - (m_LineOffset - 1);
				if ( le > m_MaxLines ) 
					le = m_MaxLines;

				DrawPatternData(devc,ts,te,ls,le);
			}
			numPatternDraw = 0;
		}
	}

	playpos = newplaypos;
	selpos = newselpos;
	m_EditLastPosition = m_EditCursor;

	devc->SelectFont((HFONT)oldFont);

	updateMode = DMNone;
	updatePar = 0;
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("DefaultPatternView::DrawPatEditor End %d \n"),timeGetTime() - m_elapsedTime);
#endif
}


// ADVISE! [m_LineOffset+lstart..m_LineOffset+lend] and [m_TrackOffset+tstart..m_TrackOffset+tend] HAVE TO be valid!
void DefaultPatternView::DrawPatternData(CDC * const devc,int tstart,int tend, int lstart, int lend)
{

#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("DrawPatternData() Start %d ms\n"),timeGetTime() - m_elapsedTime);
#endif

//	if (lstart > VISLINES)
	if (lstart > m_MaxLines)
	{
		return;
	}
	else if (lstart < 0)
	{
		lstart = 0;
	}


	if (lend < 0)
	{
		return;
	}
	else if (lend > m_MaxLines)
//	else if (lend > VISLINES+1)
	{
//		lend = VISLINES+1;
		lend = m_MaxLines;
	}

//	if (tstart > VISTRACKS)
	if (tstart > m_MaxTracks)
	{
		return;
	}
	else if (tstart < 0)
	{
		tstart = 0;
	}

	if (tend < 0)
	{
		return;
	}
//	else if (tend > VISTRACKS+1)
	else if (tend > m_MaxTracks)
	{
//		tend = VISTRACKS+1;
		tend = m_MaxTracks;
	}

	int yOffset = lstart * m_ROWHEIGHT + m_YOFFSET;
	int linecount = m_LineOffset + lstart;
	TCHAR tBuf[16];

	COLORREF* pBkg;
	for (int i = lstart; i < lend; i++) // Lines
	{
		// break this up into several more general loops for speed
		if((linecount%m_pSong->TicksPerBeat()) == 0)
		{
			if ((linecount % (m_pSong->TicksPerBeat() * 4)) == 0) 
				pBkg = pvc_row4beat;
			else 
				pBkg = pvc_rowbeat;
		}
		else
		{
			pBkg = pvc_row;
		}

		if ((m_XOFFSET != 1))// && (tstart == 0))
		{
			if ((linecount == m_EditCursor.line) && (Global::pConfig->_linenumbersCursor))
			{
				devc->SetBkColor(pvc_cursor[0]);
				devc->SetTextColor(pvc_fontCur[0]);
			}
			else if (linecount == newplaypos)
			{
				devc->SetBkColor(pvc_playbar[0]);
				devc->SetTextColor(pvc_fontPlay[0]);
			}
			else 
			{
				devc->SetBkColor(pBkg[0]);
				devc->SetTextColor(pvc_font[0]);
			}
			if (Global::pConfig->_linenumbersHex)
			{
				_stprintf(tBuf,_T(" %.2X"),linecount);
				TXTFLAT(devc,tBuf,1,yOffset,m_XOFFSET - 2,m_ROWHEIGHT - 1);	// Print Line Number.
			}
			else
			{
				_stprintf(tBuf,_T("%3i"),linecount);
				TXTFLAT(devc,tBuf,1,yOffset,m_XOFFSET - 2,m_ROWHEIGHT - 1);	// Print Line Number.
			}
		}

		PatternEntry *_pentry = reinterpret_cast<PatternEntry *>(_ppattern() +
									(linecount * MULTIPLY) + (tstart + m_TrackOffset) * EVENT_SIZE);

		int xOffset = m_XOFFSET + (tstart * m_ROWWIDTH);
		int trackcount = tstart + m_TrackOffset;
		for (int t = tstart; t < tend; t++)
		{
			if (linecount == newplaypos)
			{
				devc->SetBkColor(pvc_playbar[trackcount]);
				devc->SetTextColor(pvc_fontPlay[trackcount]);
			}
			else if ((linecount >= newselpos.top) && 
				(linecount < newselpos.bottom) &&
				(trackcount >= newselpos.left) &&
				(trackcount < newselpos.right))
			{

				if (pBkg == pvc_rowbeat)
				{
					devc->SetBkColor(pvc_selectionbeat[trackcount]);
				}
				else if (pBkg == pvc_row4beat)
				{
					devc->SetBkColor(pvc_selection4beat[trackcount]);
				}
				else
				{
					devc->SetBkColor(pvc_selection[trackcount]);
				}
				devc->SetTextColor(pvc_fontSel[trackcount]);
			}
			else
			{
				devc->SetBkColor(pBkg[trackcount]);
/*				if((*patOffset) < 120){
					//_textcolor = RGB((BYTE)((DWORD)GetRValue(pvc_font[trackcount]) * (DWORD)(*(patOffset + 5)) / 256),GetGValue(pvc_font[trackcount]),GetBValue(pvc_font[trackcount]));
					_textcolor = RGB((BYTE)(255 * (DWORD)(*(patOffset + 5)) / 256),GetGValue(pvc_font[trackcount]),GetBValue(pvc_font[trackcount]));
					devc->SetTextColor(_textcolor);
				} else {
					devc->SetTextColor(pvc_font[trackcount]);
				}
*/
				devc->SetTextColor(pvc_font[trackcount]);
			}
			
			OutNote(devc,xOffset + m_COLX[0],yOffset,_pentry->_note);

			// Instrument
			if (_pentry->_inst == 255 )
			{
				OutData(devc,xOffset + m_COLX[1],yOffset,0,true);
			}
			else
			{
				OutData(devc,xOffset + m_COLX[1],yOffset,_pentry->_inst,false);
			}

			// MachineNo
			if (_pentry->_mach == 255 )
			{
				OutData(devc,xOffset + m_COLX[3],yOffset,0,true);
			}
			else 
			{
				OutData(devc,xOffset + m_COLX[3],yOffset,_pentry->_mach,false);
			}
			
			bool _bNoteOrSpace = (_pentry->_note <= 120 || _pentry->_note == 255 );

			if (_pentry->_cmd == 0 && _pentry->_parameter == 0 && _bNoteOrSpace)
			{
				OutData(devc,xOffset + m_COLX[5],yOffset,0,true);
				OutData(devc,xOffset + m_COLX[7],yOffset,0,true);
			}
			else
			{
				OutData(devc,xOffset + m_COLX[5],yOffset,_pentry->_cmd,false);
				OutData(devc,xOffset + m_COLX[7],yOffset,_pentry->_parameter,false);
			}
			
			// Print Volume(VolCmd)
			if (_pentry->_volcmd == 0 && _bNoteOrSpace)
			{
				OutData(devc,xOffset+m_COLX[9],yOffset,0,true);
			} else {
				OutData(devc,xOffset+m_COLX[9],yOffset,_pentry->_volcmd,false);
			}

			if (_pentry->_volume == 0 && _bNoteOrSpace && _pentry->_volcmd == 0)
			{
				OutData(devc,xOffset+m_COLX[11],yOffset,0,true);
			} else {
				OutData(devc,xOffset+m_COLX[11],yOffset,_pentry->_volume,false);
			}


			// could optimize this check some, make separate loops
			if ((linecount == m_EditCursor.line) && (trackcount == m_EditCursor.track))
			{
				devc->SetBkColor(pvc_cursor[trackcount]);
				devc->SetTextColor(pvc_fontCur[trackcount]);
				switch (m_EditCursor.col)
				{
				case 0:
					OutNote(devc,xOffset + m_COLX[0],yOffset,_pentry->_note);
					break;
				case 1:
					if (_pentry->_inst == 255 )
					{
						OutData4(devc,xOffset + m_COLX[1],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[1],yOffset,(_pentry->_inst) >> 4,false);
					}
					break;
				case 2:
					if (_pentry->_inst == 255 )
					{
						OutData4(devc,xOffset + m_COLX[2],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[2],yOffset,_pentry->_inst,false);
					}
					break;
				case 3:
					if (_pentry->_mach == 255 )
					{
						OutData4(devc,xOffset + m_COLX[3],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[3],yOffset,(_pentry->_mach) >> 4,false);
					}
					break;
				case 4:
					if (_pentry->_mach == 255 )
					{
						OutData4(devc,xOffset + m_COLX[4],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[4],yOffset,(_pentry->_mach),false);
					}
					break;
				case 5:
					if (_pentry->_cmd == 0 && _pentry->_parameter == 0 && _bNoteOrSpace)
					{
						OutData4(devc,xOffset + m_COLX[5],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[5],yOffset,(_pentry->_cmd) >> 4,false);
					}
					break;
				case 6:
					if (_pentry->_cmd == 0 && _pentry->_parameter == 0 && _bNoteOrSpace)
					{
						OutData4(devc,xOffset + m_COLX[6],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[6],yOffset,_pentry->_cmd,false);
					}
					break;
				case 7:
					if (_pentry->_cmd == 0 && _pentry->_parameter == 0 && _bNoteOrSpace)
					{
						OutData4(devc,xOffset + m_COLX[7],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[7],yOffset,(_pentry->_parameter) >> 4,false);
					}
					break;
				case 8:
					if (_pentry->_cmd == 0 && _pentry->_parameter == 0 && _bNoteOrSpace)
					{
						OutData4(devc,xOffset + m_COLX[8],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset + m_COLX[8],yOffset,(_pentry->_parameter),false);
					}

					break;
				case 9:
					if (_pentry->_volcmd == 0  && _bNoteOrSpace)
					{
						OutData4(devc,xOffset+m_COLX[9],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+m_COLX[9],yOffset,(_pentry->_volcmd) >> 4,false);
					}
					break;
				case 10:
					if (_pentry->_volcmd == 0  && _bNoteOrSpace)
					{
						OutData4(devc,xOffset+m_COLX[10],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+m_COLX[10],yOffset,(_pentry->_volcmd),false);
					}
					break;
				case 11:
					if (_pentry->_volume == 0  && _bNoteOrSpace && _pentry->_volcmd == 0)
					{
						OutData4(devc,xOffset+m_COLX[11],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+m_COLX[11],yOffset,(_pentry->_volume) >> 4,false);
					}
					break;
				case 12:
					if (_pentry->_volume == 0  && _bNoteOrSpace && _pentry->_volcmd == 0)
					{
						OutData4(devc,xOffset+m_COLX[12],yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+m_COLX[12],yOffset,(_pentry->_volume),false);
					}
					break;

				}
			}
			trackcount++;
			_pentry++;
//			patOffset++;
			xOffset += m_ROWWIDTH;
		}
		linecount++;
		yOffset += m_ROWHEIGHT;
	}

	#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("DrawPatternData() End %d ms\n"),timeGetTime() - m_elapsedTime);
	#endif
}

void DefaultPatternView::NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd)
{
	if (viewMode == VMPattern)
	{
		if (!(updatePar & DRAW_FULL_DATA))
		{
			// inserts pattern data to be drawn into the list
			if (numPatternDraw < MAX_DRAW_MESSAGES)
			{
				for (int i=0; i < numPatternDraw; i++)
				{
					if ((m_pPatternDraw[i].drawTrackStart <= drawTrackStart) &&
						(m_pPatternDraw[i].drawTrackEnd >= drawTrackEnd) &&
						(m_pPatternDraw[i].drawLineStart <= drawLineStart) &&
						(m_pPatternDraw[i].drawLineEnd >= drawLineEnd))
					{
						return;
					}
				}
				m_pPatternDraw[numPatternDraw].drawTrackStart = drawTrackStart;
				m_pPatternDraw[numPatternDraw].drawTrackEnd = drawTrackEnd;
				m_pPatternDraw[numPatternDraw].drawLineStart = drawLineStart;
				m_pPatternDraw[numPatternDraw].drawLineEnd = drawLineEnd;
				numPatternDraw++;
			}
			else if (numPatternDraw == MAX_DRAW_MESSAGES)
			{
				// this should never have to happen with a 32 message buffer, but just incase....
				numPatternDraw++;
				PreparePatternRefresh(DMAll);
			}
		}
	}
	else
	{
		numPatternDraw=0;
	}
}

void DefaultPatternView::RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2)
{
	// makes an array of colours between source1 and source2
	float p0 = float((source1>>16)&0xff);
	float p1 = float((source1>>8)&0xff);
	float p2 = float(source1&0xff);

	float d0 = float((source2>>16)&0xff);
	float d1 = float((source2>>8)&0xff);
	float d2 = float(source2&0xff);

	int len = m_pSong->SongTracks()+1;

	float a0=(d0-p0)/(len);
	float a1=(d1-p1)/(len);
	float a2=(d2-p2)/(len);

	for (int i = 0; i < len; i++)
	{
		pDest[i] = (f2i(p0*0x10000)&0xff0000)
					| (f2i(p1*0x100)&0xff00)
					| (f2i(p2)&0xff);
		p0+=a0;
		p1+=a1;
		p2+=a2;

		if (p0 < 0)
		{
			p0 = 0;
		}
		else if (p0 > 255)
		{
			p0 = 255;
		}

		if (p1 < 0)
		{
			p1 = 0;
		}
		else if (p1 > 255)
		{
			p1 = 255;
		}

		if (p2 < 0)
		{
			p2 = 2;
		}
		else if (p2 > 255)
		{
			p2 = 255;
		}
	}
}

COLORREF DefaultPatternView::ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add)
{
	int a0 = ((add>>16)&0x0ff)+((adjust>>16)&0x0ff)-((base>>16)&0x0ff);
	int a1 = ((add>>8 )&0x0ff)+((adjust>>8 )&0x0ff)-((base>>8 )&0x0ff);
	int a2 = ((add    )&0x0ff)+((adjust    )&0x0ff)-((base    )&0x0ff);

	if (a0 < 0)
	{
		a0 = 0;
	}
	else if (a0 > 255)
	{
		a0 = 255;
	}

	if (a1 < 0)
	{
		a1 = 0;
	}
	else if (a1 > 255)
	{
		a1 = 255;
	}

	if (a2 < 0)
	{
		a2 = 0;
	}
	else if (a2 > 255)
	{
		a2 = 255;
	}

	COLORREF pa = (a0<<16) | (a1<<8) | (a2);
	return pa;
}

void DefaultPatternView::RecalculateColourGrid()
{
	RecalculateColour(pvc_background, Global::pConfig->pvc_background, Global::pConfig->pvc_background2);
	RecalculateColour(pvc_separator, Global::pConfig->pvc_separator, Global::pConfig->pvc_separator2);
	RecalculateColour(pvc_row4beat, Global::pConfig->pvc_row4beat, Global::pConfig->pvc_row4beat2);
	RecalculateColour(pvc_rowbeat, Global::pConfig->pvc_rowbeat, Global::pConfig->pvc_rowbeat2);
	RecalculateColour(pvc_row, Global::pConfig->pvc_row, Global::pConfig->pvc_row2);
	RecalculateColour(pvc_selection, Global::pConfig->pvc_selection, Global::pConfig->pvc_selection2);
	RecalculateColour(pvc_playbar, Global::pConfig->pvc_playbar, Global::pConfig->pvc_playbar2);
	RecalculateColour(pvc_cursor, Global::pConfig->pvc_cursor, Global::pConfig->pvc_cursor2);
	RecalculateColour(pvc_font, Global::pConfig->pvc_font, Global::pConfig->pvc_font2);
	RecalculateColour(pvc_fontPlay, Global::pConfig->pvc_fontPlay, Global::pConfig->pvc_fontPlay2);
	RecalculateColour(pvc_fontCur, Global::pConfig->pvc_fontCur, Global::pConfig->pvc_fontCur2);
	RecalculateColour(pvc_fontSel, Global::pConfig->pvc_fontSel, Global::pConfig->pvc_fontSel2);
	RecalculateColour(pvc_selectionbeat, ColourDiffAdd(Global::pConfig->pvc_row, Global::pConfig->pvc_rowbeat, Global::pConfig->pvc_selection), ColourDiffAdd(Global::pConfig->pvc_row2, Global::pConfig->pvc_rowbeat2, Global::pConfig->pvc_selection2));
	RecalculateColour(pvc_selection4beat, ColourDiffAdd(Global::pConfig->pvc_row, Global::pConfig->pvc_row4beat, Global::pConfig->pvc_selection), ColourDiffAdd(Global::pConfig->pvc_row2, Global::pConfig->pvc_row4beat2, Global::pConfig->pvc_selection2));
}

void DefaultPatternView::KeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// undo code not required, enter note handles it
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
	if (cmd.GetType() == CT_Note)
	{
		const int outnote = cmd.GetNote();
		if(viewMode == VMPattern && bEditMode && Global::pPlayer->_playing && Global::pConfig->_followSong && Global::pConfig->_RecordNoteoff)
		{ 
			EnterNote(outnote,0,true);	// note end
		}
		else
		{
			Global::pInputHandler->StopNote(outnote);
		}
	}
	else if ((nChar == 16) && ((nFlags & 0xC000) == 0xC000) && ChordModeOffs)
	{
		// shift is up, abort chord mode
		editcur.line = m_ChordModeLine;
		editcur.track = m_ChordModeTrack;
		ChordModeOffs = 0;
		AdvanceLine(patStep,Global::pConfig->_wrapAround,true);
//		m_pMainFrame->StatusBarIdle();
//		Repaint(DMCursor);
	}
}

void DefaultPatternView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// undo code not required, enter note handles it
	KeyUp(nChar, nRepCnt, nFlags);
	//SetMsgHandled(FALSE);
}

void DefaultPatternView::KeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// undo code not required, enter not and msbput handle it
	BOOL bRepeat = nFlags&0x4000;

	if(viewMode == VMPattern && bEditMode)
	{
		if (!(Global::pPlayer->_playing && Global::pConfig->_followSong && bRepeat))
		{
			bool success;
			// add data
			success = Global::pInputHandler->EnterData(nChar,nFlags);

			if ( success )
			{
				SetMsgHandled(FALSE);
				return;
			}
		}
	}
	else
	{
		ChordModeOffs = 0;
	}

	// get command
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

	if(cmd.IsValid())
	{
		if((cmd.GetType() == CT_Immediate) ||
		   (cmd.GetType() == CT_Editor && viewMode == VMPattern) ) 
		{			
			Global::pInputHandler->PerformCmd(cmd,bRepeat);
		}
		else if (cmd.GetType() == CT_Note )
		{
			if(!bRepeat) 
			{	
				const int outnote = cmd.GetNote();
				// play note
				Global::pInputHandler->PlayNote(outnote); 
			}
		}
	}
}

void DefaultPatternView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	KeyDown(nChar, nRepCnt, nFlags);
	SetMsgHandled(TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MidiPatternNote
//
// DESCRIPTION	  : Called by the MIDI input interface to insert pattern notes
// PARAMETERS     : int outnote - note to insert . int velocity - velocity of the note
// RETURNS		  : <void>
// 

//
// Mark!!!!! Please, check if the following function is ok. I have some doubts about the
// NoteOff. And check if "if(outnote >= 0 && outnote <= 120)" is necessary.
//

void DefaultPatternView::MidiPatternNote(int outnote, int velocity)
{
	// undo code not required, enter note handles it
/*	if(outnote >= 0 && outnote <= 120)  // I really believe this is not necessary.
	{									// outnote <= 120 is checked before calling this function
										// and outnote CAN NOT be negative since it's taken from
										//	(dwParam1 & 0xFF00) >>8;
	*/
		if(viewMode == VMPattern && bEditMode)
		{ 
			// add note
			if(velocity > 0 && outnote != 120)
			{
				EnterNote(outnote,velocity,false);
			}
			else
			{
				if(Global::pConfig->_RecordNoteoff && Global::pPlayer->_playing && Global::pConfig->_followSong)
				{
					EnterNote(outnote,0,false);	// note end
				}
				else
				{
					Global::pInputHandler->StopNote(outnote,false);	// note end
				}
			}			
		}
		else 
		{
			// play note
			if(velocity>0)
				Global::pInputHandler->PlayNote(outnote,velocity,false);
			else
				Global::pInputHandler->StopNote(outnote,false);
		}
//	}
}

void DefaultPatternView::MidiPatternTweak(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._cmd = (value>>8)&255;
	entry._parameter = value&255;
	entry._inst = command;
	entry._note = cdefTweakM;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				// play it
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				// play
				if (pMachine)
				{
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if (pentry->_note >= 120)
		{
			if ((pentry->_mach != entry._mach) 
				|| (pentry->_cmd != entry._cmd)
				|| (pentry->_parameter != entry._parameter) 
				|| (pentry->_inst != entry._inst) 
				|| ((pentry->_note != cdefTweakM) && (pentry->_note != cdefTweakE) && (pentry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				pentry->_mach = entry._mach;
				pentry->_cmd = entry._cmd;
				pentry->_parameter = entry._parameter;
				pentry->_inst = entry._inst;
				pentry->_note = entry._note;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		// play it
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void DefaultPatternView::MidiPatternTweakSlide(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._cmd = (value>>8)&255;
	entry._parameter = value&255;
	entry._inst = command;
	entry._note = cdefTweakS;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if (pentry->_note >= 120)
		{
			if ((pentry->_mach != entry._mach) 
				|| (pentry->_cmd != entry._cmd)
				|| (pentry->_parameter != entry._parameter) 
				|| (pentry->_inst != entry._inst) 
				|| ((pentry->_note != cdefTweakM) && (pentry->_note != cdefTweakE) && (pentry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				pentry->_mach = entry._mach;
				pentry->_cmd = entry._cmd;
				pentry->_parameter = entry._parameter;
				pentry->_inst = entry._inst;
				pentry->_note = entry._note;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void DefaultPatternView::MidiPatternCommand(int command, int value)
{
	// UNDO CODE MIDI PATTERN
	if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xff) value = 0xff; // no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._inst = m_pSong->AuxcolSelected();
	entry._cmd = command;
	entry._parameter = value;
	entry._note = 255;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if ((pentry->_mach != entry._mach) 
			|| (pentry->_inst != entry._inst) 
			|| (pentry->_cmd != entry._cmd) 
			|| (pentry->_parameter != entry._parameter))
		{
			AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
			pentry->_mach = entry._mach;
			pentry->_cmd = entry._cmd;
			pentry->_parameter = entry._parameter;
			pentry->_inst = entry._inst;

			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(DMData);
		}
	}
//	else
	{
		// play it
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void DefaultPatternView::MidiPatternMidiCommand(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._cmd = (value>>8)&0xFF;
	entry._parameter = value&0xFF;
	entry._inst = command;
	entry._note = cdefMIDICC;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if (pentry->_note >= 120)
		{
			if ((pentry->_mach != entry._mach) 
				|| (pentry->_cmd != entry._cmd) 
				|| (pentry->_parameter != entry._parameter) 
				|| (pentry->_inst != entry._inst) 
				|| (pentry->_note != cdefMIDICC))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				pentry->_mach = entry._mach;
				pentry->_cmd = entry._cmd;
				pentry->_parameter = entry._parameter;
				pentry->_inst = entry._inst;
				pentry->_note = entry._note;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void DefaultPatternView::MidiPatternInstrument(int value)
{
	// UNDO CODE MIDI PATTERN
	if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xff) value = 0xff; // no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._inst = value;
	entry._cmd = 255;
	entry._parameter = 255;
	entry._note = 255;
	entry._volume = 0;
	entry._volcmd = 0;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if ((pentry->_mach != entry._mach) 
			|| (pentry->_inst != entry._inst))
		{
			AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
			pentry->_mach = entry._mach;
			pentry->_inst = entry._inst;

			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(DMData);
		}
	}
//	else
	{
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void DefaultPatternView::MousePatternTweak(int machine, int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset;
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{	
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			toffset = _ptrackline(ps);
			line = editcur.line;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if (entry->_note >= 120)
		{
			if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != cdefTweakM) && (entry->_note != cdefTweakE) && (entry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_mach = machine;
				entry->_cmd = (value>>8)&255;
				entry->_parameter = value&255;
				entry->_inst = command;
				entry->_note = cdefTweakM;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
}

void DefaultPatternView::MousePatternTweakSlide(int machine, int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow
	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset;
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{	
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			toffset = _ptrackline(ps);
			line = editcur.line;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if (entry->_note >= 120)
		{
			if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != cdefTweakM) && (entry->_note != cdefTweakE) && (entry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_mach = machine;
				entry->_cmd = (value>>8)&255;
				entry->_parameter = value&255;
				entry->_inst = command;
				entry->_note = cdefTweakS;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
}


void DefaultPatternView::EnterNote(int note, int velocity, bool bTranspose)
{
	int line;

	// UNDO CODE ENTER NOTE
	const int ps = _ps();
	unsigned char * toffset;
	
	if (note < 0 || note > cdefTweakS ) return;

	// octave offset
	if(note<120)
	{
		if(bTranspose)
			note+=m_pSong->CurrentOctave() * 12;

		if (note > 119) 
			note = 119;
	}
	
	// realtime note entering
	if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
	{
		if(m_pSong->TrackArmedCount())
		{
			if (velocity == 0)
			{
				for (int i = 0; i < m_pSong->SongTracks(); i++)
				{
					if (m_pSong->IsTrackArmed(i))
					{
						if (Global::pInputHandler->notetrack[i] == note)
						{
							editcur.track = i;
							break;
						}
					}
				}
				if (i == m_pSong->SongTracks())
				{
					Global::pInputHandler->StopNote(note,false);
					return;
				}
			}
			else
			{
				SelectNextTrack();
			}
		}
		else if (!Global::pConfig->_RecordUnarmed)
		{
			// build entry
			PatternEntry entry;
			entry._note = note;
			entry._mach = m_pSong->SeqBus();

			if ( note < 120)
			{
				if (Global::pConfig->_RecordTweaks && Global::pConfig->_midiRecordVel )
				{
					// command
					// entry._cmd = Global::pConfig->_midiCommandVel;
					int par = Global::pConfig->_midiFromVel + 
										(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * velocity)/127);
					if (par > 255) 
					{
						par = 255;
					}
					else if (par < 0) 
					{
						par = 0;
					}
					entry._volume = par;
					entry._volcmd = PatternCmd::VELOCITY;//CMD::VOLUME
				} else {
					int v = Global::pConfig->_midiFromVel + 
										(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * 64)/127);
					if (v > 255) 
					{
						v = 255;
					}
					else if (v < 0) 
					{
						v = 0;
					}
					entry._volume = v;
					entry._volcmd = PatternCmd::VELOCITY;//CMD::VOLUME
				}
			}

			if (note>120)
			{
				entry._inst = m_pSong->AuxcolSelected();
			}

			Machine *tmac = m_pSong->pMachine(m_pSong->SeqBus());
			if (tmac)
			{
				if (tmac->_type == MACH_SAMPLER)
				{
					entry._inst = m_pSong->AuxcolSelected();
				}
				else if (tmac->_type == MACH_VST) // entry->_inst is the MIDI channel for VSTi's
				{
					entry._inst = m_pSong->AuxcolSelected();
				}
				
				if ( note < 120)
				{
					tmac->Tick(editcur.track, &entry);
				}
			}
			Global::pInputHandler->notetrack[editcur.track]=note;
			return;
		}
		line = Global::pPlayer->_lineCounter;
		toffset = _ptrack(ps)+(line*MULTIPLY);
		ChordModeOffs = 0;
	}
	else 
	{
		if ((GetKeyState(VK_SHIFT)<0) && (note != cdefTweakM) && (note != cdefTweakE) && (note != cdefTweakS) && (note != cdefMIDICC))
		{
			if (ChordModeOffs == 0)
			{
				m_ChordModeLine = editcur.line;
				m_ChordModeTrack = editcur.track;
			}
			editcur.track = (m_ChordModeTrack+ChordModeOffs)%m_pSong->SongTracks();
			editcur.line = line = m_ChordModeLine;
			toffset = _ptrackline(ps, editcur.track, line);
			ChordModeOffs++;
		}
		else
		{
			if (ChordModeOffs) // this should never happen because the shift check should catch it... but..
			{					// ok pooplog, now it REALLY shouldn't happen (now that the shift check works)
				editcur.line = m_ChordModeLine;
				editcur.track = m_ChordModeTrack;
				ChordModeOffs = 0;
				AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			}
			line = editcur.line;
			toffset = _ptrackline(ps);
		}
	}

	// build entry
	PatternEntry *entry = (PatternEntry*) toffset;
	if (velocity==0)
	{
		Global::pInputHandler->StopNote(note,false);
		if (entry->_note == note)
		{
			return;
		}
		note = 120;
	}
	AddUndo(ps,editcur.track,line,1,1,editcur.track,line,editcur.col,editPosition);
	entry->_note = note;
	entry->_mach = m_pSong->SeqBus();

	if ( note < 120)
	{
		if (Global::pConfig->_RecordTweaks && Global::pConfig->_midiRecordVel)
		{
			// command
			//entry->_cmd = Global::pConfig->_midiCommandVel;
			int par = Global::pConfig->_midiFromVel + 
								(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * velocity)/127);
			if (par > 255) 
			{
				par = 255;
			}
			else if (par < 0) 
			{
				par = 0;
			}
			//entry->_parameter = par;
			entry->_volume = par;
			entry->_volcmd = PatternCmd::VELOCITY;
		} else {
			int v = Global::pConfig->_midiFromVel + 
								(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * 64)/127);
			if (v > 255) 
			{
				v = 255;
			}
			else if (v < 0) 
			{
				v = 0;
			}
			entry->_volume = v;
			entry->_volcmd = PatternCmd::VELOCITY;
		}
	}

	if (note>120)
	{
		entry->_inst = m_pSong->AuxcolSelected();
	}

	Machine *tmac = m_pSong->pMachine(m_pSong->SeqBus());
	if (tmac)
	{
		if (tmac->_type == MACH_SAMPLER)
		{
			entry->_inst = m_pSong->AuxcolSelected();
		}
		else if (tmac->_type == MACH_VST) // entry->_inst is the MIDI channel for VSTi's
		{
			entry->_inst = m_pSong->AuxcolSelected();
		}
		
		if ( note < 120)
		{
			tmac->Tick(editcur.track, entry);
		}
	}

	Global::pInputHandler->notetrack[editcur.track]=note;
	NewPatternDraw(editcur.track,editcur.track,line,line);
	if (!(Global::pPlayer->_playing&&Global::pConfig->_followSong))
	{
		if (ChordModeOffs)
		{
			AdvanceLine(-1,Global::pConfig->_wrapAround,false);
		}
		else
		{
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
		}
	}

	bScrollDetatch=false;
	Global::pInputHandler->bDoingSelection = false;
	Repaint(DMData);
}

void DefaultPatternView::EnterNoteoffAny()
{
	if (viewMode == VMPattern)
	{
		// UNDO CODE ENTER NOTE
		const int ps = _ps();
		unsigned char * toffset;
		
		// realtime note entering
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			toffset = _ptrack(ps)+(Global::pPlayer->_lineCounter*MULTIPLY);
		}
		else
		{
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
		entry->_note = 120;

		Global::pInputHandler->notetrack[editcur.track]=120;

		NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

		if (!(Global::pPlayer->_playing&&Global::pConfig->_followSong))
		{
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
		}

		bScrollDetatch=false;
		Global::pInputHandler->bDoingSelection = false;
		Repaint(DMData);
	}
}

bool DefaultPatternView::MSBPut(int nChar)
{
	// UNDO CODE MSB PUT
	// init
	const int ps = _ps();
	unsigned char * toffset = _ptrackline(ps) + (editcur.col+1)/2;

	int oldValue = *toffset;	
	int sValue = -1;

	if	(	nChar>='0'		&&	nChar<='9')			{ sValue = nChar - '0'; }
	else if(nChar>=VK_NUMPAD0&&nChar<=VK_NUMPAD9)	{ sValue = nChar - VK_NUMPAD0; }
	else if(nChar>='A'		&&	nChar<='F')			{ sValue = nChar - 'A' + 10; }
	else											{ return false; }

	if (editcur.col < 5 && oldValue == 255)	{ oldValue = 0; }

	AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

	switch ((editcur.col+1)%2)
	{
	case 0:	
		*toffset = (oldValue&0xF)+(sValue<<4); 
		break;
	
	case 1:	
		*toffset = (oldValue&0xF0)+(sValue); 
		break;
	}

	if (Global::pConfig->_cursorAlwaysDown)
	{
		AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
	}
	else
	{
		switch (editcur.col)
		{
		case 0:
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			break;
		case 1:
		case 3:
		case 5:
		case 6:
		case 7:
		case 9:
		case 11:
		case 10:
			NextCol(false,false);
			break;
		case 8:
		case 12:
			PrevCol(false,false);
			PrevCol(false,false);
		case 2:
		case 4:
			PrevCol(false,false);
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			break;
		}
	}
	bScrollDetatch=false;
	Global::pInputHandler->bDoingSelection = false;
	NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
	Repaint(DMData);
	return true;
}

void DefaultPatternView::ClearCurr() // delete content at Cursor pos.
{
	// UNDO CODE CLEAR
	const int ps = _ps();
	unsigned char * offset = _ptrack(ps);
	unsigned char * toffset = _ptrackline(ps);

	AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

	// &&&&& hardcoded # of bytes per event
	if ( editcur.col == 0 )
	{
		memset(offset+(editcur.line*MULTIPLY),255,3*sizeof(char));
		memset(offset+(editcur.line*MULTIPLY)+3,0,3*sizeof(char));
	}
	else if (editcur.col < 5 )	{	*(toffset+(editcur.col+1)/2)= 255; }
	else						{	*(toffset+(editcur.col+1)/2)= 0; }

	NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

	AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}

void DefaultPatternView::DeleteCurr()
{
	// UNDO CODE DELETE
	const int ps = _ps();
	unsigned char * offset = _ptrack(ps);
	int patlines = m_pSong->PatternLines(ps);

	if ( Global::pInputHandler->bFT2DelBehaviour )
	{
		if(editcur.line==0)
			return;
		else
			editcur.line--;
	}

	AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

	for (int i=editcur.line; i < patlines-1; i++)
		memcpy(offset+(i*MULTIPLY), offset+((i+1)*MULTIPLY), EVENT_SIZE);

	//unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),BLANK_EVENT,EVENT_SIZE);

	NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}

void DefaultPatternView::InsertCurr()
{
	// UNDO CODE INSERT
	const int ps = _ps();
	unsigned char * offset = _ptrack(ps);
	int patlines = m_pSong->PatternLines(ps);

	AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

	for (int i=patlines-1; i > editcur.line; i--)
		memcpy(offset+(i*MULTIPLY), offset+((i-1)*MULTIPLY), EVENT_SIZE);

	//unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),BLANK_EVENT,EVENT_SIZE * sizeof(char));

	NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}



//////////////////////////////////////////////////////////////////////
// Plays entire row of the pattern

void DefaultPatternView::PlayCurrentRow(void)
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	PatternEntry* pEntry = (PatternEntry*)_ptrackline(_ps(),0,editcur.line);

	for (int i = 0; i < m_pSong->SongTracks();i++)
	{
		if (pEntry->_mach < MAX_MACHINES && !m_pSong->IsTrackMuted(i))
		{
			Machine *pMachine = m_pSong->pMachine(pEntry->_mach);
			if (pMachine)
			{
				if ( !pMachine->_mute)	
				{
					pMachine->Tick(i, pEntry);
				}
			}
		}
		pEntry++;
	}
}

void DefaultPatternView::PlayCurrentNote(void)
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}

	PatternEntry* pEntry = (PatternEntry*)_ptrackline();
	if (pEntry->_mach < MAX_MACHINES)
	{
		Machine *pMachine = m_pSong->pMachine(pEntry->_mach);
		if (pMachine)
		{
			if ( !pMachine->_mute)	
			{
				pMachine->Tick(editcur.track, pEntry);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Cursor Moving Functions
void DefaultPatternView::PrevCol(bool wrap,bool updateDisplay)
{
	if(--editcur.col < EDIT_COLUMNS_START)
	{
		editcur.col = EDIT_COLUMNS_END;
		m_pMainFrame->StatusBarIdle();
		if (editcur.track == 0)
		{
			if ( wrap ) 
				editcur.track = m_pSong->SongTracks() - 1;
			else 
				editcur.col = EDIT_COLUMNS_START;
		}
		else 
			--editcur.track;
	}
	if (updateDisplay) 
	{
		Repaint(DMCursor);
	}
}

void DefaultPatternView::NextCol(bool wrap,bool updateDisplay)
{
	if (++editcur.col > EDIT_COLUMNS_END)
	{
		editcur.col = 0;
		m_pMainFrame->StatusBarIdle();
		if (editcur.track == m_pSong->SongTracks() - 1)
		{
			if ( wrap ) 
				editcur.track = 0;
			else 
				editcur.col=EDIT_COLUMNS_END;
		}
		else 
			++editcur.track;
	}
	if (updateDisplay) 
	{
		Repaint(DMCursor);
	}
}

void DefaultPatternView::PrevLine(int x, bool wrap,bool updateDisplay)
{
	const int nl = m_pSong->PatternLines(_ps());

	editcur.line -= x;

	if(editcur.line<EDIT_COLUMNS_START)
	{
		if(wrap)
		{ 
			editcur.line = nl + editcur.line % nl; 
		}
		else	
		{ 
			editcur.line = EDIT_COLUMNS_START;	
		}
	}
	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void DefaultPatternView::AdvanceLine(int x,bool wrap,bool updateDisplay)
{
	const int nl = m_pSong->PatternLines(_ps());

	if ( x >= 0)	
	{
		editcur.line += x;
	}
	else
	{
		editcur.track+=1;
		if (editcur.track >= m_pSong->SongTracks())
		{
			editcur.track=0;
			editcur.line+=1;
		}
	}

	if (editcur.line >= nl)
	{
		if(wrap){ editcur.line = editcur.line % nl; }
		else	{ editcur.line = nl-1; }
	}

	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void DefaultPatternView::AdvanceTrack(int x,bool wrap,bool updateDisplay)
{
	editcur.track+=x;
	editcur.col=0;
	
	if(editcur.track>= m_pSong->SongTracks())
	{
		if ( wrap ) editcur.track=0;
		else editcur.track = m_pSong->SongTracks() - 1;
	}
	
	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void DefaultPatternView::PrevTrack(int x,bool wrap,bool updateDisplay)
{
	editcur.track-=x;
	editcur.col=0;
	
	if(editcur.track<0)
	{
		if (wrap) editcur.track=m_pSong->SongTracks() - 1;
		else editcur.track=0;
	}
	
	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}


//////////////////////////////////////////////////////////////////////
// Pattern Modifier functions ( Copy&paste , Transpose, ... )

void DefaultPatternView::patCut()
{
	if(viewMode == VMPattern)
	{
		// UNDO CODE PATT CUT
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
		//unsigned char blank[5]={255,255,255,0,0};

		m_PatBufferLines = m_pSong->PatternLines(ps);
		AddUndo(ps,0,0,MAX_TRACKS,m_PatBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

		int length = m_PatBufferLines*EVENT_SIZE*MAX_TRACKS;
		
		memcpy(m_PatBufferData,soffset,length);
		for	(int c=0; c<length; c+=EVENT_SIZE)
		{
			memcpy(soffset,BLANK_EVENT,EVENT_SIZE);
			soffset+=EVENT_SIZE;
		}
		m_PatBufferCopy = true;

		NewPatternDraw(0,m_pSong->SongTracks(),0,m_PatBufferLines - 1);
		Repaint(DMData);
	}
}

void DefaultPatternView::patCopy()
{
	if(viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
		
		m_PatBufferLines = m_pSong->PatternLines(ps);
		int length = m_PatBufferLines * EVENT_SIZE * MAX_TRACKS;
		
		memcpy(m_PatBufferData,soffset,length);
		
		m_PatBufferCopy = true;
	}
}

void DefaultPatternView::patPaste()
{
	// UNDO CODE PATT PASTE
	if(m_PatBufferCopy && viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
		// **************** funky shit goin on here yo with the pattern resize or some shit
		AddUndo(ps,0,0,MAX_TRACKS,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
		if ( m_PatBufferLines != m_pSong->PatternLines(ps) )
		{
			AddUndoLength(ps,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
			m_pSong->AllocNewPattern(ps,_T(""),m_PatBufferLines,false);
		}
		
		memcpy(soffset,m_PatBufferData,m_PatBufferLines * EVENT_SIZE * MAX_TRACKS);
		Repaint(DMPattern);
	}
}

void DefaultPatternView::patMixPaste()
{
	// UNDO CODE PATT PASTE
	if(m_PatBufferCopy && viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char* offset_target = _ppattern(ps);
		unsigned char* offset_source = m_PatBufferData;
		// **************** funky shit goin on here yo with the pattern resize or some shit
		AddUndo(ps,0,0,MAX_TRACKS,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
		if ( m_PatBufferLines != m_pSong->PatternLines(ps) )
		{
			AddUndoLength(ps,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
			m_pSong->AllocNewPattern(ps,_T(""),m_PatBufferLines,false);
		}

		for (int i = 0; i < MAX_TRACKS*m_PatBufferLines; i++)
		{
			if (*offset_target == 0xFF) *(offset_target)=*offset_source;
			if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
			if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
			if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
			if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
			if (*(offset_target+5)== 0) *(offset_target+5)=*(offset_source+5);
			if (*(offset_target+6)== 0) *(offset_target+6)=*(offset_source+6);
			offset_target+= EVENT_SIZE;
			offset_source+= EVENT_SIZE;
		}
		
		Repaint(DMPattern);
	}
}

void DefaultPatternView::patDelete()
{
	if(viewMode == VMPattern)
	{
		// UNDO CODE PATT CUT
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
//		unsigned char blank[5]={255,255,255,0,0};

		m_PatBufferLines = m_pSong->PatternLines(ps);
		AddUndo(ps,0,0,MAX_TRACKS,m_PatBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

		int length = m_PatBufferLines*EVENT_SIZE*MAX_TRACKS;
		
		for	(int c=0; c<length; c+=EVENT_SIZE)
		{
			memcpy(soffset,BLANK_EVENT,EVENT_SIZE);
			soffset+=EVENT_SIZE;
		}

		NewPatternDraw(0,m_pSong->SongTracks(),0,m_PatBufferLines-1);
		Repaint(DMData);
	}
}

void DefaultPatternView::patTranspose(int trp)
{
	// UNDO CODE PATT TRANSPOSE
	const int ps = _ps();
	unsigned char *soffset = _ppattern(ps);

	if(viewMode == VMPattern)
	{
		int pLines = m_pSong->PatternLines(ps);
		int length = pLines * EVENT_SIZE * MAX_TRACKS;

		AddUndo(ps,0,0,MAX_TRACKS,pLines,editcur.track,editcur.line,editcur.col,editPosition);

		for	(int c=editcur.line*EVENT_SIZE*MAX_TRACKS;c<length;c+=EVENT_SIZE)
		{
			int not=*(soffset+c);
			
			if(not<120)
			{
				not+=trp;
				if(not<0)not=0;
				if(not>119)not=119;
				soffset[c]=static_cast<unsigned char>(not);
			}
		}
		NewPatternDraw(0,m_pSong->SongTracks(),editcur.line,pLines-1);

		Repaint(DMData);
	}
}

void DefaultPatternView::StartBlock(int track,int line, int col)
{
	m_BlockSelection.start.track=track;
	m_BlockSelection.start.line=line;
	m_IniSelection = m_BlockSelection.start;

	if ( blockSelected )
	{
		if(m_BlockSelection.end.line<m_BlockSelection.start.line)
			m_BlockSelection.end.line=m_BlockSelection.start.line;
		
		if(m_BlockSelection.end.track<m_BlockSelection.start.track)
			m_BlockSelection.end.track=m_BlockSelection.start.track;
	}
	else
	{
		m_BlockSelection.end.line=line;
		m_BlockSelection.end.track=track;
	}
	blockSelected=true;

	Repaint(DMSelection);
}
void DefaultPatternView::ChangeBlock(int track,int line, int col)
{
	if ( blockSelected )
	{
		if ( track > m_IniSelection.track )
		{
			m_BlockSelection.start.track = m_IniSelection.track;
			m_BlockSelection.end.track = track;
		}
		else
		{
			m_BlockSelection.start.track = track;
			m_BlockSelection.end.track = m_IniSelection.track;
		}
		if ( line > m_IniSelection.line )
		{
			m_BlockSelection.start.line = m_IniSelection.line;
			m_BlockSelection.end.line = line;
		}
		else
		{
			m_BlockSelection.start.line = line;
			m_BlockSelection.end.line = m_IniSelection.line;
		}
	}
	else
	{
		m_BlockSelection.start.track=track;
		m_BlockSelection.start.line=line;
		m_BlockSelection.end.track=track;
		m_BlockSelection.end.line=line;
		m_IniSelection = m_BlockSelection.start;
	}
	blockSelected=true;

	Repaint(DMSelection);
}

void DefaultPatternView::EndBlock(int track,int line, int col)
{
	m_BlockSelection.end.track=track;
	m_BlockSelection.end.line=line;
	
	if ( blockSelected )
	{
		ATLTRACE(_T("%i,%i"),m_BlockSelection.end.line,m_BlockSelection.start.line);
		if(m_BlockSelection.end.line<m_BlockSelection.start.line)
		{
			int tmp = m_BlockSelection.start.line;
			m_BlockSelection.start.line=m_BlockSelection.end.line;
			m_BlockSelection.end.line=tmp;
//			m_BlockSelection.end.line=m_BlockSelection.start.line;
		}
		
		ATLTRACE(_T("%i,%i"),m_BlockSelection.end.track,m_BlockSelection.start.track);
		if(m_BlockSelection.end.track<m_BlockSelection.start.track)
		{
			int tmp = m_BlockSelection.start.track;
			m_BlockSelection.start.track=m_BlockSelection.end.track;
			m_BlockSelection.end.track=tmp;
//			m_BlockSelection.end.track=m_BlockSelection.start.track;
		}
		
	}
	else
	{
		m_BlockSelection.start.track=track;
		m_BlockSelection.start.line=line;
		m_IniSelection = m_BlockSelection.start;
	}
	blockSelected=true;

	Repaint(DMSelection);
}

void DefaultPatternView::BlockUnmark()
{
	blockSelected=false;

	Repaint(DMSelection);
}

void DefaultPatternView::CopyBlock(bool cutit)
{
	// UNDO CODE HERE CUT
	if(blockSelected)
	{
		isBlockCopied=true;
		blockNTracks=(m_BlockSelection.end.track-m_BlockSelection.start.track)+1;
		blockNLines=(m_BlockSelection.end.line-m_BlockSelection.start.line)+1;
		
		int ps=m_pSong->PlayOrder(editPosition);
		
		int ls=0;
		int ts=0;
		//unsigned char blank[5]={255,255,255,0,0};

		if (cutit)
		{
			AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
		}
		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			ls=0;
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *offset_target=m_BlockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);				
				unsigned char *offset_source=_ptrackline(ps,t,l);
				
				memcpy(offset_target,offset_source,EVENT_SIZE);
				
				if(cutit)
					memcpy(offset_source,BLANK_EVENT,EVENT_SIZE);
				
				++ls;
			}
			++ts;
		}
		if(cutit)
		{
			NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);

			Repaint(DMData);
		}
	}
}

void DefaultPatternView::DeleteBlock()
{
	// UNDO CODE HERE CUT
	if(blockSelected)
	{
		int ps=m_pSong->PlayOrder(editPosition);
		
		//unsigned char blank[5]={255,255,255,0,0};

		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				memcpy(_ptrackline(ps,t,l),BLANK_EVENT,EVENT_SIZE);
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}

void DefaultPatternView::PasteBlock(int tx,int lx,bool mix)
{
	// UNDO CODE PASTE AND MIX PASTE
	if(isBlockCopied)
	{
		int ps=m_pSong->PlayOrder(editPosition);
		const int nl = m_pSong->PatternLines(ps);

		AddUndo(ps,tx,lx,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);

		int ls=0;
		int ts=0;
		
		for (int t=tx;t<tx+blockNTracks;t++)
		{
			ls=0;
			for (int l=lx;l<lx+blockNLines;l++)
			{
				if(l<nl && t<m_pSong->SongTracks())
				{
					unsigned char* offset_source=m_BlockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);
					unsigned char* offset_target=_ptrackline(ps,t,l);
					if ( mix )
					{
						if (*offset_target == 0xFF) *(offset_target)=*offset_source;
						if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
						if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
						if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
						if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
						if (*(offset_target+5)== 0) *(offset_target+5)=*(offset_source+5);
						if (*(offset_target+6)== 0) *(offset_target+6)=*(offset_source+6);

					}
					else
					{
						memcpy(offset_target,offset_source,EVENT_SIZE);
					}
				}
				++ls;
			}
			++ts;
		}
		
		if (lx+blockNLines < nl ) editcur.line = lx+blockNLines;
		else editcur.line = nl-1;

		bScrollDetatch=false;
		NewPatternDraw(tx,tx+blockNTracks-1,lx,lx+blockNLines-1);
		Repaint(DMData);
	}
	
}

void DefaultPatternView::SaveBlock(FILE* file)
{

	int ps = _ps();
	int nlines = m_pSong->PatternLines(ps);
	int _songTracks = m_pSong->SongTracks();
	fwrite(&_songTracks, sizeof(int), 1, file);
	fwrite(&nlines, sizeof(int), 1, file);

	for (int t=0;t<m_pSong->SongTracks();t++)
	{
		for (int l=0;l<nlines;l++)
		{
			unsigned char* offset_source=_ptrackline(ps,t,l);
			
			fwrite(offset_source,sizeof(char),EVENT_SIZE,file);
		}
	}
}

void DefaultPatternView::LoadBlock(FILE* file)
{
	int nt, nl;
	fread(&nt,sizeof(int),1,file);
	fread(&nl,sizeof(int),1,file);

	if ((nt > 0) && (nl > 0))
	{

		int ps = _ps();
		int nlines = m_pSong->PatternLines(ps);
		AddUndo(ps,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
		if (nlines != nl)
		{
			AddUndoLength(ps,nlines,editcur.track,editcur.line,editcur.col,editPosition);
			m_pSong->PatternLines(ps,nl);
		}

		for (int t=0;t<nt;t++)
		{
			for (int l=0;l<nl;l++)
			{
				if(l<MAX_LINES && t<MAX_TRACKS)
				{
					unsigned char* offset_target=_ptrackline(ps,t,l);
					fread(offset_target,sizeof(char),EVENT_SIZE,file);
				}
			}
		}
		//unsigned char blank[5]={255,255,255,0,0};

		for (t = nt; t < MAX_TRACKS;t++)
		{
			for (int l = nl; l < MAX_LINES; l++)
			{
				unsigned char* offset_target=_ptrackline(ps,t,l);
				memcpy(offset_target,BLANK_EVENT,EVENT_SIZE);
			}
		}
		Repaint(DMPattern);
	}
}

void DefaultPatternView::DoubleLength()
{
	// UNDO CODE DOUBLE LENGTH
	unsigned char *toffset;
	//unsigned char blank[5]={255,255,255,0,0};
	int st, et, sl, el,nl;

	int ps = _ps();
	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=m_BlockSelection.start.track;		
		et=m_BlockSelection.end.track+1;
		sl=m_BlockSelection.start.line;			
		nl=((m_BlockSelection.end.line-sl)/2)+1;
		el=m_BlockSelection.end.line;
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,nl*2,editcur.track,editcur.line,editcur.col,editPosition);
	}
	else 
	{
		st = 0;		
		et = m_pSong->SongTracks();		
		sl = 0;
		nl = m_pSong->PatternLines(ps) / 2;	
		el = m_pSong->PatternLines(ps) - 1;
		AddUndo(ps,0,0,MAX_TRACKS,el+1,editcur.track,editcur.line,editcur.col,editPosition);
	}

	for (int t=st;t<et;t++)
	{
		toffset=_ptrack(ps,t);
		memcpy(toffset+el*MULTIPLY,BLANK_EVENT,EVENT_SIZE);
		for (int l=nl-1;l>0;l--)
		{
			memcpy(toffset+(sl+l*2)*MULTIPLY,toffset+(sl+l)*MULTIPLY,EVENT_SIZE);
			memcpy(toffset+(sl+(l*2)-1)*MULTIPLY,BLANK_EVENT,EVENT_SIZE);
		}
	}

	NewPatternDraw(st,et,sl,el);
	Repaint(DMData);
}

void DefaultPatternView::HalveLength()
{
	// UNDO CODE HALF LENGTH
	unsigned char *toffset;
	int st, et, sl, el,nl;
	int ps = _ps();
	//unsigned char blank[5]={255,255,255,0,0};

	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=m_BlockSelection.start.track;	
		et=m_BlockSelection.end.track+1;
		sl=m_BlockSelection.start.line;		
		nl=m_BlockSelection.end.line-sl+1;
		el=nl/2;
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,nl,editcur.track,editcur.line,editcur.col,editPosition);
	}
	else 
	{
		st=0;	
		et=m_pSong->SongTracks();		
		sl=0;
		nl=m_pSong->PatternLines(ps);	
		el=m_pSong->PatternLines(ps) / 2;
		AddUndo(ps,0,0,MAX_TRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition);
	}
	
	for (int t=st;t<et;t++)
	{
		toffset=_ptrack(ps,t);
		for (int l=1;l<el;l++)
		{
			memcpy(toffset+(l+sl)*MULTIPLY,toffset+((l*2)+sl)*MULTIPLY,EVENT_SIZE);
		}
		while (l < nl)
		{
			memcpy(toffset+((l+sl)*MULTIPLY),BLANK_EVENT,EVENT_SIZE);
			l++;
		}
	}

	NewPatternDraw(st,et,sl,nl+sl);
	Repaint(DMData);
}


void DefaultPatternView::BlockTranspose(int trp)
{
	// UNDO CODE TRANSPOSE
	if ( blockSelected == true ) 
	{
		int ps = _ps();

		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *toffset=_ptrackline(ps,t,l);
				
				int not=*(toffset);
			
				if(not<120)
				{
					not+=trp;
					if(not<0)not=0;
					if(not>119)not=119;
					*toffset=static_cast<unsigned char>(not);
				}
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}


void DefaultPatternView::BlockGenChange(int x)
{
	// UNDO CODE BLOCK GENERATOR CHANGE
	if ( blockSelected == true ) 
	{
		int ps = _ps();
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *toffset=_ptrackline(ps,t,l)+2;
				
				unsigned char gen=*(toffset);
				
				if ( gen != 255 )
				{
					gen=x;
					if(gen<0)gen=0;
					if(gen>=MAX_MACHINES)gen=MAX_MACHINES-1;
					*toffset=gen;
				}
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}

void DefaultPatternView::BlockInsChange(int x)
{
	// UNDO CODE BLOCK INS CHANGE
	if ( blockSelected == true ) 
	{
		const int ps=_ps();

		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *toffset=_ptrackline(ps,t,l)+1;
				unsigned char ins=*(toffset);
			
				if (ins != 255 )
				{
					ins=x;
					if(ins<0)ins=0;
					if(ins>255)ins=255;
					*toffset=ins;
				}
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}

void DefaultPatternView::BlockParamInterpolate()
{
	// UNDO CODE BLOCK INTERPOLATE
	if (blockSelected)
	{
		const int ps = _ps();
		///////////////////////////////////////////////////////// Add ROW
		unsigned char *toffset=_ppattern(ps);
		
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);
		
		const int initvalue = 
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.start.line*MULTIPLY+3) * 0x100 +
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.start.line*MULTIPLY+4);
		const int endvalue =
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.end.line*MULTIPLY+3) * 0x100 +
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.end.line*MULTIPLY+4);
		const float addvalue = (float)(endvalue -initvalue)/(m_BlockSelection.end.line-m_BlockSelection.start.line);
		const unsigned char comd = *(toffset+m_BlockSelection.start.track * EVENT_SIZE + m_BlockSelection.start.line*MULTIPLY+3);
		int displace2=(m_BlockSelection.start.track*EVENT_SIZE)+((m_BlockSelection.start.line+1)*MULTIPLY);
		
		if ( toffset[displace2] == cdefTweakM || toffset[displace2] == cdefTweakE || toffset[displace2] == cdefTweakS)
		{
			unsigned char note = toffset[displace2];
			unsigned char aux = toffset[displace2+1];
			unsigned char mac = toffset[displace2+2];
			unsigned char vel = toffset[displace2+6];
			unsigned char volcmd = toffset[displace2+5];


			for (int l=m_BlockSelection.start.line+1;l<m_BlockSelection.end.line;l++)
			{
				toffset[displace2]=note;
				toffset[displace2+1]=aux;
				toffset[displace2+2]=mac;
				int val=f2i(initvalue+addvalue*(l-m_BlockSelection.start.line));
				toffset[displace2+3]=static_cast<unsigned char>(val/0x100);
				toffset[displace2+4]=static_cast<unsigned char>(val%0x100);
				toffset[displace2+5]=volcmd;
				toffset[displace2+6]=vel;
				displace2+=MULTIPLY;
			}
			toffset[displace2]=note;
			toffset[displace2+1]=aux;
			toffset[displace2+2]=mac;
			toffset[displace2+6]=vel;
			toffset[displace2+5]=volcmd;
		}
		else
		{
			for (int l=m_BlockSelection.start.line+1;l<m_BlockSelection.end.line;l++)
			{
				int val=f2i(initvalue+addvalue*(l-m_BlockSelection.start.line));
				toffset[displace2+3]=static_cast<unsigned char>(val/0x100);
				toffset[displace2+4]=static_cast<unsigned char>(val%0x100);
				displace2+=MULTIPLY;
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}


void DefaultPatternView::IncCurPattern()
{
	if(m_pSong->PlayOrder(editPosition) < (MAX_PATTERNS-1))
	{
		AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition);
		m_pSong->PlayOrder(m_pSong->PlayOrder(editPosition) + 1);
		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}


void DefaultPatternView::DecCurPattern()
{
	if(m_pSong->PlayOrder(editPosition) > 0)
	{
		AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition);
		m_pSong->PlayOrder(editPosition,m_pSong->PlayOrder(editPosition) - 1);
		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void DefaultPatternView::DecPosition()
{
//	case cdefPlaySkipBack:
	if (Global::pPlayer->_playing && Global::pConfig->_followSong)
	{
		if (Global::pPlayer->_playPosition > 0 )
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(Global::pPlayer->_playPosition-1,0);
			Global::pPlayer->_playBlock = b;
		}
		else
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(m_pSong->PlayLength() - 1,0);
			Global::pPlayer->_playBlock = b;
		}
	}
	else
	{
		if(editPosition>0)
		{
			--editPosition;
		}
		else
		{
//			editPosition = m_pSong->playLength-1;
			editPosition = 0;
		}
		
		for(int i = 0;i < MAX_SONG_POSITIONS;i++)
		{
			m_pSong->PlayOrderSel(editPosition,false);
		}
		//memset(m_pSong->PlayOrderSel(),0,MAX_SONG_POSITIONS*sizeof(bool));
		
		m_pSong->PlayOrderSel(editPosition,true);

		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void DefaultPatternView::IncPosition(bool bRepeat)
{
//	case cdefPlaySkipAhead:
	if (Global::pPlayer->_playing && Global::pConfig->_followSong)
	{
		if (Global::pPlayer->_playPosition < m_pSong->PlayLength() - 1)
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(Global::pPlayer->_playPosition+1,0);
			Global::pPlayer->_playBlock = b;
		}
		else
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(0,0);
			Global::pPlayer->_playBlock = b;
		}
	}
	else 
	{
		if(editPosition < m_pSong->PlayLength() - 1)
		{
			++editPosition;
		}
		else if (!bRepeat) // This prevents adding patterns when only trying to reach the end.
		{
//			editPosition = 0;
			++editPosition;
			AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition-1);
			int const ep=m_pSong->GetBlankPatternUnused();
			m_pSong->PlayLength(editPosition + 1);
			m_pSong->PlayOrder(editPosition,ep);
			if (m_pSong->PatternLines(editPosition) != Global::pConfig->defaultPatLines)
			{
				AddUndoLength(editPosition,m_pSong->PatternLines(editPosition),editcur.track,editcur.line,editcur.col,editPosition);
				m_pSong->PatternLines(editPosition,Global::pConfig->defaultPatLines);
			}
			
			m_pMainFrame->UpdateSequencer();
		}
		
		for(int i = 0;i < MAX_SONG_POSITIONS;i++)
		{
			m_pSong->PlayOrderSel(i,0);
		}
		//memset(m_pSong->m_PlayOrderSel,0,MAX_SONG_POSITIONS * sizeof(bool));
		m_pSong->PlayOrderSel(editPosition,true);

		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void DefaultPatternView::SelectMachineUnderCursor()
{
	unsigned char *toffset=_ptrackline();

	PatternEntry *entry = (PatternEntry*) toffset;

	if ( entry->_mach < MAX_BUSES*2 ) m_pSong->SeqBus(entry->_mach);
	m_pMainFrame->ChangeGen(m_pSong->SeqBus());
	if ( entry->_inst != 255 ) m_pSong->AuxcolSelected(entry->_inst);
	m_pMainFrame->ChangeIns(m_pSong->AuxcolSelected());

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// undo/redo code
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DefaultPatternView::AddMacViewUndo()
{
	// i have not written the undo code yet for machine and instruments
	// however, for now it at least tracks changes for save/new/open/close warnings
	UndoMacCounter++;
	SetTitleBarText();
}

void DefaultPatternView::AddUndo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;

	// fill data
	unsigned char* pData = new unsigned char[tracks*lines*EVENT_SIZE*sizeof(char)];
	pNew->pData = pData;
	pNew->pattern = pattern;
	pNew->x = x;
	pNew->y = y;
	if (tracks+x > m_pSong->SongTracks())
	{
		tracks = m_pSong->SongTracks() - x;
	}
	pNew->tracks = tracks;
	const int nl = m_pSong->PatternLines(pattern);
	if (lines+y > nl)
	{
		lines = nl-y;
	}
	pNew->lines = lines;
	pNew->type = UNDO_PATTERN;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	for (int t=x;t<x+tracks;t++)
	{
		for (int l=y;l<y+lines;l++)
		{
			unsigned char *offset_source=_ptrackline(pattern,t,l);
			
			memcpy(pData,offset_source,EVENT_SIZE);
			pData+=EVENT_SIZE;
		}
	}
	if (bWipeRedo)
	{
		KillRedo();
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void DefaultPatternView::AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	unsigned char* pData = new unsigned char[tracks*lines*EVENT_SIZE*sizeof(char)];
	pNew->pData = pData;
	pNew->pattern = pattern;
	pNew->x = x;
	pNew->y = y;
	if (tracks+x > m_pSong->SongTracks())
	{
		tracks = m_pSong->SongTracks() - x;
	}
	pNew->tracks = tracks;
	const int nl = m_pSong->PatternLines(pattern);
	if (lines+y > nl)
	{
		lines = nl-y;
	}
	pNew->tracks = tracks;
	pNew->lines = lines;
	pNew->type = UNDO_PATTERN;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;

	for (int t=x;t<x+tracks;t++)
	{
		for (int l=y;l<y+lines;l++)
		{
			unsigned char *offset_source=_ptrackline(pattern,t,l);
			
			memcpy(pData,offset_source,EVENT_SIZE);
			pData+=EVENT_SIZE;
		}
	}
}

void DefaultPatternView::AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	pNew->pData = NULL;
	pNew->pattern = pattern;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_LENGTH;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void DefaultPatternView::AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	pNew->pData = NULL;
	pNew->pattern = pattern;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_LENGTH;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void DefaultPatternView::AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS];
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pNew->pData + i)  = m_pSong->PlayOrder(i);
	}
	//memcpy(pNew->pData, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_SEQUENCE;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void DefaultPatternView::AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	// count used patterns
	unsigned char count = 0;
	for (unsigned char i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			count++;
		}
	}
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS+sizeof(count)+MAX_PATTERNS+count*MULTIPLY2];
	unsigned char *pWrite=pNew->pData;
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pWrite + i) = m_pSong->PlayOrder(i);
	}
	//memcpy(pWrite, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	pWrite += MAX_SONG_POSITIONS * sizeof(char);

	memcpy(pWrite, &count, sizeof(count));
	
	pWrite += sizeof(count);

	for (i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			memcpy(pWrite, &i, sizeof(i));
			pWrite += sizeof(i);
			
			memcpy(pWrite, m_pSong->pPatternData(i), MULTIPLY2);
			pWrite += MULTIPLY2;
		}
	}

	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = m_pSong->PlayLength();
	pNew->type = UNDO_SONG;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void DefaultPatternView::AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	// count used patterns
	unsigned char count = 0;
	for (unsigned char i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			count++;
		}
	}
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS+sizeof(count)+MAX_PATTERNS+count*MULTIPLY2];
	unsigned char *pWrite=pNew->pData;
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pWrite + i) = m_pSong->PlayOrder(i);
	}
	//memcpy(pWrite, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	pWrite+=MAX_SONG_POSITIONS*sizeof(char);

	memcpy(pWrite, &count, sizeof(count));
	pWrite+=sizeof(count);

	for (i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			memcpy(pWrite, &i, sizeof(i));
			pWrite+=sizeof(i);
			memcpy(pWrite, m_pSong->pPatternData(i), MULTIPLY2);
			pWrite+=MULTIPLY2;
		}
	}

	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = m_pSong->PlayLength();
	pNew->type = UNDO_SONG;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void DefaultPatternView::AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS];

	//memcpy(pNew->pData, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pNew->pData + i) = m_pSong->PlayOrder(i);
	}
	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_SEQUENCE;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void DefaultPatternView::OnEditUndo() 
{
	// TODO: Add your command handler code here
	if (pUndoList)
	{
		switch (pUndoList->type)
		{
		case UNDO_PATTERN:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddRedo(pUndoList->pattern,pUndoList->x,pUndoList->y,pUndoList->tracks,pUndoList->lines,editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
				// do undo
				unsigned char* pData = pUndoList->pData;

				for (int t=pUndoList->x;t<pUndoList->x+pUndoList->tracks;t++)
				{
					for (int l=pUndoList->y;l<pUndoList->y+pUndoList->lines;l++)
					{
						unsigned char *offset_source=_ptrackline(pUndoList->pattern,t,l);
						
						memcpy(offset_source,pData,EVENT_SIZE*sizeof(char));
						pData+=EVENT_SIZE*sizeof(char);
					}
				}
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				if (pUndoList->seqpos == editPosition)
				{
					// display changes
					NewPatternDraw(pUndoList->x,pUndoList->x+pUndoList->tracks,pUndoList->y,pUndoList->y+pUndoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pUndoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
					Repaint(DMPattern);
					
				}
				// delete undo from list
				SPatternUndo* pTemp = pUndoList->pPrev;
				delete (pUndoList->pData);
				delete (pUndoList);
				pUndoList = pTemp;
			}
			break;
		case UNDO_LENGTH:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddRedoLength(pUndoList->pattern,m_pSong->PatternLines(pUndoList->pattern),editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
				// do undo
				m_pSong->PatternLines(pUndoList->pattern,pUndoList->lines);
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				if (pUndoList->seqpos != editPosition)
				{
					editPosition = pUndoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
				}
				// display changes
				Repaint(DMPattern);
				
				// delete undo from list
				SPatternUndo* pTemp = pUndoList->pPrev;
				delete (pUndoList->pData);
				delete (pUndoList);
				pUndoList = pTemp;
				break;
			}
		case UNDO_SEQUENCE:
			{
				AddRedoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
				// do undo
				for(int i = 0;i <  MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pUndoList->pData + i));
				}
				//memcpy(m_pSong->PlayOrder(), pUndoList->pData, MAX_SONG_POSITIONS * sizeof(char));
				m_pSong->PlayLength(pUndoList->lines);
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				editPosition = pUndoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
				// display changes
				Repaint(DMPattern);
				
				// delete undo from list
				{
					SPatternUndo* pTemp = pUndoList->pPrev;
					delete (pUndoList->pData);
					delete (pUndoList);
					pUndoList = pTemp;
				}
				break;
			}
		case UNDO_SONG:
			{
				AddRedoSong(editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
				// do undo
				unsigned char * pData = pUndoList->pData;
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pData + 1));
				}
				//memcpy(m_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
				pData += MAX_SONG_POSITIONS;
				unsigned char count = *pData;
				pData += sizeof(count);
				for (int i = 0; i < count; i++)
				{
					unsigned char index = *pData;
					pData += sizeof(index);
					unsigned char* pWrite = _ppattern(index);

					memcpy(pWrite,pData,MULTIPLY2);
					pData+= MULTIPLY2;
				}
				m_pSong->PlayLength(pUndoList->lines);
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				editPosition = pUndoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
				// display changes
				Repaint(DMPattern);
				
				// delete undo from list
				{
					SPatternUndo* pTemp = pUndoList->pPrev;
					delete (pUndoList->pData);
					delete (pUndoList);
					pUndoList = pTemp;
				}
				break;
			}

		}
		SetTitleBarText();
	}
}


void DefaultPatternView::OnEditRedo() 
{
	// TODO: Add your command handler code here
	if (pRedoList)
	{
		switch (pRedoList->type)
		{
		case UNDO_PATTERN:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddUndo(pRedoList->pattern,pRedoList->x,pRedoList->y,pRedoList->tracks,pRedoList->lines,editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do redo
				unsigned char* pData = pRedoList->pData;

				for (int t=pRedoList->x;t<pRedoList->x+pRedoList->tracks;t++)
				{
					for (int l=pRedoList->y;l<pRedoList->y+pRedoList->lines;l++)
					{
						unsigned char *offset_source=_ptrackline(pRedoList->pattern,t,l);

						
						memcpy(offset_source,pData,5*sizeof(char));
						pData+=5*sizeof(char);
					}
				}
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos == editPosition)
				{
					// display changes
					NewPatternDraw(pRedoList->x,pRedoList->x+pRedoList->tracks,pRedoList->y,pRedoList->y+pRedoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pRedoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
					Repaint(DMPattern);
					
				}
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
			}
			break;
		case UNDO_LENGTH:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddUndoLength(pRedoList->pattern,m_pSong->PatternLines(pUndoList->pattern),editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do undo
				m_pSong->PatternLines(pRedoList->pattern,pRedoList->lines);
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos != editPosition)
				{
					editPosition = pRedoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
				}
				// display changes
				Repaint(DMPattern);
				
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
				break;
			}
		case UNDO_SEQUENCE:
			{
				AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
				// do undo
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pRedoList->pData + i));
				}
				//memcpy(m_pSong->m_PlayOrder, pRedoList->pData, MAX_SONG_POSITIONS*sizeof(char));
				m_pSong->PlayLength(pRedoList->lines);
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				editPosition = pRedoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
				// display changes
				Repaint(DMPattern);
				
				{
					// delete redo from list
					SPatternUndo* pTemp = pRedoList->pPrev;
					delete (pRedoList->pData);
					delete (pRedoList);
					pRedoList = pTemp;
				}
				break;
			}
		case UNDO_SONG:
			{
				AddUndoSong(editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
				// do undo
				unsigned char * pData = pRedoList->pData;
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pData + i));
				}

				//memcpy(m_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
				pData += MAX_SONG_POSITIONS;
				unsigned char count = *pData;
				pData += sizeof(count);
				for (int i = 0; i < count; i++)
				{
					unsigned char index = *pData;
					pData += sizeof(index);
					unsigned char* pWrite = _ppattern(index);

					memcpy(pWrite,pData,MULTIPLY2);
					pData+= MULTIPLY2;
				}

				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				editPosition = pRedoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
				// display changes
				Repaint(DMPattern);
				
				{
					// delete redo from list
					SPatternUndo* pTemp = pRedoList->pPrev;
					delete (pRedoList->pData);
					delete (pRedoList);
					pRedoList = pTemp;
				}
				break;
			}
		}
		SetTitleBarText();
	}
}

void DefaultPatternView::KillUndo()
{
	while (pUndoList)
	{
		SPatternUndo* pTemp = pUndoList->pPrev;
		delete (pUndoList->pData);
		delete (pUndoList);
		pUndoList = pTemp;
	}

	m_UndoCounter = 0;
	m_UndoSaved = 0;

	UndoMacCounter=0;
	UndoMacSaved=0;

//	SetTitleBarText();
}


void DefaultPatternView::SelectNextTrack()
{
	m_EditCursor.Track(m_EditCursor.Track() + 1);
	
	if(m_EditCursor.Track() >= m_pSong->SongTracks())
	{
		m_EditCursor.Track(0);
	}
	
	while(m_pSong->IsTrackArmed(m_EditCursor.Track()) == 0)
	{
		m_EditCursor.Track(m_EditCursor.Track() + 1);
		if(m_EditCursor.Track() >= m_pSong->SongTracks())
		{
			m_EditCursor.Track(0);
		}
	}

	m_EditCursor.Column(0);
}
