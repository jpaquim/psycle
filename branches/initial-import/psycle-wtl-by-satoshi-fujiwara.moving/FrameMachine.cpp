/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.6 $
 */
// FrameMachine.cpp : implementation file
//

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
#include "FrameMachine.h"
#include "PsycleWTLView.h"
#include "NewVal.h"
#include "PresetsDlg.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "InputHandler.h"
#include "Helpers.h"
#include "MainFrm.h"
#include "Machine.h"
#include ".\framemachine.h"



//	extern CPsycleApp theApp;

#define K_XSIZE				28
#define K_YSIZE				28
#define K_NUMFRAMES			63

/////////////////////////////////////////////////////////////////////////////
// CFrameMachine

//IMPLEMENT_DYNCREATE(CFrameMachine, CFrameWnd)

CFrameMachine::CFrameMachine()
{
}

CFrameMachine::~CFrameMachine()
{
}



/////////////////////////////////////////////////////////////////////////////
// CFrameMachine message handlers

void CFrameMachine::Generate(const HWND hwndParent,const bool bChild)
{
	if(!bChild){
		CreateEx(hwndParent,NULL,WS_POPUPWINDOW | WS_CAPTION,WS_EX_CLIENTEDGE);
	} else {
		CreateEx(hwndParent,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ,WS_EX_CLIENTEDGE);
	}

	istweak = false;
	tweakpar = 0;
	tweakbase = 0;
	finetweak = false;
	ultrafinetweak = false;
	
	b_knob.LoadBitmap(IDB_KNOB);
	b_font.CreatePointFont(80,_T("MS UI Gothic"));
	UpdateWindow();
}

void CFrameMachine::SelectMachine(Machine* const  pMachine)
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
		CMenu _menu(::GetSubMenu(GetMenu(),0));
		
		_menu.ModifyMenu(0, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, CA2T(((Plugin*)_pMachine)->GetInfo()->Command));
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		numParameters = ((VSTPlugin*)_pMachine)->NumParameters();
		ncol = 1;
		while ( (numParameters/ncol)*K_YSIZE > ncol*cxsize ) ncol++;
	}
	parspercol = numParameters/ncol;
	if ( parspercol*ncol < numParameters) parspercol++;
	
	int const winh = parspercol * K_YSIZE;

	CWindow dsk(GetDesktopWindow());
	CRect rClient;
	dsk.GetClientRect((LPRECT)rClient);

	MoveWindow(rClient.Width() / 2 - (cxsize * ncol / 2), 
		rClient.Height() / 2 - ( 48 + winh) / 2, cxsize * ncol,
	9 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE) + winh, true);	
	
	ShowWindow(SW_SHOWNORMAL);
//	SetActiveWindow();
//	UpdateWindow();
}


///////////////////////////////////////////////////////////////////////
// PAINT GUI HERE
///////////////////////////////////////////////////////////////////////


