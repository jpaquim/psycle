// GearTracker.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearTracker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearTracker dialog


CGearTracker::CGearTracker(CWnd* pParent /*=NULL*/)
	: CDialog(CGearTracker::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearTracker)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGearTracker::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearTracker)
	DDX_Control(pDX, IDC_COMBO1, m_interpol);
	DDX_Control(pDX, IDC_TRACKSLIDER2, m_polyslider);
	DDX_Control(pDX, IDC_TRACKLABEL2, m_polylabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearTracker, CDialog)
	//{{AFX_MSG_MAP(CGearTracker)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACKSLIDER2, OnCustomdrawTrackslider2)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearTracker message handlers

BOOL CGearTracker::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_interpol.AddString("None [Lowest quality]");
	m_interpol.AddString("Linear [Medium quality]");
	m_interpol.AddString("Spline [Higher Quality]");

	m_interpol.SetCurSel(_pMachine->_resampler._quality);

	char buffer[64];
	sprintf(buffer,_pMachine->_editName);
	SetWindowText(buffer);

	m_polyslider.SetRange(2, SAMPLER_MAX_POLYPHONY, true);
	m_polyslider.SetPos(_pMachine->_numVoices);

	doit = true;

	return TRUE;
}

void CGearTracker::OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
}

void CGearTracker::OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Assign new value
	if (doit)
	{
		_pMachine->_numVoices = m_polyslider.GetPos();
	}

	for(int c=0; c<SAMPLER_MAX_POLYPHONY; c++)
	{
		_pMachine->NoteOffFast(c);
	}

	// Label on dialog display
	char buffer[8];
	sprintf(buffer, "%d", _pMachine->_numVoices);
	m_polylabel.SetWindowText(buffer);

	*pResult = 0;
}

void CGearTracker::OnSelchangeCombo1() 
{
	if (doit)
	{
		_pMachine->_resampler.SetQuality((ResamplerQuality)m_interpol.GetCurSel());
	}
}