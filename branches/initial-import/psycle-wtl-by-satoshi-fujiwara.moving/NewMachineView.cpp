/** @file 
 *  @brief implementation of the MachineView class
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
#include "NewMachineView.h"
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
#include "inputhandler.h"
#include "VstEditorDlg.h"
#include "masterdlg.h"

#include <math.h> // SwingFill

#include "keybhandler.cpp"
#include "mouseHandler.cpp"

//////////////////////////////////////////////////////////////////////
// Include GDI sources

//#include "machineview.cpp"
// 


/////////////////////////////


//////////////////////////////////////////////////////////////////////
// Enviroment Initialization

//MachineView		*pParentMain;

unsigned idletime = 0;

/////////////////////////////////////////////////////////////////////////////
// MachineView


namespace Configration {
}
MachineView::MachineView() : m_FmtPan(IDS_MSG0014),m_FmtPan1(IDS_MSG0015)

{
	m_bPrepare = false;


};

void MachineView::Initialize()
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
		m_WireDialog[c] = NULL;
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
	pMainFrame->OnMachineview();
}

MachineView::~MachineView()
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
	

}



/////////////////////////////////////////////////////////////////////////////
// MachineView message handlers
/*
BOOL MachineView::PreCreateWindow(CREATESTRUCT& cs) 
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
// allow MachineView call functions of the CMainFrm parent object
// Call this function after creating both the MachineView object and
// the cmainfrm object

void MachineView::ValidateParent()
{
	//pParentMain = (MachineView *)rMainFrame;
	pMainFrame->_pSong = Global::_pSong;
}



void MachineView::EnableSound()
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

void MachineView::OnDestroy()
{
	if (Global::pConfig->_pOutputDriver->Initialized())
	{
		Global::pConfig->_pOutputDriver->Reset();
	}
	KillTimer(31);
	KillTimer(159);

}


void MachineView::Repaint(int drawMode)
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

void MachineView::OnSize(UINT nType, CSize & size) 
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
		ATLTRACE(_T("MachineView::OnResize(). Delete bmpDC"));
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



void MachineView::ShowPatternDlg(void)
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

void MachineView::NewMachine(int x, int y, int mac) 
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





#define TWOPI_F (2.0f*3.141592665f)

void MachineView::ShowSwingFillDlg(bool bTrackMode)
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

void MachineView::OnPopCut() { CopyBlock(true); }
void MachineView::OnPopCopy() { CopyBlock(false); }
void MachineView::OnPopPaste() { PasteBlock(editcur.track,editcur.line,false); }
void MachineView::OnPopMixpaste() { PasteBlock(editcur.track,editcur.line,true); }

void MachineView::OnPopDelete() { DeleteBlock(); }

void MachineView::OnPopInterpolate() { BlockParamInterpolate(); }

void MachineView::OnPopChangegenerator() { BlockGenChange(_pSong->SeqBus()); }

void MachineView::OnPopChangeinstrument() { BlockInsChange(_pSong->AuxcolSelected()); }

void MachineView::OnPopTranspose1() { BlockTranspose(1); }

void MachineView::OnPopTranspose12() { BlockTranspose(12); }

void MachineView::OnPopTranspose_1() { BlockTranspose(-1); }

void MachineView::OnPopTranspose_12() { BlockTranspose(-12); }

void MachineView::OnPopPattenproperties() 
{
	ShowPatternDlg();
}

void MachineView::OnPopBlockSwingfill()
{
	// fill block
	ShowSwingFillDlg(FALSE);
}

void MachineView::OnPopTrackSwingfill()
{
	// fill track
	ShowSwingFillDlg(TRUE);
}



//



//#define _UGLY_DEFAULT_SKIN_

void MachineView::LoadMachineSkin()
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

void MachineView::FindMachineSkin(CString findDir, CString findName, BOOL *result)
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

void MachineView::LoadPatternHeaderSkin()
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

void MachineView::FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result)
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


void MachineView::RecalcMetrics()
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


void MachineView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
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

void MachineView::TransparentBlt(CDC* pDC,
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

void MachineView::patTrackMute()
{
	if (viewMode == VMPattern)
	{
		_pSong->IsTrackMuted(editcur.track,!_pSong->IsTrackMuted(editcur.track));
		Repaint(DMTrackHeader);
	}
}

void MachineView::patTrackSolo()
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

void MachineView::patTrackRecord()
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

void MachineView::DoMacPropDialog(int propMac)
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




void MachineView::LoadMachineBackground()
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

BOOL MachineView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT MachineView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
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
		ATLTRACE(_T("MachineView::OnPaint(). bmpDC  0x%.8X initialize\n"),(int)bmpDC);
#endif
	}
	else if ( bmpDC != NULL && !Global::pConfig->useDoubleBuffer ) // buffer deletion
	{
#ifdef _DEBUG_PATVIEW
		ATLTRACE(_T("MachineView::OnPaint(). bmpDC (0x%.8X) deleted\n"),(int)bmpDC);
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


void MachineView::UpdateUI()
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


void MachineView::DrawAllMachineVumeters(CDC *devc)
{
	if (Global::pConfig->draw_vus)
	{
		if (_pSong->IsMachineLock())
		{
			return;
		}
		// Draw machine boxes
		for (int c=0; c<MAX_MACHINES-1; c++)
		{
			Machine* pMac = _pSong->pMachine(c);
			if (pMac)
			{
				pMac->_volumeMaxCounterLife--;
				if ((pMac->_volumeDisplay > pMac->_volumeMaxDisplay)
					|| (pMac->_volumeMaxCounterLife <= 0))
				{
					pMac->_volumeMaxDisplay = pMac->_volumeDisplay-1;
					pMac->_volumeMaxCounterLife = 60;
				}
				DrawMachineVol(c, devc);
			}
		}
	}
}

void MachineView::DrawMachineVumeters(int c, CDC *devc)
{
	if (Global::pConfig->draw_vus)
	{
		if (_pSong->IsMachineLock())
		{
			return;
		}
		// Draw machine boxes

		Machine* pMac = _pSong->pMachine(c);
		if (pMac)
		{
			pMac->_volumeMaxCounterLife--;
			if ((pMac->_volumeDisplay > pMac->_volumeMaxDisplay)
				|| (pMac->_volumeMaxCounterLife <= 0))
			{
				pMac->_volumeMaxDisplay = pMac->_volumeDisplay-1;
				pMac->_volumeMaxCounterLife = 60;
			}
			DrawMachineVol(c, devc);
		}
	}
}


void MachineView::DrawMachineEditor(WTL::CDC *devc)
{
	if (_pSong->IsMachineLock())
	{
		return;
	}

	WTL::CBrush fillbrush;
	WTL::CBrushHandle oldbrush(devc->SelectBrush(fillbrush.CreateSolidBrush(Global::pConfig->mv_polycolour)));
		
	if (Global::pConfig->bBmpBkg)
	{
		WTL::CDC memDC;
		memDC.CreateCompatibleDC((HDC)devc);
		//oldbmp = memDC.SelectBitmap((HBITMAP)machinebkg);
		CBitmapHandle oldbmp(memDC.SelectBitmap((HBITMAP)machinebkg));

		if ((CW > bkgx) || (CH > bkgy))
		{
			for (int cx = 0; cx < CW; cx += bkgx)
			{
				for (int cy = 0; cy < CH; cy += bkgy)
				{
					devc->BitBlt(cx,cy,bkgx,bkgy,(HDC)memDC,0,0,SRCCOPY);
				}
			}
		}
		else
		{
			devc->BitBlt(0,0,CW,CH,(HDC)memDC,0,0,SRCCOPY);
		}

		memDC.SelectBitmap((HBITMAP)oldbmp);
		memDC.DeleteDC();

	}
	else
	{
		CRect rClient;
		GetClientRect(&rClient);
		devc->FillSolidRect(&rClient,Global::pConfig->mv_colour);
	}

	if (Global::pConfig->mv_wireaa)
	{
		CPen linepen1;
		CPen linepen2;
		CPen linepen3; 
		
		linepen1.CreatePen( PS_SOLID, Global::pConfig->mv_wirewidth + (Global::pConfig->mv_wireaa * 2), Global::pConfig->mv_wireaacolour);
		linepen2.CreatePen( PS_SOLID, Global::pConfig->mv_wirewidth + (Global::pConfig->mv_wireaa), Global::pConfig->mv_wireaacolour2); 
		linepen3.CreatePen( PS_SOLID, Global::pConfig->mv_wirewidth, Global::pConfig->mv_wirecolour);
		
		WTL::CPenHandle oldpen(devc->SelectPen((HPEN)linepen1));
		
		// Draw wire [connections]
		for(int c = 0; c < MAX_MACHINES; c++)
		{
			Machine *tmac = _pSong->pMachine(c);
			if(tmac)
			{
				int oriX;
				int oriY;
				switch (tmac->_mode)
				{
				case MACHMODE_GENERATOR:
					oriX = tmac->_x + (MachineCoords.sGenerator.width / 2);
					oriY = tmac->_y + (MachineCoords.sGenerator.height / 2);
					break;
				case MACHMODE_FX:
					oriX = tmac->_x + (MachineCoords.sEffect.width / 2);
					oriY = tmac->_y + (MachineCoords.sEffect.height / 2);
					break;

				case MACHMODE_MASTER:
					oriX = tmac->_x + (MachineCoords.sMaster.width / 2);
					oriY = tmac->_y + (MachineCoords.sMaster.height / 2);
					break;
				}

				for (int w = 0; w < MAX_CONNECTIONS; w++)
				{
					if (tmac->_connection[w])
					{
						int desX = 0;
						int desY = 0;
						Machine* pout = _pSong->pMachine(tmac->_outputMachines[w]);
						if (pout)
						{
							switch (pout->_mode)
							{
							case MACHMODE_GENERATOR:
								desX = pout->_x + (MachineCoords.sGenerator.width / 2);
								desY = pout->_y + (MachineCoords.sGenerator.height / 2);
								break;
							case MACHMODE_FX:
								desX = pout->_x + (MachineCoords.sEffect.width / 2);
								desY = pout->_y + (MachineCoords.sEffect.height / 2);
								break;

							case MACHMODE_MASTER:
								desX = pout->_x + (MachineCoords.sMaster.width / 2);
								desY = pout->_y + (MachineCoords.sMaster.height / 2);
								break;
							}
						}
						
						int const f1 = (desX + oriX)/2;
						int const f2 = (desY + oriY)/2;

						double modX = double(desX - oriX);
						double modY = double(desY - oriY);
						double modT = sqrt(modX * modX + modY * modY);
						
						modX = modX / modT;
						modY = modY / modT;
								
						CPoint pol[4];
						
						pol[0].x = f1 - Dsp::F2I(modX * triangle_size_center);
						pol[0].y = f2 - Dsp::F2I(modY * triangle_size_center);
						pol[1].x = pol[0].x + Dsp::F2I(modX*triangle_size_tall);
						pol[1].y = pol[0].y + Dsp::F2I(modY*triangle_size_tall);
						pol[2].x = pol[0].x - Dsp::F2I(modY*triangle_size_wide);
						pol[2].y = pol[0].y + Dsp::F2I(modX*triangle_size_wide);
						pol[3].x = pol[0].x + Dsp::F2I(modY*triangle_size_wide);
						pol[3].y = pol[0].y - Dsp::F2I(modX*triangle_size_wide);

						devc->SelectPen((HPEN)linepen1);
						amosDraw(devc, oriX, oriY, desX, desY);
						devc->Polygon(&pol[1], 3);
						devc->SelectPen((HPEN)linepen2);
						amosDraw(devc, oriX, oriY, desX, desY);
						devc->Polygon(&pol[1], 3);
						devc->SelectPen((HPEN)linepen3);
						amosDraw(devc, oriX, oriY, desX, desY);
						devc->Polygon(&pol[1], 3);

						tmac->_connectionPoint[w].x = f1-triangle_size_center;
						tmac->_connectionPoint[w].y = f2-triangle_size_center;
					}
				}
			}// Machine actived
		}
		devc->SelectPen((HPEN)oldpen);
		
		linepen1.DeleteObject();
		linepen2.DeleteObject();
		linepen3.DeleteObject();
	}
	else
	{
		CPen linepen;
		linepen.CreatePen( PS_SOLID, Global::pConfig->mv_wirewidth, Global::pConfig->mv_wirecolour); 
		CPenHandle oldpen(devc->SelectPen((HPEN)linepen));
		// Draw wire [connections]
		for(int c = 0; c < MAX_MACHINES; c++)
		{
			Machine *tmac = _pSong->pMachine(c);
			if(tmac)
			{
				int oriX;
				int oriY;
				switch (tmac->_mode)
				{
				case MACHMODE_GENERATOR:
					oriX = tmac->_x + (MachineCoords.sGenerator.width / 2);
					oriY = tmac->_y + (MachineCoords.sGenerator.height / 2);
					break;
				case MACHMODE_FX:
					oriX = tmac->_x + (MachineCoords.sEffect.width / 2);
					oriY = tmac->_y + (MachineCoords.sEffect.height / 2);
					break;

				case MACHMODE_MASTER:
					oriX = tmac->_x + (MachineCoords.sMaster.width / 2);
					oriY = tmac->_y + (MachineCoords.sMaster.height / 2);
					break;
				}

				for (int w = 0; w < MAX_CONNECTIONS; w++)
				{
					if (tmac->_connection[w])
					{
						int desX = 0;
						int desY = 0;
						Machine* pout = _pSong->pMachine(tmac->_outputMachines[w]);
						if (pout)
						{
							switch (pout->_mode)
							{
							case MACHMODE_GENERATOR:
								desX = pout->_x+(MachineCoords.sGenerator.width/2);
								desY = pout->_y+(MachineCoords.sGenerator.height/2);
								break;
							case MACHMODE_FX:
								desX = pout->_x+(MachineCoords.sEffect.width/2);
								desY = pout->_y+(MachineCoords.sEffect.height/2);
								break;

							case MACHMODE_MASTER:
								desX = pout->_x+(MachineCoords.sMaster.width/2);
								desY = pout->_y+(MachineCoords.sMaster.height/2);
								break;
							}
						}
						
						int const f1 = (desX+oriX)/2;
						int const f2 = (desY+oriY)/2;

						amosDraw(devc, oriX, oriY, desX, desY);
						
						double modX = double(desX-oriX);
						double modY = double(desY-oriY);
						double modT = sqrt(modX*modX+modY*modY);
						
						modX = modX/modT;
						modY = modY/modT;
								
						CPoint pol[4];
						
						pol[0].x = f1 - Dsp::F2I(modX*triangle_size_center);
						pol[0].y = f2 - Dsp::F2I(modY*triangle_size_center);
						pol[1].x = pol[0].x + Dsp::F2I(modX*triangle_size_tall);
						pol[1].y = pol[0].y + Dsp::F2I(modY*triangle_size_tall);
						pol[2].x = pol[0].x - Dsp::F2I(modY*triangle_size_wide);
						pol[2].y = pol[0].y + Dsp::F2I(modX*triangle_size_wide);
						pol[3].x = pol[0].x + Dsp::F2I(modY*triangle_size_wide);
						pol[3].y = pol[0].y - Dsp::F2I(modX*triangle_size_wide);

						devc->Polygon(&pol[1], 3);

						tmac->_connectionPoint[w].x = f1-triangle_size_center;
						tmac->_connectionPoint[w].y = f2-triangle_size_center;
					}
				}
			}// Machine actived
		}
		devc->SelectPen((HPEN)oldpen);
		linepen.DeleteObject();
	}


	// Draw machine boxes
	for (int c=0; c<MAX_MACHINES; c++)
	{
		if(_pSong->pMachine(c))
		{
			DrawMachine(c , devc);
		}// Machine exist
	}

	// draw vumeters
	DrawAllMachineVumeters((CClientDC*)devc);

	if (wiresource != -1)
	{
		int prevROP = devc->SetROP2(R2_NOT);
		amosDraw(devc, wireSX, wireSY, wireDX, wireDY);
		devc->SetROP2(prevROP);
	}
	devc->SelectBrush(oldbrush);
	fillbrush.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// Draws a single machine

void MachineView::DrawMachineVol(int c,CDC *devc)
{
	Machine* pMac = Global::_pSong->pMachine(c);
	if (pMac)
	{
		WTL::CDC memDC;
		CBitmapHandle oldbmp;
		memDC.CreateCompatibleDC((HDC)(*devc));
		oldbmp = memDC.SelectBitmap((HBITMAP)machineskin);

		int vol = pMac->_volumeDisplay;
		int max = pMac->_volumeMaxDisplay;

		switch (pMac->_mode)
		{
		case MACHMODE_GENERATOR:
			// scale our volumes
			vol *= MachineCoords.dGeneratorVu.width;
			vol /= 96;

			max *= MachineCoords.dGeneratorVu.width;
			max /= 96;

			if (MachineCoords.bHasTransparency)
			{
				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				if (vol > 0)
				{
					if (MachineCoords.sGeneratorVu0.width)
					{
						vol /= MachineCoords.sGeneratorVu0.width;// restrict to leds
						vol *= MachineCoords.sGeneratorVu0.width;
					}
				}
				else
				{
					vol = 0;
				}

				RECT r;
				r.left = pMac->_x+vol+MachineCoords.dGeneratorVu.x;
				r.top = pMac->_y+MachineCoords.dGeneratorVu.y;
				r.right = r.left + MachineCoords.dGeneratorVu.width-vol;
				r.bottom = r.top + MachineCoords.sGeneratorVu0.height;
				devc->FillSolidRect(&r,Global::pConfig->mv_colour);

				TransparentBlt(devc,
							r.left, 
							r.top, 
							MachineCoords.dGeneratorVu.width-vol, 
							MachineCoords.sGeneratorVu0.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGenerator.x+MachineCoords.dGeneratorVu.x, 
							MachineCoords.sGenerator.y+MachineCoords.dGeneratorVu.y);

				if (max > 0)
				{
					if (MachineCoords.sGeneratorVuPeak.width)
					{
						max /= MachineCoords.sGeneratorVuPeak.width;// restrict to leds
						max *= MachineCoords.sGeneratorVuPeak.width;
						TransparentBlt(devc,
									pMac->_x+max+MachineCoords.dGeneratorVu.x, 
									pMac->_y+MachineCoords.dGeneratorVu.y, 
									MachineCoords.sGeneratorVuPeak.width, 
									MachineCoords.sGeneratorVuPeak.height, 
									&memDC, 
									&machineskinmask,
									MachineCoords.sGeneratorVuPeak.x, 
									MachineCoords.sGeneratorVuPeak.y);
					}
				}

				if (vol > 0)
				{
					TransparentBlt(devc,
								pMac->_x+MachineCoords.dGeneratorVu.x, 
								pMac->_y+MachineCoords.dGeneratorVu.y, 
								vol, 
								MachineCoords.sGeneratorVu0.height, 
								&memDC, 
								&machineskinmask,
								MachineCoords.sGeneratorVu0.x, 
								MachineCoords.sGeneratorVu0.y);
				}
			}
			else
			{
				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				if (vol > 0)
				{
					if (MachineCoords.sGeneratorVu0.width)
					{
						vol /= MachineCoords.sGeneratorVu0.width;// restrict to leds
						vol *= MachineCoords.sGeneratorVu0.width;
					}
				}
				else
				{
					vol = 0;
				}

				devc->BitBlt(pMac->_x + vol + MachineCoords.dGeneratorVu.x, 
								pMac->_y + MachineCoords.dGeneratorVu.y, 
								MachineCoords.dGeneratorVu.width - vol, 
								MachineCoords.sGeneratorVu0.height, 
								(HDC)memDC, 
								MachineCoords.sGenerator.x + MachineCoords.dGeneratorVu.x, 
								MachineCoords.sGenerator.y + MachineCoords.dGeneratorVu.y, 
								SRCCOPY); //background

				if (max > 0)
				{
					if (MachineCoords.sGeneratorVuPeak.width)
					{
						max /= MachineCoords.sGeneratorVuPeak.width;// restrict to leds
						max *= MachineCoords.sGeneratorVuPeak.width;
						devc->BitBlt(pMac->_x + max+MachineCoords.dGeneratorVu.x, 
									pMac->_y + MachineCoords.dGeneratorVu.y, 
									MachineCoords.sGeneratorVuPeak.width, 
									MachineCoords.sGeneratorVuPeak.height, 
									(HDC)memDC, 
									MachineCoords.sGeneratorVuPeak.x, 
									MachineCoords.sGeneratorVuPeak.y, 
									SRCCOPY); //peak
					}
				}

				if (vol > 0)
				{
					devc->BitBlt(pMac->_x + MachineCoords.dGeneratorVu.x, 
								pMac->_y + MachineCoords.dGeneratorVu.y, 
								vol, 
								MachineCoords.sGeneratorVu0.height, 
								(HDC)memDC, 
								MachineCoords.sGeneratorVu0.x, 
								MachineCoords.sGeneratorVu0.y, 
								SRCCOPY); // leds
				}
			}

			break;
		case MACHMODE_FX:
			// scale our volumes
			vol *= MachineCoords.dEffectVu.width;
			vol /= 96;

			max *= MachineCoords.dEffectVu.width;
			max /= 96;

			if (MachineCoords.bHasTransparency)
			{
				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				if (vol > 0)
				{
					if (MachineCoords.sEffectVu0.width)
					{
						vol /= MachineCoords.sEffectVu0.width;// restrict to leds
						vol *= MachineCoords.sEffectVu0.width;
					}
				}
				else
				{
					vol = 0;
				}

				RECT r;
				r.left = pMac->_x + vol + MachineCoords.dEffectVu.x;
				r.top = pMac->_y + MachineCoords.dEffectVu.y;
				r.right = r.left + MachineCoords.dEffectVu.width - vol;
				r.bottom = r.top + MachineCoords.sEffectVu0.height;
				devc->FillSolidRect(&r,Global::pConfig->mv_colour);

				TransparentBlt(devc,
							r.left, 
							r.top, 
							MachineCoords.dEffectVu.width - vol, 
							MachineCoords.sEffectVu0.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffect.x+MachineCoords.dEffectVu.x, 
							MachineCoords.sEffect.y+MachineCoords.dEffectVu.y);

				if (max > 0)
				{
					if (MachineCoords.sEffectVuPeak.width)
					{
						max /= MachineCoords.sEffectVuPeak.width;// restrict to leds
						max *= MachineCoords.sEffectVuPeak.width;
						TransparentBlt(devc,
									pMac->_x+max+MachineCoords.dEffectVu.x, 
									pMac->_y+MachineCoords.dEffectVu.y, 
									MachineCoords.sEffectVuPeak.width, 
									MachineCoords.sEffectVuPeak.height, 
									&memDC, 
									&machineskinmask,
									MachineCoords.sEffectVuPeak.x, 
									MachineCoords.sEffectVuPeak.y);
					}
				}

				if (vol > 0)
				{
					TransparentBlt(devc,
								pMac->_x+MachineCoords.dEffectVu.x, 
								pMac->_y+MachineCoords.dEffectVu.y, 
								vol, 
								MachineCoords.sEffectVu0.height, 
								&memDC, 
								&machineskinmask,
								MachineCoords.sEffectVu0.x, 
								MachineCoords.sEffectVu0.y);
				}
			}
			else
			{
				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				if (vol > 0)
				{
					if (MachineCoords.sEffectVu0.width)
					{
						vol /= MachineCoords.sEffectVu0.width;// restrict to leds
						vol *= MachineCoords.sEffectVu0.width;
					}
				}
				else
				{
					vol = 0;
				}

				devc->BitBlt(pMac->_x + vol + MachineCoords.dEffectVu.x, 
								pMac->_y + MachineCoords.dEffectVu.y, 
								MachineCoords.dEffectVu.width - vol, 
								MachineCoords.sEffectVu0.height, 
								(HDC)memDC, 
								MachineCoords.sEffect.x + MachineCoords.dEffectVu.x, 
								MachineCoords.sEffect.y + MachineCoords.dEffectVu.y, 
								SRCCOPY); //background

				if (max > 0)
				{
					if (MachineCoords.sEffectVuPeak.width)
					{
						max /= MachineCoords.sEffectVuPeak.width;// restrict to leds
						max *= MachineCoords.sEffectVuPeak.width;
						devc->BitBlt(pMac->_x+max+MachineCoords.dEffectVu.x, 
									pMac->_y+MachineCoords.dEffectVu.y, 
									MachineCoords.sEffectVuPeak.width, 
									MachineCoords.sEffectVuPeak.height, 
									(HDC)memDC, 
									MachineCoords.sEffectVuPeak.x, 
									MachineCoords.sEffectVuPeak.y, 
									SRCCOPY); //peak
					}
				}

				if (vol > 0)
				{
					devc->BitBlt(pMac->_x+MachineCoords.dEffectVu.x, 
								pMac->_y+MachineCoords.dEffectVu.y, 
								vol, 
								MachineCoords.sEffectVu0.height, 
								(HDC)memDC, 
								MachineCoords.sEffectVu0.x, 
								MachineCoords.sEffectVu0.y, 
								SRCCOPY); // leds
				}
			}

			break;

		}

		memDC.SelectBitmap((HBITMAP)oldbmp);
		memDC.DeleteDC();
	}
}

void MachineView::ClearMachineSpace(int macnum, CDC *devc)
{
	Machine* mac = _pSong->pMachine(macnum);
	if(!mac)
	{
	    return;
	}

	if (MachineCoords.bHasTransparency)
	{
		RECT r;
		switch (mac->_mode)
		{
		case MACHMODE_GENERATOR:
			r.left = mac->_x;
			r.top = mac->_y;
			r.right = r.left + MachineCoords.sGenerator.width;
			r.bottom = r.top + MachineCoords.sGenerator.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			break;
		case MACHMODE_FX:
			r.left = mac->_x;
			r.top = mac->_y;
			r.right = r.left + MachineCoords.sEffect.width;
			r.bottom = r.top + MachineCoords.sEffect.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			break;
		}
	}
}

void MachineView::DrawMachine(int macnum, WTL::CDC *devc)
{
	Machine* mac = _pSong->pMachine(macnum);
	if(!mac)
	{
	    return;
	}

	int x = mac->_x;
	int y = mac->_y;

	WTL::CDC memDC;
	memDC.CreateCompatibleDC((HDC)(*devc));
	CBitmapHandle oldbmp = memDC.SelectBitmap((HBITMAP)machineskin);

	// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
	if (MachineCoords.bHasTransparency)
	{
		switch (mac->_mode)
		{
		case MACHMODE_GENERATOR:
			/*
			RECT r;
			r.left = x;
			r.top = y;
			r.right = r.left + MachineCoords.sGenerator.width;
			r.bottom = r.top + MachineCoords.sGenerator.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			*/

			TransparentBlt(devc,
						x, 
						y, 
						MachineCoords.sGenerator.width, 
						MachineCoords.sGenerator.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sGenerator.x, 
						MachineCoords.sGenerator.y);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dGeneratorPan.width;
				panning /= 128;
				TransparentBlt(devc,
							x+panning+MachineCoords.dGeneratorPan.x, 
							y+MachineCoords.dGeneratorPan.y, 
							MachineCoords.sGeneratorPan.width, 
							MachineCoords.sGeneratorPan.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorPan.x, 
							MachineCoords.sGeneratorPan.y);
			}
			if (mac->_mute)
			{
				TransparentBlt(devc,
							x+MachineCoords.dGeneratorMute.x, 
							y+MachineCoords.dGeneratorMute.y, 
							MachineCoords.sGeneratorMute.width, 
							MachineCoords.sGeneratorMute.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorMute.x, 
							MachineCoords.sGeneratorMute.y);
			}
			else if (_pSong->MachineSoloed() == macnum)
			{
				TransparentBlt(devc,
							x+MachineCoords.dGeneratorSolo.x, 
							y+MachineCoords.dGeneratorSolo.y, 
							MachineCoords.sGeneratorSolo.width, 
							MachineCoords.sGeneratorSolo.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorSolo.x, 
							MachineCoords.sGeneratorSolo.y);
			}
			// Draw text
			{
				CFontHandle oldFont(devc->SelectFont((HFONT)(Global::pConfig->generatorFont)));
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_generator_fontcolour);
				if (Global::pConfig->draw_mac_index)
				{
					TCHAR name[sizeof(mac->_editName) + 6];
					_stprintf(name,_T("%.2X:%s"),mac->_macIndex,mac->_editName);
					devc->TextOut(x + MachineCoords.dGeneratorName.x,
						y+MachineCoords.dGeneratorName.y, 
						name
					);
				}
				else
				{
					devc->TextOut(x+MachineCoords.dGeneratorName.x, y+MachineCoords.dGeneratorName.y, mac->_editName);
				}
				devc->SetBkMode(OPAQUE);
				devc->SelectFont((HFONT)oldFont);
			}
			break;
		case MACHMODE_FX:
			/*
			RECT r;
			r.left = x;
			r.top = y;
			r.right = r.left + MachineCoords.sEffect.width;
			r.bottom = r.top + MachineCoords.sEffect.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			*/

			TransparentBlt(devc,
						x, 
						y,
						MachineCoords.sEffect.width, 
						MachineCoords.sEffect.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sEffect.x, 
						MachineCoords.sEffect.y);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dEffectPan.width;
				panning /= 128;
				TransparentBlt(devc,
							x+panning+MachineCoords.dEffectPan.x, 
							y+MachineCoords.dEffectPan.y, 
							MachineCoords.sEffectPan.width, 
							MachineCoords.sEffectPan.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectPan.x, 
							MachineCoords.sEffectPan.y);
			}
			if (mac->_mute)
			{
				TransparentBlt(devc,
							x+MachineCoords.dEffectMute.x, 
							y+MachineCoords.dEffectMute.y, 
							MachineCoords.sEffectMute.width, 
							MachineCoords.sEffectMute.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectMute.x, 
							MachineCoords.sEffectMute.y);
			}
			if (mac->_bypass)
			{
				TransparentBlt(devc,
							x+MachineCoords.dEffectBypass.x, 
							y+MachineCoords.dEffectBypass.y, 
							MachineCoords.sEffectBypass.width, 
							MachineCoords.sEffectBypass.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectBypass.x, 
							MachineCoords.sEffectBypass.y);
			}
			// Draw text
			{
				CFontHandle oldFont(devc->SelectFont((HFONT)(&Global::pConfig->effectFont)));
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_effect_fontcolour);
				if (Global::pConfig->draw_mac_index)
				{
					TCHAR name[sizeof(mac->_editName)+6];
					_stprintf(name,_T("%.2X:%s"),mac->_macIndex,mac->_editName);
					devc->TextOut(x+MachineCoords.dEffectName.x, y+MachineCoords.dEffectName.y, name);
				}
				else
				{
					devc->TextOut(x+MachineCoords.dEffectName.x, y+MachineCoords.dEffectName.y, mac->_editName);
				}
				devc->SetBkMode(OPAQUE);
				devc->SelectFont((HFONT)oldFont);
			}
			break;

		case MACHMODE_MASTER:
			TransparentBlt(devc,
						x, 
						y,
						MachineCoords.sMaster.width, 
						MachineCoords.sMaster.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sMaster.x, 
						MachineCoords.sMaster.y);
			break;
		}
	}
	else
	{
		switch (mac->_mode)
		{
		case MACHMODE_GENERATOR:
			devc->BitBlt(x, 
						y, 
						MachineCoords.sGenerator.width, 
						MachineCoords.sGenerator.height, 
						(HDC)memDC, 
						MachineCoords.sGenerator.x, 
						MachineCoords.sGenerator.y, 
						SRCCOPY);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dGeneratorPan.width;
				panning /= 128;
				devc->BitBlt(x+panning+MachineCoords.dGeneratorPan.x, 
							y+MachineCoords.dGeneratorPan.y, 
							MachineCoords.sGeneratorPan.width, 
							MachineCoords.sGeneratorPan.height, 
							(HDC)memDC, 
							MachineCoords.sGeneratorPan.x, 
							MachineCoords.sGeneratorPan.y, 
							SRCCOPY);
			}
			if (mac->_mute)
			{
				devc->BitBlt(x+MachineCoords.dGeneratorMute.x, 
							y+MachineCoords.dGeneratorMute.y, 
							MachineCoords.sGeneratorMute.width, 
							MachineCoords.sGeneratorMute.height, 
							(HDC)memDC, 
							MachineCoords.sGeneratorMute.x, 
							MachineCoords.sGeneratorMute.y, 
							SRCCOPY);
			}
			else if (_pSong->MachineSoloed() == macnum)
			{
				devc->BitBlt(x+MachineCoords.dGeneratorSolo.x, 
							y+MachineCoords.dGeneratorSolo.y, 
							MachineCoords.sGeneratorSolo.width, 
							MachineCoords.sGeneratorSolo.height, 
							(HDC)memDC, 
							MachineCoords.sGeneratorSolo.x, 
							MachineCoords.sGeneratorSolo.y, 
							SRCCOPY);
			}
			// Draw text
			{
				CFontHandle oldFont(devc->SelectFont((HFONT)(Global::pConfig->generatorFont)));
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_generator_fontcolour);
				if (Global::pConfig->draw_mac_index)
				{
					TCHAR name[sizeof(mac->_editName)+6];
					_stprintf(name,_T("%.2X:%s"),mac->_macIndex,mac->_editName);
					devc->TextOut(x+MachineCoords.dGeneratorName.x, y+MachineCoords.dGeneratorName.y, name);
				}
				else
				{
					devc->TextOut(x+MachineCoords.dGeneratorName.x, y+MachineCoords.dGeneratorName.y, mac->_editName);
				}
				devc->SetBkMode(OPAQUE);
				devc->SelectFont((HFONT)oldFont);
			}
			break;
		case MACHMODE_FX:
			devc->BitBlt(x, 
						y,
						MachineCoords.sEffect.width, 
						MachineCoords.sEffect.height, 
						(HDC)memDC, 
						MachineCoords.sEffect.x, 
						MachineCoords.sEffect.y, 
						SRCCOPY);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dEffectPan.width;
				panning /= 128;
				devc->BitBlt(x+panning+MachineCoords.dEffectPan.x, 
							y+MachineCoords.dEffectPan.y, 
							MachineCoords.sEffectPan.width, 
							MachineCoords.sEffectPan.height, 
							(HDC)memDC, 
							MachineCoords.sEffectPan.x, 
							MachineCoords.sEffectPan.y, 
							SRCCOPY);
			}
			if (mac->_mute)
			{
				devc->BitBlt(x+MachineCoords.dEffectMute.x, 
							y+MachineCoords.dEffectMute.y, 
							MachineCoords.sEffectMute.width, 
							MachineCoords.sEffectMute.height, 
							(HDC)memDC, 
							MachineCoords.sEffectMute.x, 
							MachineCoords.sEffectMute.y, 
							SRCCOPY);
			}
			if (mac->_bypass)
			{
				devc->BitBlt(x+MachineCoords.dEffectBypass.x, 
							y+MachineCoords.dEffectBypass.y, 
							MachineCoords.sEffectBypass.width, 
							MachineCoords.sEffectBypass.height, 
							(HDC)memDC, 
							MachineCoords.sEffectBypass.x, 
							MachineCoords.sEffectBypass.y, 
							SRCCOPY);
			}
			// Draw text
			{
				CFontHandle oldFont(devc->SelectFont((HFONT)(Global::pConfig->effectFont)));
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_effect_fontcolour);
				if (Global::pConfig->draw_mac_index)
				{
					TCHAR name[sizeof(mac->_editName)+6];
					_stprintf(name,_T("%.2X:%s"),mac->_macIndex,mac->_editName);
					devc->TextOut(x+MachineCoords.dEffectName.x, y+MachineCoords.dEffectName.y, name);
				}
				else
				{
					devc->TextOut(x+MachineCoords.dEffectName.x, y+MachineCoords.dEffectName.y, mac->_editName);
				}
				devc->SetBkMode(OPAQUE);
				devc->SelectFont((HFONT)oldFont);
			}
			break;

		case MACHMODE_MASTER:
			devc->BitBlt(x, 
						y,
						MachineCoords.sMaster.width, 
						MachineCoords.sMaster.height, 
						(HDC)memDC, 
						MachineCoords.sMaster.x, 
						MachineCoords.sMaster.y, 
						SRCCOPY);
			break;
		}
	}
	memDC.SelectBitmap((HBITMAP)oldbmp);
	memDC.DeleteDC();
}