LRESULT CFrameMachine::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	WTL::CPaintDC dc(m_hWnd); // device context for painting

	dc.SelectFont((HFONT)b_font);
	
	CRect rect;
	GetClientRect(&rect);
	int const cxsize=134;
	int const K_XSIZE2=K_XSIZE+8;
	int const K_YSIZE2=K_YSIZE/2;
	int hsp=0;

	CDC memDC;
	;

	memDC.CreateCompatibleDC((HDC)dc);
	CBitmapHandle oldbmp(memDC.SelectBitmap((HBITMAP)b_knob));

	int y_knob = 0;
	int x_knob = 0;
	int knob_c = 0;
	TCHAR parName[64];
	memset(parName,0,64);

	for (int c = 0; c < numParameters; c++)
	{
		TCHAR buffer[128];

		BOOL bDrawKnob = TRUE;
		int min_v;
		int max_v;
		int val_v;
		if ( _pMachine->_type == MACH_PLUGIN )
		{
			if (((Plugin*)_pMachine)->GetInfo()->Parameters[c]->Flags & MPF_STATE)
			{
				char _buf[128];
				min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MinValue;
				max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MaxValue;
				val_v = ((Plugin*)_pMachine)->GetInterface()->Vals[c];
				if (((Plugin*)_pMachine)->GetInterface()->DescribeValue(_buf,c,val_v) == false)
				{
					_stprintf(buffer,_T("%d"),val_v);
				} else {
					_tcscpy(buffer,CA2T(_buf));
				}
			}
			else
			{
				bDrawKnob = FALSE;
			}
			_tcscpy(parName, CA2T(((Plugin*)_pMachine)->GetInfo()->Parameters[c]->Name));
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			min_v = 0;
			max_v = VST_QUANTIZATION;
			val_v = f2i(((VSTPlugin*)_pMachine)->GetParameter(c)*VST_QUANTIZATION);
			memset(buffer,0,sizeof(buffer));

			if (((VSTPlugin*)_pMachine)->DescribeValue(c,buffer) == false)
			{
				_stprintf(buffer,_T("%d"),val_v);
			}
			((VSTPlugin*)_pMachine)->Dispatch(effGetParamName, c, 0, parName, 0);
		}

		if (bDrawKnob)
		{
			int const amp_v = max_v - min_v;
			int const rel_v = val_v - min_v;

			int const frame = (K_NUMFRAMES*rel_v)/amp_v;
			int const xn = frame*K_XSIZE;



			dc.BitBlt(x_knob,y_knob,K_XSIZE,K_YSIZE,(HDC)memDC,xn,0,SRCCOPY);
		
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
			dc.ExtTextOut(K_XSIZE2+x_knob, y_knob, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), parName);
			
			dc.SetBkColor(0x00687D83 + nc*2);
			dc.SetTextColor(0x0044EEFF + nc);
			dc.ExtTextOut(K_XSIZE2 + x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(K_XSIZE+x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), buffer);
		
		}
		else
		{
			dc.SetBkColor(0x00788D93);
			dc.SetTextColor(0x00CCDDEE);
			dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), parName);

			dc.SetBkColor(0x00687D83);
			dc.SetTextColor(0x0044EEFF);
			dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), _T(""));
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
			dc.ExtTextOut(x_knob, y_knob, ETO_OPAQUE, CRect(x_knob, y_knob, cxsize+x_knob, y_knob+K_YSIZE2), _T(""));

			dc.SetBkColor(0x00687D83);
			dc.SetTextColor(0x0044EEFF);
			dc.ExtTextOut(x_knob, y_knob+K_YSIZE2, ETO_OPAQUE, CRect(x_knob, y_knob+K_YSIZE2, cxsize+x_knob, y_knob+K_YSIZE), _T(""));

			y_knob += K_YSIZE;
		}
	}
	memDC.SelectBitmap((HBITMAP)oldbmp);
	memDC.DeleteDC();
	return 0;
}

LRESULT CFrameMachine::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint point(lParam);

	tweakpar = (point.y / K_YSIZE) + ((point.x / 134) * parspercol);
	if ((tweakpar > -1) && (tweakpar < numParameters))
	{
		sourcepoint = point.y;

		if ( _pMachine->_type == MACH_PLUGIN )
		{
			tweakbase = ((Plugin*)_pMachine)->GetInterface()->Vals[tweakpar];
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			tweakbase = int(((VSTPlugin*)_pMachine)->GetParameter(tweakpar)*VST_QUANTIZATION);
		}
		istweak = true;
		SetCapture();
	}
	else
	{
		istweak = false;
	}
	return 0;
}

