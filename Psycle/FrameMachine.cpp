// FrameMachine.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "Song.h"
#include "FrameMachine.h"
#include "NewVal.h"
#include "PresetsDlg.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "InputHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define K_XSIZE				28
#define K_YSIZE				28
#define K_NUMFRAMES			63

/////////////////////////////////////////////////////////////////////////////
// CFrameMachine

IMPLEMENT_DYNCREATE(CFrameMachine, CFrameWnd)

CFrameMachine::CFrameMachine()
{
}

CFrameMachine::~CFrameMachine()
{
}


BEGIN_MESSAGE_MAP(CFrameMachine, CFrameWnd)
	//{{AFX_MSG_MAP(CFrameMachine)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
	ON_COMMAND(ID_PARAMETERS_RESETPARAMETERS, OnParametersResetparameters)
	ON_COMMAND(ID_MACHINE_COMMAND, OnParametersCommand)
	ON_COMMAND(ID_MACHINE_ABOUTTHISMACHINE, OnMachineAboutthismachine)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_PARAMETERS_SHOWPRESET, OnParametersShowpreset)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFrameMachine message handlers

void CFrameMachine::Generate()
{
	istweak=false;
	tweakpar=0;
	tweakbase=0;
	finetweak=false;
	ultrafinetweak=false;
	
	b_knob.LoadBitmap(IDB_KNOB);
	b_font.CreatePointFont(80,"Tahoma");

	UpdateWindow();
}

void CFrameMachine::SelectMachine(Machine* pMachine)
{
	_pMachine = pMachine;
	me = true;
	int const cxsize=134;

	// Get NumParameters
	int ncol;
	if ( _pMachine->_type == MACH_PLUGIN )
	{
		numParameters = ((Plugin*)_pMachine)->GetInfo()->numParameters;
		ncol = ((Plugin*)_pMachine)->GetInfo()->numCols;
		GetMenu()->GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, ((Plugin*)_pMachine)->GetInfo()->Command);
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		numParameters = ((VSTPlugin*)_pMachine)->NumParameters();
		ncol = 1;
		while ( (numParameters/ncol)*K_YSIZE > ncol*cxsize ) ncol++;
	}
	parspercol = numParameters/ncol;
	if ( parspercol*ncol < numParameters) parspercol++;
	
	int const winh = parspercol*K_YSIZE;

	CWnd *dsk = GetDesktopWindow();
	CRect rClient;
	dsk->GetClientRect(&rClient);

	MoveWindow(rClient.Width()/2-(cxsize*ncol/2), rClient.Height()/2-(48+winh)/2, cxsize*ncol,
	9+GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE)+winh, true);	
	
	ShowWindow(SW_SHOWNORMAL);
//	SetActiveWindow();
//	UpdateWindow();
}

void CFrameMachine::OnDestroy() 
{
	if ( _pActive != NULL ) *_pActive = false;
	CFrameWnd::OnDestroy();
}

///////////////////////////////////////////////////////////////////////
// PAINT GUI HERE
///////////////////////////////////////////////////////////////////////

