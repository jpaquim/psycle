#include <project.private.hpp>
#include "psycle.hpp"
#include "WaveInMacDlg.hpp"
#include "Global.hpp"
#include "ChildView.hpp"
#include "AudioDriver.hpp"
#include "Configuration.hpp"
#include "internal_machines.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

CWaveInMacDlg::CWaveInMacDlg(CChildView* pParent)
: CDialog(CWaveInMacDlg::IDD, pParent)
{
	m_pParent = pParent;
}

void CWaveInMacDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_listbox);
}

BEGIN_MESSAGE_MAP(CWaveInMacDlg, CDialog)
	ON_CBN_SELENDOK(IDC_COMBO1, OnCbnSelendokCombo1)
END_MESSAGE_MAP()

BOOL CWaveInMacDlg::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

BOOL CWaveInMacDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillCombobox();
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
	m_listbox.SetCurSel(pRecorder->_captureidx);
}

void CWaveInMacDlg::OnCbnSelendokCombo1()
{
	AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
	if ( pRecorder->_initialized )
	{
		mydriver.RemoveCapturePort(pRecorder->_captureidx);
		pRecorder->_initialized=false;
	}
	pRecorder->_captureidx = m_listbox.GetCurSel();
	pRecorder->Init();
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


NAMESPACE__END
NAMESPACE__END