void MachineView::amosDraw(CDC *devc, int oX,int oY,int dX,int dY)
{
	if (oX == dX)
	{
		oX++;
	}
	if (oY == dY)
	{
		oY++;
	}

	devc->MoveTo(oX,oY);
	devc->LineTo(dX,dY);	
}

int MachineView::GetMachine(CPoint point)
{
	int tmac = -1;
	
	for (int c=MAX_MACHINES-1; c>=0; c--)
	{
		Machine* pMac = Global::_pSong->pMachine(c);
		if (pMac)
		{
			int x1 = pMac->_x;
			int y1 = pMac->_y;
			int x2,y2;
			switch (pMac->_mode)
			{
			case MACHMODE_GENERATOR:
				x2 = pMac->_x+MachineCoords.sGenerator.width;
				y2 = pMac->_y+MachineCoords.sGenerator.height;
				break;
			case MACHMODE_FX:
				x2 = pMac->_x+MachineCoords.sEffect.width;
				y2 = pMac->_y+MachineCoords.sEffect.height;
				break;

			case MACHMODE_MASTER:
				x2 = pMac->_x+MachineCoords.sMaster.width;
				y2 = pMac->_y+MachineCoords.sMaster.height;
				break;
			}
			
			if (point.x > x1 && point.x < x2 && point.y > y1 && point.y < y2)
			{
				tmac = c;
				break;
			}
		}
	}
	return tmac;
}

