// -*- mode:c++; indent-tabs-mode:t -*-
#include <psycle/project.private.hpp>
#include "WaveInMacDlg.hpp"
#include "psycle.hpp"
#include "Global.hpp"
#include "ChildView.hpp"
#include "AudioDriver.hpp"
#include "Configuration.hpp"
#include "internal_machines.hpp"
#include "dsp.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

CWaveInMacDlg::CWaveInMacDlg(CChildView* pParent)
: CDialog(CWaveInMacDlg::IDD, pParent)
{
	m_pParent = pParent;
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
	m_volslider.SetPos(pRecorder->_gainvol*256);
	char label[30];
	sprintf(label,"%.01fdB", helpers::dsp::dB(pRecorder->_gainvol));
	m_vollabel.SetWindowText(label);
	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CWaveInMacDlg::FillCombobox()
{
	AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
	std::vector<std::string> ports;
	mydriver.GetCapturePorts(ports);
	for (unsigned int i =0; i < ports.size(); ++i)
	{
		m_listbox.AddString(ports[i].c_str());
	}
	if (ports.size()==0) m_listbox.AddString("No Inputs Available");
	m_listbox.SetCurSel(pRecorder->_captureidx);
}

void CWaveInMacDlg::OnCbnSelendokCombo1()
{
	pRecorder->ChangePort(m_listbox.GetCurSel());
}

void CWaveInMacDlg::OnCancel()
{
	m_pParent->WaveInMachineDialog = NULL;
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
	pRecorder->_gainvol = m_volslider.GetPos()*0.00390625f;
	sprintf(label,"%.01fdB", helpers::dsp::dB(pRecorder->_gainvol));
	m_vollabel.SetWindowText(label);
	*pResult = 0;
}

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
