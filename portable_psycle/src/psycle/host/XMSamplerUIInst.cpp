#include <project.private.hpp>
#include "Psycle.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSampler.hpp"
#include "Player.hpp"
#include ".\xmsampleruiinst.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

// XMSamplerUIInst

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

	DDX_Control(pDX, IDC_NOTEMODNOTE, m_NoteModNote);
	DDX_Control(pDX, IDC_NOTEMODOCTAVE, m_NoteModOctave);
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

}

BEGIN_MESSAGE_MAP(XMSamplerUIInst, CPropertyPage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VOLCUTOFFPAN, OnNMCustomdrawVolCutoffPan)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SWING1, OnNMCustomdrawSwing1Glide)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_FADEOUTRES, OnNMCustomdrawFadeoutRes)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SWING2, OnNMCustomdrawSwing2)
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
	ON_CBN_SELENDOK(IDC_NOTEMODNOTE, OnCbnSelendokNotemodnote)
	ON_CBN_SELENDOK(IDC_NOTEMODOCTAVE, OnCbnSelendokNotemodoctave)
	ON_BN_CLICKED(IDC_ENVCHECK, OnBnClickedEnvcheck)
	ON_CBN_SELENDOK(IDC_INS_NNACOMBO, OnCbnSelendokInsNnacombo)
	ON_CBN_SELENDOK(IDC_INS_DCTCOMBO, OnCbnSelendokInsDctcombo)
	ON_CBN_SELENDOK(IDC_INS_DCACOMBO, OnCbnSelendokInsDcacombo)
	ON_BN_CLICKED(IDC_LOADINS, OnBnClickedLoadins)
	ON_BN_CLICKED(IDC_SAVEINS, OnBnClickedSaveins)
	ON_BN_CLICKED(IDC_DUPEINS, OnBnClickedDupeins)
	ON_BN_CLICKED(IDC_DELETEINS, OnBnClickedDeleteins)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_INS_ENVELOPE, OnNMCustomdrawInsEnvelope)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_INS_NOTEMAP, OnNMCustomdrawInsNotemap)
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
//	m_SampleAssignEditor.Initialize(m_pMachine,inst);

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
	//m_NoteModNote.SetCurSel(inst.NoteModVolCenter()/12);
	//m_NoteModOctave.SetCurSel(inst.NoteModVolCenter()%12);
	//m_SlNoteMod.SetPos(inst.NoteModVolSep());

	//m_EnvelopeEditor.Initialize(m_pMachine,inst.AmpEnvelope());
	((CScrollBar*)GetDlgItem(IDC_ENV_SCROLLBAR))->ShowWindow(SW_SHOW);
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
	m_NoteModOctave.SetCurSel(inst.NoteModPanCenter()/12);
	m_NoteModNote.SetCurSel(inst.NoteModPanCenter()%12);
	m_SlNoteMod.SetPos(inst.NoteModPanSep());

	//m_EnvelopeEditor.Initialize(m_pMachine,inst.PanningEnvelope());
	((CScrollBar*)GetDlgItem(IDC_ENV_SCROLLBAR))->ShowWindow(SW_SHOW);
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
	m_SlVolCutoffPan.SetPos(inst.FilterCutoff());
	m_SlFadeoutRes.SetPos(inst.FilterResonance());
	m_SlSwing1Glide.SetPos(inst.RandomCutoff()*100.0f);
	m_SlSwing2.SetPos(inst.RandomResonance()*100.0f);
	//m_NoteModNote.SetCurSel(inst.NoteModFilterCenter()/12);
	//m_NoteModOctave.SetCurSel(inst.NoteModFilterCenter()%12);
	//m_SlNoteMod.SetPos(inst.NoteModFilterSep());

	//m_EnvelopeEditor.Initialize(m_pMachine,inst.FilterEnvelope());
	((CScrollBar*)GetDlgItem(IDC_ENV_SCROLLBAR))->ShowWindow(SW_SHOW);
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

	//m_EnvelopeEditor.Initialize(m_pMachine,inst.PitchEnvelope());
	((CScrollBar*)GetDlgItem(IDC_ENV_SCROLLBAR))->ShowWindow(SW_SHOW);
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

		m_NNA.AddString(_T("Note Continue"));
		m_NNA.AddString(_T("Note Fadeout"));
		m_NNA.AddString(_T("Note Off"));
		m_NNA.AddString(_T("Note Cut"));

		m_DCT.AddString(_T("Instrument"));
		m_DCT.AddString(_T("Disabled"));
		m_DCT.AddString(_T("Note"));
		m_DCT.AddString(_T("Sample"));

		m_DCA.AddString(_T("Note Cut"));
		m_DCA.AddString(_T("Note Continue"));
		m_DCA.AddString(_T("Note Off"));
		m_DCA.AddString(_T("Note Fadeout"));

		m_SlSwing2.SetRangeMax(100);

		m_NoteModNote.AddString("C-");
		m_NoteModNote.AddString("C#");
		m_NoteModNote.AddString("D-");
		m_NoteModNote.AddString("D#");
		m_NoteModNote.AddString("E-");
		m_NoteModNote.AddString("F-");
		m_NoteModNote.AddString("F#");
		m_NoteModNote.AddString("G-");
		m_NoteModNote.AddString("G#");
		m_NoteModNote.AddString("A-");
		m_NoteModNote.AddString("A#");
		m_NoteModNote.AddString("B-");

		m_NoteModOctave.AddString("0");
		m_NoteModOctave.AddString("1");
		m_NoteModOctave.AddString("2");
		m_NoteModOctave.AddString("3");
		m_NoteModOctave.AddString("4");
		m_NoteModOctave.AddString("5");
		m_NoteModOctave.AddString("6");
		m_NoteModOctave.AddString("7");
		m_NoteModOctave.AddString("8");
		m_NoteModOctave.AddString("9");

		m_SlNoteMod.SetRangeMax(32);
		m_SlNoteMod.SetRangeMin(-32);

		m_SlADSRBase.SetRangeMax(256);
		m_SlADSRMod.SetRangeMax(256);
		m_SlADSRMod.SetRangeMin(-256);
		m_SlADSRAttack.SetRangeMax(512);
		m_SlADSRDecay.SetRangeMax(512);
		m_SlADSRSustain.SetRangeMax(256);
		m_SlADSRRelease.SetRangeMax(512);

//		m_SampleAssignEditor.SubclassWindow(GetDlgItem(IDC_INS_NOTEMAP));
//		m_SampleAssignEditor.Initialize(m_pMachine,0);

