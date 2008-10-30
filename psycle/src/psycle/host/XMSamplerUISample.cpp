#include <psycle/project.private.hpp>
#include "XMSamplerUISample.hpp"
#include "Psycle.hpp"
#include "XMSampler.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

// CWaveScopeCtrl
CWaveScopeCtrl::CWaveScopeCtrl()
: m_pWave(NULL)
{
	cpen_lo.CreatePen(PS_SOLID,0,0xFF0000);
	cpen_med.CreatePen(PS_SOLID,0,0xCCCCCC);
	cpen_hi.CreatePen(PS_SOLID,0,0x00FF00);
	cpen_sus.CreatePen(PS_DOT,0,0xFF0000);
}
CWaveScopeCtrl::~CWaveScopeCtrl(){
	cpen_lo.DeleteObject();
	cpen_med.DeleteObject();
	cpen_hi.DeleteObject();
	cpen_sus.DeleteObject();
}

void CWaveScopeCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if (m_pWave && lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
	{
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		int wrHeight = 0, yLow = 0, yHi = 0;
		double OffsetStep = 0;
		__int32 c, d;

		CRect rect;
		GetClientRect(&rect);

		int const nWidth=rect.Width();
		int const nHeight=rect.Height();
		int const my=nHeight/2;
		if(rWave().IsWaveStereo()) wrHeight=my/2;
		else wrHeight=my;
		helpers::dsp::Cubic resampler;
		resampler.SetQuality(helpers::dsp::R_SPLINE);

		dc.FillSolidRect(&rect,RGB(255,255,255));
		dc.SetBkMode(TRANSPARENT);

		if(rWave().WaveLength())
		{

			// Draw preliminary stuff

			CPen *oldpen= dc.SelectObject(&cpen_med);

			// Left channel 0 amplitude line
			dc.MoveTo(0,wrHeight);
			dc.LineTo(nWidth,wrHeight);

			int const wrHeight_R = my + wrHeight;

			if(rWave().IsWaveStereo())
			{
				// Right channel 0 amplitude line
				dc.MoveTo(0,wrHeight_R);
				dc.LineTo(nWidth,wrHeight_R);

				// Stereo channels separator line
				dc.SelectObject(&cpen_lo);
				dc.MoveTo(0,my);
				dc.LineTo(nWidth,my);
			}

			dc.SelectObject(&cpen_hi);

			OffsetStep = (double) rWave().WaveLength() / nWidth;

			if ( OffsetStep > 4)
			{
				for(c = 0; c < nWidth; c++)
				{
					long const offset = (long)floorf(c * OffsetStep);
					yLow=0;yHi=0;
					// Alternate search. Doing the same than in the next "else if"
					// can be slow on big ( 100.000+ sample) samples
					for (d = offset; d < offset + OffsetStep; d+=(OffsetStep/4))
					{
						if (yLow > rWave().WaveDataL(d)) yLow = rWave().WaveDataL(d);
						if (yHi < rWave().WaveDataL(d)) yHi = rWave().WaveDataL(d);
					}
					int const ryLow = (wrHeight * yLow)/32768;
					int const ryHi = (wrHeight * yHi)/32768;
					dc.MoveTo(c,(wrHeight) + ryLow);
					dc.LineTo(c,(wrHeight) + ryHi);
				}
			}
			else if ( OffsetStep >1)
			{
				for(c = 0; c < nWidth; c++)
				{
					long const offset = (long)floorf(c * OffsetStep);
					yLow=0;yHi=0;
					for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
					{
						if (yLow > rWave().WaveDataL(d)) yLow = rWave().WaveDataL(d);
						if (yHi < rWave().WaveDataL(d)) yHi = rWave().WaveDataL(d);
					}
					int const ryLow = (wrHeight * yLow)/32768;
					int const ryHi = (wrHeight * yHi)/32768;
					dc.MoveTo(c,(wrHeight) + ryLow);
					dc.LineTo(c,(wrHeight) + ryHi);
				}
			}
			else
			{
				for(c = 0; c < nWidth; c++)
				{
					ULARGE_INTEGER posin;
					posin.QuadPart = c * OffsetStep* 4294967296.0f;
					yHi=0;
					yLow=resampler._pWorkFn(rWave().pWaveDataL()+posin.HighPart,posin.HighPart,posin.LowPart,rWave().WaveLength());

					int const ryLow = (wrHeight * yLow)/32768;
					int const ryHi = (wrHeight * yHi)/32768;
					dc.MoveTo(c,(wrHeight) + ryLow);
					dc.LineTo(c,(wrHeight) + ryHi);
				}
			}

			if(rWave().IsWaveStereo())
			{
				if ( OffsetStep > 4)
				{
					for(c = 0; c < nWidth; c++)
					{
						long const offset = (long)floorf(c * OffsetStep);
						yLow=0;yHi=0;
						for (d = offset; d < offset + OffsetStep; d+=(OffsetStep/4))
						{
							if (yLow > rWave().WaveDataR(d)) yLow = rWave().WaveDataR(d);
							if (yHi < rWave().WaveDataR(d)) yHi = rWave().WaveDataR(d);
						}
						int const ryLow = (wrHeight * yLow)/32768;
						int const ryHi = (wrHeight * yHi)/32768;
						dc.MoveTo(c,wrHeight_R - ryLow);
						dc.LineTo(c,wrHeight_R - ryHi);
					}
				}
				else if ( OffsetStep >1)
				{
					for(c = 0; c < nWidth; c++)
					{
						long const offset = (long)floorf(c * OffsetStep);
						yLow=0;yHi=0;
						for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
						{
							if (yLow > rWave().WaveDataR(d)) yLow = rWave().WaveDataR(d);
							if (yHi < rWave().WaveDataR(d)) yHi = rWave().WaveDataR(d);
						}
						int const ryLow = (wrHeight * yLow)/32768;
						int const ryHi = (wrHeight * yHi)/32768;
						dc.MoveTo(c,wrHeight_R - ryLow);
						dc.LineTo(c,wrHeight_R - ryHi);
					}
				}
				else
				{
					for(c = 0; c < nWidth; c++)
					{
						ULARGE_INTEGER posin;
						posin.QuadPart = c * OffsetStep* 4294967296.0f;
						yHi=0;
						yLow=resampler._pWorkFn(rWave().pWaveDataR()+posin.HighPart,posin.HighPart,posin.LowPart,rWave().WaveLength());

						int const ryLow = (wrHeight * yLow)/32768;
						int const ryHi = (wrHeight * yHi)/32768;
						dc.MoveTo(c,wrHeight_R - ryLow);
						dc.LineTo(c,wrHeight_R - ryHi);
					}
				}
			}
			if ( rWave().WaveLoopType() != XMInstrument::WaveData::LoopType::DO_NOT )
			{
				dc.SelectObject(&cpen_lo);
				int ls = (rWave().WaveLoopStart()* nWidth) /rWave().WaveLength();
				dc.MoveTo(ls,0);
				dc.LineTo(ls,nHeight);
				dc.TextOut(ls,12,"Start");
				int le = (rWave().WaveLoopEnd()* nWidth)/rWave().WaveLength();
				dc.MoveTo(le,0);
				dc.LineTo(le,nHeight);
				dc.TextOut(le-18,nHeight-24,"End");

			}
			if ( rWave().WaveSusLoopType() != XMInstrument::WaveData::LoopType::DO_NOT )
			{
				dc.SelectObject(&cpen_sus);
				int ls = (rWave().WaveSusLoopStart()* nWidth)/rWave().WaveLength();
				dc.MoveTo(ls,0);
				dc.LineTo(ls,nHeight);
				dc.TextOut(ls,0,"Start");
				int le = (rWave().WaveSusLoopEnd()* nWidth)/rWave().WaveLength();
				dc.MoveTo(le,0);
				dc.LineTo(le,nHeight);
				dc.TextOut(le-18,nHeight-12,"End");
			}
			dc.SelectObject(oldpen);
		}
		else
		{
			dc.TextOut(4,4,"No Wave Data");
		}
		dc.Detach();
	}
}

