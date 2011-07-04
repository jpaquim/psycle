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
#include "Registry.h"
#include "Configuration.h"

#include "ChildView.h"
#include "Bitmap.cpp"
#include "direct.h"

// Included dialogs
#include "gearPsychosc.h"
#include "gearDistort.h"
#include "gearTracker.h"
#include "gearDelay.h"
#include "gearFilter.h"
#include "gearGainer.h"
#include "gearFlanger.h"
#include "gearVst.h"

#include "MainFrm.h"
#include "ADriverDialog.h"
#include "WireDlg.h"
#include "RecorderDlg.h"
#include "PtnDialog.h"
#include "MacProp.h"
#include "NewMachine.h"
#include "PatDlg.h"
#include "SaveDlg.h"
#include "GreetDialog.h"
#include "SkinDlg.h"
#include "SongpDlg.h"
#include "MasterDlg.h"
#include "OutputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Enviroment Initialization

// Modeless Dialogs
CRecorderDlg	*recordDlg;
CMainFrame		*pParentMain;

Bitmap mv_bg;
Bitmap pv_bg;
Bitmap sv_bg;

// Enviroment variables
int smac=-1;
int smacmode=0;
int wiresource=-1;
int wiredest=-1;
int wireSX=0;
int wireSY=0;
int wireDX=0;
int wireDY=0;

int newXSC=0;
int oldXSC=-1;
unsigned idletime=0;

int lbc=-1;

// Dsp prototypes
void machwork(int numSamples);
void vSwap(int *x,int *y);
void dspAdd(float *sSamples,float *dSamples, int numSamples, float vol);
void dspClear(float *sSamples,int numSamples);


//////////////////////////////////////////////////////////////////////
// Creating the only and the alone SONG object at the dialog open

psySong* SONG=new psySong;
Cubic Resampler; // Pointer to resampler object
FilterCoef OFilter;

//////////////////////////////////////////////////////////////////////
// Core Engine

int amount;
float *dSamples=new float[65536];
float sx=0;
float speed=1.0f;
int framed=0;
AudioDriver* pAudioDriver;

inline int f2i(double d)
{
	const double magic = 6755399441055744.0; // 2^51 + 2^52
	double tmp = (d-0.5) + magic;
	return *(int*) &tmp;
};

void WaveToFile(float *data,int ns)
{
	do
	{
		short pl=f2i(*data);
		++data;
		short pr=f2i(*data);
		++data;
		SONG->m_WaveFile.WriteStereoSample(pl,pr);
	}
	while(--ns);
}

float *myfunc(int& nsamples)
{
	float *pSamplesm=dSamples;
	int numSamplex=nsamples;
	do
	{
		if (numSamplex>255)
		{
			amount=256;
		}
		else
		{
			amount=numSamplex;
		}
		
		//////////////////////////////////////////////////////////////////////
		// Tick handler function
		
		if (SONG->PlayMode && amount>=SONG->TicksRemaining)
		{
			amount=SONG->TicksRemaining;
		}
		
		//////////////////////////////////////////////////////////////////////
		//	Song play
		
		SONG->ExecuteLine();
		
		//////////////////////////////////////////////////////////////////////
		// Processing plant
		
		// Clearing 'MASTER' machine samples buffers
		if (amount>0)
		{
			dspClear(SONG->machine[0]->samplesLeft,amount);
			dspClear(SONG->machine[0]->samplesRight,amount);
			
			SONG->PW_Work(SONG->machine[0]->samplesLeft,SONG->machine[0]->samplesRight,amount);
			
			machwork(amount);
			SONG->machine[0]->Work(pSamplesm,NULL,amount,0);
			
			if (SONG->m_WaveStage)
			{
				WaveToFile(pSamplesm,amount);
			}
			pSamplesm+=amount*2;
			numSamplex-=amount;
		}
		if (SONG->PlayMode)
		{
			SONG->TicksRemaining-=amount;
		}
	}
	while(numSamplex);
	
	return dSamples;
}

void vSwap(int *x,int *y)
{
	int const t=*x;
	*x=*y;
	*y=t;
}

void dspAdd(float *sSamples,float *dSamples, int numSamples, float vol)
{
	--sSamples;
	--dSamples;
	
	do
	{
		*++dSamples += *++sSamples*vol;
	}
	while(--numSamples);
}

void dspClear(float *sSamples,int numSamples)
{
	--sSamples;
	do
	{
		*++sSamples=0;
	}
	while(--numSamples);
}

//////////////////////////////////////////////////////////////////////
// Machine Routing Routine

