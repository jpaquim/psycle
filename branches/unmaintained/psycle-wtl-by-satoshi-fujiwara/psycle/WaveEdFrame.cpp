/** @file 
 *  @brief
 *  $Date: 2004/11/09 12:40:35 $
 *  $Revision: 1.5 $
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

#include "Song.h"
#include "Instrument.h"
#include "WaveEdFrame.h"
#include "MainFrm.h"
#include ".\waveedframe.h"

//IMPLEMENT_DYNAMIC(CWaveEdFrame, CFrameWnd)
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_SIZE,
	ID_INDICATOR_MODE
};

CWaveEdFrame::CWaveEdFrame()
{
}
CWaveEdFrame::CWaveEdFrame(Song* _sng,CMainFrame* pframe)
{
	this->m_pSong=_sng;
	m_WaveView.SetSong(this->m_pSong);
	m_WaveView.SetParent(pframe);
}

CWaveEdFrame::~CWaveEdFrame()
{

}

BOOL CWaveEdFrame::OnIdle(){
	m_WaveView.UpdateUICompo();
	UIUpdateStatusBar();
	UIUpdateMenuBar();
	return TRUE;
}


void CWaveEdFrame::GenerateView() 
{	
	this->m_WaveView.GenerateAndShow(); 
}
/*
void CWaveEdFrame::OnUpdateStatusBar(CCmdUI *pCmdUI)  
{     
	pCmdUI->Enable ();  
}
*/
void CWaveEdFrame::AdjustStatusBar(int ins, int wav)
{
	int	wl = m_pSong->pInstrument(ins)->waveLength[wav];
	//m_StatusBar.SetPaneText(ID_INDICATOR_SIZE,buff,TRUE);
	UISetText(1,(SF::tformat(SF::CResourceString(IDS_MSG0095)) % wl).str().data());
	if (wl)
	{
		if (m_pSong->pInstrument(ins)->waveStereo[wav]){
			//m_StatusBar.SetPaneText(ID_INDICATOR_MODE, _T("Mode: Stereo"), TRUE);
			UISetText( 2, SF::CResourceString(IDS_MSG0096) );// Stereo
		} else {
			//m_StatusBar.SetPaneText(ID_INDICATOR_MODE,_T("Mode: Mono"),TRUE);
			UISetText(2,SF::CResourceString(IDS_MSG0097));// Mono
		}
	} else {
		//m_StatusBar.SetPaneText(ID_INDICATOR_MODE,_T("Mode: Empty"),TRUE);
			UISetText(2,SF::CResourceString(IDS_MSG0098));// Empty

	}
}


void CWaveEdFrame::Notify(void)
{
	m_WaveView.SetViewData(m_pSong->InstSelected(), m_pSong->WaveSelected());
	AdjustStatusBar(m_pSong->InstSelected(), m_pSong->WaveSelected());
}

LRESULT CWaveEdFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// register object for message filtering and idle updates

	CMessageLoop* pLoop = SF::CMessageLoopContainer::Instance().GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);	
	
	m_hWndStatusBar = m_StatusBar.Create(*this);
	this->UIAddStatusBar(m_hWndStatusBar);
	int anPanes[] = { ID_SEPARATOR, ID_INDICATOR_SIZE, 
                  ID_INDICATOR_MODE };
	m_StatusBar.SetPanes(anPanes,3,true);
	m_StatusBar.SetPaneWidth(ID_INDICATOR_SIZE,100);
	m_StatusBar.SetPaneWidth(ID_INDICATOR_MODE,100);
	
	
	//m_StatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	//m_StatusBar.SetPaneStyle(0, SBPS_NORMAL);
	//m_StatusBar.SetPaneInfo(1, ID_INDICATOR_SIZE, SBPS_NORMAL, 100);
	//m_StatusBar.SetPaneInfo(2, ID_INDICATOR_MODE, SBPS_NORMAL, 70);
	
	//WTL::CRect _rect(0,0,0,0);
	RECT _rect = {0,0,0,0};
	ATL::_U_RECT __rect(_rect);

	//m_hWndClient = m_WaveView.Create(m_hWnd,__rect,_T("Psycle Wave エディター"),WS_DEFAULT_VIEW);
	SF::CResourceString _title(IDS_MSG0093);
	m_hWndClient = m_WaveView.Create(m_hWnd,__rect,_title);
	
	m_WaveView.SetParent(CMainFrame::GetInstancePtr());
	m_WaveView.SetWaveEdFrame(this);
	
	/*	toolbar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY); */
	m_WaveView.ShowWindow(SW_SHOW);	

	this->SetWindowText(_title);
	//UIAddMenuBar(GetMenu());
	UpdateLayout();
	OnIdle();
	return TRUE;
}

LRESULT CWaveEdFrame::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//CFrameWnd::OnShowWindow(bShow, nStatus);

	Notify();
//	AdjustStatusBar(m_pSong->InstSelected(), m_pSong->WaveSelected());
//	OnIdle();
	UpdateWindow();
	bHandled = FALSE;
	return 0;
}

LRESULT CWaveEdFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ShowWindow(SW_HIDE);
	bHandled = TRUE;
	return 0;
}

