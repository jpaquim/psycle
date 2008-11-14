///\file
///\brief implementation file for psycle::host::CDirectoryDlg.

#include "XMSamplerUIGeneral.hpp"
#include "Psycle.hpp"
#include "XMInstrument.hpp"
#include "XMSampler.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

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
		DDX_Control(pDX, IDC_CHECK1, m_bAmigaSlides);
		DDX_Control(pDX, IDC_CHECK2, m_ckFilter);
		DDX_Control(pDX, IDC_XMPANNINGMODE, m_cbPanningMode);
	}
	BEGIN_MESSAGE_MAP(XMSamplerUIGeneral, CPropertyPage)
		ON_CBN_SELCHANGE(IDC_XMINTERPOL, OnCbnSelchangeXminterpol)
		ON_NOTIFY(NM_CUSTOMDRAW, IDC_XMPOLY, OnNMCustomdrawXmpoly)
		ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
		ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
		ON_CBN_SELENDOK(IDC_XMPANNINGMODE, OnCbnSelendokXmpanningmode)
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
		m_interpol.AddString(_T("512p. Sinc Interpolation"));

		m_interpol.SetCurSel(_pMachine->ResamplerQuality());


		m_polyslider.SetRange(2, XMSampler::MAX_POLYPHONY);
		m_polyslider.SetPos(_pMachine->NumVoices());

		m_ECommandInfo.SetWindowText("Track Commands:\r\n\t\
01xx: Portamento Up ( Fx: fine, Ex: Extra fine)\r\n\t\
02xx: Portamento Down (Fx: fine, Ex: Extra fine)\r\n\t\
03xx: Tone Portamento\r\n\t\
04xy: Vibrato with speed y and depth x\r\n\t\
05xx: Continue Portamento and Volume Slide with speed xx\r\n\t\
06xx: Continue Vibrato and Volume Slide with speed xx\r\n\t\
07xx: Tremolo\r\n\t\
08xx: Pan. 0800 Left 08FF right\r\n\t\
09xx: Panning slide x0 Left, 0x Right\r\n\t\
0Axx: Channel Volume, 00 = Min, 40 = Max\r\n\t\
0Bxx: Channel VolSlide x0 Up (xF fine), 0x Down (Fx Fine)\r\n\t\
0Cxx: Volume (0C80 : 100%)\r\n\t\
0Dxx: Volume Slide x0 Up (xF fine), 0x Down (Fx Fine)\r\n\t\
0Exy: Extended (see below).\r\n\t\
0Fxx: Filter.\r\n\t\
10xy: Arpeggio with note, note+x and note+y\r\n\t\
11xy: Retrig note after y ticks\r\n\t\
12xx: Global Volume, 00 = Min, 80 = Max\r\n\t\
13xx: Global Volume Slide x0 Up (xF fine), 0x Down (Fx Fine)\r\n\t\
14xx: Fine Vibrato with speed y and depth x\r\n\t\
15xx: Set Envelope position\r\n\t\
17xy: Tremor Effect ( ontime x, offtime y )\r\n\t\
18xx: Panbrello\r\n\t\
9xxx: Sample Offset\r\n\r\n\
Extended Commands:\r\n\t\
30/1: Glissando mode Off/on\r\n\t\
4x: Vibrato Wave\r\n\t\
6x: Panbrello Wave\r\n\t\
7x: Tremolo Wave\r\n\t\
Waves: 0:Sinus, 1:Square\r\n\t\
2:Ramp Up, 3:Ramp Down, 4: Random\r\n\t\
90: Surround Off\r\n\t\
91: Surround On\r\n\t\
9E: Play Forward\r\n\t\
9F: Play Backward\r\n\t\
Cx: Delay NoteCut by x ticks\r\n\t\
Dx: Delay New Note by x ticks\r\n\t\
E0: Send Notecut to past notes\r\n\t\
E1: Send Noteoff to past notes\r\n\t\
E2: Send NoteFade to past notes\r\n\t\
E3: Set NNA NoteCut for this voice\r\n\t\
E4: Set NNA NoteContinue for this voice\r\n\t\
E5: Set NNA Noteoff for this voice\r\n\t\
E6: Set NNA NoteFade for this channel\r\n\t\
E7/8: Disable/Enable Volume Envelope\r\n\t\
E9/A: Disable/Enable Pan Envelope\r\n\t\
EB/C: Disable/Enable Pitch/Filter Envelope\r\n\t\
Fx : Set Filter Mode.\r\n\r\n\
Volume Column:\r\n\t\
00..3F: Set volume to x*2\r\n\t\
4x: Volume slide up\r\n\t\
5x: Volume slide down\r\n\t\
6x: Fine Volslide up\r\n\t\
7x: Fine Volslide down\r\n\t\
8x: Panning (0:Left, F:Right)\r\n\t\
9x: PanSlide Left\r\n\t\
Ax: PanSlide Right\r\n\t\
Bx: Vibrato\r\n\t\
Cx: TonePorta\r\n\t\
Dx: Pitch slide up\r\n\t\
Ex: Pitch slide down");

	m_bInitialize=true;

	m_bAmigaSlides.SetCheck(_pMachine->IsAmigaSlides()?1:0);
	m_ckFilter.SetCheck(_pMachine->UseFilters()?1:0);

	m_cbPanningMode.AddString(_T("Linear (Cross)"));
	m_cbPanningMode.AddString(_T("Two Sliders (FT2)"));
	m_cbPanningMode.AddString(_T("Equal Power"));

	m_cbPanningMode.SetCurSel(_pMachine->PanningMode());

	return true;  // return true unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return false
	}

	void XMSamplerUIGeneral::OnCbnSelchangeXminterpol()
	{
		_pMachine->ResamplerQuality((helpers::dsp::ResamplerQuality)m_interpol.GetCurSel());
	}

	void XMSamplerUIGeneral::OnNMCustomdrawXmpoly(NMHDR *pNMHDR, LRESULT *pResult)
	{
		//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
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

	void XMSamplerUIGeneral::OnBnClickedCheck1()
	{
		_pMachine->IsAmigaSlides(( m_bAmigaSlides.GetCheck() == 1 )?true:false);
	}

	void XMSamplerUIGeneral::OnBnClickedCheck2()
	{
		_pMachine->UseFilters(m_ckFilter.GetCheck()?true:false);
	}

	void XMSamplerUIGeneral::OnCbnSelendokXmpanningmode()
	{
		_pMachine->PanningMode(m_cbPanningMode.GetCurSel());
	}

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
