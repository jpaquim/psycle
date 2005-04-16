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
		DDX_Control(pDX, IDC_XMINTERPOL, m_interpol);
		DDX_Control(pDX, IDC_XMPOLY, m_polyslider);
		DDX_Control(pDX, IDC_XMPOLYLABEL, m_polylabel);
		//}}AFX_DATA_MAP
		DDX_Control(pDX, IDC_COMMANDINFO, m_ECommandInfo);
	}
BEGIN_MESSAGE_MAP(XMSamplerUIGeneral, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_XMINTERPOL, OnCbnSelchangeXminterpol)
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
	m_bInitialize=false;
	m_interpol.AddString(_T("No Interpolation"));
	m_interpol.AddString(_T("Linear Interpolation"));
	m_interpol.AddString(_T("Spline Interpolation"));

	m_interpol.SetCurSel(_pMachine->ResamplerQuality());


	m_polyslider.SetRange(2, XMSampler::MAX_POLYPHONY);
	m_polyslider.SetPos(_pMachine->NumVoices());

	m_ECommandInfo.SetWindowText("Track Commands:\r\n\t\
01xx: Portamento Up ( Fx = fine, Ex = Extra fine)\r\n\t\
02xx: Portamento Down (Fx = fine, Ex = Extra fine)\r\n\t\
03xx: Tone Portamento (Fx = fine, Ex = Extra fine)\r\n\t\
04xy: Vibrato with speed y and depth x\r\n\t\
05xx: Continue Portamento and Volume Slide with speed xx\r\n\t\
06xx: Continue Vibrato and Volume Slide with speed xx\r\n\t\
07xx: Tremolo\r\n\t\
08xx: Panoramization. 0800 Leftmost 08FF rightmost\r\n\t\
09xy: Panning slide x0 Left, 0x Right\r\n\t\
0Axy: Channel Volume, 0x -> Down (Fx Fine)\r\n\t\
0Bxx: Channel Volume Slide x0 -> Up (xF fine), 0x -> Down (Fx Fine)\r\n\t\
0Cxx: Volume (0C80 : Normal)\r\n\t\
0Dxx: Volume Slide x0 -> -> Up (xF fine), 0x -> Down (Fx Fine)\r\n\t\
10xy: Arpeggio with note, note+x and note+y\r\n\t\
9xxx: Sample Offset");

	m_bInitialize=true;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	}

void XMSamplerUIGeneral::OnCbnSelchangeXminterpol()
	{
	_pMachine->ResamplerQuality((dsp::ResamplerQuality)m_interpol.GetCurSel());

	}

void XMSamplerUIGeneral::OnNMCustomdrawXmpoly(NMHDR *pNMHDR, LRESULT *pResult)
	{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	for(int c = _pMachine->NumVoices(); c < XMSampler::MAX_POLYPHONY; c++)
		{
		_pMachine->rVoice(c).NoteOffFast();
		}

		_pMachine->NumVoices(m_polyslider.GetPos());
	// Label on dialog display
	char buffer[15];
	sprintf(buffer,"%d",_pMachine->NumVoices());
	m_polylabel.SetWindowText(buffer);
	*pResult = 0;
	}
NAMESPACE__END
NAMESPACE__END