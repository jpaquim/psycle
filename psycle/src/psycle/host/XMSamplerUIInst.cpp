// XMSamplerUIInst.cpp : XMSamplerUIInst
/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#include <psycle/project.private.hpp>
#include "XMSamplerUIInst.hpp"
#include "Psycle.hpp"
#include "Player.hpp"
#include "XMInstrument.hpp"
#include "XMSampler.hpp"
#include "XMSongLoader.hpp"
#include "ITModule2.h"
#include "Configuration.hpp"
//using namespace Gdiplus;

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)
// XMSamplerUIInst
IMPLEMENT_DYNAMIC(XMSamplerUIInst, CPropertyPage)
XMSamplerUIInst::XMSamplerUIInst()
: CPropertyPage(XMSamplerUIInst::IDD)
, m_bInitialized(false)
, m_iCurrentSelected(0)
{
}

XMSamplerUIInst::~XMSamplerUIInst()
{
}

void XMSamplerUIInst::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INSTRUMENTLIST, m_InstrumentList);

	DDX_Control(pDX, IDC_FILTERTYPE, m_FilterType);
	DDX_Control(pDX, IDC_VOLCUTOFFPAN, m_SlVolCutoffPan);
	DDX_Control(pDX, IDC_SWING1, m_SlSwing1Glide);
	DDX_Control(pDX, IDC_FADEOUTRES, m_SlFadeoutRes);
	DDX_Control(pDX, IDC_SWING2, m_SlSwing2);

	DDX_Control(pDX, IDC_SLNOTEMODNOTE, m_SlNoteModNote);
	DDX_Control(pDX, IDC_NOTEMOD, m_SlNoteMod);
	
	DDX_Control(pDX, IDC_ADSRBASE, m_SlADSRBase);
	DDX_Control(pDX, IDC_ADSRMOD, m_SlADSRMod);
	DDX_Control(pDX, IDC_ADSRATT, m_SlADSRAttack);
	DDX_Control(pDX, IDC_ADSRDEC, m_SlADSRDecay);
	DDX_Control(pDX, IDC_ADSRSUS, m_SlADSRSustain);
	DDX_Control(pDX, IDC_ADSRREL, m_SlADSRRelease);

	DDX_Control(pDX, IDC_INS_NAME, m_InstrumentName);
	DDX_Control(pDX, IDC_INS_NNACOMBO, m_NNA);
	DDX_Control(pDX, IDC_INS_DCTCOMBO, m_DCT);
	DDX_Control(pDX, IDC_INS_DCACOMBO, m_DCA);
	DDX_Control(pDX, IDC_CUTOFFPAN, m_cutoffPan);
	DDX_Control(pDX, IDC_RESSONANCE, m_Ressonance);
	DDX_Control(pDX, IDC_ENVCHECK, m_EnvEnabled);

	DDX_Control(pDX, IDC_INS_ENVELOPE, m_EnvelopeEditor);
	DDX_Control(pDX, IDC_INS_NOTEMAP, m_SampleAssign);

	DDX_Control(pDX, IDC_INS_NOTESCROLL, m_SampleAssign.m_scBar);
}

BEGIN_MESSAGE_MAP(XMSamplerUIInst, CPropertyPage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VOLCUTOFFPAN, OnNMCustomdrawVolCutoffPan)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SWING1, OnNMCustomdrawSwing1Glide)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_FADEOUTRES, OnNMCustomdrawFadeoutRes)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SWING2, OnNMCustomdrawSwing2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLNOTEMODNOTE, OnNMCustomdrawNotemodnote)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_NOTEMOD, OnNMCustomdrawNoteMod)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ADSRBASE, OnNMCustomdrawADSRBase)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ADSRMOD, OnNMCustomdrawADSRMod)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ADSRATT, OnNMCustomdrawADSRAttack)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ADSRDEC, OnNMCustomdrawADSRDecay)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ADSRSUS, OnNMCustomdrawADSRSustain)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ADSRREL, OnNMCustomdrawADSRRelease)
	ON_LBN_SELCHANGE(IDC_INSTRUMENTLIST, OnLbnSelchangeInstrumentlist)
	ON_EN_CHANGE(IDC_INS_NAME, OnEnChangeInsName)
	ON_CBN_SELENDOK(IDC_FILTERTYPE, OnCbnSelendokFiltertype)
	ON_BN_CLICKED(IDC_ENVCHECK, OnBnClickedEnvcheck)
	ON_CBN_SELENDOK(IDC_INS_NNACOMBO, OnCbnSelendokInsNnacombo)
	ON_CBN_SELENDOK(IDC_INS_DCTCOMBO, OnCbnSelendokInsDctcombo)
	ON_CBN_SELENDOK(IDC_INS_DCACOMBO, OnCbnSelendokInsDcacombo)
	ON_BN_CLICKED(IDC_LOADINS, OnBnClickedLoadins)
	ON_BN_CLICKED(IDC_SAVEINS, OnBnClickedSaveins)
	ON_BN_CLICKED(IDC_DUPEINS, OnBnClickedDupeins)
	ON_BN_CLICKED(IDC_DELETEINS, OnBnClickedDeleteins)
	ON_BN_CLICKED(IDC_INS_TGENERAL, OnBnClickedInsTgeneral)
	ON_BN_CLICKED(IDC_INS_TAMP, OnBnClickedInsTamp)
	ON_BN_CLICKED(IDC_INS_TPAN, OnBnClickedInsTpan)
	ON_BN_CLICKED(IDC_INS_TFILTER, OnBnClickedInsTfilter)
	ON_BN_CLICKED(IDC_INS_TPITCH, OnBnClickedInsTpitch)
	ON_BN_CLICKED(IDC_ENVADSR, OnBnClickedEnvadsr)
	ON_BN_CLICKED(IDC_ENVFREEFORM, OnBnClickedEnvfreeform)
	ON_WM_HSCROLL()

END_MESSAGE_MAP()


void XMSamplerUIInst::SetInstrumentData(const int instno)
{
	TRACE("in setInstrumentData\n");
	XMInstrument& inst = m_pMachine->rInstrument(instno);
	m_iCurrentSelected=instno;

	m_InstrumentName.SetWindowText(inst.Name().c_str());
	SetNewNoteAction(inst.NNA(),inst.DCT(),inst.DCA());
	m_SampleAssign.Initialize(m_pMachine,&m_pMachine->rInstrument(instno),this);

	m_FilterType.SetCurSel((int)inst.FilterType());

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
		AssignAmplitudeValues(inst);
	else if (((CButton*)GetDlgItem(IDC_INS_TPAN))->GetCheck())
		AssignPanningValues(inst);
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
		AssignFilterValues(inst);
	else if (((CButton*)GetDlgItem(IDC_INS_TPITCH))->GetCheck())
		AssignPitchValues(inst);

}

void XMSamplerUIInst::SetNewNoteAction(const int nna,const int dct,const int dca)
{
	m_NNA.SetCurSel(nna);
	m_DCT.SetCurSel(dct);
	m_DCA.SetCurSel(dca);
}