//		m_EnvelopeEditor.SubclassWindow(GetDlgItem(IDC_INS_ENVELOPE));
//		m_EnvelopeEditor.Initialize(m_pMachine,m_pMachine->rInstrument(0).AmpEnvelope());


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
	m_NoteModNote.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC4))->ShowWindow(SW_HIDE); //NOTEMOD
	m_NoteModOctave.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_HIDE); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_HIDE); //NOTEMOD

	m_EnvEnabled.ShowWindow(SW_HIDE);
	((CButton*)GetDlgItem(IDC_ENVADSR))->ShowWindow(SW_HIDE);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->ShowWindow(SW_HIDE);

	((CStatic*)GetDlgItem(IDC_INS_ENVELOPE))->ShowWindow(SW_HIDE);
	((CScrollBar*)GetDlgItem(IDC_ENV_SCROLLBAR))->ShowWindow(SW_HIDE);
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
	m_SlFadeoutRes.SetRangeMax(1024);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->ShowWindow(SW_SHOW); 
	((CStatic*)GetDlgItem(IDC_STATIC18))->ShowWindow(SW_HIDE); //Swing2
	m_SlSwing2.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LSWING2))->ShowWindow(SW_HIDE); 

	((CStatic*)GetDlgItem(IDC_STATIC3))->ShowWindow(SW_SHOW); //NOTEMOD
	m_NoteModNote.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC4))->ShowWindow(SW_SHOW); //NOTEMOD
	m_NoteModOctave.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_SHOW); //NOTEMOD

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
	m_NoteModNote.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC4))->ShowWindow(SW_SHOW); //NOTEMOD
	m_NoteModOctave.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_SHOW); //NOTEMOD

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
	m_NoteModNote.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC4))->ShowWindow(SW_SHOW); //NOTEMOD
	m_NoteModOctave.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_SHOW); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_SHOW);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_SHOW); //NOTEMOD

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
	m_NoteModNote.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC4))->ShowWindow(SW_HIDE); //NOTEMOD
	m_NoteModOctave.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_STATIC5))->ShowWindow(SW_HIDE); //NOTEMOD
	m_SlNoteMod.ShowWindow(SW_HIDE);
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->ShowWindow(SW_HIDE); //NOTEMOD

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
		sprintf(tmp,"%.0fms",(float)m_SlFadeoutRes.GetPos() / (24.0f * Global::pPlayer->bpm/60000.0f));
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

	((CStatic*)GetDlgItem(IDC_LSWING))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUIInst::OnCbnSelendokNotemodnote()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnCbnSelendokNotemodoctave()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}