// XMSamplerUISample

IMPLEMENT_DYNAMIC(XMSamplerUISample, CPropertyPage)
XMSamplerUISample::XMSamplerUISample()
: CPropertyPage(XMSamplerUISample::IDD)
, m_Init(false)
, m_pMachine(NULL)
, m_pWave(NULL)
{
}

XMSamplerUISample::~XMSamplerUISample()
{
}

void XMSamplerUISample::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLELIST, m_SampleList);
	DDX_Control(pDX, IDC_WAVESCOPE, m_WaveScope);
}



BEGIN_MESSAGE_MAP(XMSamplerUISample, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_SAMPLELIST, OnLbnSelchangeSamplelist)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DEFVOLUME, OnNMCustomdrawDefvolume)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_GLOBVOLUME, OnNMCustomdrawGlobvolume)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PAN, OnNMCustomdrawPan)
	ON_CBN_SELENDOK(IDC_VIBRATOTYPE, OnCbnSelendokVibratotype)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIBRATOATTACK, OnNMCustomdrawVibratoAttack)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIBRATOSPEED, OnNMCustomdrawVibratospeed)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIBRATODEPTH, OnNMCustomdrawVibratodepth)
	ON_CBN_SELENDOK(IDC_LOOP, OnCbnSelendokLoop)
	ON_CBN_SELENDOK(IDC_SUSTAINLOOP, OnCbnSelendokSustainloop)
	ON_EN_CHANGE(IDC_LOOPSTART, OnEnChangeLoopstart)
	ON_EN_CHANGE(IDC_LOOPEND, OnEnChangeLoopend)
	ON_EN_CHANGE(IDC_SUSTAINSTART, OnEnChangeSustainstart)
	ON_EN_CHANGE(IDC_SUSTAINEND, OnEnChangeSustainend)
	ON_EN_CHANGE(IDC_WAVENAME, OnEnChangeWavename)
	ON_EN_CHANGE(IDC_SAMPLERATE, OnEnChangeSamplerate)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSAMPLERATE, OnDeltaposSpinsamplerate)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SAMPLENOTE, OnNMCustomdrawSamplenote)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_FINETUNE, OnNMCustomdrawFinetune)
	ON_BN_CLICKED(IDC_OPENWAVEEDITOR, OnBnClickedOpenwaveeditor)
	ON_BN_CLICKED(IDC_LOAD, OnBnClickedLoad)
	ON_BN_CLICKED(IDC_SAVE, OnBnClickedSave)
	ON_BN_CLICKED(IDC_DUPE, OnBnClickedDupe)
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_BN_CLICKED(IDC_PANENABLED, OnBnClickedPanenabled)
END_MESSAGE_MAP()