void XMSamplerUIInst::AssignAmplitudeValues(XMInstrument& inst)
{
	TRACE("in assingamplvals, reading globvol\n");
	m_SlVolCutoffPan.SetPos(inst.GlobVol()*128.0f);
	m_SlFadeoutRes.SetPos(inst.VolumeFadeSpeed()*1024.0f);
	m_SlSwing1Glide.SetPos(inst.RandomVolume()*100.0f);
//	m_SlNoteModNote.SetPos(inst.NoteModVolCenter());
//	m_SlNoteMod.SetPos(inst.NoteModVolSep());

	m_EnvelopeEditor.Initialize(m_pMachine,inst.AmpEnvelope());
	((CStatic*)GetDlgItem(IDC_STATIC6))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRBase.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRBASE))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC7))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRMOD))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC8))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRAttack.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRATT))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC9))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRDecay.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRDEC))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC10))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRSustain.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRSUS))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC11))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRRelease.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRREL))->ShowWindow(SW_SHOW); 
}
void XMSamplerUIInst::AssignPanningValues(XMInstrument& inst)
{
	m_cutoffPan.SetCheck(inst.PanEnabled());
	m_SlVolCutoffPan.SetPos((inst.Pan()*128.0f)-64.0f);
	//FIXME: This is not showing the correct value. Should check if randompanning
	//is erroneous or is not the value to check.
	m_SlSwing1Glide.SetPos(inst.RandomPanning()*100.0f);
	m_SlNoteModNote.SetPos(inst.NoteModPanCenter());
	m_SlNoteMod.SetPos(inst.NoteModPanSep());

	m_EnvelopeEditor.Initialize(m_pMachine,inst.PanEnvelope());
	((CStatic*)GetDlgItem(IDC_STATIC6))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRBase.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRBASE))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC7))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRMOD))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC8))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRAttack.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRATT))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC9))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRDecay.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRDEC))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC10))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRSustain.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRSUS))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC11))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRRelease.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRREL))->ShowWindow(SW_SHOW); 
}
void XMSamplerUIInst::AssignFilterValues(XMInstrument& inst)
{
	m_SlVolCutoffPan.SetPos(inst.FilterCutoff()&0x7F);
	if (inst.FilterCutoff()) m_cutoffPan.SetCheck(!(inst.FilterCutoff()&0x80));
	else m_cutoffPan.SetCheck(0);

	m_SlFadeoutRes.SetPos(inst.FilterResonance()&0x7F);
	if (inst.FilterResonance()) m_Ressonance.SetCheck(!(inst.FilterResonance()&0x80));
	else m_Ressonance.SetCheck(0);
	m_SlSwing1Glide.SetPos(inst.RandomCutoff()*100.0f);
	m_SlSwing2.SetPos(inst.RandomResonance()*100.0f);
//	m_SlNoteModNote.SetPos(inst.NoteModFilterCenter());
//	m_SlNoteMod.SetPos(inst.NoteModFilterSep());

	m_EnvelopeEditor.Initialize(m_pMachine,inst.FilterEnvelope());
	((CStatic*)GetDlgItem(IDC_STATIC6))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRBase.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRBASE))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC7))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRMOD))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC8))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRAttack.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRATT))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC9))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRDecay.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRDEC))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC10))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRSustain.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRSUS))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC11))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRRelease.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRREL))->ShowWindow(SW_SHOW); 
}
void XMSamplerUIInst::AssignPitchValues(XMInstrument& inst)
{
//	m_SlVolCutoffPan.SetPos(inst.Tune());
//	m_SlSwing1Glide.SetPos(inst.Glide());
	m_SlNoteMod.SetPos(inst.Lines());

	m_EnvelopeEditor.Initialize(m_pMachine,inst.PitchEnvelope());
	((CStatic*)GetDlgItem(IDC_STATIC6))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRBase.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRBASE))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC7))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRMOD))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC8))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRAttack.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRATT))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC9))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRDecay.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRDEC))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC10))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRSustain.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRSUS))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC11))->ShowWindow(SW_SHOW); //ENVADSR
	m_SlADSRRelease.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LADSRREL))->ShowWindow(SW_SHOW); 
}


// Controladores de mensajes de XMSamplerUISample
BOOL XMSamplerUIInst::OnSetActive()
{
	TRACE("in setActive\n");
	if ( m_bInitialized == false )
	{
		((CEdit*)GetDlgItem(IDC_INS_NAME))->SetLimitText(31);
		m_FilterType.AddString(_T("LowPass"));
		m_FilterType.AddString(_T("HighPass"));
		m_FilterType.AddString(_T("BandPass"));
		m_FilterType.AddString(_T("NotchBand"));
		m_FilterType.AddString(_T("Off"));

		m_NNA.AddString(_T("Note Cut"));
		m_NNA.AddString(_T("Note Continue"));
		m_NNA.AddString(_T("Note Off"));
		m_NNA.AddString(_T("Note Fadeout"));

		m_DCT.AddString(_T("Disabled"));
		m_DCT.AddString(_T("Note"));
		m_DCT.AddString(_T("Sample"));
		m_DCT.AddString(_T("Instrument"));

		m_DCA.AddString(_T("Note Cut"));
		m_DCA.AddString(_T("Note Continue"));
		m_DCA.AddString(_T("Note Off"));
		m_DCA.AddString(_T("Note Fadeout"));

		m_SlSwing2.SetRangeMax(100);

		m_SlNoteModNote.SetRangeMin(0);
		m_SlNoteModNote.SetRangeMax(119);

		m_SlNoteMod.SetRangeMax(32);
		m_SlNoteMod.SetRangeMin(-32);
		m_SlNoteMod.SetPos(1);

		m_SlADSRBase.SetRangeMax(256);
		m_SlADSRMod.SetRangeMax(256);
		m_SlADSRMod.SetRangeMin(-256);
		m_SlADSRAttack.SetRangeMax(512);
		m_SlADSRDecay.SetRangeMax(512);
		m_SlADSRSustain.SetRangeMax(256);
		m_SlADSRRelease.SetRangeMax(512);

		for (int i=0;i<XMSampler::MAX_INSTRUMENT;i++)
		{
			char line[48];
			XMInstrument& inst = m_pMachine->rInstrument(i);
			sprintf(line,"%02X%s: ",i,inst.IsEnabled()?"*":" ");
			strcat(line,inst.Name().c_str());
			m_InstrumentList.AddString(line);
		}
		m_InstrumentList.SetCurSel(0);

		((CButton*)GetDlgItem(IDC_INS_TGENERAL))->SetCheck(1);
		OnBnClickedInsTgeneral();
	
		SetInstrumentData(0);
		m_bInitialized = true;

	}

	return CPropertyPage::OnSetActive();
}