void machwork(int numSamples)
{
	CSingleLock crit(&SONG->door,TRUE);
	idletime=0;
	
	__asm	rdtsc				// Read time stamp to EAX
		__asm	mov		idletime, eax
		
		// Reset all machines
		bool doloop;
	
	for(int c=1;c<MAX_MACHINES;c++)
	{
		if (SONG->Activemachine[c])SONG->machine[c]->Worked=false;
	}
	
	do{
		doloop=false;
		
		for(c=1;c<MAX_MACHINES;c++)
		{
			if (SONG->Activemachine[c]){
				psyGear *tmach=SONG->machine[c];
				
				if (!tmach->Worked)
				{ // Machine without working found
					bool doitm=true; // Let's supposse that it will work now...
					
					for(int w=0;w<MAX_CONNECTIONS;w++)
					{   // Checking if all stuff plugged worked.
						if (tmach->inCon[w] && !SONG->machine[tmach->inputSource[w]]->Worked)
						{	// Some of your parents not worked, so, you'll have to wait 
							// to the next loop!
							doloop=true;
							doitm=false;
						}
					}//Input conection bucle
					
					// Ok, everybody behind you worked, now, work you...
					if (doitm)
					{
						tmach->Work(tmach->samplesLeft,tmach->samplesRight,numSamples,SONG->SONGTRACKS);
						
						for (w=0;w<MAX_CONNECTIONS;w++)
						{
							// Now, feed your samples to your childs...
							if (tmach->conection[w])
							{
								int destIndex=tmach->outputDest[w];
								dspAdd(tmach->samplesRight,SONG->machine[destIndex]->samplesRight,numSamples,tmach->rVol*tmach->connectionVol[w]);
								dspAdd(tmach->samplesLeft,SONG->machine[destIndex]->samplesLeft,numSamples,tmach->lVol*tmach->connectionVol[w]);
							}// Exist connection
						}// Connection bucle
						
						tmach->Worked=true;
						
						dspClear(tmach->samplesLeft,amount);
						dspClear(tmach->samplesRight,amount);
						
					}// Machine that worked
				}// Machine without working found
			}// Machine is activated on the song
		}// For bucle
	}while(doloop);
	
	
	__asm	rdtsc				
		__asm	sub		eax, idletime	// Find the difference
		__asm	mov		idletime, eax
		
		unsigned cpudspz=numSamples*(SONG->CPUHZ/44100);
	SONG->cpuIdle=(idletime*1000)/cpudspz;
}

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
	seqFont.CreatePointFont(80,"Tahoma");
	patStep=1;
	uiplay=0;
	InitConfig();
	updateMode=0;
	DoTheVus=false;
	updatePar=0;
	viewMode=0;
	blockNTracks=0;
	blockNLines=0;
	isBlockCopied=false;
	blockSelected=false;
	
	showIBOnPatview=true;
	patBufferCopy=false;
	OFilter.MakeCoefs();
	
	blockLineStart=0;
	blockLineEnd=0;
	blockTrackStart=0;
	blockTrackEnd=0;
	
	_getcwd(m_appdir,_MAX_PATH);
	
	stuffbmp.LoadBitmap(IDB_STUFF);
	
	InitConfig();
	ReadConfig();
	UpdateConfig();
	
	// Creates a new song object. The application SONG.
	SONG->Reset();
	char buffer[_MAX_PATH];
	
	sprintf(buffer,"%s/plugins",m_appdir);
	SONG->ReadBuzzfx(buffer);
	_chdir(m_appdir);
	
	SONG->newSong();
	
	// Referencing the childView song pointer to the
	// Main SONG object [The application SONG]
	childSong=SONG;
	
	for(int c=0;c<256;c++)
	{
		FLATSIZES[c]=8;
	}
	
	// Initializing enviroment data
	seqOffset=0;
	seqStep=16;
	seqRow=0;
	SONG->seqBus=0;
	SONG->patTrack=0;
	patOffset=0;
	patRow=0;
	patLine=0;
	mcd_x=0;
	mcd_y=0;
	
	// Silent audio driver as default
	//
	pAudioDriver = new AudioDriver;
}

CChildView::~CChildView()
{
	if (recordDlg && ::IsWindow(recordDlg->m_hWnd))
		recordDlg->DestroyWindow();
	
	WriteConfig();
	SONG->Stop();
	pAudioDriver->Enable(false);
	Sleep(LOCK_LATENCY);
	pParentMain->CloseAllMacGuis();
	delete SONG;	// Delete the song created
	delete dSamples;
	delete pAudioDriver;
}

