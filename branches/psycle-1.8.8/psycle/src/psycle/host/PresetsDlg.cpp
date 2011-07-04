///\file
///\brief implementation file for psycle::host::CPresetsDlg.

#include "PresetsDlg.hpp"
#include "Configuration.hpp"
#include "Plugin.hpp"

#include <cstring>

namespace psycle { namespace host {

		CPresetsDlg::CPresetsDlg(CWnd* pParent) : CDialog(CPresetsDlg::IDD, pParent)
		{
			numParameters = -1;
			dataSizeStruct = 0;
		}

		void CPresetsDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_EXPORT, m_exportButton);
			DDX_Control(pDX, IDC_PREVIEW, m_preview);
			DDX_Control(pDX, IDC_PRESETSLIST, m_preslist);
		}

		BEGIN_MESSAGE_MAP(CPresetsDlg, CDialog)
			ON_BN_CLICKED(IDC_SAVE, OnSave)
			ON_BN_CLICKED(IDC_DELETE, OnDelete)
			ON_BN_CLICKED(IDC_IMPORT, OnImport)
			ON_CBN_SELCHANGE(IDC_PRESETSLIST, OnSelchangePresetslist)
			ON_CBN_DBLCLK(IDC_PRESETSLIST, OnDblclkPresetslist)
			ON_BN_CLICKED(IDC_EXPORT, OnExport)
		END_MESSAGE_MAP()

		BOOL CPresetsDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_preslist.LimitText(32);
			presetChanged = false;

			numParameters = _pMachine->GetNumParams();
			dataSizeStruct = 0;
			void* pData = NULL;
			try
			{
				if( _pMachine->_type == MACH_PLUGIN)
				{
					dataSizeStruct = ((Plugin *)_pMachine)->proxy().GetDataSize();
				}
				if(dataSizeStruct > 0) //Can only happen for native plugins.
				{
					pData = new char[dataSizeStruct];
					reinterpret_cast<Plugin *>(_pMachine)->proxy().GetData(pData); // Internal Save
					iniPreset.Init(numParameters , "", reinterpret_cast<Plugin *>(_pMachine)->proxy().Vals(), dataSizeStruct, pData);
					delete[] pData;
				}
				else
				{
					iniPreset.Init(numParameters);
					for(int i=0; i < numParameters; ++i)
					{
						iniPreset.SetParam(i , _pMachine->GetParamValue(i));
					}
				}
			}
			catch(const std::exception &)
			{
				dataSizeStruct = 0;
				delete[] pData;
				iniPreset.Init(numParameters);
			}

			CString buffer;
			buffer = _pMachine->GetDllName();
			buffer = Global::psycleconf().GetPresetsDir().c_str() + buffer.Mid(buffer.ReverseFind('\\'));
			buffer = buffer.Left(buffer.GetLength()-4);
			buffer += ".prs";
			presetFile= buffer;
			ReadPresets();
			UpdateList();
			return TRUE;
		}

		void CPresetsDlg::OnSave() 
		{
			char str[32];
			if ( m_preslist.GetCurSel() == CB_ERR ) // Save into a new preset the current values.
			{
				m_preslist.GetWindowText(str,32);
				if ( str[0] == '\0' )
				{
					MessageBox("You have not specified any name. Operation Aborted.","Preset Save Error",MB_OK);
					return;
				}
				iniPreset.SetName(str);
				CPreset preset = iniPreset;
				presets.push_back(preset);
			}
			else // Update an existing preset with the current preset values.
			{
				int sel = m_preslist.GetCurSel();
				std::list<CPreset>::iterator preset = presets.begin();
				for(int i=0; preset != presets.end(); i++, preset++)
				{
					if(i == sel) {
						preset->GetName(str); // If we don't do this,
						iniPreset.SetName(str);// the name gets lost.
						*preset = iniPreset;
						break;
					}
				}
			}
			UpdateList();
			SavePresets();
		}

		void CPresetsDlg::OnDelete() 
		{
			int sel = m_preslist.GetCurSel();
			if ( sel != CB_ERR )
			{
				m_preslist.DeleteString(sel);
				std::list<CPreset>::iterator preset = presets.begin();
				for(int i=0; preset != presets.end(); i++, preset++)
				{
					if(i == sel) {
						presets.erase(preset);
						break;
					}
				}				
			}	
			UpdateList();
			SavePresets();
		}

		void CPresetsDlg::OnImport() 
		{
			if( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX) return;

			char szFile[MAX_PATH]; // buffer for file name
			szFile[0]='\0';

			OPENFILENAME ofn; // common dialog box structure
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = "Presets\0*.prs\0All\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Global::pConfig->GetPluginDir().c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if(GetOpenFileName(&ofn) == TRUE)
			{
				if(ofn.nFilterIndex == 1)
				{
					std::list<CPreset> import;
					PresetIO::LoadPresets(szFile, numParameters, dataSizeStruct, import);
					presets.merge(import);
				}
				UpdateList();
				SavePresets();
			}
		}

		void CPresetsDlg::OnExport() 
		{
			if( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX) return;

			if ( m_preslist.GetCurSel() == CB_ERR )
			{
				MessageBox("You have to select the preset(s) to export.","File Save Error",MB_OK);
			}
			char szFile[MAX_PATH];       // buffer for file name
			szFile[0]='\0';
			OPENFILENAME ofn;       // common dialog box structure
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = "Presets\0*.prs\0All\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Global::pConfig->GetPluginDir().c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	

			// Display the Open dialog box. 
			
			if (GetSaveFileName(&ofn)==TRUE)
			{
				int sel=m_preslist.GetCurSel();
				std::list<CPreset> presexport;
				std::list<CPreset>::iterator preset = presets.begin();
				for(int i=0; preset != presets.end(); i++, preset++)
				{
					///\todo: Change the presets list dialog to use a list instead of a combobox to allow multiselect
					if(i == sel) presexport.push_back(*preset);
				}
				PresetIO::SavePresets(szFile, presexport);
			}
		}

		void CPresetsDlg::OnSelchangePresetslist() 
		{
			if ( m_preview.GetCheck())
			{
				int sel=m_preslist.GetCurSel();
				std::list<CPreset>::iterator preset = presets.begin();
				for(int i=0; preset != presets.end(); i++, preset++)
				{
					if(i == sel) {
						TweakMachine(*preset);
						break;
					}
				}
			}
		}

		void CPresetsDlg::OnDblclkPresetslist() 
		{
			CPresetsDlg::OnOK();
		}

		void CPresetsDlg::OnOK() 
		{
			int sel=m_preslist.GetCurSel();
			std::list<CPreset>::iterator preset = presets.begin();
			for(int i=0; preset != presets.end(); i++, preset++)
			{
				if(i == sel) {
					TweakMachine(*preset);
					break;
				}
			}
			
			CDialog::OnOK();
		}

		void CPresetsDlg::OnCancel() 
		{
			if (presetChanged ) // Restore old preset if changed.
			{
				TweakMachine(iniPreset);
			}
			
			CDialog::OnCancel();
		}

		void CPresetsDlg::UpdateList()
		{
			char cbuf[32];
			m_preslist.ResetContent();
			presets.sort();

			std::list<CPreset>::iterator preset = presets.begin();
			for(;preset != presets.end(); preset++) 
			{
				preset->GetName(cbuf);
				m_preslist.AddString(cbuf);
			}
		}

		void CPresetsDlg::ReadPresets()
		{
			PresetIO::LoadPresets(presetFile, numParameters, dataSizeStruct, presets, false);
			UpdateList();
		}

		void CPresetsDlg::SavePresets()
		{
			PresetIO::SavePresets(presetFile, presets);
		}

		void CPresetsDlg::TweakMachine(const CPreset &preset)
		{
			int num=preset.GetNumPars();
			for(int i(0) ; i < num ; ++i)
			{
				try
				{
					_pMachine->SetParameter(i, preset.GetParam(i));
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				catch(...) // reinterpret_cast sucks
				{
					// o_O`
				}
			}
			if(preset.GetData() && _pMachine->_type == MACH_PLUGIN)
			{
				try
				{
					reinterpret_cast<Plugin *>(_pMachine)->proxy().PutData(preset.GetData()); // Internal save
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				catch(...) // reinterpret_cast sucks
				{
					// o_O`
				}
			}
			presetChanged = true;
		}

	}   // namespace
}   // namespace