void XMSamplerUIInst::OnBnClickedInsTgeneral()
{
	m_bInitialized = false;
	
	((CStatic*)GetDlgItem(IDC_INS_NOTESCROLL))->ShowWindow(SW_SHOW);

	((CStatic*)GetDlgItem(IDC_FRAMENNA))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC14))->ShowWindow(SW_SHOW);//Instrument Name
	((CEdit*)GetDlgItem(IDC_INS_NAME))->ShowWindow(SW_SHOW);//Instrument Name
	((CStatic*)GetDlgItem(IDC_STATIC15))->ShowWindow(SW_SHOW);
	m_NNA.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC16))->ShowWindow(SW_SHOW);
	m_DCT.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC17))->ShowWindow(SW_SHOW);
	m_DCA.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_INS_NOTEMAP))->ShowWindow(SW_SHOW);

	((CStatic*)GetDlgItem(IDC_STATIC1))->ShowWindow(SW_HIDE); //FilterType
	m_FilterType.ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_STATIC12))->ShowWindow(SW_HIDE); //Volume
	m_cutoffPan.ShowWindow(SW_HIDE);
	m_SlVolCutoffPan.ShowWindow(SW_HIDE);
	m_SlVolCutoffPan.SetRangeMax(128);
	m_SlVolCutoffPan.SetRangeMin(0);
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC2))->ShowWindow(SW_HIDE); //Swing1
	m_SlSwing1Glide.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LSWING))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC13))->ShowWindow(SW_HIDE); //Fadeout
	m_Ressonance.ShowWindow(SW_HIDE);
	m_SlFadeoutRes.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC18))->ShowWindow(SW_HIDE); //Swing2
	m_SlSwing2.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LSWING2))->ShowWindow(SW_HIDE); //Volume

	((CStatic*)GetDlgItem(IDC_STATIC3))->ShowWindow(SW_HIDE); //NOTEMOD
	m_SlNoteModNote.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->ShowWindow(SW_HIDE); //NOTEMOD
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_HIDE); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_HIDE); //NOTEMOD

	m_EnvEnabled.ShowWindow(SW_HIDE);
	((CButton*)GetDlgItem(IDC_ENVADSR))->ShowWindow(SW_HIDE);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_INS_ENVELOPE))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC6))->ShowWindow(SW_HIDE); //ENVADSR
	m_SlADSRBase.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LADSRBASE))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC7))->ShowWindow(SW_HIDE); //ENVADSR
	m_SlADSRMod.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LADSRMOD))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC8))->ShowWindow(SW_HIDE); //ENVADSR
	m_SlADSRAttack.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LADSRATT))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC9))->ShowWindow(SW_HIDE); //ENVADSR
	m_SlADSRDecay.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LADSRDEC))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC10))->ShowWindow(SW_HIDE); //ENVADSR
	m_SlADSRSustain.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LADSRSUS))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC11))->ShowWindow(SW_HIDE); //ENVADSR
	m_SlADSRRelease.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LADSRREL))->ShowWindow(SW_HIDE); 

	m_bInitialized = true;
	
}
void XMSamplerUIInst::OnBnClickedInsTamp()
{
	TRACE("inbtninsTamp\n");
	m_bInitialized = false;
	int i= m_iCurrentSelected;
	XMInstrument& inst = m_pMachine->rInstrument(i);

	((CStatic*)GetDlgItem(IDC_INS_NOTESCROLL))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_FRAMENNA))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC14))->ShowWindow(SW_HIDE);//Instrument Name
	((CEdit*)GetDlgItem(IDC_INS_NAME))->ShowWindow(SW_HIDE);//Instrument Name
	((CStatic*)GetDlgItem(IDC_STATIC15))->ShowWindow(SW_HIDE);
	m_NNA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC16))->ShowWindow(SW_HIDE);
	m_DCT.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC17))->ShowWindow(SW_HIDE);
	m_DCA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_INS_NOTEMAP))->ShowWindow(SW_HIDE);


	((CStatic*)GetDlgItem(IDC_STATIC1))->ShowWindow(SW_HIDE); //FilterType
	m_FilterType.ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_STATIC12))->ShowWindow(SW_SHOW); //Volume
	((CStatic*)GetDlgItem(IDC_STATIC12))->SetWindowText("Global Volume");
	m_cutoffPan.ShowWindow(SW_HIDE);
	m_SlVolCutoffPan.ShowWindow(SW_SHOW);
	m_SlVolCutoffPan.SetRangeMax(128);
	m_SlVolCutoffPan.SetRangeMin(0);
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC2))->ShowWindow(SW_SHOW); //Swing1
	((CStatic*)GetDlgItem(IDC_STATIC2))->SetWindowText("Swing (Randomize)");
	m_SlSwing1Glide.ShowWindow(SW_SHOW);
	m_SlSwing1Glide.SetRangeMax(100);
	((CStatic*)GetDlgItem(IDC_LSWING))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC13))->ShowWindow(SW_SHOW); //Fadeout
	m_Ressonance.ShowWindow(SW_HIDE);
	m_SlFadeoutRes.ShowWindow(SW_SHOW);
	m_SlFadeoutRes.SetRangeMax(256);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC18))->ShowWindow(SW_HIDE); //Swing2
	m_SlSwing2.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LSWING2))->ShowWindow(SW_HIDE); 

	((CStatic*)GetDlgItem(IDC_STATIC3))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteModNote.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->ShowWindow(SW_SHOW); //NOTEMOD
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_SHOW); //NOTEMOD

	m_EnvEnabled.ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVADSR))->ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->ShowWindow(SW_SHOW);

	((CStatic*)GetDlgItem(IDC_INS_ENVELOPE))->ShowWindow(SW_SHOW);

	AssignAmplitudeValues(inst);
	m_bInitialized = true;
}

void XMSamplerUIInst::OnBnClickedInsTpan()
{
	m_bInitialized = false;
	int i= m_iCurrentSelected;
	XMInstrument& inst = m_pMachine->rInstrument(i);

	((CStatic*)GetDlgItem(IDC_INS_NOTESCROLL))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_FRAMENNA))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC14))->ShowWindow(SW_HIDE);//Instrument Name
	((CEdit*)GetDlgItem(IDC_INS_NAME))->ShowWindow(SW_HIDE);//Instrument Name
	((CStatic*)GetDlgItem(IDC_STATIC15))->ShowWindow(SW_HIDE);
	m_NNA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC16))->ShowWindow(SW_HIDE);
	m_DCT.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC17))->ShowWindow(SW_HIDE);
	m_DCA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_INS_NOTEMAP))->ShowWindow(SW_HIDE);


	((CStatic*)GetDlgItem(IDC_STATIC1))->ShowWindow(SW_HIDE); //FilterType
	m_FilterType.ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_STATIC12))->ShowWindow(SW_HIDE); //Volume
	m_cutoffPan.ShowWindow(SW_SHOW);
	m_cutoffPan.SetWindowText("Instrument Pan");
	m_SlVolCutoffPan.ShowWindow(SW_SHOW);
	m_SlVolCutoffPan.SetRangeMax(64);
	m_SlVolCutoffPan.SetRangeMin(-64);
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC2))->ShowWindow(SW_SHOW); //Swing1
	((CStatic*)GetDlgItem(IDC_STATIC2))->SetWindowText("Swing (Randomize)");
	m_SlSwing1Glide.ShowWindow(SW_SHOW);
	m_SlSwing1Glide.SetRangeMax(100);
	((CStatic*)GetDlgItem(IDC_LSWING))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC13))->ShowWindow(SW_HIDE); //Fadeout
	m_Ressonance.ShowWindow(SW_HIDE);
	m_SlFadeoutRes.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC18))->ShowWindow(SW_HIDE); //Swing2
	m_SlSwing2.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LSWING2))->ShowWindow(SW_HIDE); 

	((CStatic*)GetDlgItem(IDC_STATIC3))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteModNote.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->ShowWindow(SW_SHOW); //NOTEMOD
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_SHOW); //NOTEMOD

	m_EnvEnabled.ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVADSR))->ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_INS_ENVELOPE))->ShowWindow(SW_SHOW);
	
	AssignPanningValues(inst);
	m_bInitialized = true;
}
	
void XMSamplerUIInst::OnBnClickedInsTfilter()
{
	m_bInitialized = false;
	int i= m_iCurrentSelected;
	XMInstrument& inst = m_pMachine->rInstrument(i);
	
	((CStatic*)GetDlgItem(IDC_INS_NOTESCROLL))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_FRAMENNA))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC14))->ShowWindow(SW_HIDE);//Instrument Name
	((CEdit*)GetDlgItem(IDC_INS_NAME))->ShowWindow(SW_HIDE);//Instrument Name
	((CStatic*)GetDlgItem(IDC_STATIC15))->ShowWindow(SW_HIDE);
	m_NNA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC16))->ShowWindow(SW_HIDE);
	m_DCT.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC17))->ShowWindow(SW_HIDE);
	m_DCA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_INS_NOTEMAP))->ShowWindow(SW_HIDE);


	((CStatic*)GetDlgItem(IDC_STATIC1))->ShowWindow(SW_SHOW); //FilterType
	m_FilterType.ShowWindow(SW_SHOW);

	((CStatic*)GetDlgItem(IDC_STATIC12))->ShowWindow(SW_HIDE); //Volume
	m_cutoffPan.ShowWindow(SW_SHOW);
	m_cutoffPan.SetWindowText("Filter Cutoff");
	m_SlVolCutoffPan.ShowWindow(SW_SHOW);
	m_SlVolCutoffPan.SetRangeMax(127);
	m_SlVolCutoffPan.SetRangeMin(0);
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC2))->ShowWindow(SW_SHOW); //Swing1
	((CStatic*)GetDlgItem(IDC_STATIC2))->SetWindowText("Swing (Randomize)");
	m_SlSwing1Glide.ShowWindow(SW_SHOW);
	m_SlSwing1Glide.SetRangeMax(100);
	((CStatic*)GetDlgItem(IDC_LSWING))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC13))->ShowWindow(SW_HIDE); //Fadeout
	m_Ressonance.ShowWindow(SW_SHOW);
	m_SlFadeoutRes.ShowWindow(SW_SHOW);
	m_SlFadeoutRes.SetRangeMax(127);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC18))->ShowWindow(SW_SHOW); //Swing2
	m_SlSwing2.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LSWING2))->ShowWindow(SW_SHOW); 

	((CStatic*)GetDlgItem(IDC_STATIC3))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteModNote.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->ShowWindow(SW_SHOW); //NOTEMOD
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_SHOW); //NOTEMOD

	m_EnvEnabled.ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVADSR))->ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->ShowWindow(SW_SHOW);
	((CScrollView*)GetDlgItem(IDC_INS_ENVELOPE))->ShowWindow(SW_SHOW);

	AssignFilterValues(inst);
	m_bInitialized = true;
}
	