BEGIN_MESSAGE_MAP(CChildView,CWnd )
//{{AFX_MSG_MAP(CChildView)
ON_WM_PAINT()
ON_COMMAND(ID_CONFIGURATION_AUDIODRIVER, OnConfigurationAudiodriver)
ON_WM_LBUTTONDOWN()
ON_WM_RBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDBLCLK()
ON_COMMAND(ID_HELP_PSYCLEENVIROMENTINFO, OnHelpPsycleenviromentinfo)
ON_WM_DESTROY()
ON_COMMAND(ID_APP_EXIT, OnAppExit)
ON_COMMAND(ID_MACHINEVIEW, OnMachineview)
ON_WM_KEYDOWN()
ON_COMMAND(ID_PATTERNVIEW, OnPatternView)	
ON_COMMAND(ID_BARPLAY, OnBarplay)
ON_COMMAND(ID_BARSTOP, OnBarstop)
ON_WM_TIMER()
ON_COMMAND(ID_SHOWIED, OnShowied)
ON_COMMAND(ID_RECORDB, OnRecordb)
ON_COMMAND(ID_FILE_SAVESONG, OnFileSavesong)
ON_COMMAND(ID_FILE_LOADSONG, OnFileLoadsong)
ON_COMMAND(ID_HELP_SALUDOS, OnHelpSaludos)
ON_COMMAND(ID_CONFIGURATION_SETTINGS_BACKGROUNDSKIN, OnConfigurationSettingsBackgroundskin)
ON_UPDATE_COMMAND_UI(ID_PATTERNVIEW, OnUpdatePatternView)
ON_UPDATE_COMMAND_UI(ID_MACHINEVIEW, OnUpdateMachineview)
ON_UPDATE_COMMAND_UI(ID_BARPLAY, OnUpdateBarplay)
ON_COMMAND(ID_FILE_SONGPROPERTIES, OnFileSongproperties)
ON_COMMAND(ID_VIEW_INSTRUMENTEDITOR, OnViewInstrumenteditor)
ON_COMMAND(ID_FILE_NEW, OnFileNew)
ON_COMMAND(ID_NEWMACHINE, OnNewmachine)
ON_COMMAND(ID_CONFIGURATION_KEYBOARDLAYOUT_FRENCH, OnConfigurationKeyboardlayoutFrench)
ON_COMMAND(ID_CONFIGURATION_KEYBOARDLAYOUT_STANDARD, OnConfigurationKeyboardlayoutStandard)
ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_KEYBOARDLAYOUT_FRENCH, OnUpdateConfigurationKeyboardlayoutFrench)
ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_KEYBOARDLAYOUT_STANDARD, OnUpdateConfigurationKeyboardlayoutStandard)
ON_UPDATE_COMMAND_UI(ID_RECORDB, OnUpdateRecordb)
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
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);
	
	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this);
	CRect rClient;
	
	bool do_skin=false;
	if (appConfig.mv_skin && viewMode==0)do_skin=true;
	if (appConfig.pv_skin && viewMode==2)do_skin=true;
	
	if (updateMode==0)
	{
		if (do_skin)
		{
			Draw_BackSkin();
		}
		else
		{
			GetClientRect(&rClient);
			
			switch(viewMode)
			{
			case 0:SetBkColor(dc,appConfig.mv_colour);break;
			case 2:SetBkColor(dc,appConfig.pv_colour);break;
			}
			
			dc.ExtTextOut(0,0,ETO_OPAQUE,&rClient,0,0,0);
		}
	}
	
	//////////////////////////////////////////////////////////////////
	// Machine view paint handler
	
	if (viewMode==0)
	{
		switch (updateMode)
		{
		case 0:
			DrawMachineEditor(&dc);
			break;
		case 1:
			psyGear *ptMac=SONG->machine[updatePar];
			DrawMachine(ptMac->x,ptMac->y,ptMac->editName,&dc,ptMac->panning,ptMac->type);
			updateMode=0;
			break;
		}
	}
	
	//////////////////////////////////////////////////////////////////
	// Pattern view paint handler
	
	if (viewMode==2)
	{
		DrawPatEditor(&dc);
		updateMode=0;
	}
	
}

void CChildView::OnConfigurationAudiodriver() 
{
	ConfigAudio(true);
}

