// GearTracker.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearTracker.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearTracker dialog


CGearTracker::CGearTracker(CChildView* pParent /*=NULL*/)
	: CDialog(CGearTracker::IDD, pParent)
{
	m_pParent = pParent;
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

	SetWindowText(_pMachine->_editName);

	m_polyslider.SetRange(2, SAMPLER_MAX_POLYPHONY, true);
	m_polyslider.SetPos(_pMachine->_numVoices);

	return TRUE;
}

void CGearTracker::OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
}

void CGearTracker::OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Assign new value
	_pMachine->_numVoices = m_polyslider.GetPos();

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
	_pMachine->_resampler.SetQuality((ResamplerQuality)m_interpol.GetCurSel());

}


BOOL CGearTracker::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearTracker::OnCancel()
{
	m_pParent->SamplerMachineDialog = NULL;
	DestroyWindow();
	delete this;
}