void XMSamplerUIInst::OnBnClickedInsTpitch()
{
	m_bInitialized = false;
	int i= m_iCurrentSelected;
	XMInstrument& inst = m_pMachine->rInstrument(i);
	
	((CStatic*)GetDlgItem(IDC_INS_NOTESCROLL))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_FRAMENNA))->ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC14))->ShowWindow(SW_HIDE);//Instrument Name
	((CEdit*)GetDlgItem(IDC_INS_NAME))->ShowWindow(SW_HIDE);//Instrument Name
	((CStatic*)GetDlgItem(IDC_STATIC15))->ShowWindow(SW_HIDE);
	m_NNA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC16))->ShowWindow(SW_HIDE);
	m_DCT.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC17))->ShowWindow(SW_HIDE);
	m_DCA.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_INS_NOTEMAP))->ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_STATIC1))->ShowWindow(SW_HIDE); //FilterType
	m_FilterType.ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_STATIC12))->ShowWindow(SW_SHOW); //Volume/Tune
	((CStatic*)GetDlgItem(IDC_STATIC12))->SetWindowText("Seminote Tunning");
	m_cutoffPan.ShowWindow(SW_HIDE);
	m_SlVolCutoffPan.ShowWindow(SW_SHOW);
	m_SlVolCutoffPan.SetRangeMax(48);
	m_SlVolCutoffPan.SetRangeMin(-48);
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC2))->ShowWindow(SW_SHOW); //Swing1
	((CStatic*)GetDlgItem(IDC_STATIC2))->SetWindowText("Note Glide");
	m_SlSwing1Glide.ShowWindow(SW_SHOW);
	m_SlSwing1Glide.SetRangeMax(256);
	((CStatic*)GetDlgItem(IDC_LSWING))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC13))->ShowWindow(SW_HIDE); //Fadeout
	m_Ressonance.ShowWindow(SW_HIDE);
	m_SlFadeoutRes.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->ShowWindow(SW_HIDE); 
	((CStatic*)GetDlgItem(IDC_STATIC18))->ShowWindow(SW_HIDE); //Swing2
	m_SlSwing2.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LSWING2))->ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_STATIC3))->ShowWindow(SW_HIDE); //NOTEMOD
	m_SlNoteModNote.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->ShowWindow(SW_HIDE); //NOTEMOD
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_HIDE); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_HIDE); //NOTEMOD

	m_EnvEnabled.ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVADSR))->ShowWindow(SW_SHOW);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_INS_ENVELOPE))->ShowWindow(SW_SHOW);
	
	AssignPitchValues(inst);

	m_bInitialized = true;
}

void XMSamplerUIInst::OnCbnSelendokFiltertype()
{
	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);

	_inst.FilterType((dsp::FilterType)m_FilterType.GetCurSel());
}


void XMSamplerUIInst::OnNMCustomdrawVolCutoffPan(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	TRACE("incustomdraw\n");

	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	char tmp[64];
	
	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
		if (m_bInitialized) {
			TRACE("setting globvol for %d",i);
			_inst.GlobVol(m_SlVolCutoffPan.GetPos()/128.0f);
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TPAN))->GetCheck())
	{
		switch(m_SlVolCutoffPan.GetPos()+64)
		{
		case 0: sprintf(tmp,"||%02d  ",m_SlVolCutoffPan.GetPos()); break;
		case 64: sprintf(tmp," |%02d| ",m_SlVolCutoffPan.GetPos()); break;
		case 128: sprintf(tmp,"  %02d||",m_SlVolCutoffPan.GetPos()); break;
		default:
			if ( m_SlVolCutoffPan.GetPos() < -32) sprintf(tmp,"<<%02d  ",m_SlVolCutoffPan.GetPos());
			else if ( m_SlVolCutoffPan.GetPos() < 0) sprintf(tmp," <%02d< ",m_SlVolCutoffPan.GetPos());
			else if ( m_SlVolCutoffPan.GetPos() <= 32) sprintf(tmp," >%02d> ",m_SlVolCutoffPan.GetPos());
			else sprintf(tmp,"  %02d>>",m_SlVolCutoffPan.GetPos());
			break;
		}
		if (m_bInitialized) {
			_inst.Pan((m_SlVolCutoffPan.GetPos()+64)/128.0f);
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
		if (m_bInitialized) {
			_inst.FilterCutoff(m_SlVolCutoffPan.GetPos());
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TPITCH))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
		//_inst.Tune(m_SlVolCutoffPan.GetPos());
	}

	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->SetWindowText(tmp);

	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawFadeoutRes(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	char tmp[64];

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
	{
//		1024 / getpos() = number of ticks that needs to decrease to 0.
//		(24.0f * Global::player().bpm/60.0f) = number of ticks in a second.
//		sprintf(tmp,"%.0fms",(float) (1024/m_SlFadeoutRes.GetPos()) / (24.0f * Global::player().bpm/60.0f));
		if (m_SlFadeoutRes.GetPos() == 0) strcpy(tmp,"off");
		else sprintf(tmp,"%.0fms",2560000.0f/ (Global::pPlayer->bpm *m_SlFadeoutRes.GetPos()) );

		if (m_bInitialized)
		{
			_inst.VolumeFadeSpeed(m_SlFadeoutRes.GetPos()/1024.0f);
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlFadeoutRes.GetPos());
		if (m_bInitialized)
		{
			_inst.FilterResonance(m_SlFadeoutRes.GetPos());
		}
	}

	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->SetWindowText(tmp);

	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawSwing1Glide(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);

	char tmp[64];

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
		if (m_bInitialized)
		{
			_inst.RandomVolume(m_SlSwing1Glide.GetPos()/100.0f);
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TPAN))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
		if (m_bInitialized)
		{
			_inst.RandomPanning(m_SlSwing1Glide.GetPos()/100.0f);
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
		if (m_bInitialized)
		{
			_inst.RandomCutoff(m_SlSwing1Glide.GetPos()/100.0f);
		}
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TPITCH))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlSwing1Glide.GetPos());
		if (m_bInitialized)
		{
			//_inst.Glide(m_SlVolCutoffPan.GetPos());
		}
	}

	((CStatic*)GetDlgItem(IDC_LSWING))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawSwing2(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	
	char tmp[64];
	if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing2.GetPos());
		if (m_bInitialized)
		{
			_inst.RandomResonance(m_SlSwing2.GetPos()/100.0f);
		}
	}

	((CStatic*)GetDlgItem(IDC_LSWING2))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawNotemodnote(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SLNOTEMODNOTE);

	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	if (m_bInitialized)
	{
		_inst.NoteModPanCenter(slid->GetPos());
	}

	char tmp[40], tmp2[40];
	char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
	sprintf(tmp,"%s",notes[slid->GetPos()%12]);
	sprintf(tmp2,"%s%d",tmp,(slid->GetPos()/12));
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->SetWindowText(tmp2);
	
	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawNoteMod(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_NOTEMOD);

	int i= m_iCurrentSelected;
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	if (m_bInitialized)
	{
		_inst.NoteModPanSep(slid->GetPos());
	}
  
	char tmp[40];
	sprintf(tmp,"%.02f%%",(slid->GetPos()/2.56f));
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->SetWindowText(tmp);
	*pResult = 0;
}
void XMSamplerUIInst::OnBnClickedEnvcheck()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedEnvadsr()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}
		