void CFrameMachine::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.SelectObject(&b_font);

	CRect rect;
	GetClientRect(&rect);
	int const cxsize=134;
	int const K_XSIZE2=K_XSIZE+8;
	int const K_YSIZE2=K_YSIZE/2;
	int hsp=0;

	CDC memDC;
	CBitmap* oldbmp;

	memDC.CreateCompatibleDC(&dc);
	oldbmp=memDC.SelectObject(&b_knob);

	int y_knob = 0;
	int x_knob = 0;
	int knob_c = 0;
	char parName[64];
	memset(parName,0,64);

	for (int c=0; c<numParameters; c++)
	{
		char buffer[32];

		BOOL bDrawKnob = TRUE;
		int min_v;
		int max_v;
		int val_v;
		if ( _pMachine->_type == MACH_PLUGIN )
		{
			if (((Plugin*)_pMachine)->GetInfo()->Parameters[c]->Flags & MPF_STATE)
			{
				min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MinValue;
				max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MaxValue;
				val_v = ((Plugin*)_pMachine)->GetInterface()->Vals[c];
				if (((Plugin*)_pMachine)->GetInterface()->DescribeValue(buffer,c,val_v) == false)
				{
					sprintf(buffer,"%d",val_v);
				}
			}
			else
			{
				bDrawKnob = FALSE;
			}
			strcpy(parName, ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->Name);
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			min_v = 0;
			max_v = 1000;
			val_v = (int)(((VSTPlugin*)_pMachine)->GetParameter(c)*1000.0f);
			memset(buffer,0,32);

			if (((VSTPlugin*)_pMachine)->DescribeValue(c,buffer) == false)
			{
				sprintf(buffer,"%d",val_v);
			}
			((VSTPlugin*)_pMachine)->Dispatch(effGetParamName, c, 0, parName, 0);
		}

		if (bDrawKnob)
		{
			int const amp_v = max_v - min_v;
			int const rel_v = val_v - min_v;

			int const frame = (K_NUMFRAMES*rel_v)/amp_v;
			int const xn = frame*K_XSIZE;



			dc.BitBlt(x_knob,y_knob,K_XSIZE,K_YSIZE,&memDC,xn,0,SRCCOPY);
		
			int nc;
			
			if ((tweakpar == c) && (istweak))
			{
				nc = 0x00221100;
			}
			else
			{
				nc = 0;
			}
			
			dc.SetBkColor(0x00788D93 + nc*2);
			dc.SetTextColor(0x00CCDDEE + nc);
			dc.ExtTextOut(K_XSIZE2+x_knob, y_knob, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), CString(parName), NULL);
			
			dc.SetBkColor(0x00687D83 + nc*2);
			dc.SetTextColor(0x0044EEFF + nc);
			dc.ExtTextOut(K_XSIZE2 + x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), CString(buffer), NULL);
		
		}
		else
		{
			dc.SetBkColor(0x00788D93);
			dc.SetTextColor(0x00CCDDEE);
			dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), CString(parName), NULL);

			dc.SetBkColor(0x00687D83);
			dc.SetTextColor(0x0044EEFF);
			dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), "", NULL);
		}
		y_knob += K_YSIZE;

		++knob_c;

		if (knob_c >= parspercol)
		{
			knob_c = 0;
			x_knob += cxsize;
			y_knob = 0;
		}
	}

	int exess;
	if ( _pMachine->_type == MACH_PLUGIN )
	{
		exess = parspercol*((Plugin*)_pMachine)->GetInfo()->numCols;
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		int ncol = 1;
		while ( (numParameters/ncol)*K_YSIZE > ncol*cxsize ) ncol++;

		exess = parspercol*ncol;
	}
	if ( exess > numParameters )
	{
		for (int c=numParameters; c<exess; c++)
		{
			dc.SetBkColor(0x00788D93);
			dc.SetTextColor(0x00CCDDEE);
			dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), "", NULL);

			dc.SetBkColor(0x00687D83);
			dc.SetTextColor(0x0044EEFF);
			dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), "", NULL);

			y_knob += K_YSIZE;
		}
	}
	memDC.SelectObject(oldbmp);
	memDC.DeleteDC();
}

void CFrameMachine::OnLButtonDown(UINT nFlags, CPoint point) 
{
	tweakpar = (point.y/K_YSIZE) + ((point.x/134)*parspercol);
	if ((tweakpar > -1) && (tweakpar < numParameters))
	{
		sourcepoint = point.y;

		if ( _pMachine->_type == MACH_PLUGIN )
		{
			tweakbase = ((Plugin*)_pMachine)->GetInterface()->Vals[tweakpar];
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			tweakbase = (int)(((VSTPlugin*)_pMachine)->GetParameter(tweakpar)*1000.0f);
		}
		istweak = true;
		SetCapture();
	}
	else
	{
		istweak = false;
	}
	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CFrameMachine::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (istweak)
	{
		int min_v;
		int max_v;
		if ( _pMachine->_type == MACH_PLUGIN )
		{
			min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MinValue;
			max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[tweakpar]->MaxValue;
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			min_v = 0;
			max_v = 1000;
		}



		if (( ultrafinetweak && !(nFlags & MK_SHIFT )) || //shift-key has been left.
			( !ultrafinetweak && (nFlags & MK_SHIFT))) //shift-key has just been pressed
		{
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				tweakbase=((Plugin*)_pMachine)->GetInterface()->Vals[tweakpar];
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				tweakbase=(int)(((VSTPlugin*)_pMachine)->GetParameter(tweakpar)*1000.0f);
			}
			sourcepoint=point.y;
			ultrafinetweak=!ultrafinetweak;
		}
		else if (( finetweak && !(nFlags & MK_CONTROL )) || //control-key has been left.
			( !finetweak && (nFlags & MK_CONTROL))) //control-key has just been pressed
		{
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				tweakbase=((Plugin*)_pMachine)->GetInterface()->Vals[tweakpar];
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				tweakbase=(int)(((VSTPlugin*)_pMachine)->GetParameter(tweakpar)*1000.0f);
			}
			sourcepoint=point.y;
			finetweak=!finetweak;
		}

		double freak;
		if ( ultrafinetweak) freak = 0.25f;
		else if ( finetweak ) freak = (max_v-min_v)/630.0f;
		else freak = (max_v-min_v)/63.0f;

		double nv = (double)(sourcepoint - point.y)*freak + (double)tweakbase;

		if (nv < min_v)
		{
			nv = min_v;
		}
		if (nv > max_v)
		{
			nv=max_v;
		}

		if ( _pMachine->_type == MACH_PLUGIN )
		{
			((Plugin*)_pMachine)->GetInterface()->ParameterTweak(tweakpar, (int)nv);
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			((VSTPlugin*)_pMachine)->SetParameter(tweakpar,(float)(nv/1000.0f));
		}

		Invalidate(false);
	}
	CFrameWnd::OnMouseMove(nFlags, point);
}