void MachineView::ShowMachineGui(const int tmac, const CPoint point)
{
	Machine *ma = _pSong->pMachine(tmac);

	if (ma)
	{
		if (m_bGUIOpen[tmac])
		{
			m_pWndMac[tmac]->SetActiveWindow();
		}
		else
		{
			AddMacViewUndo();

			switch (ma->_type)
			{
			case MACH_MASTER:
				if (!MasterMachineDialog)
				{
					MasterMachineDialog = new CMasterDlg(&m_view);
					MasterMachineDialog->_pMachine = (Master*)ma;
					for (int i=0;i<MAX_CONNECTIONS; i++)
					{
						if ( ma->_inputCon[i])
						{
							if (_pSong->pMachine(ma->_inputMachines[i]))
							{
								_tcscpy(MasterMachineDialog->macname[i],_pSong->pMachine(ma->_inputMachines[i])->_editName);
							}
						}
					}
					MasterMachineDialog->Create(m_view);
					CenterWindowOnPoint((HWND)(*(MasterMachineDialog)), point);
					MasterMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_SAMPLER:
				if (SamplerMachineDialog)
				{
					if (SamplerMachineDialog->_pMachine != (Sampler*)ma)
					{
						SamplerMachineDialog->OnCancel();
						SamplerMachineDialog = new CGearTracker(&m_view);
						SamplerMachineDialog->_pMachine = (Sampler*)ma;
						SamplerMachineDialog->Create(m_view);
						CenterWindowOnPoint(SamplerMachineDialog->m_hWnd, point);
						SamplerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					SamplerMachineDialog = new CGearTracker(&m_view);
					SamplerMachineDialog->_pMachine = (Sampler*)ma;
					SamplerMachineDialog->Create(m_view);
					CenterWindowOnPoint(SamplerMachineDialog->m_hWnd, point);
					SamplerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_XMSAMPLER:
				if (XMSamplerMachineDialog)
				{
					if (XMSamplerMachineDialog->pMachine() != (SF::XMSampler*)ma)
					{
						XMSamplerMachineDialog->OnCancel();
						XMSamplerMachineDialog 
							= new SF::XMSamplerUI(&m_view,(SF::XMSampler*)ma,ma->_editName,0,m_view);
						XMSamplerMachineDialog->Create();
						CenterWindowOnPoint(SamplerMachineDialog->m_hWnd, point);
						XMSamplerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					XMSamplerMachineDialog 
						= new SF::XMSamplerUI(&m_view,(SF::XMSampler*)ma,ma->_editName,0,m_view);
					XMSamplerMachineDialog->Create();
					CenterWindowOnPoint(XMSamplerMachineDialog->m_hWnd, point);
					XMSamplerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
#ifndef CONVERT_INTERNAL_MACHINES
			case MACH_SINE:
				if (PsychMachineDialog)
				{
					if (PsychMachineDialog->_pMachine != (Sine*)ma)
					{
						PsychMachineDialog->OnCancel();
						PsychMachineDialog = new CGearPsychOsc(&m_view);
						PsychMachineDialog->_pMachine = (Sine*)ma;
						PsychMachineDialog->Create();
						CenterWindowOnPoint(PsychMachineDialog, point);
						PsychMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					PsychMachineDialog = new CGearPsychOsc(&m_view);
					PsychMachineDialog->_pMachine = (Sine*)ma;
					PsychMachineDialog->Create();
					CenterWindowOnPoint(PsychMachineDialog, point);
					PsychMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_DIST:
				if (DistortionMachineDialog)
				{
					if (DistortionMachineDialog->_pMachine != (Distortion*)ma)
					{
						DistortionMachineDialog->OnCancel();
						DistortionMachineDialog = new CGearDistort(&m_view);
						DistortionMachineDialog->_pMachine = (Distortion*)ma;
						DistortionMachineDialog->Create();
						CenterWindowOnPoint(DistortionMachineDialog, point);
						DistortionMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					DistortionMachineDialog = new CGearDistort(&m_view);
					DistortionMachineDialog->_pMachine = (Distortion*)ma;
					DistortionMachineDialog->Create();
					CenterWindowOnPoint(DistortionMachineDialog, point);
					DistortionMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_DELAY:
				if (DelayMachineDialog)
				{
					if (DelayMachineDialog->_pMachine != (Delay*)ma)
					{
						DelayMachineDialog->OnCancel();
						DelayMachineDialog = new CGearDelay(&m_view);
						DelayMachineDialog->_pMachine = (Delay*)ma;
						DelayMachineDialog->SPT = &Global::_pSong->SamplesPerTick;
						DelayMachineDialog->Create();
						CenterWindowOnPoint(DelayMachineDialog, point);
						DelayMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					DelayMachineDialog = new CGearDelay(&m_view);
					DelayMachineDialog->_pMachine = (Delay*)ma;
					DelayMachineDialog->SPT = &Global::_pSong->SamplesPerTick;
					DelayMachineDialog->Create();
					CenterWindowOnPoint(DelayMachineDialog, point);
					DelayMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_2PFILTER:
				if (FilterMachineDialog)
				{
					if (FilterMachineDialog->_pMachine != (Filter2p*)ma)
					{
						FilterMachineDialog->OnCancel();
						FilterMachineDialog = new CGearfilter(&m_view);
						FilterMachineDialog->_pMachine = (Filter2p*)ma;
						FilterMachineDialog->Create();
						CenterWindowOnPoint(FilterMachineDialog, point);
						FilterMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					FilterMachineDialog = new CGearfilter(&m_view);
					FilterMachineDialog->_pMachine = (Filter2p*)ma;
					FilterMachineDialog->Create();
					CenterWindowOnPoint(FilterMachineDialog, point);
					FilterMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_GAIN:
				if (GainerMachineDialog)
				{
					if (GainerMachineDialog->_pMachine != (Gainer*)ma)
					{
						GainerMachineDialog->OnCancel();
						GainerMachineDialog = new CGearGainer(&m_view);
						GainerMachineDialog->_pMachine = (Gainer*)ma;
						GainerMachineDialog->Create();
						CenterWindowOnPoint(GainerMachineDialog, point);
						GainerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					GainerMachineDialog = new CGearGainer(&m_view);
					GainerMachineDialog->_pMachine = (Gainer*)ma;
					GainerMachineDialog->Create();
					CenterWindowOnPoint(GainerMachineDialog, point);
					GainerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_FLANGER:
				if (FlangerMachineDialog)
				{
					if (FlangerMachineDialog->_pMachine != (Flanger*)ma)
					{
						FlangerMachineDialog->OnCancel();
						FlangerMachineDialog = new CGearFlanger(&m_view);
						FlangerMachineDialog->_pMachine = (Flanger*)ma;
						FlangerMachineDialog->Create();
						CenterWindowOnPoint(FlangerMachineDialog, point);
						FlangerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					FlangerMachineDialog = new CGearFlanger(&m_view);
					FlangerMachineDialog->_pMachine = (Flanger*)ma;
					FlangerMachineDialog->Create();
					CenterWindowOnPoint(FlangerMachineDialog, point);
					FlangerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
#endif
			case MACH_PLUGIN:
				{
					m_pWndMac[tmac] = new CFrameMachine(tmac);
//					((CFrameMachine*)m_pWndMac[tmac])->SetParent((HWND)m_view);
					((CFrameMachine*)m_pWndMac[tmac])->_pActive = &m_bGUIOpen[tmac];
					((CFrameMachine*)m_pWndMac[tmac])->wndView = &m_view;
					((CFrameMachine*)m_pWndMac[tmac])->MachineIndex=_pSong->FindBusFromIndex(tmac);
					((CFrameMachine*)m_pWndMac[tmac])->Generate(m_hWnd);
					((CFrameMachine*)m_pWndMac[tmac])->SelectMachine(ma);
					TCHAR winname[32];
					_stprintf(winname,_T("%.2X : %s"),((CFrameMachine*)m_pWndMac[tmac])->MachineIndex
											 ,ma->_editName);
					((CFrameMachine*)m_pWndMac[tmac])->SetWindowText(winname);
					m_bGUIOpen[tmac] = true;
					CenterWindowOnPoint(m_pWndMac[tmac]->m_hWnd, point);
				}
				break;
			case MACH_VST:
			case MACH_VSTFX:
				{
				m_pWndMac[tmac] = new CVstEditorDlg(0);
				((CVstEditorDlg*)m_pWndMac[tmac])->pEditorActive(&m_bGUIOpen[tmac]);
				((CVstEditorDlg*)m_pWndMac[tmac])->pView(&m_view);
				((CVstEditorDlg*)m_pWndMac[tmac])->MachineIndex(_pSong->FindBusFromIndex(tmac));
				((CVstEditorDlg*)m_pWndMac[tmac])->pMachine((VSTPlugin*)ma);
				((CVstEditorDlg*)m_pWndMac[tmac])->CreateEx(*this);
				((VSTPlugin*)ma)->editorWnd = NULL;
				((VSTPlugin*)ma)->editorWnd = m_pWndMac[tmac];
				TCHAR winname[32];
				_stprintf(winname,_T("%.2X : %s"),((CVstEditorDlg*)m_pWndMac[tmac])->MachineIndex()
										,ma->_editName);

				((CVstEditorDlg*)m_pWndMac[tmac])->SetWindowText(winname);
				((CVstEditorDlg*)m_pWndMac[tmac])->ShowWindow(SW_SHOWNORMAL);
				m_bGUIOpen[tmac] = true;
				CenterWindowOnPoint(m_pWndMac[tmac]->m_hWnd, point);
			break;
				}
			}
		}
	}
}
void MachineView::CloseMacGui(const int mac,const bool closewiredialogs)
{
	if (closewiredialogs ) 
	{
		for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
		{
			if (m_WireDialog[i])
			{
				if ((m_WireDialog[i]->_pSrcMachine == _pSong->pMachine(mac)) ||
					(m_WireDialog[i]->_pDstMachine == _pSong->pMachine(mac)))
				{
					m_WireDialog[i]->OnCancel();
				}
			}
		}
	}
	if (_pSong->pMachine(mac))
	{
		switch (_pSong->pMachine(mac)->_type)
		{
			case MACH_MASTER:
				if (MasterMachineDialog) MasterMachineDialog->Cancel();
				break;
			case MACH_SAMPLER:
				if (SamplerMachineDialog) SamplerMachineDialog->OnCancel();
				break;
#ifndef CONVERT_INTERNAL_MACHINES
			case MACH_SINE:
				if (PsychMachineDialog) PsychMachineDialog->OnCancel();
				break;
			case MACH_DIST:
				if (DistortionMachineDialog) DistortionMachineDialog->OnCancel();
				break;
			case MACH_DELAY:
				if (DelayMachineDialog) DelayMachineDialog->OnCancel();
				break;
			case MACH_2PFILTER:
				if (FilterMachineDialog) FilterMachineDialog->OnCancel();
				break;
			case MACH_GAIN:
				if (GainerMachineDialog) GainerMachineDialog->OnCancel();
				break;
			case MACH_FLANGER:
				if (FlangerMachineDialog) FlangerMachineDialog->OnCancel();
				break;
#endif
			case MACH_PLUGIN:
			case MACH_VST:
			case MACH_VSTFX:
				if (isguiopen[mac])
				{
					m_pWndMac[mac]->DestroyWindow();
					isguiopen[mac] = false;
				}
				break;
			default:break;
		}
	}
}

void MachineView::CloseAllMacGuis()
{
	for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
	{
		if (m_WireDialog[i])
		{
			m_WireDialog[i]->OnCancel();
		}
	}
	for (int c=0; c<MAX_MACHINES; c++)
	{
		if ( _pSong->pMachine(c) ) CloseMacGui(c,false);
	}
}





