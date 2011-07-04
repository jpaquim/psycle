// FrameMachine.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "FrameMachine.h"
#include "NewVal.h"

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
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
	ON_COMMAND(ID_PARAMETERS_RESETPARAMETERS, OnParametersResetparameters)
	ON_COMMAND(ID_PARAMETERS_COMMAND, OnParametersCommand)
	ON_COMMAND(ID_MACHINE_ABOUTTHISMACHINE, OnMachineAboutthismachine)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFrameMachine message handlers

CFrameMachine::Generate()
{
	istweak=false;
	tweakpar=0;
	tweakbase=0;
	
	b_knob.LoadBitmap(IDB_KNOB);
	b_font.CreatePointFont(80,"Tahoma");

	UpdateWindow();
}

CFrameMachine::SelectMachine(int tmac)
{
	frame_index=tmac;
	machineRef=songRef->machine[tmac];
	me=true;
	
	// Get NumParameters
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");
	numParameters=bfxGetInfo()->numParameters;
	
	int ncol=bfxGetInfo()->numCols;
	parspercol=numParameters/ncol;

	SetWindowText(machineRef->editName);
	
	int const winh=parspercol*K_YSIZE;

	MoveWindow(machineRef->x,machineRef->y,134*ncol,48+winh,true);
	
	GetMenu()->GetSubMenu(0)->ModifyMenu(0,MF_BYPOSITION | MF_STRING,ID_PARAMETERS_COMMAND,bfxGetInfo()->Command);

	ShowWindow(SW_SHOWNORMAL);
	SetActiveWindow();
	UpdateWindow();
}

void CFrameMachine::OnClose() 
{
	// Hide the window on close, dont destroy it
	//	ShowWindow(SW_HIDE);
	guimatrix[frame_index]=false;

	CFrameWnd::OnClose();
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

	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");

	CDC memDC;

	memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(&b_knob);

	int y_knob=0;
	int x_knob=0;
	int knob_c=0;

	for(int c=0;c<numParameters;c++)
	{
		int const min_v=bfxGetInfo()->Parameters[c]->MinValue;
		int const max_v=bfxGetInfo()->Parameters[c]->MaxValue;
		int const amp_v=max_v-min_v;

		int const val_v=machineRef->mi->Vals[c];
		int const rel_v=val_v-min_v;

		int const frame=(K_NUMFRAMES*rel_v)/amp_v;
		int const xn=frame*K_XSIZE;

		char buffer[32];
		if(machineRef->mi->DescribeValue(buffer,c,val_v)==false)
		sprintf(buffer,"%d",val_v);

		dc.BitBlt(x_knob,y_knob,K_XSIZE,K_YSIZE,&memDC,xn,0,SRCCOPY);
	
		int nc;
		
		if(tweakpar==c && istweak)
		nc=0x00221100;
		else
		nc=0;
		
		dc.SetBkColor(0x00788D93+nc*2);
		dc.SetTextColor(0x00CCDDEE+nc);
		dc.ExtTextOut(K_XSIZE2+x_knob,y_knob,ETO_OPAQUE,CRect(K_XSIZE+x_knob,y_knob,cxsize+x_knob,y_knob+K_YSIZE2),CString(bfxGetInfo()->Parameters[c]->Name),NULL);
		
		dc.SetBkColor(0x00687D83+nc*2);
		dc.SetTextColor(0x0044EEFF+nc);
		dc.ExtTextOut(K_XSIZE2+x_knob,y_knob+K_YSIZE2,ETO_OPAQUE,CRect(K_XSIZE+x_knob,y_knob+K_YSIZE2,cxsize+x_knob,y_knob+K_YSIZE),CString(buffer),NULL);
		
		y_knob+=K_YSIZE;

		++knob_c;

		if(knob_c>=parspercol)
		{
			knob_c=0;
			x_knob+=cxsize;
			y_knob=0;
		}

	}
}