void XMSamplerUIInst::OnNMCustomdrawNoteMod(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}
void XMSamplerUIInst::OnBnClickedEnvcheck()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnBnClickedEnvadsr()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnBnClickedEnvfreeform()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnNMCustomdrawADSRBase(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRMod(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRAttack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRDecay(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRSustain(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}
void XMSamplerUIInst::OnNMCustomdrawADSRRelease(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
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
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnCbnSelendokInsDctcombo()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnCbnSelendokInsDcacombo()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnBnClickedLoadins()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnBnClickedSaveins()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnBnClickedDupeins()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnBnClickedDeleteins()
{
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
}

void XMSamplerUIInst::OnNMCustomdrawInsEnvelope(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}

void XMSamplerUIInst::OnNMCustomdrawInsNotemap(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquÌ su cÛdigo de controlador de notificaciÛn de control
	*pResult = 0;
}


#if 0

//////////////////////////////////////////////////////////////////////////////
// EnvelopeEditor ------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
XMSamplerUIInst::EnvelopeEditor::EnvelopeEditor() : m_Color(255, 0, 0, 255)
{
	m_bInitialized = false;
	m_bPointEditing = false;
	
	m_pEnvelope = NULL;
	m_pXMSampler = NULL;

	// ***** [bohan] iso-(10)646 encoding only please! *****
	// SubclassWindowÇ***** [bohan] iso-(10)646 encoding only please! *****
	::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
	_wc.lpfnWndProc = &DummyWindowProc;
	::RegisterClassEx(&_wc);
}// XMSamplerUIInst::EnvelopeEditor::EnvelopeEditor

void XMSamplerUIInst::EnvelopeEditor::Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope)
{
	m_bInitialized = false;
	// 
	WTL::CRect _rect_client;

	// ***** [bohan] iso-(10)646 encoding only please! *****
	GetClientRect(&_rect_client);
	// H ScrollBar ***** [bohan] iso-(10)646 encoding only please! *****
	SetScrollSize(_rect_client.Width() * 2,_rect_client.Height(),false);
	// ***** [bohan] iso-(10)646 encoding only please! ***** client size ***** [bohan] iso-(10)646 encoding only please! *****
	GetClientRect(&_rect_client);
	// H ScrollBar ***** [bohan] iso-(10)646 encoding only please! *****
	SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	// ***** [bohan] iso-(10)646 encoding only please! ***** client size ***** [bohan] iso-(10)646 encoding only please! *****
	GetClientRect(&_rect_client);
	// ***** [bohan] iso-(10)646 encoding only please! *****
	SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	// ***** [bohan] iso-(10)646 encoding only please! *****

	//LPVOID lpMsgBuf;
	//if (_ret == 0 && FormatMessage( 
 //   FORMAT_MESSAGE_ALLOCATE_BUFFER | 
 //   FORMAT_MESSAGE_FROM_SYSTEM | 
 //   FORMAT_MESSAGE_IGNORE_INSERTS,
 //   NULL,
 //   GetLastError(),
 //   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
 //   (LPTSTR) &lpMsgBuf,
 //   0,
 //   NULL ))
	//{
	//	MessageBox( (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	//	::LocalFree(lpMsgBuf);
	//}

	m_pXMSampler = pSampler;
	m_CurrentScrollWidth = 0;
	m_CurrentScrollHeight = 0;
	EditEnvelope(pEnvelope);

	m_Zoom = 0.005f;

	//ShowScrollBar(SB_HORZ);
	//EnableScrollBar(SB_HORZ);

	//if(m_pEnvelope->IsEnabled()){
	
	int _num_pts = m_pEnvelope->NumOfPoints();
	int _max = 0;

	//if(_num_pts > 0){
	//	_scroll_size.cx = (int)(m_Zoom *  (float)(pEnvelope->Point(_num_pts - 1)));
	//	_scroll_size.cx += 100 /* pixel */; 
	//} else {

	//	_scroll_size.cx = _rect_client.right;
	//}

	//_scroll_size.cy = _rect_client.bottom; 
	//
	//m_CurrentScrollWidth = _scroll_size.cx;
	//m_CurrentScrollHeight = _scroll_size.cy;
	
	//SetScrollSize(_scroll_size);
///*
	
	if(_num_pts > 0){
		AdjustScrollRect((int)(m_Zoom *  (float)(m_pEnvelope->Point(_num_pts - 1))));
	} else {

		m_CurrentScrollWidth = _rect_client.Width();
		m_CurrentScrollHeight = _rect_client.Height();
		
		SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	}
  
		//SetScrollLine(1,1);
		//SetScrollPage(1,1);
		//ShowScrollBar(SB_HORZ,TRUE);


//*/
		
		//::SCROLLINFO _info;
	/*
		ZeroMemory(&_info,sizeof(::SCROLLINFO));
		_info.cbSize = sizeof(SCROLLINFO);
		_info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS ;
		_info.nMin = 0;
		_info.nMax = 2048;
		_info.nPage = 2;
		_info.nPos = 0;*/


		//this->SetScrollInfo(SB_HORZ,&_info);
		
		//m_pEnvelope = pEnvelope;

		//if(!m_bInitialized)
		//{
		//	 ÉeÉNÉXÉ`ÉÉÇì«Ç›çûÇﬁ
		//	LPDIRECT3DDEVICE9 const _pDevice = D3D::GetInstance().pD3DDevice();
		//	HRESULT hr = S_OK;

		//	hr = ::D3DXCreateTextureFromFile(_pDevice, "c:\\test.png", &m_pTexture);
		//	hr = ::D3DXCreateLine(_pDevice,&m_pLine);
		//	 lineÇÃstyleÇê›íËÇ∑ÇÈ
		//	m_pLine->SetAntialias(TRUE);
		//	m_pLine->SetGLLines(TRUE);
		//	m_pLine->SetPattern(1);
		//	m_pLine->SetWidth(2.0f);
		//	m_pLine->SetPatternScale(0.25f);
		//}

		//m_Vect[0].x = 0.0f;
		//m_Vect[0].y = 0.0f;
		//m_Vect[1].x = 1024.0f;
		//m_Vect[1].y = 768.0f;
		m_bInitialized = true;
	//}

	Invalidate();
	//OnPaint(NULL);
}// XMSamplerUIInst::EnvelopeEditor::Initialize

	
void XMSamplerUIInst::EnvelopeEditor::EditEnvelope(XMInstrument::Envelope * const pEnvelope)
{
	m_pEnvelope = pEnvelope;
}

/** ÉGÉìÉxÉçÅ[ÉvÉGÉfÉBÉ^Çï`âÊÇµÇ‹Ç∑ÅB 
	* @param CDCHandle Device Context */
void XMSamplerUIInst::EnvelopeEditor::DoPaint(CDCHandle dc)
{

	if(m_bInitialized){
		
		// ***** [bohan] iso-(10)646 encoding only please! *****
		HDC hdc = dc.m_hDC;
		CPoint _pt_viewport_org;
		dc.GetViewportOrg(&_pt_viewport_org);// ***** [bohan] iso-(10)646 encoding only please! *****
		WTL::CRect _rect;
		GetClientRect(&_rect);

		Graphics graphics(hdc);

		Pen      _line_pen(m_Color,1.0);
		Pen      _gridpen(Gdiplus::Color(32, 0, 0, 255),1.0);
		Pen      _gridpen1(Gdiplus::Color(32, 255, 0, 0),1.0);
		Pen      _gridpen2(Gdiplus::Color(64, 255, 0, 0),1.0);
		
		Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 255, 255, 255));

		//graphics.DrawLine(&_line_pen, 0, 0, 255, 255);
		const int _points =  m_pEnvelope->NumOfPoints();


		// ***** [bohan] iso-(10)646 encoding only please! *****
		
		graphics.Clear(Gdiplus::Color(255,255,255,255));
		graphics.SetRenderingOrigin(_pt_viewport_org.x,_pt_viewport_org.y);
		
		// Draw Grid
		
		Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
		Gdiplus::FontFamily  fontFamily(L"Times New Roman");
		Gdiplus::Font        font(&fontFamily, 8, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::PointF      pointF(10.0f, 20.0f);
		
		// ***** [bohan] iso-(10)646 encoding only please! ***** Alias
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
		
		const REAL _stepx = (REAL)(4410.0 * m_Zoom );
		const REAL _width = (m_CurrentScrollWidth > _rect.right)?m_CurrentScrollWidth:(REAL)_rect.right;
		int _mod = 0;
		float _sec = 0.0f;
		
		std::wstring _str;

		// ***** [bohan] iso-(10)646 encoding only please! *****
		if(_stepx >= 4.0){
			for(REAL i = 0.0; i < _width; i += _stepx)
			{
				if(_mod == 5)
				{
					graphics.DrawLine(&_gridpen1,i,0.0,i,(REAL)m_CurrentScrollHeight);
					_mod++;

					// ***** [bohan] iso-(10)646 encoding only please! *****
					_sec += 0.5f;
					_str = boost::lexical_cast<std::wstring,float>(_sec);		
					graphics.DrawString(_str.c_str(),_str.length(),&font,PointF(i,0),&_fontbrush);

				} else if(_mod == 10) {
					_mod = 1;
					graphics.DrawLine(&_gridpen2,i,0.0,i,(REAL)m_CurrentScrollHeight);
					// ***** [bohan] iso-(10)646 encoding only please! *****
					_sec += 0.5f;
					_str = boost::lexical_cast<std::wstring,float>(_sec);		
					graphics.DrawString(_str.c_str(),_str.length(),&font,PointF(i,0),&_fontbrush);
				} else {
					graphics.DrawLine(&_gridpen,i,0.0,i,(REAL)m_CurrentScrollHeight);
					_mod++;
				}
			}
		}

		// ***** [bohan] iso-(10)646 encoding only please! *****
		REAL _stepy = ((REAL)(m_CurrentScrollHeight)) / 100.0f * 10.0f;
		
		for(REAL i = (REAL)_rect.top; i <= (REAL)m_CurrentScrollHeight; i += _stepy)
		{
			graphics.DrawLine(&_gridpen,0.0,i,_width,i);
		}

		
		Gdiplus::Point _pt_start(0,m_CurrentScrollHeight);
		Gdiplus::SolidBrush  _point_brush(Gdiplus::Color(255, 255, 128, 128));
		Gdiplus::SolidBrush  _point_edit_brush(Gdiplus::Color(255, 128, 64, 64));
		Pen _point_pen(Gdiplus::Color(128, 255, 128, 255),1.0);
		Pen _point_edit_pen(Gdiplus::Color(128, 255, 128, 255),1.0);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		
		// ***** [bohan] iso-(10)646 encoding only please! *****
		
		for(int i = 0;i < _points ;i++)
		{
			Point _pt_end;
			_pt_end.Y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)));
			_pt_end.X = (int)(m_Zoom * (float)m_pEnvelope->Point(i)); 
			graphics.DrawLine(&_line_pen,_pt_start,_pt_end);
			_pt_start = _pt_end;
	
		}
		

		// ***** [bohan] iso-(10)646 encoding only please! *****
		if(m_bPointEditing){// ***** [bohan] iso-(10)646 encoding only please! *****
			_pt_start.X = 0;
			_pt_start.Y = m_CurrentScrollHeight;
			// ***** [bohan] iso-(10)646 encoding only please! *****
			for(int i = 0;i < _points ;i++)
			{
				Point _pt_end;
				_pt_end.Y = (int)((float)m_CurrentScrollHeight * (1.0f - m_EnvelopeEditing.Value(i)));
				_pt_end.X = (int)(m_Zoom * (float)m_EnvelopeEditing.Point(i)); 
				graphics.DrawLine(&_point_edit_pen,_pt_start,_pt_end);
				_pt_start = _pt_end;
			}
		}
		
		// ***** [bohan] iso-(10)646 encoding only please! *****
		for(int i = 0;i < _points ;i++)
		{
			Point _pt(
				/* X */ (int)(m_Zoom * (float)m_pEnvelope->Point(i)), 
				/* Y */ (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)))
			);
			graphics.FillRectangle(&_point_brush,_pt.X - POINT_SIZE / 2,_pt.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);
		}
		
		// ***** [bohan] iso-(10)646 encoding only please! *****
		
		if(m_bPointEditing)
		{
			Point _pt_org((int)(m_Zoom * (float)m_pEnvelope->Point(m_EditPointOrig)),(int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_EditPointOrig))));
			Point _pt_edit(m_EditPointX,m_EditPointY);
			Pen _edit_line_pen(Gdiplus::Color(64, 64, 64, 64),1.0);
			REAL dashValues[2] = {2,2};
			_edit_line_pen.SetDashPattern(dashValues, 2);
			graphics.DrawLine(&_edit_line_pen,_pt_org,_pt_edit);
			
			graphics.FillRectangle(&_point_brush,_pt_edit.X - POINT_SIZE / 2,_pt_edit.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);
			graphics.FillRectangle(&_point_edit_brush,_pt_org.X - POINT_SIZE / 2,_pt_org.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);

		}
		
		// Sustain Point ***** [bohan] iso-(10)646 encoding only please! *****

		if(m_pEnvelope->SustainBegin() != XMInstrument::Envelope::INVALID)
		{
			Point _pt_st(
				/* X */(int)(m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->SustainBegin())), 
				/* Y */(int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->SustainBegin())))
			);

			Point _pt_st_lbl;

			if(_pt_st.X > m_CurrentScrollWidth / 2)
			{
				_pt_st_lbl.X = _pt_st.X  - 5;
			} else {
				_pt_st_lbl.X = _pt_st.X  + 5;
			}
			
			if(_pt_st.Y < m_CurrentScrollHeight / 8)
			{
				_pt_st_lbl.Y = _pt_st.Y + m_CurrentScrollHeight / 8;
			} else {
				/* 3 / 4*/
				_pt_st_lbl.Y = _pt_st.Y - m_CurrentScrollHeight / 8 ;
			}


			// Envelope Point ***** [bohan] iso-(10)646 encoding only please! ***** Sustain Label ***** [bohan] iso-(10)646 encoding only please! *****
			Pen _edit_line_pen(Gdiplus::Color(96, 128, 128, 64),1.0);
			REAL dashValues[2] = {2,2};
			_edit_line_pen.SetDashPattern(dashValues, 2);

			graphics.DrawLine(&_edit_line_pen,_pt_st,_pt_st_lbl);
			
			// ***** [bohan] iso-(10)646 encoding only please! *****
			Gdiplus::FontFamily  _font_family_sustain(L"Times New Roman");
			Gdiplus::Font        _font_sustain(&_font_family_sustain, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			Gdiplus::SolidBrush  _font_sustain_brush(Gdiplus::Color(255, 255, 128, 64));
		
			graphics.DrawString(L"Sustain",7,&_font_sustain,PointF((float)_pt_st_lbl.X,(float)_pt_st_lbl.Y - 3),&_font_sustain_brush);

		}
		
		// Loop Start ***** [bohan] iso-(10)646 encoding only please! ***** Loop End ***** [bohan] iso-(10)646 encoding only please! *****

		if(m_pEnvelope->LoopStart() != XMInstrument::Envelope::INVALID && 
			m_pEnvelope->LoopEnd() != XMInstrument::Envelope::INVALID)
		{
			const int _pt_loop_start_x = m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->LoopStart());
			const int _pt_loop_start_y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->LoopStart())));

			const int _pt_loop_end_x = m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->LoopEnd());
			const int _pt_loop_end_y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->LoopEnd())));

			// Envelope Point ***** [bohan] iso-(10)646 encoding only please! ***** Sustain Label ***** [bohan] iso-(10)646 encoding only please! *****
			Pen _loop_pen(Gdiplus::Color(255, 0, 192, 0),1.0);
			REAL dashValues[2] = {2,2};
			_loop_pen.SetDashPattern(dashValues, 2);

			Gdiplus::SolidBrush  _loop_brush(Gdiplus::Color(64, 0, 128, 0));
			graphics.FillRectangle(&_loop_brush,_pt_loop_start_x,0,_pt_loop_end_x - _pt_loop_start_x,m_CurrentScrollHeight);

			graphics.DrawLine(&_loop_pen,_pt_loop_start_x,0,_pt_loop_start_x,m_CurrentScrollHeight);
			graphics.DrawLine(&_loop_pen,_pt_loop_end_x,0,_pt_loop_end_x,m_CurrentScrollHeight);
		
			Gdiplus::FontFamily  _font_family_loop(L"Times New Roman");
			Gdiplus::Font        _font_loop(&_font_family_loop, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			Gdiplus::SolidBrush  _font_loop_brush(Gdiplus::Color(255, 0, 64, 0));

			graphics.DrawString(L"Loop",4,&_font_loop,
				PointF(	(float)((_pt_loop_end_x - _pt_loop_start_x) / 2 + _pt_loop_start_x - 20),
						(float)(m_CurrentScrollHeight / 2)),&_font_loop_brush);

		}

