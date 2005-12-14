#include <project.private.hpp>
#include "Psycle.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSampler.hpp"
#include "Player.hpp"
#include ".\xmsampleruiinst.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

//////////////////////////////////////////////////////////////////////////////
// XMSamplerUIInst -----------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(XMSamplerUIInst, CPropertyPage)
XMSamplerUIInst::XMSamplerUIInst()
: CPropertyPage(XMSamplerUIInst::IDD)
, m_bInitialized(false)
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

END_MESSAGE_MAP()


void XMSamplerUIInst::SetInstrumentData(const int instno)
{
	m_bInitialized = false;
	XMInstrument& inst = m_pMachine->rInstrument(instno);

	m_InstrumentName.SetWindowText(inst.Name().c_str());
	SetNewNoteAction(inst.NNA(),inst.DCT(),inst.DCA());
	m_SampleAssign.Initialize(m_pMachine,&m_pMachine->rInstrument(instno));

	m_FilterType.SetCurSel((int)inst.FilterType());

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
		AssignAmplitudeValues(inst);
	else if (((CButton*)GetDlgItem(IDC_INS_TPAN))->GetCheck())
		AssignPanningValues(inst);
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
		AssignFilterValues(inst);
	else if (((CButton*)GetDlgItem(IDC_INS_TPITCH))->GetCheck())
		AssignPitchValues(inst);

	m_bInitialized = true;
}

void XMSamplerUIInst::SetNewNoteAction(const int nna,const int dct,const int dca)
{
	m_NNA.SetCurSel(nna);
	m_DCT.SetCurSel(dct);
	m_DCA.SetCurSel(dca);
}

void XMSamplerUIInst::AssignAmplitudeValues(XMInstrument& inst)
{
	m_SlVolCutoffPan.SetPos(inst.GlobVol()*128.0f);
	m_SlFadeoutRes.SetPos(inst.VolumeFadeSpeed()*1024.0f);
	m_SlSwing1Glide.SetPos(inst.RandomVolume()*100.0f);
	//m_SlNoteModNote.SetPos(inst.NoteModVolCenter());
	//m_SlNoteMod.SetPos(inst.NoteModVolSep());

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
	m_SlVolCutoffPan.SetPos((inst.Pan()*128.0f)-64.0f);
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
	if ( inst.FilterCutoff()&0x80)
	{
		m_SlVolCutoffPan.SetPos(inst.FilterCutoff()&0x80);
	} else {
		m_SlVolCutoffPan.SetPos(inst.FilterCutoff());
	}

	m_SlFadeoutRes.SetPos(inst.FilterResonance());
	m_SlSwing1Glide.SetPos(inst.RandomCutoff()*100.0f);
	m_SlSwing2.SetPos(inst.RandomResonance()*100.0f);
	//m_SlNoteModNote.SetPos(inst.NoteModFilterCenter());
	//m_SlNoteMod.SetPos(inst.NoteModFilterSep());

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
	//m_SlVolCutoffPan.SetPos(inst.Tune());
	//m_SlSwing1Glide.SetPos(inst.Glide());
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
	m_bInitialized = false;
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& inst = m_pMachine->rInstrument(i);

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
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& inst = m_pMachine->rInstrument(i);

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
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& inst = m_pMachine->rInstrument(i);

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
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& inst = m_pMachine->rInstrument(i);

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
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& _inst = m_pMachine->rInstrument(i);

	_inst.FilterType((dsp::FilterType)m_FilterType.GetCurSel());
}