void CChildView::ConfigAudio(
	bool forceConfig) 
{
	DWORD size;
	DWORD type;
	Registry reg;
	COutputDlg dlg;
	bool configured = false;
	
	if (pAudioDriver != NULL)
	{
		delete pAudioDriver;
		pAudioDriver = NULL;
	}
	
	dlg.m_driverIndex = 0;
	if (reg.OpenRootKey(HKEY_CURRENT_USER, SOFTWARE_ROOT_KEY) == ERROR_SUCCESS)
	{
		if (reg.OpenKey(CONFIG_KEY) == ERROR_SUCCESS)
		{
			size = sizeof(dlg.m_driverIndex);
			if (reg.QueryValue("OutputDriver", &type, (BYTE*)&dlg.m_driverIndex, &size) == ERROR_SUCCESS)
			{
				configured = true;
			}
			reg.CloseKey();
		}
		reg.CloseRootKey();
	}
	
	if (forceConfig || !configured)
	{
		dlg.DoModal();
		
		if (reg.OpenRootKey(HKEY_CURRENT_USER, SOFTWARE_ROOT_KEY) == ERROR_SUCCESS)
		{
			if (reg.OpenKey(CONFIG_KEY) == ERROR_SUCCESS)
			{
				reg.SetValue("OutputDriver", REG_DWORD, (BYTE*)&dlg.m_driverIndex, sizeof(dlg.m_driverIndex));
				reg.CloseKey();
			}
			reg.CloseRootKey();
		}

	}
	
	pAudioDriver = dlg.m_pDrivers[dlg.m_driverIndex];

	if (!pAudioDriver->Initialized())
	{
		pAudioDriver->Initialize(m_hWnd, myfunc);
	}
	if (!pAudioDriver->Configured())
	{
		pAudioDriver->Configure();
	}
}

//////////////////////////////////////////////////////////////////////
// Timer initialization

void CChildView::InitTimer()
{
	if (!SetTimer(31,10,NULL))MessageBox("Couldn't initializate timer","Psycle ERROR!", MB_ICONERROR);
}

//////////////////////////////////////////////////////////////////////
// Draw Machine view. Full update.

void CChildView::amosDraw(CPaintDC *devc, int oX,int oY,int dX,int dY)
{
	if (oX==dX)
	{
		oX++;
	}
	if (oY==dY)
	{
		oY++;
	}
	devc->MoveTo(oX,oY);
	devc->LineTo(dX,dY);			
}

//////////////////////////////////////////////////////////////////////
// Include GDI sources

#include "machineview.cpp"
#include "patview.cpp"

//////////////////////////////////////////////////////////////////////
// User-Mouse Responses

void CChildView::OnRButtonDown( UINT nFlags, CPoint point )
{	
	if (viewMode==0)
	{
		// Check for right pressed connection
		int propMac=GetMachine(point);
		
		if (propMac>0)
		{
			// Shows machine properties dialog
			CMacProp dlg;
			dlg.machineRef=SONG->machine[propMac];
			dlg.songRef=SONG;
			dlg.thisMac=propMac;
			
			if (dlg.DoModal()==IDOK)
			{
				sprintf(dlg.songRef->machine[propMac]->editName,dlg.txt);
				pParentMain->StatusBarText(dlg.txt);
				pParentMain->UpdateEnvInfo();
				UpdateSBrowseDlg();
			}
			
			if (dlg.deleted)
			{
				pParentMain->CloseMacGui(propMac);
				SONG->DestroyMachine(propMac);
				pParentMain->UpdateEnvInfo();
				UpdateSBrowseDlg();
			}
			
			Repaint();
		}
	}
	
	if (viewMode==2)
	{
		CPatDlg dlg2;
		dlg2.songRef=SONG;
		dlg2.DoModal();
		updateMode=0;
		Invalidate(false);
	}	
}