LRESULT CFrameMachine::OnMouseMove(UINT /*uMsg*/, WPARAM nFlags, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint point(lParam);

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
			max_v = VST_QUANTIZATION;
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
				tweakbase=f2i(((VSTPlugin*)_pMachine)->GetParameter(tweakpar)*VST_QUANTIZATION);
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
				tweakbase=f2i(((VSTPlugin*)_pMachine)->GetParameter(tweakpar)*VST_QUANTIZATION);
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

		wndView->AddMacViewUndo();
		if ( _pMachine->_type == MACH_PLUGIN )
		{
			((Plugin*)_pMachine)->GetInterface()->ParameterTweak(tweakpar, (int)nv);
			// well, this isn't so hard... just put the twk record here
			if (Global::pConfig->_RecordTweaks)
			{
				if (Global::pConfig->_RecordMouseTweaksSmooth)
				{
					wndView->MousePatternTweakSlide(MachineIndex, tweakpar, ((int)nv)-min_v);
				}
				else
				{
					wndView->MousePatternTweak(MachineIndex, tweakpar, ((int)nv)-min_v);
				}
			}
		}
		else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
		{
			((VSTPlugin*)_pMachine)->SetParameter(tweakpar,(float)(nv/(float)VST_QUANTIZATION));
			// well, this isn't so hard... just put the twk record here
			if (Global::pConfig->_RecordTweaks)
			{
				if (Global::pConfig->_RecordMouseTweaksSmooth)
				{
					wndView->MousePatternTweakSlide(MachineIndex, tweakpar, (int)nv);
				}
				else
				{
					wndView->MousePatternTweak(MachineIndex, tweakpar, (int)nv);
				}
			}
		}

		Invalidate(false);
	}
	return 0;
}

LRESULT CFrameMachine::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	istweak = false;
	Invalidate(false);	
	ReleaseCapture();
	return 0;
}

LRESULT CFrameMachine::OnRButtonUp(UINT /*uMsg*/, WPARAM nFlags, LPARAM lParam, BOOL& bHandled)
{
	CPoint point(lParam);
	bHandled = FALSE;

	tweakpar = (point.y/K_YSIZE) + ((point.x/134)*parspercol);
	if ((tweakpar > -1) && (tweakpar < numParameters))
	{
		if (nFlags & MK_CONTROL)
		{
			Global::_pSong->SeqBus(MachineIndex);//Global::_pSong->FindBusFromIndex(MachineIndex);
			CMainFrame::GetInstance().UpdateComboGen(false);
			CComboBox cb2(CMainFrame::GetInstance().m_wndControl2.GetDlgItem(IDC_AUXSELECT));
			cb2.SetCurSel(AUX_PARAMS); // PARAMS
			Global::_pSong->AuxcolSelected(tweakpar);
			CMainFrame::GetInstance().UpdateComboIns();
		}
		else 
		{		
			int const thispar = (point.y/K_YSIZE) + ((point.x/134)*parspercol);
			int min_v;
			int max_v;
			TCHAR name[64];
			memset(name,0,64);
			CNewVal dlg;
			
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				min_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->MinValue;
				max_v = ((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->MaxValue;
				_tcscpy(name ,CA2T(((Plugin*)_pMachine)->GetInfo()->Parameters[thispar]->Name));
				dlg.m_Value = ((Plugin*)_pMachine)->GetInterface()->Vals[thispar];
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				min_v = 0;
				max_v = VST_QUANTIZATION;
				boost::scoped_array<char> _name(new char[sizeof(name)]); 
				((VSTPlugin*)_pMachine)->Dispatch(effGetParamName, thispar, 0, _name.get(), 0);
				_tcscpy(name,CA2T(_name.get()));
				dlg.m_Value = f2i(((VSTPlugin*)_pMachine)->GetParameter(thispar)*VST_QUANTIZATION);

			}
			
			_stprintf(
				dlg.Title, SF::CResourceString(IDS_MSG0027)
				,thispar
				,name
				,min_v
				,max_v);
			dlg.min = min_v;
			dlg.max = max_v;
			dlg.macindex = MachineIndex;
			dlg.paramindex = tweakpar;

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
			wndView->AddMacViewUndo();
			if ( _pMachine->_type == MACH_PLUGIN )
			{
				((Plugin*)_pMachine)->GetInterface()->ParameterTweak(thispar, nv);
			}
			else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
			{
				((VSTPlugin*)_pMachine)->SetParameter(thispar,(float)(nv/(float)VST_QUANTIZATION));
				SetFocus();
			}
			Invalidate(false);
		}
	}
	return 0;
}

LRESULT CFrameMachine::OnParametersResetparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (int c=0; c<((Plugin*)_pMachine)->GetInfo()->numParameters; c++)
	{
		int dv = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->DefValue;
		wndView->AddMacViewUndo();
		((Plugin*)_pMachine)->GetInterface()->ParameterTweak(c,dv);
	}

	if (istweak)
	{
		istweak = false;
	}
	Invalidate(false);
	return 0;
}