//			Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
		
		//Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 0, 0, 255));
		//Gdiplus::FontFamily  fontFamily(L"Times New Roman");
		//Gdiplus::Font        font(&fontFamily, 24, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		//Gdiplus::PointF      pointF(10.0f, 20.0f);
		//graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
// 		graphics.DrawString(L"Hello World!", -1, &font, pointF, &brush);

//		XMInstrument& _inst = m_pMachine->Instrument(instno);

		//LPDIRECT3DDEVICE9 const _pDevice = D3D::GetInstance().pD3DDevice();

//		_pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
//								0x00000000, 1.0f, 0L );
//		_pDevice->BeginScene();
//		D3D::GetInstance().Render(*this);
//		m_pLine->Begin();
//
//		HRESULT hr = _pDevice->SetTexture(0, m_pTexture);	// ***** [bohan] iso-(10)646 encoding only please! *****
//
//		// ***** [bohan] iso-(10)646 encoding only please! *****
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
////		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTSS_COLOROP);
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
//		hr = _pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
//		hr = _pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
//		RECT _rect,_rect1;
//		GetClientRect(&_rect);
//
//		m_Vect[1].x = _rect.right;
//		m_Vect[1].y = _rect.bottom;
//		
//		m_pLine->Draw(m_Vect,2,D3DCOLOR(0xffffffff));
//		
//		m_pLine->End();
//		_pDevice->EndScene();
//
//		_rect1 = _rect;

//		_rect1.top += 1.0f;
//		_rect1.bottom += 1.0f;
		
//		_rect1.left += 2.0f;
//		_rect1.right -= 6.0f;

		//D3D::GetInstance().Window().GetClientRect(&_rect1);
//		_pDevice->Present(&_rect1,NULL,m_hWnd,NULL);
//		_pDevice->Present(NULL,NULL,m_hWnd,NULL);
		// EndPaint(&ps);
	}
	SetMsgHandled(false);
}// XMSamplerUIInst::EnvelopeEditor::OnPaint

/** ***** [bohan] iso-(10)646 encoding only please! *****
	* @param vKey ***** [bohan] iso-(10)646 encoding only please! *****
	* @param position ***** [bohan] iso-(10)646 encoding only please! ***** */