void CFrameMachine::OnLButtonUp(UINT nFlags, CPoint point) 
{
	istweak = false;
	Invalidate(false);	
	ReleaseCapture();
	CFrameWnd::OnLButtonUp(nFlags, point);
}

void CFrameMachine::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if((tweakpar > -1) && (tweakpar < numParameters))
	{		
		int const thispar = (point.y/K_YSIZE) + ((point.x/134)*parspercol);
		int min_v;
		int max_v;
		char name[64];
		memset(name,0,64);
		CNewVal dlg;

		if ( _pMachine->_type == MACH_PLUGIN )
		{
			min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->MinValue;
			max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->MaxValue;
			strcpy(name ,((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->Name);
			dlg.m_Value = ((Plugin*)_pMachine)->GetInterface()->Vals[thispar];
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			min_v = 0;
			max_v = 1000;
			((VSTPlugin*)_pMachine)->Dispatch(effGetParamName, thispar, 0, name, 0);
			dlg.m_Value = (int)(((VSTPlugin*)_pMachine)->GetParameter(thispar)*1000.0f);
		}
		
		sprintf(
			dlg.Title, "Param:'%.2x:%s' (Range from %d to %d)"
			,thispar
			,name
			,min_v
			,max_v);

		dlg.DoModal();
		int nv = dlg.m_Value;
		if (nv < min_v)
		{
			nv = min_v;
		}
		if (nv > max_v)
		{
			nv = max_v;
		}
		if ( _pMachine->_type == MACH_PLUGIN )
		{
			((Plugin*)_pMachine)->GetInterface()->ParameterTweak(thispar, (int)nv);
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			((VSTPlugin*)_pMachine)->SetParameter(thispar,(float)(nv/1000.0));
			SetFocus();
		}
		Invalidate(false);
	}
	CFrameWnd::OnRButtonDown(nFlags, point);
}

void CFrameMachine::OnParametersRandomparameters() 
{
	// Randomize controls
	for (int c=0; c<((Plugin*)_pMachine)->GetInfo()->numParameters; c++)
	{
		int minran = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MinValue;
		int maxran = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MaxValue;

		int dif = maxran-minran;

		float randsem = (float)rand()*0.000030517578125f;

		float roffset = randsem*(float)dif;

		((Plugin*)_pMachine)->GetInterface()->ParameterTweak(c, minran+int(roffset));
	}

	Invalidate(false);
}

void CFrameMachine::OnParametersResetparameters() 
{
	for (int c=0; c<((Plugin*)_pMachine)->GetInfo()->numParameters; c++)
	{
		int dv = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->DefValue;
		((Plugin*)_pMachine)->GetInterface()->ParameterTweak(c,dv);
	}

	if (istweak)
	{
		istweak = false;
	}
	Invalidate(false);
}

void CFrameMachine::OnParametersCommand() 
{
	((Plugin*)_pMachine)->GetCallback()->hWnd = m_hWnd;
	((Plugin*)_pMachine)->GetInterface()->Command();
}

void CFrameMachine::OnMachineAboutthismachine() 
{
	if (istweak)
	{
		istweak = false;
	}
	MessageBox(
		"Machine coded by "+CString(((Plugin*)_pMachine)->GetInfo()->Author),
		"About "+CString(((Plugin*)_pMachine)->GetInfo()->Name)
		);
}

int CFrameMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	return 0;
}

void CFrameMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// ignore repeats: nFlags&0x4000
	const BOOL bRepeat = nFlags&0x4000;
	CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
	if(!bRepeat && cmd.IsValid())
	{
		switch(cmd.GetType())
		{
		case CT_Note:
			{
				const int outnote = cmd.GetNote();
				Global::pInputHandler->PlayNote(outnote,127,true,_pMachine);
			}
			break;

		case CT_Immediate:
			Global::pInputHandler->PerformCmd(cmd,nRepCnt);
			break;
		}
	}

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
}

void CFrameMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
	const int outnote = cmd.GetNote();
	if(outnote>=0)
		Global::pInputHandler->StopNote(outnote,true,_pMachine);
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CFrameMachine::OnParametersShowpreset() 
{
	CPresetsDlg dlg;
	dlg._pMachine=_pMachine;
	dlg.m_wndFrame=this;
	dlg.DoModal();
}

void CFrameMachine::OnSetFocus(CWnd* pOldWnd) 
{
	CFrameWnd::OnSetFocus(pOldWnd);
	
	Invalidate(false);
}
