///\file
///\brief implementation file for psycle::host::CDirectoryDlg.
#include <project.private.hpp>
#include "Psycle.hpp"
#include "XMSamplerUIGeneral.hpp"
#include "XMInstrument.hpp"
#include "XMSampler.hpp"
NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

IMPLEMENT_DYNCREATE(XMSamplerUIGeneral, CPropertyPage)

XMSamplerUIGeneral::XMSamplerUIGeneral() : CPropertyPage(XMSamplerUIGeneral::IDD)
	{
		m_bInitialize = false;
	}

void XMSamplerUIGeneral::DoDataExchange(CDataExchange* pDX)
	{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDirectoryDlg)
	DDX_Control(pDX, IDC_XMTEMPO, m_Tempo);
	DDX_Control(pDX, IDC_XMSPEED, m_Speed);
	DDX_Control(pDX, IDC_XMINTERPOL, m_interpol);
	DDX_Control(pDX, IDC_XMPOLY, m_polyslider);
	DDX_Control(pDX, IDC_XMPOLYLABEL, m_polylabel);
	//}}AFX_DATA_MAP
	}
BEGIN_MESSAGE_MAP(XMSamplerUIGeneral, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_XMINTERPOL, OnCbnSelchangeXminterpol)
	ON_EN_CHANGE(IDC_XMSPEED, OnEnChangeXmspeed)
	ON_EN_CHANGE(IDC_XMTEMPO, OnEnChangeXmtempo)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_XMPOLY, OnNMCustomdrawXmpoly)
END_MESSAGE_MAP()

XMSamplerUIGeneral::~XMSamplerUIGeneral()
	{
	}
/////////////////////////////////////////////////////////////////////////////
// XMSamplerUIGeneral message handlers

BOOL XMSamplerUIGeneral::OnInitDialog() 
	{
	CPropertyPage::OnInitDialog();
	m_bInitialize=true;
	m_interpol.AddString(_T("No Interpolation"));
	m_interpol.AddString(_T("Linear Interpolation"));
	m_interpol.AddString(_T("Spline Interpolation"));

	m_interpol.SetCurSel(_pMachine->ResamplerQuality());

	SetWindowText(_pMachine->_editName);

	m_polyslider.SetRange(2, XMSampler::MAX_POLYPHONY);
	m_polyslider.SetPos(_pMachine->NumVoices());

	std::string tmp;
	tmp=_pMachine->BPM();
	m_Tempo.SetWindowText(tmp.c_str());
	tmp=_pMachine->TicksPerRow();
	m_Speed.SetWindowText(tmp.c_str());


	m_bInitialize=false;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	}

void XMSamplerUIGeneral::OnCbnSelchangeXminterpol()
	{
	_pMachine->ResamplerQuality((ResamplerQuality)m_interpol.GetCurSel());

	}

void XMSamplerUIGeneral::OnEnChangeXmspeed()
	{
	if(!m_bInitialize)
		{
		return;
		}

	TCHAR buf[256];
	m_Speed.GetWindowText(buf,256);
	_pMachine->TicksPerRow(atoi(buf));
	_pMachine->CalcBPMAndTick();
	}

void XMSamplerUIGeneral::OnEnChangeXmtempo()
	{
	if(!m_bInitialize)
		{
		return;
		}
	TCHAR buf[256];
	m_Tempo.GetWindowText(buf,256);
	_pMachine->BPM(atoi(buf));
	_pMachine->CalcBPMAndTick();
	}

void XMSamplerUIGeneral::OnNMCustomdrawXmpoly(NMHDR *pNMHDR, LRESULT *pResult)
	{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	_pMachine->NumVoices(m_polyslider.GetPos());

	for(int c = 0; c < XMSampler::MAX_POLYPHONY; c++)
		{
		_pMachine->rVoice(c).NoteOffFast();
		}

	// Label on dialog display
	std::string tmp;
	tmp = _pMachine->NumVoices();
	m_polylabel.SetWindowText(tmp.c_str());
	*pResult = 0;
	}
NAMESPACE__END
NAMESPACE__END