void XMSamplerUIInst::EnvelopeEditor::OnLButtonDown(const UINT vKey, WTL::CPoint position)
{
	
	SetFocus();

	if(!m_bPointEditing){
		const int _points =  m_pEnvelope->NumOfPoints();
		WTL::CRect _rect;
		GetClientRect(&_rect);
		POINT _offset;
		GetScrollOffset(_offset);
		position.x += _offset.x;
		position.y += _offset.y;

		// ***** [bohan] iso-(10)646 encoding only please! *****
		EnvelopePointIndex _edit_point = GetEnvelopePointIndexAtPoint(position.x,position.y);
		if(_edit_point)
		{
			m_bPointEditing = true;
			SetCapture();
			m_EditPointOrig = m_EditPoint = *_edit_point;
			m_EnvelopeEditing = *m_pEnvelope;

		}
	}

}

/** ***** [bohan] iso-(10)646 encoding only please! *****
	* @param vKey ***** [bohan] iso-(10)646 encoding only please! *****
	* @param position ***** [bohan] iso-(10)646 encoding only please! ***** */
void XMSamplerUIInst::EnvelopeEditor::OnLButtonUp(const UINT vKey, WTL::CPoint position)
{
	if(m_bPointEditing){
		ReleaseCapture();
		m_bPointEditing =  false;
		CPoint _offset;
		GetScrollOffset(_offset);
			
		position.x += _offset.x;
		position.y += _offset.y;
		
		int _new_point = (int)((float)position.x / m_Zoom);
		float _new_value = (1.0f - (float)position.y / (float)m_CurrentScrollHeight);
		
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

		m_EnvelopeEditing.PointAndValue(m_EditPoint,_new_point,_new_value);

		*m_pEnvelope = m_EnvelopeEditing;
		
		if(m_pEnvelope->NumOfPoints() > 0){
			AdjustScrollRect((int)(m_Zoom *  (float)(m_pEnvelope->Point(m_pEnvelope->NumOfPoints() - 1))));
		}

		Invalidate();
	}
}

void XMSamplerUIInst::EnvelopeEditor::OnMouseMove(const UINT vKey, WTL::CPoint position)
{
	if(m_bPointEditing)
	{
		CPoint _offset;
		GetScrollOffset(_offset);
		
		m_EditPointX = position.x + _offset.x;
		m_EditPointY = position.y + _offset.y;
		m_EditPoint = m_EnvelopeEditing.PointAndValue(m_EditPoint,(int)((float)m_EditPointX / m_Zoom),
			(1.0f - (float)m_EditPointY / (float)m_CurrentScrollHeight));

		//m_pEnvelope->Value(m_EditPoint,	(1.0f - (float)position.y / (float)m_CurrentScrollHeight));
		//_pt.x = (int)(m_Zoom * (float)m_pEnvelope->Point(i));
		//m_pEnvelope->Point(m_EditPoint, (int)((float)position.x / m_Zoom));

		Invalidate();

	}
}

/** Scroll Size ***** [bohan] iso-(10)646 encoding only please! *****
* ***** [bohan] iso-(10)646 encoding only please! *****W(maxPoint)***** [bohan] iso-(10)646 encoding only please! *****Scroll***** [bohan] iso-(10)646 encoding only please! *****Scroll***** [bohan] iso-(10)646 encoding only please! *****maxPoint + 100***** [bohan] iso-(10)646 encoding only please! *****
* ***** [bohan] iso-(10)646 encoding only please! *****W(maxPoint)***** [bohan] iso-(10)646 encoding only please! *****Scroll***** [bohan] iso-(10)646 encoding only please! *****Scroll***** [bohan] iso-(10)646 encoding only please! *****maxPoint + 100***** [bohan] iso-(10)646 encoding only please! *****Adjust
*/
void XMSamplerUIInst::EnvelopeEditor::AdjustScrollRect(const int maxPoint)
{
	
	bool _bchanged = false;
	SIZE _scroll_size;
	CPoint _pt_scroll_offset;
	GetScrollSize(_scroll_size);
	GetScrollOffset(_pt_scroll_offset);

	WTL::CRect _rect;
	GetClientRect(&_rect);

	if((maxPoint + MARGIN_RIGHT) > _scroll_size.cx){
		_scroll_size.cx = maxPoint;
		_scroll_size.cx += MARGIN_RIGHT /* pixel */;
		if(_scroll_size.cx < _rect.Width()){
			_scroll_size.cx = _rect.Width();
		}
		_bchanged = true;
	} else if(maxPoint < (_scroll_size.cx - MARGIN_RIGHT))
	{
		_scroll_size.cx = maxPoint + MARGIN_RIGHT;
		if(_scroll_size.cx < _rect.Width())
		{
			_scroll_size.cx = _rect.Width();
		}
		_bchanged = true;
	}

	if(_bchanged){

		GetClientRect(_rect);

		if(_scroll_size.cy < _rect.Height()){
			_scroll_size.cy = _rect.Height();
		}
		
		m_CurrentScrollWidth = _scroll_size.cx;
		m_CurrentScrollHeight = _scroll_size.cy;

		SetScrollSize(_scroll_size.cx,_scroll_size.cy,false);
		// ***** [bohan] iso-(10)646 encoding only please! *****
		if(_scroll_size.cx > _rect.Width())
		{
			SetScrollOffset(_pt_scroll_offset,false);
		}
		Invalidate();

	}

}

/** ***** [bohan] iso-(10)646 encoding only please! ***** */
void XMSamplerUIInst::EnvelopeEditor::OnRButtonDown(const UINT vKey, WTL::CPoint position)
{
	if(!m_bPointEditing){
		// ***** [bohan] iso-(10)646 encoding only please! *****
		m_EditPointX = position.x;
		m_EditPointY = position.y;
		
		CPoint _offset;
		GetScrollOffset(_offset);

		m_EditPointX += _offset.x;
		m_EditPointY += _offset.y;
	    
		// ***** [bohan] iso-(10)646 encoding only please! *****
		ClientToScreen(&position);
	    
		// ***** [bohan] iso-(10)646 encoding only please! *****
		CMenu _menuPopup;
		_menuPopup.LoadMenu(IDR_MENU_ENV_EDIT);
		// ***** [bohan] iso-(10)646 encoding only please! *****
		EnvelopePointIndex _edit_point = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
		if(!_edit_point)
		{
			_menuPopup.EnableMenuItem(ID_SET_SUSTAIN,MF_GRAYED);
			_menuPopup.EnableMenuItem(ID_SET_LOOPSTART,MF_GRAYED);
			_menuPopup.EnableMenuItem(ID_SET_LOOPEND,MF_GRAYED);
			_menuPopup.EnableMenuItem(ID_DEL_POINT,MF_GRAYED);
		}

		_menuPopup.GetSubMenu(0).TrackPopupMenu(
			TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, position.x, position.y, m_hWnd);
	}

}

