// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\implementation psycle::host::CDSoundConfig.
#include <psycle/project.private.hpp>
#include "DSoundConfig.hpp"
#include <diversalis/compiler.hpp>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "winmm")
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <dsound.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
#endif

#include <iomanip>

#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		CDSoundConfig::CDSoundConfig(CWnd* pParent)
		:
			CDialog(CDSoundConfig::IDD, pParent),
			device_guid(), // DSDEVID_DefaultPlayback <-- unresolved external symbol
			exclusive(),
			dither(),
			sample_rate(44100),
			buffer_count(4),
			buffer_size(4096)
		{
		}

		void CDSoundConfig::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_EXCLUSIVE, m_exclusiveCheck);
			DDX_Control(pDX, IDC_DSOUND_LATENCY, m_latency);
			DDX_Control(pDX, IDC_DSOUND_BUFNUM_SPIN, m_numBuffersSpin);
			DDX_Control(pDX, IDC_DSOUND_BUFSIZE_SPIN, m_bufferSizeSpin);
			DDX_Control(pDX, IDC_DSOUND_DITHER, m_ditherCheck);
			DDX_Control(pDX, IDC_DSOUND_BUFSIZE_EDIT, m_bufferSizeEdit);
			DDX_Control(pDX, IDC_DSOUND_BUFNUM_EDIT, m_numBuffersEdit);
			DDX_Control(pDX, IDC_DSOUND_SAMPLERATE_COMBO, m_sampleRateCombo);
			DDX_Control(pDX, IDC_DSOUND_DEVICE, m_deviceComboBox);
		}

		BEGIN_MESSAGE_MAP(CDSoundConfig, CDialog)
			ON_CBN_SELENDOK(IDC_DSOUND_SAMPLERATE_COMBO, OnSelendokSamplerate)
			ON_EN_CHANGE(IDC_DSOUND_BUFNUM_EDIT, OnChangeBufnumEdit)
			ON_EN_CHANGE(IDC_DSOUND_BUFSIZE_EDIT, OnChangeBufsizeEdit)
			ON_CBN_SELCHANGE(IDC_DSOUND_DEVICE, OnSelchangeDevice)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_EXCLUSIVE, OnExclusive)
			ON_BN_CLICKED(IDC_DSOUND_DITHER, OnDither)
		END_MESSAGE_MAP()

		BOOL CDSoundConfig::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			// displays the DirectSound device in the combo box
			{
				class callback
				{
					public:
						BOOL static CALLBACK DirectSoundEnumerateCallback(GUID * device_guid, char const * description, char const * name, void * context)
						{
							CDSoundConfig & enclosing(*reinterpret_cast<CDSoundConfig*>(context));
							// add the entry to the combo box, making a copy of the guid and the description
							name; // unused
							enclosing.m_deviceComboBox.AddString(description);
							enclosing.m_deviceComboBox.SetItemData
								(
									enclosing.m_deviceComboBox.FindString(0, description),
									reinterpret_cast<DWORD_PTR>(device_guid ? new GUID(*device_guid) : new GUID(GUID())) // DSDEVID_DefaultPlayback <-- unresolved external symbol
								);
							return TRUE;
						}
				};
				/*directsound*/::DirectSoundEnumerate(callback::DirectSoundEnumerateCallback, this);
				m_deviceComboBox.SetCurSel(0);
				for(int i(m_deviceComboBox.GetCount() - 1) ; i >= 0 ; --i)
				{
					if(this->device_guid == *reinterpret_cast<GUID const * const>(m_deviceComboBox.GetItemData(i)))
					{
						m_deviceComboBox.SetCurSel(i);
						#if defined NDEBUG
							break;
						#endif
					}
				}
			}
			// displays the driver options (boolean values) in the check boxes
			{
				m_ditherCheck.SetCheck(dither);
				m_exclusiveCheck.SetCheck(exclusive);
			}
			// displays the sample rate
			{
				CString str;
				str.Format("%d", sample_rate);
				int const i(m_sampleRateCombo.SelectString(-1, str));
				if(i == CB_ERR) m_sampleRateCombo.SelectString(-1, "44100");
			}
			// displays the buffer count
			{
				CString str;
				str.Format("%d", buffer_count);
				m_numBuffersEdit.SetWindowText(str);
				m_numBuffersSpin.SetRange(buffer_count_min, buffer_count_max);
			}
			// displays the buffer size
			{
				CString str;
				str.Format("%d", buffer_size);
				m_bufferSizeEdit.SetWindowText(str);
				m_bufferSizeSpin.SetRange32(buffer_size_min, buffer_size_max);
				// sets the spin button increments
				{
					UDACCEL accel;
					accel.nSec = 0;
					accel.nInc = 512;
					m_bufferSizeSpin.SetAccel(1, &accel);
				}
			}
			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CDSoundConfig::OnOK() 
		{
			if(!m_deviceComboBox.GetCount())
			{
				// no DirectSound driver ...
				::MessageBox(0, "no DirectSound driver", "Warning", MB_ICONWARNING);
				// we cancel instead.
				CDialog::OnCancel();
				return;
			}
			CDialog::OnOK();
		}

		void CDSoundConfig::RecalcLatency()
		{
			// computes the latency
			float latency_time_seconds;
			{
				int const latency_bytes(buffer_size * buffer_count);
				int const latency_samples(latency_bytes / 4); ///\todo hardcoded to stereo 16-bit
				latency_time_seconds = float(latency_samples) / sample_rate;
			}
			// displays the latency in the gui
			{
				std::ostringstream s;
				s << "Latency: "
					//<< std::setprecision(0)
					<< static_cast<int>(latency_time_seconds * 1e3) << "ms";
				m_latency.SetWindowText(s.str().c_str());
			}
		}

		void CDSoundConfig::OnSelchangeDevice() 
		{
			// read the selected device guid from the gui
			device_guid = *reinterpret_cast<GUID const *>(m_deviceComboBox.GetItemData(m_deviceComboBox.GetCurSel()));
		}
		void CDSoundConfig::OnExclusive()
		{
			exclusive = m_exclusiveCheck.GetState() & 1;
		}

		void CDSoundConfig::OnDither()
		{
			dither = m_ditherCheck.GetState() & 1;
		}

		void CDSoundConfig::OnChangeBufsizeEdit() 
		{
			if(!IsWindow(m_bufferSizeEdit.GetSafeHwnd())) return;
			// read the buffer size from the gui
			CString s;
			m_bufferSizeEdit.GetWindowText(s);
			buffer_size = std::atoi(s);
			RecalcLatency();
		}

		void CDSoundConfig::OnChangeBufnumEdit() 
		{
			if(!IsWindow(m_numBuffersEdit.GetSafeHwnd())) return;
			// read the buffer count from the gui
			CString s;
			m_numBuffersEdit.GetWindowText(s);
			buffer_count = std::atoi(s);
			RecalcLatency();
		}

		void CDSoundConfig::OnSelendokSamplerate() 
		{
			if(!IsWindow(m_sampleRateCombo.GetSafeHwnd())) return;
			// read the sample rate from the gui
			CString s;
			m_sampleRateCombo.GetWindowText(s);
			sample_rate = std::atoi(s);
			RecalcLatency();
		}

		void CDSoundConfig::OnDestroy() 
		{
			for(int i(m_deviceComboBox.GetCount()) ; i > 0 ; delete reinterpret_cast<void*>(m_deviceComboBox.GetItemData(--i))); // must hard cast due to mfc's design
			CDialog::OnDestroy();
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