void XMSamplerUIInst::OnBnClickedEnvfreeform()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnNMCustomdrawADSRBase(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	///\todo Agregue aquí su código de controlador de notificación de control
	if (m_bInitialized)
	{
	}
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRMod(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	///\todo Agregue aquí su código de controlador de notificación de control
	if (m_bInitialized)
	{
	}
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRAttack(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	///\todo Agregue aquí su código de controlador de notificación de control
	if (m_bInitialized)
	{
	}
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRDecay(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	///\todo Agregue aquí su código de controlador de notificación de control
	if (m_bInitialized)
	{
	}
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRSustain(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	///\todo Agregue aquí su código de controlador de notificación de control
	if (m_bInitialized)
	{
	}
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRRelease(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	///\todo Agregue aquí su código de controlador de notificación de control
	if (m_bInitialized)
	{
	}
	*pResult = 0;
}

	
void XMSamplerUIInst::OnLbnSelchangeInstrumentlist()
{
	TRACE("inselchangeinstlist\n");
	if(m_bInitialized)
	{
		m_bInitialized = false;
		SetInstrumentData(m_InstrumentList.GetCurSel());
		m_bInitialized = true;
	}
}

void XMSamplerUIInst::OnEnChangeInsName()
{
	if(m_bInitialized)
	{
		int i= m_iCurrentSelected;
		XMInstrument& _inst = m_pMachine->rInstrument(i);
		TCHAR _buf[256];
		m_InstrumentName.GetWindowText(_buf,sizeof(_buf));
		_inst.Name(_buf);
	}
}

			
void XMSamplerUIInst::OnCbnSelendokInsNnacombo()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}
	
void XMSamplerUIInst::OnCbnSelendokInsDctcombo()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnCbnSelendokInsDcacombo()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}
			
void XMSamplerUIInst::OnBnClickedLoadins()
{
	OPENFILENAME ofn; // common dialog box structure
	char szFile[_MAX_PATH]; // buffer for file name
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter =
		"All Instrument types (*.xi *.iti" "\0" "*.xi;*.iti;" "\0"
		"FastTracker II Instruments (*.xi)"              "\0" "*.xi"                  "\0"
		"Impulse Tracker Instruments (*.iti)"             "\0" "*.iti"                  "\0"
		"All (*)"                                  "\0" "*"                     "\0"
		;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	std::string tmpstr = Global::pConfig->GetCurrentInstrumentDir();
	ofn.lpstrInitialDir = tmpstr.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn)==TRUE)
	{
		CString str = ofn.lpstrFile;
		int index = str.ReverseFind('.');
		if (index != -1)
		{
			CString ext = str.Mid(index+1);
			if (ext.CompareNoCase("XI") == 0)
			{
				XMSongLoader xmsong;
				xmsong.Open(ofn.lpstrFile);
				xmsong.LoadInstrumentFromFile(*pMachine(),m_iCurrentSelected);
				xmsong.Close();
			}
			else if (ext.CompareNoCase("ITI") == 0)
			{
				ITModule2 itsong;
				itsong.Open(ofn.lpstrFile);
				itsong.LoadInstrumentFromFile(*pMachine(),m_iCurrentSelected);
			}
		}
	}
}

void XMSamplerUIInst::OnBnClickedSaveins()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedDupeins()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedDeleteins()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}
void XMSamplerUIInst::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_SampleAssign.OnHScroll(nSBCode,nPos,pScrollBar);
}
			
//////////////////////////////////////////////////////////////////////////////
// CEnvelopeEditor -----------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
XMSamplerUIInst::CEnvelopeEditor::CEnvelopeEditor()
: m_pEnvelope(NULL)
, m_pXMSampler(NULL)
, m_bInitialized(false)
, m_bPointEditing(false)
, m_EditPoint(0)
{
	_line_pen.CreatePen(PS_SOLID,0,RGB(0,0,255));
	_gridpen.CreatePen(PS_SOLID,0,RGB(224,224,255));
	_gridpen1.CreatePen(PS_SOLID,0,RGB(255,224,224));
	_point_brush.CreateSolidBrush(RGB(0,0,255));
}
XMSamplerUIInst::CEnvelopeEditor::~CEnvelopeEditor(){
	_line_pen.DeleteObject();
	_gridpen.DeleteObject();
	_gridpen1.DeleteObject();
	_point_brush.DeleteObject();
}
			
BEGIN_MESSAGE_MAP(XMSamplerUIInst::CEnvelopeEditor, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID__ENV_ADDNEWPOINT, OnPopAddPoint)
	ON_COMMAND(ID__ENV_SETSUSTAINBEGIN, OnPopSustainStart)
	ON_COMMAND(ID__ENV_SETSUSTAINEND, OnPopSustainEnd)
	ON_COMMAND(ID__ENV_SETLOOPSTART, OnPopLoopStart)
	ON_COMMAND(ID__ENV_SETLOOPEND, OnPopLoopEnd)
	ON_COMMAND(ID__ENV_REMOVEPOINT, OnPopRemovePoint)
	ON_COMMAND(ID__ENV_REMOVESUSTAIN, OnPopRemoveSustain)
	ON_COMMAND(ID__ENV_REMOVELOOP, OnPopRemoveLoop)
	ON_COMMAND(ID__ENV_REMOVEENVELOPE, OnPopRemoveEnvelope)
END_MESSAGE_MAP()
			
			
void XMSamplerUIInst::CEnvelopeEditor::Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope)
{
	m_pXMSampler =pSampler;
	m_pEnvelope = pEnvelope;
			
	CRect _rect;
	GetClientRect(&_rect);
	m_WindowHeight = _rect.Height();
	m_WindowWidth = _rect.Width();
			
	m_Zoom = 8.0f;
	const int _points =  m_pEnvelope->NumOfPoints();
	m_EditPoint = _points;

	if (_points > 0 )
				{
		while (m_Zoom * m_pEnvelope->GetTime(_points-1) > m_WindowWidth)
					{
			m_Zoom= m_Zoom/2.0f;
				}
			}

	m_bInitialized = true;
	Invalidate();
			
}
void XMSamplerUIInst::CEnvelopeEditor::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if(m_bInitialized){
		if (m_pEnvelope && lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
			{
			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
			CPen *oldpen= dc.SelectObject(&_gridpen);
			
			CRect _rect;
			GetClientRect(&_rect);

//			dc.FillRect(&_rect,&brush);
			dc.FillSolidRect(&_rect,RGB(255,255,255));
			dc.SetBkMode(TRANSPARENT);
			
			// ***** Background lines *****
			float _stepy = ((float)(m_WindowHeight)) / 100.0f * 10.0f;
			
			for(float i = 0; i <= (float)m_WindowHeight; i += _stepy)
			{
				dc.MoveTo(0,i);
				dc.LineTo(m_WindowWidth,i);
			}
			
			const int _points =  m_pEnvelope->NumOfPoints();

			int _mod = 0.0f;
			float tenthsec = m_Zoom*0.04*Global::player().bpm;
			int _sec = 0;

			for(float i = 0; i < m_WindowWidth; i+=tenthsec)
			{
				if (_mod == 5 )
				{
					dc.MoveTo(i,0);
					dc.LineTo(i,m_WindowHeight);
				} else if(_mod == 10 ) {
					_mod = 0;
					dc.SelectObject(&_gridpen1);
					dc.MoveTo(i,0);
					dc.LineTo(i,m_WindowHeight);
					dc.SelectObject(&_gridpen);
					// *****  *****
					_sec += 1;
					CString string;
					string.Format("%ds",_sec);
					dc.TextOut(i,m_WindowHeight-20,string);
				} else {
					dc.MoveTo(i,m_WindowHeight-5);
					dc.LineTo(i,m_WindowHeight);
				}
				_mod++;
			}
			
			// Sustain Point *****  *****

			if(m_pEnvelope->SustainBegin() != XMInstrument::Envelope::INVALID)
			{
				const int _pt_start_x = (m_Zoom * (float)m_pEnvelope->GetTime(m_pEnvelope->SustainBegin()));
				const int _pt_end_x = (m_Zoom * (float)m_pEnvelope->GetTime(m_pEnvelope->SustainEnd()));

				CPen _edit_line_pen(PS_DOT,0,RGB(64,192,128));

				dc.SelectObject(&_edit_line_pen);
				dc.MoveTo(_pt_start_x,0);
				dc.LineTo(_pt_start_x,m_WindowHeight);
				dc.MoveTo(_pt_end_x,0);
				dc.LineTo(_pt_end_x,m_WindowHeight);
			}
			
			// Loop Start *****  ***** Loop End *****
			
			if(m_pEnvelope->LoopStart() != XMInstrument::Envelope::INVALID && 
				m_pEnvelope->LoopEnd() != XMInstrument::Envelope::INVALID)
			{
				const int _pt_loop_start_x = m_Zoom * (float)m_pEnvelope->GetTime(m_pEnvelope->LoopStart());
				const int _pt_loop_end_x = m_Zoom * (float)m_pEnvelope->GetTime(m_pEnvelope->LoopEnd());
				
				// Envelope Point *****  ***** Sustain Label *****  *****
				CPen _loop_pen(PS_SOLID,0,RGB(64,192,128));

				/*				This would be to show the loop range, but without alpha blending, this is not nice.
				CBrush  _loop_brush;
				_loop_brush.CreateSolidBrush(RGB(64, 0, 128));
				CRect rect(_pt_loop_start_x,0,_pt_loop_end_x - _pt_loop_start_x,m_WindowHeight);
				dc.FillRect(&rect,&_loop_brush);
			
				dc.TextOut(((_pt_loop_end_x - _pt_loop_start_x) / 2 + _pt_loop_start_x - 20),(m_WindowHeight / 2),"Loop");
				_loop_brush.DeleteObject();
				*/
				dc.SelectObject(&_loop_pen);
				dc.MoveTo(_pt_loop_start_x,0);
				dc.LineTo(_pt_loop_start_x,m_WindowHeight);
				dc.MoveTo(_pt_loop_end_x,0);
				dc.LineTo(_pt_loop_end_x,m_WindowHeight);

			}


			// ***** Draw Envelope line and points *****
			CPoint _pt_start;
			if ( _points > 0 ) 
				{
				_pt_start.x=0;
				_pt_start.y=(int)((float)m_WindowHeight * (1.0f - m_pEnvelope->GetValue(0)));
				}
			for(int i = 1;i < _points ;i++)
				{
				CPoint _pt_end;
				_pt_end.x = (int)(m_Zoom * (float)m_pEnvelope->GetTime(i)); 
				_pt_end.y = (int)((float)m_WindowHeight * (1.0f - m_pEnvelope->GetValue(i)));
				dc.SelectObject(&_line_pen);
				dc.MoveTo(_pt_start);
				dc.LineTo(_pt_end);
				_pt_start = _pt_end;
				}

			for(unsigned int i = 0;i < _points ;i++)
			{
				CPoint _pt(
					(int)(m_Zoom * (float)m_pEnvelope->GetTime(i)), 
					(int)((float)m_WindowHeight * (1.0f - m_pEnvelope->GetValue(i)))
					);
				CRect rect(_pt.x - (POINT_SIZE / 2),_pt.y - (POINT_SIZE / 2),_pt.x + (POINT_SIZE / 2),_pt.y + (POINT_SIZE / 2));
				if ( m_EditPoint == i )
				{
					CBrush _edit_brush;
					_edit_brush.CreateSolidBrush(RGB(0,192,192));
					dc.FillRect(&rect,&_edit_brush);
					_edit_brush.DeleteObject();
				}
				else dc.FillRect(&rect,&_point_brush);
			}

	
			dc.SelectObject(oldpen);
			dc.Detach();
		}
			}
}