// Controladores de mensajes de XMSamplerUISample
BOOL XMSamplerUISample::OnSetActive()
{
	if (!m_Init ) {
		((CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME))->SetRangeMax(128);
		((CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME))->SetRangeMax(128);
		((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetRangeMax(128);
		((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetRangeMin(-59);
		((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetRangeMax(59);
		((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetPos(1);
		((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetRangeMax(256);
		((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetRangeMin(-256);
		((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetPos(26);
		((CSliderCtrl*)GetDlgItem(IDC_VIBRATOATTACK))->SetRangeMax(255);
		((CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED))->SetRangeMax(64);
		((CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH))->SetRangeMax(32);
		CComboBox* vibratoType = ((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE));
		vibratoType->ResetContent();
		vibratoType->AddString("Sinus");
		vibratoType->AddString("Square");
		vibratoType->AddString("RampUp");
		vibratoType->AddString("RampDown");
		vibratoType->AddString("Random");
		CComboBox* sustainLoop = ((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP));
		sustainLoop->ResetContent();
		sustainLoop->AddString("Disabled");
		sustainLoop->AddString("Forward");
		sustainLoop->AddString("Bidirection");
		CComboBox* loop =  ((CComboBox*)GetDlgItem(IDC_LOOP));
		loop->ResetContent();
		loop->AddString("Disabled");
		loop->AddString("Forward");
		loop->AddString("Bidirection");
		m_SampleList.SetCurSel(0);
		m_SampleList.ResetContent();
		for (int i=0;i<XMSampler::MAX_INSTRUMENT;i++)
		{
			char line[48];
			XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
			sprintf(line,"%02X%s: ",i,wave.WaveLength()>0?"*":" ");
			strcat(line,wave.WaveName().c_str());
			m_SampleList.AddString(line);
		}
	}
	if ( m_SampleList.GetCurSel() == -1 ) {
		m_SampleList.SetCurSel(0);
	}
	OnLbnSelchangeSamplelist();
	m_Init=true;

	return CPropertyPage::OnSetActive();
}

void XMSamplerUISample::OnLbnSelchangeSamplelist()
{
	m_Init=false;
	char tmp[40];
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	pWave(&wave);

	strcpy(tmp,wave.WaveName().c_str());
	((CEdit*)GetDlgItem(IDC_WAVENAME))->SetWindowText(tmp);
	sprintf(tmp,"%.0d",wave.WaveSampleRate());
	((CEdit*)GetDlgItem(IDC_SAMPLERATE))->SetWindowText(tmp);
	((CStatic*)GetDlgItem(IDC_WAVESTEREO))->SetWindowText(wave.IsWaveStereo()?"Stereo":"Mono");
	sprintf(tmp,"%d",wave.WaveLength());
	((CStatic*)GetDlgItem(IDC_WAVELENGTH))->SetWindowText(tmp);

	((CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME))->SetPos(int(wave.WaveGlobVolume()*128.0f));
	((CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME))->SetPos(wave.WaveVolume());
	if ( wave.PanFactor() > 1.0f )// SurrounD
	{
		((CButton*)GetDlgItem(IDC_PANENABLED))->SetCheck(wave.PanEnabled()?2:0);
		((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetPos(int((wave.PanFactor()-1.0f)*128.0f));
		((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText("SurrounD");
	}
	else
	{
		const int panpos=wave.PanFactor()*128.0f;
		((CButton*)GetDlgItem(IDC_PANENABLED))->SetCheck(wave.PanEnabled()?1:0);
		((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetPos(panpos);
		char tmp[40];
		switch(panpos)
		{
		case 0: sprintf(tmp,"||%02d  ",panpos); break;
		case 64: sprintf(tmp," |%02d| ",panpos); break;
		case 128: sprintf(tmp,"  %02d||",panpos); break;
		default:
			if ( panpos < 32) sprintf(tmp,"<<%02d  ",panpos);
			else if ( panpos < 64) sprintf(tmp," <%02d< ",panpos);
			else if ( panpos <= 96) sprintf(tmp," >%02d> ",panpos);
			else sprintf(tmp,"  %02d>>",panpos);
			break;
		}
		((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText(tmp);
	}

	((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetPos(-1*int(wave.WaveTune()));
	((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetPos(int(wave.WaveFineTune()));

	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOATTACK))->SetPos(wave.VibratoAttack());
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED))->SetPos(wave.VibratoSpeed());
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH))->SetPos(wave.VibratoDepth());
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->SetCurSel((int)wave.VibratoType());
	((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP))->SetCurSel((int)wave.WaveSusLoopType());
	sprintf(tmp,"%d",wave.WaveSusLoopStart());
	((CEdit*)GetDlgItem(IDC_SUSTAINSTART))->SetWindowText(tmp);
	sprintf(tmp,"%d",wave.WaveSusLoopEnd());
	((CEdit*)GetDlgItem(IDC_SUSTAINEND))->SetWindowText(tmp);
	((CComboBox*)GetDlgItem(IDC_LOOP))->SetCurSel((int)wave.WaveLoopType());
	sprintf(tmp,"%d",wave.WaveLoopStart());
	((CEdit*)GetDlgItem(IDC_LOOPSTART))->SetWindowText(tmp);
	sprintf(tmp,"%d",wave.WaveLoopEnd());
	((CEdit*)GetDlgItem(IDC_LOOPEND))->SetWindowText(tmp);

    //
	m_WaveScope.SetWave(&wave);
	DrawScope();
	m_Init=true;
}

void XMSamplerUISample::OnBnClickedLoad()
{
/*	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|IFF Samples (*.iff)|*.iff|All Files (*.*)|*.*||";

	CWavFileDlg dlg(true,"wav", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg._pSong = _pSong;
	std::string tmpstr = Global::configuration().GetCurrentInstrumentDir();
	dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
	if (dlg.DoModal() == IDOK)
	{
		m_wndView.AddMacViewUndo();

		int si = _pSong->instSelected;

		//added by sampler
		if ( _pSong->_pInstrument[si]->waveLength != 0)
		{
			if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;
		}
		//end of added by sampler

		CString CurrExt=dlg.GetFileExt();
		CurrExt.MakeLower();

		if ( CurrExt == "wav" )
		{
			if (_pSong->WavAlloc(si,dlg.GetFileName()))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText("New wave loaded");
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
			}
		}
		else if ( CurrExt == "iff" )
		{
			if (_pSong->IffAlloc(si,dlg.GetFileName()))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText("New wave loaded");
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
				RedrawGearRackList();
			}
		}
		CString str = dlg.m_ofn.lpstrFile;
		int index = str.ReverseFind('\\');
		if (index != -1)
		{
			Global::configuration().SetCurrentInstrumentDir(static_cast<char const *>(str.Left(index)));
		}
	}
	if ( _pSong->_pInstrument[PREV_WAV_INS]->waveLength > 0)
	{
		// Stopping wavepreview if not stopped.
		if(_pSong->PW_Stage)
		{
			_pSong->PW_Stage=0;
			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);
		}

		//Delete it.
		_pSong->DeleteLayer(PREV_WAV_INS);
		_pSong->IsInvalided(false);
	}
*/
}

void XMSamplerUISample::OnBnClickedSave()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUISample::OnBnClickedDupe()
{
	for (int j=0;j<XMSampler::MAX_INSTRUMENT;j++)
	{
		if ( m_pMachine->SampleData(j).WaveLength() == 0 ) 
		{
			XMInstrument::WaveData& wavenew = m_pMachine->SampleData(j);
			wavenew = rWave();
			return;
		}
	}
	MessageBox("Couldn't find an appropiate sample slot to copy to.","Error While Duplicating!");
}

void XMSamplerUISample::OnBnClickedDelete()
{
	rWave().Init();
	//\todo: Do a search for instruments using this sample and remove it from them.
}

void XMSamplerUISample::OnEnChangeWavename()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_WAVENAME);
		cedit->GetWindowText(tmp,40);
		rWave().WaveName(tmp);
	}
}

void XMSamplerUISample::OnEnChangeSamplerate()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_SAMPLERATE);
		cedit->GetWindowText(tmp,40);
		rWave().WaveSampleRate(atoi(tmp));
	}
}
void XMSamplerUISample::OnDeltaposSpinsamplerate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta > 0 )
	{
		rWave().WaveSampleRate(rWave().WaveSampleRate()*2);

	} else {
		rWave().WaveSampleRate(rWave().WaveSampleRate()*0.5f);
	}
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawDefvolume(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME);
	if ( m_Init )
	{
		rWave().WaveVolume(slid->GetPos());
	}
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LDEFVOL))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawGlobvolume(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME);
	if ( m_Init )
	{
		rWave().WaveGlobVolume(slid->GetPos()/128.0f);
	}
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LGLOBVOL))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnBnClickedPanenabled()
{
	CButton* check = (CButton*)GetDlgItem(IDC_PANENABLED);
	if ( m_Init )
	{
		rWave().PanEnabled(check->GetCheck()?true:false);
		if ( check->GetCheck() == 2 ) 
		{
			rWave().PanFactor(rWave().PanFactor()+1.0f);
		}
		else if ( rWave().PanFactor()>1.0f)  
		{
			rWave().PanFactor(rWave().PanFactor()-1.0f);
		}
	}
	if ( check->GetCheck() == 2 ) 
	{
		((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText("SurrounD");
	}
	else
	{
		char tmp[40];
		sprintf(tmp,"%d",(int)(rWave().PanFactor()*128));
		((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText(tmp);
	}
}

void XMSamplerUISample::OnNMCustomdrawPan(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_PAN);
	CButton* check = (CButton*)GetDlgItem(IDC_PANENABLED);
	if ( m_Init )
	{
		if ( check->GetCheck() != 2 ) // 2 == SurrounD
		{
			rWave().PanFactor(slid->GetPos()/128.0f);
		}
	}
	if ( check->GetCheck() != 2 ) // 2 == SurrounD
	{
	char tmp[40];
		switch(slid->GetPos())
		{
		case 0: sprintf(tmp,"||%02d  ",slid->GetPos()); break;
		case 64: sprintf(tmp," |%02d| ",slid->GetPos()); break;
		case 128: sprintf(tmp,"  %02d||",slid->GetPos()); break;
		default:
			if ( slid->GetPos() < 32) sprintf(tmp,"<<%02d  ",slid->GetPos());
			else if ( slid->GetPos() < 64) sprintf(tmp," <%02d< ",slid->GetPos());
			else if ( slid->GetPos() <= 96) sprintf(tmp," >%02d> ",slid->GetPos());
			else sprintf(tmp,"  %02d>>",slid->GetPos());
			break;
		}

		((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText(tmp);
	}
	else
	{
		((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText("SurrounD");
	}
	*pResult = 0;
}
void XMSamplerUISample::OnNMCustomdrawSamplenote(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE);
	if ( m_Init )
	{
		rWave().WaveTune(-1*slid->GetPos());
	}
	char tmp[40], tmp2[40];
	char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
	sprintf(tmp,"%s",notes[(60+rWave().WaveTune())%12]);
	sprintf(tmp2,"%s%d",tmp,(60+rWave().WaveTune())/12);
	((CStatic*)GetDlgItem(IDC_LSAMPLENOTE))->SetWindowText(tmp2);

	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawFinetune(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_FINETUNE);
	if ( m_Init )
	{
		rWave().WaveFineTune(slid->GetPos());
	}
	char tmp[40];
	sprintf(tmp,"%.02f",slid->GetPos()/256.0f);
	((CStatic*)GetDlgItem(IDC_LFINETUNE))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnBnClickedOpenwaveeditor()
{
	//\todo : pParent->OnWavebut();
}
void XMSamplerUISample::OnCbnSelendokVibratotype()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_VIBRATOTYPE);
	rWave().VibratoType(XMInstrument::WaveData::WaveForms(cbox->GetCurSel()));
}

void XMSamplerUISample::OnNMCustomdrawVibratoAttack(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_VIBRATOATTACK);
	if ( m_Init )
	{
		rWave().VibratoAttack(slid->GetPos());
	}
	char tmp[40];
	if ( slid->GetPos()>0 ) sprintf(tmp,"%d",slid->GetPos());
	else strcpy(tmp,"Disabled");
	((CStatic*)GetDlgItem(IDC_LVIBRATOATTACK))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawVibratospeed(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED);
	if ( m_Init )
	{
		rWave().VibratoSpeed(slid->GetPos());
	}
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LVIBRATOSPEED))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawVibratodepth(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH);
	if ( m_Init )
	{
		rWave().VibratoDepth(slid->GetPos());
	}
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LVIBRATODEPTH))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnCbnSelendokLoop()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_LOOP);
	rWave().WaveLoopType(XMInstrument::WaveData::LoopType(cbox->GetCurSel()));
	DrawScope();
}

void XMSamplerUISample::OnCbnSelendokSustainloop()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_SUSTAINLOOP);
	rWave().WaveSusLoopType(XMInstrument::WaveData::LoopType(cbox->GetCurSel()));
	DrawScope();
}

void XMSamplerUISample::OnEnChangeLoopstart()
{
	char tmp[40];
	if ( m_Init )
	{
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_LOOPSTART);
	cedit->GetWindowText(tmp,40);
		rWave().WaveLoopStart(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::OnEnChangeLoopend()
{
	char tmp[40];
	if ( m_Init )
	{
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_LOOPEND);
	cedit->GetWindowText(tmp,40);
		rWave().WaveLoopEnd(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::OnEnChangeSustainstart()
{
	char tmp[40];
	if ( m_Init )
	{
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_SUSTAINSTART);
	cedit->GetWindowText(tmp,40);
		rWave().WaveSusLoopStart(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::OnEnChangeSustainend()
{
	char tmp[40];
	if ( m_Init )
	{
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_SUSTAINEND);
	cedit->GetWindowText(tmp,40);
		rWave().WaveSusLoopEnd(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::DrawScope()
{
	m_WaveScope.Invalidate();
}

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