void XMSamplerUIInst::EnvelopeEditor::OnAddPoint(const UINT uNotifyCode,const int nID,const  HWND hWndCtl)
{
	if(m_pEnvelope->NumOfPoints() > XMInstrument::Envelope::MAX_POINT)
	{
		// ***** [bohan] iso-(10)646 encoding only please! *****
		return;
	}

	CPoint _offset;
	GetScrollOffset(_offset);
		
	m_EditPointX += _offset.x;
	m_EditPointY += _offset.y;
	
	int _new_point = (int)((float)m_EditPointX / m_Zoom);
	float _new_value = (1.0f - (float)m_EditPointY / (float)m_CurrentScrollHeight);
	
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
	m_pEnvelope->Insert(_new_point,_new_value);
	Invalidate();

}

/** Envelope Point ÇçÌèúÇ∑ÇÈ 
* @param uNotifyCode í ímÉRÅ[Éh
* ÉÅÉbÉZÅ[ÉWÇ™ÉRÉìÉgÉçÅ[ÉãÇ©ÇÁëóÇÁÇÍÇƒÇ¢ÇÈèÍçáÇ…ÇÕÅAÉRÉìÉgÉçÅ[ÉãÇ©ÇÁÇÃí ímÉRÅ[ÉhÇ™éwíËÇ≥ÇÍÇ‹Ç∑ÅB
* ÉAÉNÉZÉâÉåÅ[É^Ç©ÇÁëóÇÁÇÍÇΩèÍçáÇ…ÇÕ 1 Ç™éwíËÇ≥ÇÍÇ‹Ç∑ÅBÉÅÉjÉÖÅ[Ç©ÇÁëóÇÁÇÍÇΩèÍçáÇÕ 0 Ç™éwíËÇ≥ÇÍÇ‹Ç∑ÅB
* @param nID ÉRÉìÉgÉçÅ[ÉãID
* ÉÅÉjÉÖÅ[ÉAÉCÉeÉÄÅAÉRÉìÉgÉçÅ[ÉãÅAÉAÉNÉZÉâÉåÅ[É^Å[ÇÃ ID Ç™éwíËÇ≥ÇÍÇ‹Ç∑ÅB
* @param hWndCtl ÉRÉìÉgÉçÅ[ÉãÇÃÉEÉBÉìÉhÉEÉnÉìÉhÉã
* 
*/
void XMSamplerUIInst::EnvelopeEditor::OnDelPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
{
	if(m_pEnvelope->NumOfPoints() == 0)
	{
		// ***** [bohan] iso-(10)646 encoding only please! *****
		return;
	}

	const int _points =  m_pEnvelope->NumOfPoints();
	WTL::CRect _rect;
	GetClientRect(&_rect);

	EnvelopePointIndex _delete_point = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
	if(_delete_point)
	{
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->Delete(*_delete_point);
		Invalidate();
	}

}


/** ***** [bohan] iso-(10)646 encoding only please! ***** */
void XMSamplerUIInst::EnvelopeEditor::OnSetSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
{
	ATLTRACE2("OnSetSustain\n");

	EnvelopePointIndex _sustain_index = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
	if(_sustain_index)
	{
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->SustainBegin(*_sustain_index);
		m_pEnvelope->SustainEnd(*_sustain_index);
		Invalidate();
	}

}