void XMSamplerUIInst::CEnvelopeEditor::OnLButtonDown( UINT nFlags, CPoint point )
{
	SetFocus();
			
	if(!m_bPointEditing){
		const int _points =  m_pEnvelope->NumOfPoints();

		int _edit_point = GetEnvelopePointIndexAtPoint(point.x,point.y);
		if(_edit_point != _points)
			{
			m_bPointEditing = true;
			SetCapture();
			m_EditPoint = _edit_point;
		}
		else 
			{
			m_EditPoint = _points;
			Invalidate();
		}
	}
}
void XMSamplerUIInst::CEnvelopeEditor::OnLButtonUp( UINT nFlags, CPoint point )
{
		if(m_bPointEditing){
			ReleaseCapture();
			m_bPointEditing =  false;
				
		if (point.x > m_WindowWidth ) m_Zoom = m_Zoom /2.0f;
		else if ( m_pEnvelope->GetTime(m_pEnvelope->NumOfPoints()-1)*m_Zoom < m_WindowWidth/2 && m_Zoom < 8.0f) m_Zoom = m_Zoom *2.0f;
			
		/*
		//\todo: verify the necessity of this code, when it is already present in MouseMove.
		int _new_point = (int)((float)point.x / m_Zoom);
		float _new_value = (1.0f - (float)point.y / (float)m_WindowHeight);
			
			if(_new_value > 1.0f)
			{
				_new_value = 1.0f;
			}

			if(_new_value < 0.0f)
			{
				_new_value = 0.0f;
			}

			if( _new_point < 0)
			{
				_new_point = 0;
			}
		m_pEnvelope->SetTimeAndValue(m_EditPoint,_new_point,_new_value);
		*/
			Invalidate();
		}
}
void XMSamplerUIInst::CEnvelopeEditor::OnMouseMove( UINT nFlags, CPoint point )
{
		if(m_bPointEditing)
		{
		if(point.y > m_WindowHeight)
		{
			point.y = m_WindowHeight;
	}

		if(point.y < 0)
	{
			point.y = 0;
			}

		if( point.x < 0)
		{
			point.x = 0;
		}
		if ( point.x > m_WindowWidth)
			{
			//what to do? unzoom... but what about the mouse?
			}
		if ( m_EditPoint == 0 )
		{
			point.x=0;
		}
		m_EditPointX = point.x;
		m_EditPointY = point.y;
		m_EditPoint = m_pEnvelope->SetTimeAndValue(m_EditPoint,(int)((float)m_EditPointX / m_Zoom),
			(1.0f - (float)m_EditPointY / (float)m_WindowHeight));

		Invalidate();
			}
}
			
