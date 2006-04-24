///\file
///\brief interface file for psycle::host::CPresetsDlg.
#pragma once
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class Machine;
		class Plugin;
		class CFrameMachine;


		/// machine parameter preset
		class CPreset  
		{
			private:
				int numPars;
				int* params;
				long int sizeData;
				unsigned char * data;
				char name[32];
			public:
				void Clear();
				void Init(int num);
				void Init(int num, char* newname, int   const * parameters, int size, unsigned char * newdata);
				void Init(int num, char* newname, float const * parameters); // for VST .fxb's
				int GetNumPars() { return numPars; }
				void GetParsArray(int* destarray) { if(numPars>0){ memcpy(destarray,params,numPars*sizeof(int)); }}
				void GetDataArray(unsigned char * destarray) {if(sizeData>0){ memcpy(destarray,data,sizeData); }}
				unsigned char * GetData() {return data;}
				long int GetSizeData() {return sizeData;}
				void SetName(char *setname) { strcpy(name,setname); }
				void GetName(char *nname) { strcpy(nname,name); }
				int GetParam(int n);
				void SetParam(int n,int val);
				CPreset();
				virtual ~CPreset();
				void operator=(CPreset& newpreset);
		};

		/// machine parameter preset window.
		class CPresetsDlg : public CDialog
		{
			private:
				int const static MAX_PRESETS = 256;
			public:
				void AddPreset(CPreset& preset);
				void AddPreset(char* name ,float* parameters);
				void AddPreset(char* name,int* parameters, unsigned char * newdata);
				CFrameMachine* m_wndFrame;
				Machine* _pMachine;
				CPresetsDlg(CWnd* pParent = 0);
				enum { IDD = IDD_PRESETS };
				CButton	m_exportButton;
				CButton	m_preview;
				CComboBox	m_preslist;
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
				int numParameters;
				long int sizeDataStruct;
				int fileversion;
				void TweakMachine(CPreset &preset);
				void SavePresets();
				void ReadPresets();
				void UpdateList();
				CPreset iniPreset;
				CPreset presets[MAX_PRESETS];
				std::string file_name;
				bool presetChanged;
				afx_msg void OnSave();
				afx_msg void OnDelete();
				afx_msg void OnImport();
				afx_msg void OnSelchangePresetslist();
				afx_msg void OnDblclkPresetslist();
				virtual void OnOK();
				virtual void OnCancel();
				virtual BOOL OnInitDialog();
				afx_msg void OnExport();
			DECLARE_MESSAGE_MAP()
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