void XMSamplerUIInst::OnNMCustomdrawVolCutoffPan(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	char tmp[64];

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
		_inst.GlobVol(m_SlVolCutoffPan.GetPos()/128.0f);
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
		_inst.Pan((m_SlVolCutoffPan.GetPos()+64)/128.0f);
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
		_inst.FilterCutoff(m_SlVolCutoffPan.GetPos());
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
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	int i= m_InstrumentList.GetCurSel();
	XMInstrument& _inst = m_pMachine->rInstrument(i);
	char tmp[64];

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
	{
//		1024 / getpos() = number of ticks that needs to decrease to 0.
//		(24.0f * Global::pPlayer->bpm/60.0f) = number of ticks in a second.
//		sprintf(tmp,"%.0fms",(float) (1024/m_SlFadeoutRes.GetPos()) / (24.0f * Global::pPlayer->bpm/60.0f));
		if (m_SlFadeoutRes.GetPos() == 0) strcpy(tmp,"off");
		else sprintf(tmp,"%.0fms",2560000.0f/ (Global::pPlayer->bpm *m_SlFadeoutRes.GetPos()) );

		_inst.VolumeFadeSpeed(m_SlFadeoutRes.GetPos()/1024.0f);
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlFadeoutRes.GetPos());
		_inst.FilterResonance(m_SlFadeoutRes.GetPos());
	}

	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->SetWindowText(tmp);
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawSwing1Glide(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& _inst = m_pMachine->rInstrument(i);

	char tmp[64];

	if (((CButton*)GetDlgItem(IDC_INS_TAMP))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
		_inst.RandomVolume(m_SlSwing1Glide.GetPos()/100.0f);
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TPAN))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
		_inst.RandomPanning(m_SlSwing1Glide.GetPos()/100.0f);
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
		_inst.RandomCutoff(m_SlSwing1Glide.GetPos()/100.0f);
	}
	else if (((CButton*)GetDlgItem(IDC_INS_TPITCH))->GetCheck())
	{
		sprintf(tmp,"%d",m_SlSwing1Glide.GetPos());
		//_inst.Glide(m_SlVolCutoffPan.GetPos());
	}

	((CStatic*)GetDlgItem(IDC_LSWING))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawSwing2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int i= m_InstrumentList.GetCurSel();
	XMInstrument& _inst = m_pMachine->rInstrument(i);

	char tmp[64];
	if (((CButton*)GetDlgItem(IDC_INS_TFILTER))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing2.GetPos());
		_inst.RandomResonance(m_SlSwing2.GetPos()/100.0f);
	}

	((CStatic*)GetDlgItem(IDC_LSWING2))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawNotemodnote(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SLNOTEMODNOTE);