void XMSamplerUIInst::CEnvelopeEditor::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CPoint tmp;
	tmp = point;
	ScreenToClient(&tmp);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MENU_ENV_EDIT));
	int _edit_point = GetEnvelopePointIndexAtPoint(tmp.x,tmp.y);
	m_EditPointX = tmp.x;
	m_EditPointY = tmp.y;
	m_EditPoint = _edit_point;

	CMenu* pPopup = menu.GetSubMenu(0);
	assert(pPopup);

	// The ON_UPDATE_COMMAND_UI message handlers only work with CFrameWnd's, and CStatic is a CWnd.
	// This is documented to be the case, by Microsoft.
	if ( m_EditPoint == m_pEnvelope->NumOfPoints() 
		|| m_pEnvelope->SustainBegin() == m_EditPoint) menu.EnableMenuItem(ID__ENV_SETSUSTAINBEGIN,MF_GRAYED);

	if ( m_EditPoint == m_pEnvelope->NumOfPoints()
		|| m_pEnvelope->SustainEnd() == m_EditPoint) menu.EnableMenuItem(ID__ENV_SETSUSTAINEND,MF_GRAYED);
			
	if ( m_EditPoint == m_pEnvelope->NumOfPoints() 
		|| m_pEnvelope->LoopStart() == m_EditPoint) menu.EnableMenuItem(ID__ENV_SETLOOPSTART,MF_GRAYED);

	if ( m_EditPoint == m_pEnvelope->NumOfPoints()
		|| m_pEnvelope->LoopEnd() == m_EditPoint) menu.EnableMenuItem(ID__ENV_SETLOOPEND,MF_GRAYED);
	        
	if ( m_EditPoint == m_pEnvelope->NumOfPoints()) menu.EnableMenuItem(ID__ENV_REMOVEPOINT,MF_GRAYED);
	        
	if(m_pEnvelope->SustainBegin() == XMInstrument::Envelope::INVALID) menu.EnableMenuItem(ID__ENV_REMOVESUSTAIN,MF_GRAYED);

	if(m_pEnvelope->LoopStart() == XMInstrument::Envelope::INVALID) menu.EnableMenuItem(ID__ENV_REMOVELOOP,MF_GRAYED);

	if ( m_pEnvelope->NumOfPoints() == 0 ) menu.EnableMenuItem(ID__ENV_REMOVEENVELOPE,MF_GRAYED);
	//

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	menu.DestroyMenu();

	CWnd::OnContextMenu(pWnd,point);
}

			
void XMSamplerUIInst::CEnvelopeEditor::OnPopAddPoint()
{
		
		int _new_point = (int)((float)m_EditPointX / m_Zoom);
	float _new_value = (1.0f - (float)m_EditPointY / (float)m_WindowHeight);

		
		if(_new_value > 1.0f)
		{
			_new_value = 1.0f;
		}

		if(_new_value < 0.0f)
		{
			_new_value = 0.0f;
		}

		if( _new_point < 0)
		{
			_new_point = 0;
		}

		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
	//\todo : Verify that we aren't trying to add an existing point!!!

	if ( m_pEnvelope->NumOfPoints() == 0 && _new_point != 0 ) m_EditPoint = m_pEnvelope->Insert(0,1.0f);
	m_EditPoint = m_pEnvelope->Insert(_new_point,_new_value);
	Invalidate();
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopSustainStart()
{
	if ( m_EditPoint != m_pEnvelope->NumOfPoints())
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->SustainBegin(m_EditPoint);
		if (m_pEnvelope->SustainEnd()== XMInstrument::Envelope::INVALID ) m_pEnvelope->SustainEnd(m_EditPoint);
		else if (m_pEnvelope->SustainEnd() < m_EditPoint )m_pEnvelope->SustainEnd(m_EditPoint);
			Invalidate();
		}
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopSustainEnd()
{
	if ( m_EditPoint != m_pEnvelope->NumOfPoints())
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		if (m_pEnvelope->SustainBegin()== XMInstrument::Envelope::INVALID ) m_pEnvelope->SustainBegin(m_EditPoint);
		else if (m_pEnvelope->SustainBegin() > m_EditPoint )m_pEnvelope->SustainBegin(m_EditPoint);
		m_pEnvelope->SustainEnd(m_EditPoint);
			Invalidate();
		}
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopLoopStart()
{
	if ( m_EditPoint != m_pEnvelope->NumOfPoints())
	{
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->LoopStart(m_EditPoint);
		if (m_pEnvelope->LoopEnd()== XMInstrument::Envelope::INVALID ) m_pEnvelope->LoopEnd(m_EditPoint);
		else if (m_pEnvelope->LoopEnd() < m_EditPoint )m_pEnvelope->LoopEnd(m_EditPoint);
		Invalidate();
	}
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopLoopEnd()
{
	if ( m_EditPoint != m_pEnvelope->NumOfPoints())
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		if (m_pEnvelope->LoopStart()== XMInstrument::Envelope::INVALID ) m_pEnvelope->LoopStart(m_EditPoint);
		else if (m_pEnvelope->LoopStart() > m_EditPoint )m_pEnvelope->LoopStart(m_EditPoint);
		m_pEnvelope->LoopEnd(m_EditPoint);
			Invalidate();
		}
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopRemovePoint()
{
	const int _points =  m_pEnvelope->NumOfPoints();
	if(_points == 0)
	{
		return;
	}

	CRect _rect;
	GetClientRect(&_rect);


	if(m_EditPoint != _points)
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->Delete(m_EditPoint);
		m_EditPoint = _points;
			Invalidate();
		}
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopRemoveSustain()
{ 
	boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
	m_pEnvelope->SustainBegin(XMInstrument::Envelope::INVALID);
	m_pEnvelope->SustainEnd(XMInstrument::Envelope::INVALID);
	Invalidate();
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopRemoveLoop()
{ 
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->LoopStart(XMInstrument::Envelope::INVALID);
		m_pEnvelope->LoopEnd(XMInstrument::Envelope::INVALID);
		Invalidate();
}
void XMSamplerUIInst::CEnvelopeEditor::OnPopRemoveEnvelope()
{
	boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
	m_pEnvelope->Clear();
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
// SampleAssignEditor ------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

//static const unsigned int XMSamplerUIInst::CSampleAssignEditor::m_Sharpkey_Xpos[5]= {27,96,190,246,304};
//const unsigned int XMSamplerUIInst::CSampleAssignEditor::m_Sharpkey_Xpos[]= {16,55,110,148,185};
const int XMSamplerUIInst::CSampleAssignEditor::m_SharpKey_Xpos[]= {15,44,92,121,150};
const XMSamplerUIInst::CSampleAssignEditor::TNoteKey XMSamplerUIInst::CSampleAssignEditor::m_NoteAssign[]=
{XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,
XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey};
const int XMSamplerUIInst::CSampleAssignEditor::m_noteAssignindex[m_KeysPerOctave] = {0,0,1,1,2,3,2,4,3,5,4,6};

XMSamplerUIInst::CSampleAssignEditor::CSampleAssignEditor()
: m_bInitialized(false)
, m_Octave(3)
{
	m_NaturalKey.LoadBitmap(IDB_KEYS_NORMAL);
	m_SharpKey.LoadBitmap(IDB_KEYS_SHARP);
	m_BackKey.LoadBitmap(IDB_KEYS_BACK);

	BITMAP _bmp, _bmp2;
	m_NaturalKey.GetBitmap(&_bmp);
	m_SharpKey.GetBitmap(&_bmp2);
	m_naturalkey_width = _bmp.bmWidth;
	m_naturalkey_height = _bmp.bmHeight;
	m_sharpkey_width = _bmp2.bmWidth;
	m_sharpkey_height = _bmp2.bmHeight;

	m_octave_width = m_naturalkey_width * m_NaturalKeysPerOctave;
}
XMSamplerUIInst::CSampleAssignEditor::~CSampleAssignEditor()
{
	m_NaturalKey.DeleteObject();
	m_SharpKey.DeleteObject();
	m_BackKey.DeleteObject();
}

BEGIN_MESSAGE_MAP(XMSamplerUIInst::CSampleAssignEditor, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()
/*
BEGIN_MESSAGE_MAP(XMSamplerUIInst::CEnvelopeEditor, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID__ENV_ADDNEWPOINT, OnPopAddPoint)
	ON_COMMAND(ID__ENV_SETSUSTAINBEGIN, OnPopSustainStart)
	ON_COMMAND(ID__ENV_SETSUSTAINEND, OnPopSustainEnd)
	ON_COMMAND(ID__ENV_SETLOOPSTART, OnPopLoopStart)
	ON_COMMAND(ID__ENV_SETLOOPEND, OnPopLoopEnd)
	ON_COMMAND(ID__ENV_REMOVEPOINT, OnPopRemovePoint)
	ON_COMMAND(ID__ENV_REMOVESUSTAIN, OnPopRemoveSustain)
	ON_COMMAND(ID__ENV_REMOVELOOP, OnPopRemoveLoop)
	ON_COMMAND(ID__ENV_REMOVEENVELOPE, OnPopRemoveEnvelope)
END_MESSAGE_MAP()
*/
	
void XMSamplerUIInst::CSampleAssignEditor::Initialize(XMSampler * const pSampler,XMInstrument * const pInstrument,CWnd* pParent)
{
	m_pSampler = pSampler;
	m_pInst = pInstrument;
	m_bInitialized=true;
	SCROLLINFO info;
	m_scBar.GetScrollInfo(&info, SIF_PAGE|SIF_RANGE);
	info.fMask = SIF_RANGE|SIF_POS;
	info.nMin = 0;
	info.nMax  = 8;
	info.nPos = m_Octave;
	m_scBar.SetScrollInfo(&info, false);
	Invalidate();
}
	
void XMSamplerUIInst::CSampleAssignEditor::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
		if(m_bInitialized){
		if (lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
		{
			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
			CRect _rect;
			GetClientRect(&_rect);
			dc.FillSolidRect(&_rect,RGB(0,0,0));
			dc.SetBkMode(TRANSPARENT);
	
			const CString _Key_name[m_KeysPerOctave] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
			const CString _NaturalKey_name[m_NaturalKeysPerOctave] = {"C","D","E","F","G","A","B"};
			const int _NaturalKey_index[m_NaturalKeysPerOctave] = {0,2,4,5,7,9,11};
//			const CString _SharpKey_name[m_SharpKeysPerOctave] = {"C#","D#","F#","G#","A#"};
			const int _SharpKey_index[m_SharpKeysPerOctave] = {1,3,6,8,10};
//			const unsigned int _end = 2*m_NaturalKeysPerOctave;//show two octaves.
//			const unsigned int _ends = 2*m_SharpKeysPerOctave;//show two octaves.


			CDC memDC;
			memDC.CreateCompatibleDC(&dc);
			CBitmap* oldbmp = memDC.SelectObject(&m_BackKey);
			dc.SetTextColor(RGB(255,255,255));

			CString _tmp_str;
			int _index = 0,_octave = 0;

			//Draw top background image.
			for(int i = 0;i < _rect.Width() && m_Octave+_octave<10;i+=26)
			{
				dc.BitBlt(i,0, 	26,20, 	&memDC, 0,0,	SRCCOPY);
				_tmp_str.Format("%s%d",_NaturalKey_name[_index],_octave+m_Octave);
				if (m_FocusKeyRect.left>=i && m_FocusKeyRect.left<i+26 && m_FocusKeyRect.left!=m_FocusKeyRect.right){
					dc.SetTextColor(RGB(255,255,0));
					dc.TextOut(i+7,4,_tmp_str);
					dc.SetTextColor(RGB(255,255,255));
				}
				else { dc.TextOut(i+7,4,_tmp_str); }
				
				_index++;
				if(_index == m_NaturalKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}

			BITMAP _bmp, _bmps;
			m_NaturalKey.GetBitmap(&_bmp);
			m_SharpKey.GetBitmap(&_bmps);

			memDC.SelectObject(&m_NaturalKey);
			dc.SetTextColor(RGB(128,96,32));

			_index = 0,_octave = 0;
			for(int i = 0;i*_bmp.bmWidth < _rect.Width() && m_Octave+_octave<10;i++)
			{
				dc.BitBlt(i*_bmp.bmWidth,20, 
					_bmp.bmWidth,_bmp.bmHeight, &memDC, 0,0,	SRCCOPY);
				_tmp_str.Format("%s-%d"
					,_Key_name[m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).first%12]
					,m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).first/12);
				dc.TextOut(7+i*_bmp.bmWidth,68,_tmp_str);
				int _sample = m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).second;
				if ( _sample == 255 ) _tmp_str="--";
				else _tmp_str.Format("%02X",m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).second);
				dc.TextOut(7+i*_bmp.bmWidth,80,_tmp_str);
				_index++;
				if(_index == m_NaturalKeysPerOctave){
					_index = 0;
					_octave++;
				}
				}

			_index = 0,_octave = 0;
			memDC.SelectObject(&m_SharpKey);
			for(int i = 0;m_SharpKey_Xpos[_index]+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave < _rect.Width() && m_Octave+_octave<10;i++)
			{
				dc.BitBlt(m_SharpKey_Xpos[_index]+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,20, 
				_bmps.bmWidth,	_bmps.bmHeight, 	&memDC,		0,0,	SRCCOPY);
				_tmp_str.Format("%s%d"
					,_Key_name[m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).first%12]
					,m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).first/12);
				dc.TextOut(m_SharpKey_Xpos[_index]-10+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,24,_tmp_str);
				
				int _sample=m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).second;
				if ( _sample == 255 ) _tmp_str="--";
				else  _tmp_str.Format("%02X",m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).second);
				dc.TextOut(m_SharpKey_Xpos[_index]-10+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,36,_tmp_str);
				_index++;
				if(_index == m_SharpKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}

			//
			memDC.SelectObject(oldbmp);
			dc.Detach();
		}
	}
}