void CFrameMachine::OnLButtonDown(UINT nFlags, CPoint point) 
{
	tweakpar=(point.y/K_YSIZE)+((point.x/134)*parspercol);

	if(tweakpar>-1 && tweakpar<numParameters)
	{
		sourcepoint=point.y;
		tweakbase=machineRef->mi->Vals[tweakpar];
		istweak=true;
		SetCapture();
	}
	else
	{
		istweak=false;
	}

	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CFrameMachine::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(istweak)
	{
		GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");
		double const min_v=bfxGetInfo()->Parameters[tweakpar]->MinValue;
		double const max_v=bfxGetInfo()->Parameters[tweakpar]->MaxValue;

		double freak=(max_v-min_v)/63.0f;

		double nv=(double)(sourcepoint-point.y)*freak+(double)tweakbase;

		if(nv<min_v)nv=min_v;
		if(nv>max_v)nv=max_v;

		machineRef->mi->ParameterTweak(tweakpar,(int)nv);
		Invalidate(false);
	}

	CFrameWnd::OnMouseMove(nFlags, point);
}

void CFrameMachine::OnLButtonUp(UINT nFlags, CPoint point) 
{
	istweak=false;
	Invalidate(false);	
	ReleaseCapture();
	CFrameWnd::OnLButtonUp(nFlags, point);
}

void CFrameMachine::OnRButtonDown(UINT nFlags, CPoint point) 
{
	int const thispar=(point.y/K_YSIZE)+((point.x/134)*parspercol);

	if(tweakpar>-1 && tweakpar<numParameters)
	{		
		GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");
		int const min_v=bfxGetInfo()->Parameters[thispar]->MinValue;
		int const max_v=bfxGetInfo()->Parameters[thispar]->MaxValue;
		
		CNewVal dlg;
		dlg.m_Value=machineRef->mi->Vals[thispar];
		
		sprintf(
			dlg.Title,"Adjusting '%.2x:%s' in '%s' (Value range from %d to %d)"
			,thispar
			,bfxGetInfo()->Parameters[thispar]->Name
			,machineRef->editName
			,bfxGetInfo()->Parameters[thispar]->MinValue
			,bfxGetInfo()->Parameters[thispar]->MaxValue
			);
			
		dlg.DoModal();
		int nv=dlg.m_Value;
		if(nv<min_v)nv=min_v;
		if(nv>max_v)nv=max_v;

		machineRef->mi->ParameterTweak(thispar,nv);
		Invalidate(false);
	}
	
	CFrameWnd::OnRButtonDown(nFlags, point);
}

void CFrameMachine::OnParametersRandomparameters() 
{
	// Randomize controls
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");
	
	for(int c=0;c<bfxGetInfo()->numParameters;c++)
	{
	int minran=bfxGetInfo()->Parameters[c]->MinValue;
	int maxran=bfxGetInfo()->Parameters[c]->MaxValue;
	
	int dif=maxran-minran;

	float randsem=(float)rand()*0.000030517578125f;

	float roffset=randsem*(float)dif;

	machineRef->mi->ParameterTweak(c,minran+int(roffset));
	}

	Invalidate(false);
}

void CFrameMachine::OnParametersResetparameters() 
{
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");

	for(int c=0;c<bfxGetInfo()->numParameters;c++)
	{
	int dv=bfxGetInfo()->Parameters[c]->DefValue;
	machineRef->mi->ParameterTweak(c,dv);
	}

	if(istweak)
	istweak=false;

	Invalidate(false);
}

void CFrameMachine::OnParametersCommand() 
{
	songRef->micb.hWnd=m_hWnd;
	machineRef->mi->Command();
}

void CFrameMachine::OnMachineAboutthismachine() 
{
	if(istweak)
	istweak=false;

	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");

	MessageBox(
		"Machine coded by "+CString(bfxGetInfo()->Author),
		"About "+CString(bfxGetInfo()->Name)
		);
}

int CFrameMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