/*	if ()
	{
	}
	else
	{
	}
*/
	char tmp[40], tmp2[40];
	char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
	sprintf(tmp,"%s",notes[slid->GetPos()%12]);
	sprintf(tmp2,"%s%d",tmp,(slid->GetPos()/12));
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->SetWindowText(tmp2);


	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawNoteMod(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	/*	if ()
	{
	}
	else
	{
	}
	*/

	char tmp[40];
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_NOTEMOD);
	sprintf(tmp,"%.02f%%",(slid->GetPos()/2.56f));
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->SetWindowText(tmp);

	*pResult = 0;
}
void XMSamplerUIInst::OnBnClickedEnvcheck()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedEnvadsr()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedEnvfreeform()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnNMCustomdrawADSRBase(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRMod(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRAttack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRDecay(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRSustain(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRRelease(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}


void XMSamplerUIInst::OnLbnSelchangeInstrumentlist()
{
	if(m_bInitialized)
	{
		SetInstrumentData(m_InstrumentList.GetCurSel());
	}
}

void XMSamplerUIInst::OnEnChangeInsName()
{
	if(m_bInitialized)
	{
		int i= m_InstrumentList.GetCurSel();
		XMInstrument& _inst = m_pMachine->rInstrument(i);
		TCHAR _buf[256];
		m_InstrumentName.GetWindowText(_buf,sizeof(_buf));
		_inst.Name(_buf);
	}
}


void XMSamplerUIInst::OnCbnSelendokInsNnacombo()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnCbnSelendokInsDctcombo()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnCbnSelendokInsDcacombo()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedLoadins()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedSaveins()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedDupeins()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedDeleteins()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
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
			float tenthsec = m_Zoom*0.04*Global::pPlayer->bpm;
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

			for(int i = 0;i < _points ;i++)
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
	ASSERT(pPopup != NULL);

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
//const unsigned int XMSamplerUIInst::CSampleAssignEditor::m_Sharpkey_Xpos[5] = {27,96,190,246,304};
const unsigned int XMSamplerUIInst::CSampleAssignEditor::m_Sharpkey_Xpos[5] = {16,55,110,148,185};
const XMSamplerUIInst::CSampleAssignEditor::TNoteKey XMSamplerUIInst::CSampleAssignEditor::m_NoteAssign[12]
= {XMSamplerUIInst::CSampleAssignEditor::TNoteKey::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey, 
XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey,XMSamplerUIInst::CSampleAssignEditor::SharpKey,XMSamplerUIInst::CSampleAssignEditor::NaturalKey};

const int XMSamplerUIInst::CSampleAssignEditor::m_NaturalKeysPerOctave = 7;
const int XMSamplerUIInst::CSampleAssignEditor::m_SharpKeysPerOctave = 5;

XMSamplerUIInst::CSampleAssignEditor::CSampleAssignEditor()
: m_bInitialized(false)
{
	m_NaturalKey.LoadBitmap(IDB_KEYS_NORMAL);
	m_SharpKey.LoadBitmap(IDB_KEYS_SHARP);

}
XMSamplerUIInst::CSampleAssignEditor::~CSampleAssignEditor()
{
	m_NaturalKey.DeleteObject();
	m_SharpKey.DeleteObject();

}
BEGIN_MESSAGE_MAP(XMSamplerUIInst::CSampleAssignEditor, CStatic)
END_MESSAGE_MAP()

void XMSamplerUIInst::CSampleAssignEditor::Initialize(XMSampler * const pSampler,XMInstrument * const pInstrument)
{
	m_pSampler = pSampler;
	m_pInst = pInstrument;
	m_bInitialized=true;
	Invalidate();
}
void XMSamplerUIInst::CSampleAssignEditor::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if(m_bInitialized){
		if (lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
		{
			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
//			CPen *oldpen= dc.SelectObject(&_gridpen);

			CRect _rect;
			GetClientRect(&_rect);

			const unsigned int _end = 2*m_NaturalKeysPerOctave;//show two octaves.
			const CString _NaturalKey_name[m_NaturalKeysPerOctave] = {"C","D","E","F","G","A","B"};
			const int _NaturalKey_index[m_NaturalKeysPerOctave] = {0,2,4,5,7,9,11};

			dc.FillSolidRect(&_rect,RGB(0,0,0));
			dc.SetBkMode(TRANSPARENT);

			BITMAP _bmp;
			m_NaturalKey.GetBitmap(&_bmp);

			CDC memDC;
			memDC.CreateCompatibleDC(&dc);
			CBitmap* oldbmp = memDC.SelectObject(&m_NaturalKey);

			CString _tmp_str;
			int _index = 0,_octave = 0;
			for(unsigned int i = 0;i < _end;i++)
			{
				dc.BitBlt(i*_bmp.bmWidth,
					0, 
					_bmp.bmWidth,
					_bmp.bmHeight, 
					&memDC, 
					0, 
					0,
					SRCCOPY);
				dc.BitBlt(i*_bmp.bmWidth,
					_bmp.bmHeight, 
					_bmp.bmWidth,
					_bmp.bmHeight, 
					&memDC, 
					0, 
					0,
					SRCCOPY);

/*				if(m_FocusKeyIndex && *m_FocusKeyIndex == _note)
				{

				}
*/
				_tmp_str.Format("%s%d",_NaturalKey_name[_index],_octave);
				dc.TextOut((i*_bmp.bmWidth+5),70,_tmp_str);

				_index++;
				if(_index == m_NaturalKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}

			oldbmp = memDC.SelectObject(&m_SharpKey);
			BITMAP _bmp2;
			m_SharpKey.GetBitmap(&_bmp2);

			const CString _sharpkey_name[m_SharpKeysPerOctave] = {"C#","D#","F#","G#","A#"};
			const int _sharpkey_index[m_SharpKeysPerOctave] = {1,3,6,8,10};
			const unsigned int _end2 = 2*m_SharpKeysPerOctave;//show two octaves.

			_index = 0;_octave = 0;
			for(UINT i = 0;i < _end2 ;i++)
			{
				dc.BitBlt(m_Sharpkey_Xpos[_index]+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,
					0, 
					_bmp2.bmWidth,
					_bmp2.bmHeight, 
					&memDC, 
					0, 
					0,
					SRCCOPY);
				dc.BitBlt(m_Sharpkey_Xpos[_index]+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,
					_bmp.bmHeight, 
					_bmp2.bmWidth,
					_bmp2.bmHeight, 
					&memDC, 
					0, 
					0,
					SRCCOPY);
/*				if(m_FocusKeyIndex && *m_FocusKeyIndex == _sharpkey)
				{

				}
*/
				_tmp_str.Format("%s%d",_sharpkey_name[_index],i/m_SharpKeysPerOctave);
				dc.TextOut(m_Sharpkey_Xpos[_index]+5+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,40,_tmp_str);

				_index++;
				if(_index == m_SharpKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}





			//
			memDC.SelectObject(oldbmp);
//			dc.SelectObject(oldpen);
			dc.Detach();
		}
	}
}

int XMSamplerUIInst::CSampleAssignEditor::GetKeyIndexAtPoint(const int x,const int y,CRect& keyRect)
{
	BITMAP _bmp, _bmp2;
	m_NaturalKey.GetBitmap(&_bmp);
	m_SharpKey.GetBitmap(&_bmp2);

	const int _octave_width = _bmp.bmWidth * m_NaturalKeysPerOctave;

	// *****  *****
	const int _sharpkey_num = 2 * m_SharpKeysPerOctave;
	const int _sharpkey_offset[m_SharpKeysPerOctave] = {1,3,6,8,10};
	const int _sharpkey_width = _bmp2.bmWidth;
	const int _sharpkey_height = _bmp2.bmHeight;

	for(int i = 0;i < _sharpkey_num;i++)
	{
		const int _xpos = m_Sharpkey_Xpos[i % m_SharpKeysPerOctave] + (i / m_SharpKeysPerOctave) * _octave_width;
		if(x >= _xpos && x <= (_xpos + _sharpkey_width) && y >= 0 && y <= _sharpkey_height)
		{
			keyRect.top = 0;
			keyRect.bottom = _sharpkey_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + _sharpkey_width;

			return _sharpkey_offset[i % m_SharpKeysPerOctave] + (i / m_SharpKeysPerOctave) * 12;
		}
	}

	// *****  *****
	const int _notekey_num = 2 * m_NaturalKeysPerOctave ; 
	const int _notekey_offset[m_NaturalKeysPerOctave] = {0,2,4,5,7,9,11};
	const int _note_width = _bmp.bmWidth;
	const int _note_height = _bmp.bmHeight;

	for(int i = 0;i < _notekey_num;i++)
	{
		const int _xpos = _note_width * (i % m_NaturalKeysPerOctave) + (i / m_NaturalKeysPerOctave) * _octave_width;
		if(x >= _xpos && x <= (_xpos + _note_width) && y >= 0 && y <= _note_height)
		{
			keyRect.top = 0;
			keyRect.bottom = _note_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + _note_width;

			return _notekey_offset[i % m_NaturalKeysPerOctave] + (i / m_NaturalKeysPerOctave) * 12;
		}
	}

	return 255;
}


NAMESPACE__END
NAMESPACE__END
