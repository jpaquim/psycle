
#include "WaveInMacDlg.hpp"

#include "Configuration.hpp"
#include "ChildView.hpp"
#include "RecorderGui.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
	#include <psycle/audiodrivers/audiodriver.h>
	#include <psycle/core/internal_machines.h>
	using namespace psycle::core;
#else
	#include "AudioDriver.hpp"
	#include "internal_machines.hpp"
#endif

#include <psycle/helpers/dsp.hpp>

namespace psycle {
namespace host {

CWaveInMacDlg::CWaveInMacDlg(CChildView* pParent)
	: CDialog(CWaveInMacDlg::IDD, pParent),
	  gui_(0)
{
	m_pParent = pParent;
}

CWaveInMacDlg::CWaveInMacDlg(CChildView* pParent, MachineGui* gui)
	: CDialog(CWaveInMacDlg::IDD, pParent),
	  m_pParent(pParent),
	  gui_(gui)
{
}

void CWaveInMacDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_listbox);
	DDX_Control(pDX, IDC_VOLLABEL, m_vollabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volslider);
}

BEGIN_MESSAGE_MAP(CWaveInMacDlg, CDialog)
	ON_CBN_SELENDOK(IDC_COMBO1, OnCbnSelendokCombo1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
END_MESSAGE_MAP()

BOOL CWaveInMacDlg::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

BOOL CWaveInMacDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillCombobox();
	m_volslider.SetRange(0,1024);
	m_volslider.SetPos(pRecorder->GainVol()*256);
	char label[30];
	sprintf(label,"%.01fdB", helpers::dsp::dB(pRecorder->GainVol()));
	m_vollabel.SetWindowText(label);
	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CWaveInMacDlg::FillCombobox()
{
	AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
	std::vector<std::string> ports;
#if PSYCLE__CONFIGURATION__USE_PSYCORE	
	// todo use real port names in the driver
	unsigned int n = mydriver.captureSettings().numChannels();
	for ( unsigned int i = 0; i < n; ++i ) {
		std::ostringstream buffer;
		buffer << "Input" << i;
		ports.push_back(buffer.str());
	}
#else
	mydriver.GetCapturePorts(ports);
#endif
	for (unsigned int i =0; i < ports.size(); ++i)
	{
		m_listbox.AddString(ports[i].c_str());
	}
	if (ports.size()==0) m_listbox.AddString("No Inputs Available");
	m_listbox.SetCurSel(pRecorder->CaptureIdx());
}

void CWaveInMacDlg::OnCbnSelendokCombo1()
{
	pRecorder->ChangePort(m_listbox.GetCurSel());
}

void CWaveInMacDlg::OnCancel()
{
	if (gui_)
		gui_->BeforeDeleteDlg();

	DestroyWindow();
	delete this;
}

BOOL CWaveInMacDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
	{
		m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CWaveInMacDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	char label[30];
	pRecorder->setGainVol(m_volslider.GetPos()*0.00390625f);
	sprintf(label,"%.01fdB", helpers::dsp::dB(pRecorder->GainVol()));
	m_vollabel.SetWindowText(label);
	*pResult = 0;
}

void CWaveInMacDlg::Show(int x, int y)
{	
	centerWindowOnPoint(x, y);
}

void CWaveInMacDlg::centerWindowOnPoint(int x, int y) {
	CRect r;
	GetWindowRect(&r);

	x -= ((r.right-r.left)/2);
	y -= ((r.bottom-r.top)/2);

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	SetWindowPos( 0, x,	y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}


}   // namespace
}   // namespace