int XMSamplerUIInst::CSampleAssignEditor::GetKeyIndexAtPoint(const int x,const int y,CRect& keyRect)
{
	if ( y < 20 || y > 20+m_naturalkey_height ) return notecommands::empty;

	//Get the X position in natural key notes.
	int notenatural= ((x/m_octave_width)*m_NaturalKeysPerOctave);
	int indexnote = ((x%m_octave_width)/m_naturalkey_width);
	notenatural+=indexnote;

	keyRect.top = 20;
	keyRect.bottom = 20+m_naturalkey_height;
	keyRect.left = (notenatural)*m_naturalkey_width;
	keyRect.right = keyRect.left+m_naturalkey_width;

	//Adapt the index note to a 12note range instead of 7note.
	int note = ((x/m_octave_width)*m_KeysPerOctave);
	int cnt=0;
	while(m_noteAssignindex[cnt]!= indexnote) cnt++;
	indexnote=cnt;
	note += cnt;

	if ( y > 20+m_sharpkey_height ) 
	{
		return note+(m_Octave*m_KeysPerOctave);
	}


	//If the code reaches here, we have to check if it is a sharp key or a natural one.

	//Check previous sharp note
	if (indexnote > 0 && m_NoteAssign[indexnote-1]==SharpKey)
	{
		const int _xpos = m_SharpKey_Xpos[m_noteAssignindex[indexnote-1]] + (note / m_KeysPerOctave) * m_octave_width;
		if(x >= _xpos && x <= (_xpos + m_sharpkey_width))
	{
			keyRect.bottom = m_sharpkey_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + m_sharpkey_width;
			return note-1+(m_Octave*m_KeysPerOctave);
			}
			}
	//Check next sharp note
	if ( indexnote+1<m_KeysPerOctave && m_NoteAssign[indexnote+1]==SharpKey)
	{
		const int _xpos = m_SharpKey_Xpos[m_noteAssignindex[indexnote+1]] + (note / m_KeysPerOctave) * m_octave_width;
		if(x >= _xpos && x <= (_xpos + m_sharpkey_width))
		{
			keyRect.bottom = m_sharpkey_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + m_sharpkey_width;
			return note+1+(m_Octave*m_KeysPerOctave);
		}
	}
	//Not a valid sharp note. Return the already found note.
	return note+(m_Octave*m_KeysPerOctave);
}
void XMSamplerUIInst::CSampleAssignEditor::OnMouseMove( UINT nFlags, CPoint point )
{
	int tmp = m_FocusKeyIndex;
	m_FocusKeyIndex=GetKeyIndexAtPoint(point.x,point.y,m_FocusKeyRect);
	if  ( tmp != m_FocusKeyIndex) Invalidate();
}
void XMSamplerUIInst::CSampleAssignEditor::OnLButtonDown( UINT nFlags, CPoint point )
{
MessageBox("hola");
}
void XMSamplerUIInst::CSampleAssignEditor::OnLButtonUp( UINT nFlags, CPoint point )
{
MessageBox("hola");
}

void XMSamplerUIInst::CSampleAssignEditor::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
		switch(nSBCode)
	{
		case SB_TOP:
			m_Octave=0;
			m_scBar.SetScrollPos(m_Octave);
			Invalidate();
			break;
		case SB_BOTTOM:
			m_Octave=8;
			m_scBar.SetScrollPos(m_Octave);
			Invalidate();
			break;
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if ( m_Octave < 8)
	{
				m_Octave++;
				m_scBar.SetScrollPos(m_Octave);
				Invalidate();
	}
			break;
		case SB_LINELEFT:
		case SB_PAGELEFT:
			if ( m_Octave>0 )
	{
				m_Octave--;
				m_scBar.SetScrollPos(m_Octave);
				Invalidate();
			}
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			if (m_Octave!= (int)nPos)
		{
				m_Octave=(int)nPos;
				if (m_Octave > 8)
			{
					m_Octave = 8;
			}
				else if (m_Octave < 0)
			{
					m_Octave = 0;
				}
				m_scBar.SetScrollPos(m_Octave);
				Invalidate();
			}
			break;
		default: 
			break;
		}
	
		//for(int i = 0;i < XMInstrument::MAX_NOTES;i++)
		//{
		//	CPoint _pt_env;
		//	_pt_env.y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)));
		//	_pt_env.x = (int)(m_Zoom * (float)m_pEnvelope->Point(i));
		//	if(((_pt_env.x - POINT_SIZE / 2) <= x) & ((_pt_env.x + POINT_SIZE / 2) >= x) &
		//		((_pt_env.y - POINT_SIZE / 2) <= y) & ((_pt_env.y + POINT_SIZE / 2) >= y))
		//	{
		//		return i;
		//	}
		//}

	};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