/** ***** [bohan] iso-(10)646 encoding only please! ***** */ 
void XMSamplerUIInst::EnvelopeEditor::OnRemoveSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
{
	ATLTRACE2("OnRemoveSustain\n");
	boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
	m_pEnvelope->SustainBegin(XMInstrument::Envelope::INVALID);
	m_pEnvelope->SustainEnd(XMInstrument::Envelope::INVALID);
	Invalidate();

}
/** ***** [bohan] iso-(10)646 encoding only please! ***** */   
void XMSamplerUIInst::EnvelopeEditor::OnSetLoopStart(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
{
	ATLTRACE2("OnSetLoopStart\n");

	EnvelopePointIndex _loop_index = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
	if(_loop_index)
	{
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->LoopStart(*_loop_index);
		Invalidate();
	}

}

/** ***** [bohan] iso-(10)646 encoding only please! ***** */
void XMSamplerUIInst::EnvelopeEditor::OnSetLoopEnd(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
{
	ATLTRACE2("OnSetLoopEnd\n");

	EnvelopePointIndex _loop_index = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
	if(_loop_index)
	{
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->LoopEnd(*_loop_index);
		Invalidate();
	}

}

/** Loop***** [bohan] iso-(10)646 encoding only please! *****  */
void XMSamplerUIInst::EnvelopeEditor::OnRemoveLoop(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
{
	ATLTRACE2("OnRemoveLoop\n");
	boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
	m_pEnvelope->LoopStart(XMInstrument::Envelope::INVALID);
	m_pEnvelope->LoopEnd(XMInstrument::Envelope::INVALID);
	Invalidate();

}

/** ***** [bohan] iso-(10)646 encoding only please! *****
	* @brief Envelope Editor Ç™Dialog API Ç***** [bohan] iso-(10)646 encoding only please! *****Application ***** [bohan] iso-(10)646 encoding only please! *****
	*/
LRESULT CALLBACK XMSamplerUIInst::EnvelopeEditor::DummyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	if(msg == WM_CREATE){
//		::ShowScrollBar(hWnd,SB_HORZ,TRUE);
//		RECT _rect;
//		::GetClientRect(hWnd,&_rect);
//		SCROLLINFO si = { sizeof(si), SIF_ALL | SIF_DISABLENOSCROLL, 0,_rect.right - _rect.left ,0,0,0};
//	

//		int _ret = ::SetScrollInfo(hWnd,SB_HORZ,&si,TRUE);
//		int _ret1 = ::ShowScrollBar(hWnd,SB_HORZ,TRUE);
////			::SetScrollRange(hWnd,SB_HORZ,0,_rect.right,TRUE);
//		return 0;
//	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}

//////////////////////////////////////////////////////////////////////////////
// SampleAssignEditor ------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

//boost::shared_ptr<Gdiplus::Image> XMSamplerUIInst::SampleAssignEditor::m_pNaturalKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
//boost::shared_ptr<Gdiplus::Image> XMSamplerUIInst::SampleAssignEditor::m_pSharpKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
Gdiplus::Bitmap* XMSamplerUIInst::SampleAssignEditor::m_pNaturalKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
Gdiplus::Bitmap* XMSamplerUIInst::SampleAssignEditor::m_pSharpKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
const UINT XMSamplerUIInst::SampleAssignEditor::SHARPKEY_XPOS[SHARP_KEY_PER_OCTAVE] = {27,96,190,246,304};
const int XMSamplerUIInst::SampleAssignEditor::m_NoteAssign[KEY_PER_OCTAVE] 
= {NATURAL_KEY /*îí*/,SHARP_KEY /*çï*/,NATURAL_KEY,SHARP_KEY,NATURAL_KEY,NATURAL_KEY,SHARP_KEY,NATURAL_KEY,SHARP_KEY,NATURAL_KEY,SHARP_KEY,NATURAL_KEY};
int XMSamplerUIInst::SampleAssignEditor::m_CreateCount = 0;///< ***** [bohan] iso-(10)646 encoding only please! *****

/** ***** [bohan] iso-(10)646 encoding only please! ***** */
XMSamplerUIInst::SampleAssignEditor::SampleAssignEditor() : m_FocusKeyIndex()
{
	// ***** [bohan] iso-(10)646 encoding only please! *****
	//SubclassWindow***** [bohan] iso-(10)646 encoding only please! *****
	::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
	m_pWndProcBackup = _wc.lpfnWndProc;
	_wc.lpfnWndProc = &::DefWindowProc;
	::RegisterClassEx(&_wc);
	m_pXMSampler = NULL;
	

	m_bInitialized = false;
	if(m_CreateCount == 0)
	{
		std::wstring _note_fname = CT2CW(::Global::CurrentDirectory().c_str());
		_note_fname += L"\\Plugins\\SF\\XMSampler\\Resources\\notekey.png";

		std::wstring _sharpkey_fname = CT2CW(::Global::CurrentDirectory().c_str());
		_sharpkey_fname += L"\\Plugins\\SF\\XMSampler\\Resources\\sharpkey.png";
		//m_pNaturalKey.reset(Image::FromFile(_note_fname.c_str()));
		m_pNaturalKey = Bitmap::FromFile(_note_fname.c_str());
		//m_pSharpKey.reset(Image::FromFile(_sharpkey_fname.c_str()));
		m_pSharpKey = Bitmap::FromFile(_sharpkey_fname.c_str());
	}
	m_CreateCount++;
}

/** ~SampleAssignEditor() */
XMSamplerUIInst::SampleAssignEditor::~SampleAssignEditor()
{
	m_CreateCount--;
	if(m_CreateCount == 0)
	{
		delete m_pNaturalKey;
		delete m_pSharpKey;
	}
}

/** WM_PAINT ***** [bohan] iso-(10)646 encoding only please! ***** */
void XMSamplerUIInst::SampleAssignEditor::DoPaint(CDCHandle dc)
{

	if(m_bInitialized){
		ATLASSERT(m_pXMSampler != NULL);

		// ***** [bohan] iso-(10)646 encoding only please! *****
		HDC hdc = dc.m_hDC;
		
		CPoint _pt_viewport_org;
		dc.GetViewportOrg(&_pt_viewport_org);// åªç›ÇÃ
		WTL::CRect _rect;
		GetClientRect(&_rect);

		Graphics graphics(hdc);

		//Pen      _line_pen(m_Color,1.0);
		//Pen      _gridpen(Gdiplus::Color(32, 0, 0, 255),1.0);
		//Pen      _gridpen1(Gdiplus::Color(32, 255, 0, 0),1.0);
		//Pen      _gridpen2(Gdiplus::Color(64, 255, 0, 0),1.0);
		//
		//Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 255, 255, 255));

		// ***** [bohan] iso-(10)646 encoding only please! *****
		//graphics.Clear(Gdiplus::Color(255,255,255,255));
		graphics.SetRenderingOrigin(_pt_viewport_org.x,_pt_viewport_org.y);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		// ***** [bohan] iso-(10)646 encoding only please! ***** Anti Alias ***** [bohan] iso-(10)646 encoding only please! *****
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
		
		// Draw Grid
		
		//Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
		//Gdiplus::FontFamily  fontFamily(L"Times New Roman");
		//Gdiplus::Font        font(&fontFamily, 8, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		//Gdiplus::PointF      pointF(10.0f, 20.0f);
		

		const UINT _width = m_pNaturalKey->GetWidth();
//			const UINT _end = XMInstrument::MAX_NOTES / 12 * NATURAL_KEY_PER_OCTAVE * _width;
		const UINT _end = XMInstrument::MAX_NOTES / 12 * NATURAL_KEY_PER_OCTAVE;

		const std::wstring _natural_key_name[NATURAL_KEY_PER_OCTAVE] = {L"C",L"D",L"E",L"F",L"G",L"A",L"B"};
		const int _natural_key_index[NATURAL_KEY_PER_OCTAVE] = {0,2,4,5,7,9,11};
		Gdiplus::FontFamily  _font_family_keyname(L"Times New Roman");
		Gdiplus::Font        _font_keyname(&_font_family_keyname, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush  _font_keyname_brush(Gdiplus::Color(128, 0, 0, 0));

		CachedBitmap _cache_natural_key(m_pNaturalKey,&graphics);
		std::wstring _tmp_str;
		int _index = 0,_octave = 0;
		for(UINT i = 0;i < _end;i++)
		{
//				graphics.DrawString(_natural_key_name[(i / _width) % 7].c_str()) 
			_tmp_str = _natural_key_name[_index];
			int _note = _natural_key_index[_index] + _octave * KEY_PER_OCTAVE;
			_tmp_str += boost::lexical_cast<std::wstring>(i / NATURAL_KEY_PER_OCTAVE);
			//graphics.DrawCachedBitmap(&_cache_natural_key,i,0);

			if(m_FocusKeyIndex && *m_FocusKeyIndex == _note){
				SolidBrush _brush(Color(128,255,255,255));
				
				// GDI+ 1.1 ***** [bohan] iso-(10)646 encoding only please! *****
				//REAL srcWidth = (REAL)m_pNaturalKey->GetWidth();
				//REAL srcHeight = (REAL)m_pNaturalKey->GetHeight();
				//RectF srcRect(0.0f, 0.0f, srcWidth, srcHeight);
				//Matrix myMatrix(1.0f, 0.0f, 0.0f, 1.0f, i, 0);

				//BrightnessContrastParams briConParams;
				//briConParams.brightnessLevel = 50;
				//briConParams.contrastLevel = 25;
				//BrightnessContrast briCon;
				//briCon.SetParameters(&briConParams);

				//graphics.DrawImage(m_pNaturalKey, &srcRect, &myMatrix, &briCon, NULL, UnitPixel);
				graphics.DrawCachedBitmap(&_cache_natural_key,i * _width,0);
				graphics.FillRectangle(&_brush,i * _width,0,m_pNaturalKey->GetWidth(),m_pNaturalKey->GetHeight());

			} else {
				graphics.DrawCachedBitmap(&_cache_natural_key,i * _width,0);
			}
			graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
				PointF(	(float)(i * _width + 5),98.0f),&_font_keyname_brush);
			_index++;
			if(_index == NATURAL_KEY_PER_OCTAVE){
				_index = 0;
				_octave++;
			}
		}

		CachedBitmap _cache_sharpkey(m_pSharpKey,&graphics);
		
		//const UINT _blkey_xpos[5] = {27,96,190,246,304};
		const std::wstring _sharpkey_name[5] = {L"C#",L"D#",L"F#",L"G#",L"A#"};
		const int _sharpkey_index[SHARP_KEY_PER_OCTAVE] = {1,3,6,8,10};

		Gdiplus::SolidBrush  _font_sharpkeyname_brush(Gdiplus::Color(128, 255, 255, 255));

		_index = 0;_octave = 0;
		for(UINT i = 0;i < 10/*octave*/ * 5 /*key*/;i++)
		{
			_tmp_str = _sharpkey_name[_index];
			_tmp_str += boost::lexical_cast<std::wstring>(_octave);
			
			int _sharpkey = _sharpkey_index[_index] + _octave * KEY_PER_OCTAVE;

			graphics.DrawCachedBitmap(&_cache_sharpkey,
				SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave ,0);
			if(m_FocusKeyIndex && *m_FocusKeyIndex == _sharpkey)
			{
				SolidBrush _brush(Color(128,255,255,255));
				graphics.FillRectangle(&_brush,SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave,0,m_pSharpKey->GetWidth(),m_pSharpKey->GetHeight());
			}
			graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
				PointF(	(float)(SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave),55.0f),
				&_font_sharpkeyname_brush);

			_index++;
			if(_index == SHARP_KEY_PER_OCTAVE){
				_index = 0;
				_octave++;
			}
		}

	} else {
		InitializeScrollWindow();
	}
	SetMsgHandled(FALSE);
}

void XMSamplerUIInst::SampleAssignEditor::Initialize(XMSampler* const pXMSampler)
{
	
	m_bInitialized = false;
	GetSystemSettings();
	m_pXMSampler = pXMSampler;
};

/** ***** [bohan] iso-(10)646 encoding only please! *****
	* ***** [bohan] iso-(10)646 encoding only please! ***** ScrollBar ***** [bohan] iso-(10)646 encoding only please! *****
	* ***** [bohan] iso-(10)646 encoding only please! *****
	*/
void XMSamplerUIInst::SampleAssignEditor::InitializeScrollWindow()
{
	WTL::CRect _rect_client;

	const UINT _width = m_pNaturalKey->GetWidth();
	const UINT _width_total = XMInstrument::MAX_NOTES / KEY_PER_OCTAVE * NATURAL_KEY_PER_OCTAVE * _width;

	GetClientRect(&_rect_client);
	// H ScrollBar ***** [bohan] iso-(10)646 encoding only please! *****
	SetScrollSize(_width_total,_rect_client.Height());
	// ***** [bohan] iso-(10)646 encoding only please! *****
	SetScrollSize(_width_total,_rect_client.Height());
	// ***** [bohan] iso-(10)646 encoding only please! ***** Client Height ***** [bohan] iso-(10)646 encoding only please! *****
	GetClientRect(&_rect_client);
	// ***** [bohan] iso-(10)646 encoding only please! ***** Client Height ***** [bohan] iso-(10)646 encoding only please! *****
	SetScrollSize(_width_total,_rect_client.Height());

	m_bInitialized = true;

	Invalidate();
}

void XMSamplerUIInst::SampleAssignEditor::OnLButtonUp(const UINT vKey, WTL::CPoint position)
{
	ATLTRACE2("XMSamplerUIInst::SampleAssignEditor::OnLButtonUp\n");
}

void XMSamplerUIInst::SampleAssignEditor::OnMouseMove(const UINT vKey, WTL::CPoint position)
{
	ATLTRACE2("XMSamplerUIInst::SampleAssignEditor::OnMouseMove\n");
	CPoint _offset;
	GetScrollOffset(_offset);

	position.x += _offset.x;
	position.y += _offset.y;
	
	//// ***** [bohan] iso-(10)646 encoding only please! *****
	//if(m_FocusKeyIndex)
	//{
	//	// ***** [bohan] iso-(10)646 encoding only please! *****
	//	// ***** [bohan] iso-(10)646 encoding only please! *****
	//	// ***** [bohan] iso-(10)646 encoding only please! *****
	//	// ***** [bohan] iso-(10)646 encoding only please! *****
	//	if(position.x >= m_FocusKeyRect.left && position.x <= m_FocusKeyRect.right 
	//		&& position.y >= m_FocusKeyRect.top && position.y <= m_FocusKeyRect.bottom){
	//		return;
	//	}
	//}

	boost::optional<int> _index(GetKeyIndexAtPoint(position.x,position.y,m_FocusKeyRect));
	if(_index)
	{
		if(!m_FocusKeyIndex || *m_FocusKeyIndex != *_index){
			m_FocusKeyIndex = _index;
			Invalidate(FALSE);
		}
	} else {
		if(!m_FocusKeyIndex){
			return;
		} else {
			m_FocusKeyIndex = _index;
			Invalidate(FALSE);
		}
	}
}

void XMSamplerUIInst::SampleAssignEditor::OnRButtonDown(const UINT vKey, WTL::CPoint position)
{
	ATLTRACE2("XMSamplerUIInst::SampleAssignEditor::OnRButtonDown\n");

}

void XMSamplerUIInst::SampleAssignEditor::OnLButtonDown(const UINT vKey, WTL::CPoint position)
{
	ATLTRACE2("SampleAssignEditor::OnLButtonDown()\n");

	SetFocus();
}

/// ***** [bohan] iso-(10)646 encoding only please! *****
const boost::optional<int> XMSamplerUIInst::SampleAssignEditor::GetKeyIndexAtPoint(const int x,const int y,WTL::CRect& keyRect)
{
	const int _octave_width = m_pNaturalKey->GetWidth() * NATURAL_KEY_PER_OCTAVE;

	// ***** [bohan] iso-(10)646 encoding only please! *****
	const int _sharpkey_num = 10 /* octave */ * SHARP_KEY_PER_OCTAVE /* sharpkeys per octave */; 
	const int _sharpkey_offset[SHARP_KEY_PER_OCTAVE] = {1,3,6,8,10};
	const int _sharpkey_width = m_pSharpKey->GetWidth();
	const int _sharpkey_height = m_pSharpKey->GetHeight();

	for(int i = 0;i < _sharpkey_num;i++)
	{
		const int _xpos = SHARPKEY_XPOS[i % SHARP_KEY_PER_OCTAVE] + (i / SHARP_KEY_PER_OCTAVE) * _octave_width;
		if(x >= _xpos && x <= (_xpos + _sharpkey_width) && y >= 0 && y <= _sharpkey_height)
		{
			keyRect.top = 0;
			keyRect.bottom = _sharpkey_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + _sharpkey_width;

			return _sharpkey_offset[i % SHARP_KEY_PER_OCTAVE] + (i / SHARP_KEY_PER_OCTAVE) * KEY_PER_OCTAVE;
		}
	}

	// ***** [bohan] iso-(10)646 encoding only please! *****
	const int _notekey_num = 10 /* octave */ * NATURAL_KEY_PER_OCTAVE ; 
	const int _notekey_offset[NATURAL_KEY_PER_OCTAVE] = {0,2,4,5,7,9,11};
	const int _note_width = m_pNaturalKey->GetWidth();
	const int _note_height = m_pNaturalKey->GetHeight();

	for(int i = 0;i < _notekey_num;i++)
	{
		const int _xpos = _note_width * (i % NATURAL_KEY_PER_OCTAVE) + (i / NATURAL_KEY_PER_OCTAVE) * _octave_width;
		if(x >= _xpos && x <= (_xpos + _note_width) && y >= 0 && y <= _note_height)
		{
			keyRect.top = 0;
			keyRect.bottom = _note_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + _note_width;

			return _notekey_offset[i % NATURAL_KEY_PER_OCTAVE] + (i / NATURAL_KEY_PER_OCTAVE) * KEY_PER_OCTAVE;
		}
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

	return boost::optional<int>();//ñ≥å¯Ç»ílÇï‘Ç∑ÅB
};
#endif

NAMESPACE__END
NAMESPACE__END