void CChildView::OnLButtonDown( UINT nFlags, CPoint point )
{
	
	SetCapture();
	
	switch(viewMode)
	{
		int c;
		
	case 0: // User is in machine view mode
		smac=-1;
		smacmode=0;
		
		wiresource=-1;
		wiredest=-1;
		
		if (nFlags==5)
		{
			wiresource=GetMachine(point);
			
			if (wiresource>0)
			{
				wireSX=SONG->machine[wiresource]->x+74;
				wireSY=SONG->machine[wiresource]->y+24;
			}
			else
			{
				wiresource=-1;
			}
		}
		
		if (nFlags==1)
		{
			for (c=0;c<MAX_MACHINES;c++)
			{
				if (SONG->Activemachine[c])
				{
					int x1=SONG->machine[c]->x;
					int y1=SONG->machine[c]->y;
					int x2=SONG->machine[c]->x+148;
					int y2=SONG->machine[c]->y+48;
					
					if (point.x>x1 && point.x<x2 && point.y>y1 && point.y<y2)
					{
						smac=c;
						mcd_x=SONG->machine[c]->x-point.x;
						mcd_y=SONG->machine[c]->y-point.y;
						if (point.y>y1+34 && point.y<y1+46)
						{
							smacmode=1;
						}
					}
				}
			}
		}// No Shift
		
		OnMouseMove(nFlags,point);
		
		// Check for pressed connection
		
		for(c=0;c<MAX_MACHINES;c++)
		{
			if (SONG->Activemachine[c])
			{
				psyGear *tmac=SONG->machine[c];
				
				for (int w=0;w<MAX_CONNECTIONS;w++)
				{
					if (tmac->conection[w])
					{
						int xt=tmac->connectionPoint[w].x;
						int yt=tmac->connectionPoint[w].y;
						
						if (point.x>xt && point.x<xt+20 && point.y>yt && point.y<yt+20)
						{
							CWireDlg dlg;
							dlg.wireIndex=w;
							dlg.songRef=SONG;
							dlg.isrcMac=c;
							dlg.macRef=tmac;
							sprintf(dlg.destName,"%s",SONG->machine[tmac->outputDest[w]]->editName);
							dlg.DoModal();
							Repaint();
						}
					}
				}
			}
		}
		break;
		
	case 2:
		
		CRect rClient;
		GetClientRect(&rClient);
		int CW = rClient.Width();
		int WIDEROWS = CW/111;
		int snt = SONG->SONGTRACKS;
		if (--WIDEROWS < 2)
		{
			WIDEROWS=2;
		}
		
		int to = SONG->patTrack-(WIDEROWS/2);
		
		if (to > snt-WIDEROWS)
		{
			to = snt-WIDEROWS;
		}
		if (to < 0)
		{
			to=0;
		}
		
		int ttm = ((point.x-44)/111)+to;
		
		if (ttm < 0)
		{
			ttm = 0;
		}
		if (ttm >= MAX_TRACKS)
		{
			ttm=MAX_TRACKS-1;
		}
		SONG->track_st[ttm] = !SONG->track_st[ttm];
		updateMode = 1;
		Invalidate(false);
		break;
		
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

void CChildView::OnLButtonUp( UINT nFlags, CPoint point )
{
	ReleaseCapture();
	
	switch (viewMode)
	{
	case 0: // User is in machine view mode
		
		if (wiresource != -1)
		{
			wiredest=GetMachine(point);
			if (wiredest!=-1 && wiredest!=wiresource) 
			{
				if (!SONG->InsertConnection(wiresource,wiredest))
				{
					MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
				}
			}
			wiresource=-1;
			Repaint();
		}
		break;
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

void CChildView::OnMouseMove( UINT nFlags, CPoint point )
{
	switch(viewMode)
	{
	case 0: // User is in machine view mode
		
		if (smac>-1 && nFlags==1)
		{
			if (smacmode==0)
			{
				SONG->machine[smac]->x = point.x+mcd_x;
				SONG->machine[smac]->y = point.y+mcd_y;
				updateMode = 0;
				char buffer[64];
				sprintf(buffer,"%s (%d,%d)",SONG->machine[smac]->editName,SONG->machine[smac]->x,SONG->machine[smac]->y);
				pParentMain->StatusBarText(buffer);
			}
			else if (smacmode==1)
			{
				int newpan = point.x-SONG->machine[smac]->x-9;
				if (newpan < 0)
				{
					newpan = 0;
				}
				if (newpan > 128)
				{
					newpan = 128;
				}
				SONG->machine[smac]->changePan(newpan);
				char buffer[64];
				
				if (newpan!=64)
				{
					sprintf(buffer,"%s pan: %.0f%% Left / %.0f%% Right",SONG->machine[smac]->editName,100.0f-((float)newpan*0.78125f),(float)newpan*0.78125f);
				}
				else
				{
					sprintf(buffer,"%s pan: Center",SONG->machine[smac]->editName);
				}
				
				pParentMain->StatusBarText(buffer);
				updateMode = 1;
				updatePar = smac;
			}
			Invalidate(false);
		}
		
		if ((nFlags == 5) && (wiresource != -1))
		{
			wireDX = point.x;
			wireDY = point.y;
			Invalidate(false);
		}
		break;
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

#include "dblClickHandler.cpp"

int CChildView::GetMachine(CPoint point)
{
	int tmac=-1;
	
	for(int c=0;c<MAX_MACHINES;c++)
	{
		if (SONG->Activemachine[c])
		{
			int x1=SONG->machine[c]->x;
			int y1=SONG->machine[c]->y;
			int x2=SONG->machine[c]->x+148;
			int y2=SONG->machine[c]->y+48;
			
			if (point.x>x1 && point.x<x2 && point.y>y1 && point.y<y2)
			{
				tmac=c;
			}
		}
	}
	
	return tmac;
}

void CChildView::OnHelpPsycleenviromentinfo() 
{
	pParentMain->ShowPerformanceDlg();
}

//////////////////////////////////////////////////////////////////////
// Put exit destroying code here...

void CChildView::OnDestroy()
{
	if (pAudioDriver->Initialized())
	{
		pAudioDriver->Reset();
	}
}

//////////////////////////////////////////////////////////////////////
// Include psycore song structure body

void CChildView::OnAppExit() 
{
	pParentMain->ClosePsycle();
}

void CChildView::OnMachineview() 
{
	if (viewMode!=0)
	{
		viewMode=0;
		Repaint();
	}
}

#include "psy.cpp"

void CChildView::OnPatternView() 
{
	if (viewMode!=2)
	{
		viewMode=2;
		
		if (showIBOnPatview)
		{
			//		ShowIBrowseDlg();
			GetParent()->SetActiveWindow();
		}
		Repaint();
	}
	
}

//#include "winuser.h"
#include "keybhandler.cpp"

void CChildView::ShowPatternDlg(void)
{
	CPatDlg dlg;
	dlg.songRef=SONG;
	
	if (dlg.DoModal()==IDOK)
	{
		Invalidate(false);
	}
	
}

void CChildView::OnBarstop()
{
	uiplay=0;
	SONG->Stop();
}

void CChildView::OnBarplay() 
{
	SONG->Play(0);
}

//////////////////////////////////////////////////////////////////////
// Timer handler

void CChildView::OnTimer( UINT nIDEvent )
{
	if (nIDEvent==31)
	{
		bool const twk=SONG->Tweaker;
		
		if (twk)
		{
			for(int c=0;c<MAX_MACHINES;c++)
			{
				if (pParentMain->isguiopen[c])
				{
					pParentMain->m_pWndMac[c]->Invalidate(false);
				}
			}
			
			SONG->Tweaker=false;
		}
		
		pParentMain->UpdateVumeters(
			SONG->machine[0]->LMAX,SONG->machine[0]->RMAX,
			appConfig.vu1,
			appConfig.vu2,
			appConfig.vu3,
			SONG->machine[0]->clip
			);
		
		if (viewMode==0)
		{
			CClientDC dc(this);
			DrawMachineVumeters(&dc);
		}
		
		if (SONG->PlayMode)
		{
			if (SONG->LineChanged)
			{
				char buf[80];
				sprintf(buf,"[Playing] Pos: %.2X   Pat: %.2X   Lin: %d",
					SONG->playPosition,
					SONG->playPattern,
					SONG->LineCounter);
				
				pParentMain->StatusBarText(buf);
				if (viewMode==2)
				{
					SONG->LineChanged=false;
					updateMode=5;
					Invalidate(false);
				}
			}	
		}
		
		
	}
}

void CChildView::OnShowied()
{
	// Show Instrument Editor Dialog
	//CInstrumentEditor dlg;
	//dlg.songRef=SONG;
	//dlg.DoModal();
	//UpdateIBrowseDlg();
	//Repaint();
	pParentMain->ShowInstrumentEditor();
}

void CChildView::ShowSplash() 
{
	
}

void CChildView::OnRecordb() 
{
	if (SONG->m_WaveStage==0)
	{
		static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
		
		CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
		dlg.DoModal();
		SONG->StartRecord(dlg.GetFileName().GetBuffer(4));
	}
	else
	{
		SONG->StopRecord();
	}
}

//////////////////////////////////////////////////////////////////////
// Save Song Function

void CChildView::OnFileSavesong() 
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	sprintf(szFile,"%s.psy",SONG->Name);
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Songs\0*.psy\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;
	
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn)==TRUE)
	{
		CSaveDlg dlg;
		dlg.songRef=SONG;
		sprintf(dlg.szFile,szFile);
		dlg.SaveSong();
	}
	Repaint();
}

void CChildView::UpdateIBrowseDlg()
{
	pParentMain->UpdateComboIns();
}

void CChildView::UpdateSBrowseDlg()
{
	pParentMain->UpdateComboGen();
}

void CChildView::OnFileLoadsong() 
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	sprintf(szFile,"%s.psy",SONG->Name);
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Songs\0*.psy\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		pParentMain->CloseAllMacGuis();
		SONG->Stop();
		Sleep(LOCK_LATENCY);
		pAudioDriver->Enable(false);
		Sleep(LOCK_LATENCY);
		
		CSaveDlg dlg;
		dlg.songRef=SONG;
		sprintf(dlg.szFile,szFile);
		dlg.LoadSong();
		
		SONG->seqBus=0;
		seqRow=0;
		pAudioDriver->Enable(true);
		Repaint();
		UpdateIBrowseDlg();
		UpdateSBrowseDlg();
		pParentMain->SetAppSongBpm(0);
		pParentMain->WaveEditorBackUpdate();
		pParentMain->UpdateSequencer();
		pParentMain->UpdatePlayOrder(false);
	}
}

void CChildView::OnHelpSaludos() 
{
	CGreetDialog dlg;
	dlg.DoModal();
	Repaint();
}

int CChildView::SongIncBpm(int x)
{
	SONG->BeatsPerMin+=x;
	
	if (SONG->BeatsPerMin<33)SONG->BeatsPerMin=33;
	if (SONG->BeatsPerMin>999)SONG->BeatsPerMin=999;
	
	SONG->SetBPM(SONG->BeatsPerMin,44100);
	
	return SONG->BeatsPerMin;
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
	pParentMain->songRef=SONG;
}

//////////////////////////////////////////////////////////////////////
// Configuration members

void CChildView::OnConfigurationSettingsBackgroundskin() 
{
	CSkinDlg dlg;
	
	// Transfering config data to dlg members
	sprintf(dlg.mv_pathbuf,appConfig.mv_skin_path);
	sprintf(dlg.pv_pathbuf,appConfig.pv_skin_path);
	dlg.mv_check=appConfig.mv_skin;
	dlg.pv_check=appConfig.pv_skin;
	dlg.mv_c=appConfig.mv_colour;
	dlg.pv_c=appConfig.pv_colour;
	dlg.gen_c=appConfig.gen_colour;
	dlg.eff_c=appConfig.eff_colour;
	dlg.mas_c=appConfig.mas_colour;
	dlg.plg_c=appConfig.plg_colour;
	
	dlg.vub_c=appConfig.vu1;
	dlg.vug_c=appConfig.vu2;
	dlg.vuc_c=appConfig.vu3;
	
	
	if (dlg.DoModal()==IDOK)
	{
		sprintf(appConfig.mv_skin_path,dlg.mv_pathbuf);
		sprintf(appConfig.pv_skin_path,dlg.pv_pathbuf);
		appConfig.mv_skin=dlg.mv_check;
		appConfig.pv_skin=dlg.pv_check;
		appConfig.mv_colour=dlg.mv_c;
		appConfig.pv_colour=dlg.pv_c;
		appConfig.gen_colour=dlg.gen_c;
		appConfig.eff_colour=dlg.eff_c;
		appConfig.mas_colour=dlg.mas_c;
		appConfig.plg_colour=dlg.plg_c;
		appConfig.vu1=dlg.vub_c;
		appConfig.vu2=dlg.vug_c;
		appConfig.vu3=dlg.vuc_c;
		
		UpdateConfig();
	}
	
	Repaint();
}

void CChildView::UpdateConfig()
{
	if (appConfig.mv_skin)mv_bg.Load(appConfig.mv_skin_path);
	if (appConfig.pv_skin)pv_bg.Load(appConfig.pv_skin_path);
}

CChildView::InitConfig()
{
	appConfig.KEYBOARDMODE=0; // Set to Standard
	appConfig.mv_skin=false;
	appConfig.pv_skin=false;
	sprintf(appConfig.mv_skin_path,"");
	sprintf(appConfig.pv_skin_path,"");
	appConfig.mv_colour=	0x0077AA99;
	appConfig.pv_colour=	0x00AADDCC;
	appConfig.gen_colour=	0x00BBCCAA;
	appConfig.eff_colour=	0x0073BA98;
	appConfig.mas_colour=	0x00BBBBBB;
	appConfig.plg_colour=	0x00AA9977;
	appConfig.vu1=			0x0000FF00;
	appConfig.vu2=			0x00000000;
	appConfig.vu3=			0x000000FF;
}


CChildView::ReadConfig()
{
	char buffer[_MAX_PATH];
	sprintf(buffer,"%s/psy.cfg",m_appdir);
	
	FILE* hFile;
	
	hFile=fopen(buffer,"rb");
	
	if (hFile!=NULL)
	{
		fread(&appConfig,sizeof(PSYCONFIG),1,hFile);
		fclose(hFile);
	}
}

CChildView::WriteConfig()
{
	char buffer[_MAX_PATH];
	sprintf(buffer,"%s/psy.cfg",m_appdir);
	
	FILE* hFile;
	
	hFile=fopen(buffer,"wb");
	
	if (hFile!=NULL)
	{
		fwrite(&appConfig,sizeof(PSYCONFIG),1,hFile);
		fclose(hFile);
	}
}

//////////////////////////////////////////////////////////////////////
// Tool bar button check updates

void CChildView::OnUpdatePatternView(CCmdUI* pCmdUI) 
{
	if (viewMode==2)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CChildView::OnUpdateMachineview(CCmdUI* pCmdUI) 
{
	if (viewMode==0)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CChildView::OnUpdateBarplay(CCmdUI* pCmdUI) 
{
	if (SONG->PlayMode)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

CChildView::UpdateChecks(){}

void CChildView::OnFileSongproperties() 
{
	CSongpDlg dlg;
	dlg.songRef=SONG;
	dlg.DoModal();
	Repaint();
}

CChildView::Repaint()
{
	updateMode=0;
	Invalidate(false);
}

void CChildView::OnViewInstrumenteditor() {OnShowied();}

CChildView::SetPatStep(int stp)
{
	patStep=stp;
	if (viewMode==1)
	{
		Repaint();
	}
}

void CChildView::OnFileNew() 
{
	if (MessageBox("Are your sure?","New song",MB_YESNO | MB_ICONWARNING)==IDYES)
	{
		pParentMain->CloseAllMacGuis();
		SONG->Stop();
		Sleep(LOCK_LATENCY);
		pAudioDriver->Enable(false);
		Sleep(LOCK_LATENCY);
		SONG->newSong();
		SONG->seqBus=0;
		seqRow=0;
		pAudioDriver->Enable(true);
		Repaint();
		UpdateIBrowseDlg();
		UpdateSBrowseDlg();
		pParentMain->SetAppSongBpm(0);
		pParentMain->UpdatePlayOrder(false);
		pParentMain->WaveEditorBackUpdate();
		pParentMain->UpdateSequencer();
	}
}

void CChildView::OnNewmachine() 
{
	// Show new machine dialog
	CNewMachine dlg;
	dlg.songRef=SONG;
	dlg.xLoc=rand()/64;
	dlg.yLoc=rand()/80;
	dlg.Outputdll=-1;
	
	if (dlg.DoModal()==IDOK)
	{
		// Stop driver to handle possible conflicts
		// between threads.
		
		pAudioDriver->Enable(false);
		
		if (!SONG->CreateMachine(dlg.Outputmachine,dlg.xLoc,dlg.yLoc,-1,dlg.Outputdll))
		{
			MessageBox("Error!","Machine Creation Failed",MB_OK);
		}
		else if (dlg.OutBus)
		{
			SONG->seqBus=SONG->GetFreeBus();
			SONG->busMachine[SONG->seqBus]=lbc;
			UpdateSBrowseDlg();
		}
		
		// Restarting the driver...
		pParentMain->UpdateEnvInfo();
		pAudioDriver->Enable(true);
	}
	
	updateMode=0;
	Invalidate(false);	
}


void CChildView::OnConfigurationKeyboardlayoutStandard() 
{
	appConfig.KEYBOARDMODE=0;	
}

void CChildView::OnConfigurationKeyboardlayoutFrench() 
{
	appConfig.KEYBOARDMODE=1;	
}

void CChildView::OnUpdateConfigurationKeyboardlayoutStandard(CCmdUI* pCmdUI) 
{
	if (appConfig.KEYBOARDMODE==0)
	{
		pCmdUI->SetCheck(true);
	}
	else
	{
		pCmdUI->SetCheck(false);
	}
}

void CChildView::OnUpdateConfigurationKeyboardlayoutFrench(CCmdUI* pCmdUI) 
{
	if (appConfig.KEYBOARDMODE==1)
	{
		pCmdUI->SetCheck(true);
	}
	else
	{
		pCmdUI->SetCheck(false);
	}
}

void CChildView::OnUpdateRecordb(CCmdUI* pCmdUI) 
{
	if (SONG->m_WaveStage)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}