LRESULT CFrameMachine::OnParametersRandomparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Randomize controls
	for (int c=0; c<((Plugin*)_pMachine)->GetInfo()->numParameters; c++)
	{
		int minran = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MinValue;
		int maxran = ((Plugin*)_pMachine)->GetInfo()->Parameters[c]->MaxValue;

		int dif = maxran-minran;

		float randsem = (float)rand()*0.000030517578125f;

		float roffset = randsem*(float)dif;

		wndView->AddMacViewUndo();
		((Plugin*)_pMachine)->GetInterface()->ParameterTweak(c, minran+int(roffset));
	}

	Invalidate(false);
	return 0;
}

LRESULT CFrameMachine::OnParametersShowpreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CPresetsDlg dlg(_pMachine,this);
//dlg.SetParent(m_hWnd);
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CFrameMachine::OnMachineAboutthismachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (istweak)
	{
		istweak = false;
	}
// 20030804 WATANABE@NEUE-Engineering Modified
//	MessageBox(
//		_T("Machine coded by ")+CString(((Plugin*)_pMachine)->GetInfo()->Author),
//		_T("About ")+CString(((Plugin*)_pMachine)->GetInfo()->Name)
//		);
	MessageBox(_T("Machine coded by ")+CString(((Plugin*)_pMachine)->GetInfo()->Author),
				_T("About ") + CString(((Plugin*)_pMachine)->GetInfo()->Name)
				);

	return 0;
}

LRESULT CFrameMachine::OnMachineAboutpsygearv10(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコマンド ハンドラ コードを追加します。

	return 0;
}

LRESULT CFrameMachine::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	this->Invalidate(false);
	return 0;
}

LRESULT CFrameMachine::OnKeyDown(UINT /*uMsg*/, WPARAM nChar, LPARAM nFlags, BOOL& bHandled)
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
				if ( _pMachine->_mode == MACHMODE_GENERATOR ||Global::pConfig->_notesToEffects)
				{
					Global::pInputHandler->PlayNote(outnote,127,true,_pMachine);
				}
				else Global::pInputHandler->PlayNote(outnote,127,true,NULL);
			}
			break;

		case CT_Immediate:
			Global::pInputHandler->PerformCmd(cmd,bRepeat);
			break;
		}
	}

	SendMessage((HWND)(*wndView),WM_KEYDOWN,nChar, nFlags);
	bHandled = FALSE;
	return 0;
}

LRESULT CFrameMachine::OnKeyUp(UINT /*uMsg*/, WPARAM nChar, LPARAM nFlags, BOOL& /*bHandled*/)
{
	CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
	const int outnote = cmd.GetNote();
	if(outnote>=0)
	{
		if ( _pMachine->_mode == MACHMODE_GENERATOR ||Global::pConfig->_notesToEffects)
		{
			Global::pInputHandler->StopNote(outnote,true,_pMachine);
		}
		else Global::pInputHandler->StopNote(outnote,true,NULL);
	}

	SendMessage((HWND)(*wndView),WM_KEYUP,nChar, nFlags);
	return 0;
}

LRESULT CFrameMachine::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	SetTimer(2104 + MachineIndex,100,0);
	bHandled = FALSE;
	return 0;
}

LRESULT CFrameMachine::OnTimer(UINT /*uMsg*/, WPARAM nIDEvent, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ( nIDEvent == 2104 + MachineIndex )
	{
		Invalidate(false);
	}
	return 0;
}

LRESULT CFrameMachine::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ( _pActive != NULL ) *_pActive = false;
	KillTimer(2104+MachineIndex);
	return 0;
}

LRESULT CFrameMachine::OnMachineCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	((Plugin*)_pMachine)->GetCallback()->hWnd = m_hWnd;
	((Plugin*)_pMachine)->GetInterface()->Command();
	return 0;
}

//void CFrameMachine::OnFinalMessage(HWND hWnd)
//{
//	// TODO : ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
//	CFrameWindowImpl<CFrameMachine>::OnFinalMessage(hWnd);
//}

void CFrameMachine::OnFinalMessage(HWND hWnd)
{
	ThisClass::OnFinalMessage(hWnd);
	delete this;
}
