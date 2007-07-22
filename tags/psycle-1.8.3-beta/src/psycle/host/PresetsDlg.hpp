///\file
///\brief interface file for psycle::host::CPresetsDlg.
#pragma once
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		#define MAX_PRESETS 256

		class Machine;
		class CFrameMachine;

		/// machine parameter preset
		class CPreset  
		{
			int numPars;
			int* params;
			long int sizeData;
			byte* data;
			char name[32];
		public:
			void Clear();
			void Init(int num);
			void Init(int num,const char* newname,int* parameters,int size, byte* newdata);
			void Init(int num,const char* newname, float* parameters); // for VST .fxb's
			int GetNumPars() { return numPars; }
			void GetParsArray(int* destarray) { if(numPars>0){ memcpy(destarray,params,numPars*sizeof(int)); }}
			void GetDataArray(byte* destarray) {if(sizeData>0){ memcpy(destarray,data,sizeData); }}
			byte* GetData() {return data;}
			long int GetSizeData() {return sizeData;}
			void SetName(const char *setname) { strcpy(name,setname); }
			void GetName(char *nname) { strcpy(nname,name); }
			int GetParam(const int n);
			void SetParam(const int n,int val);
			CPreset();
			virtual ~CPreset();
			void operator=(CPreset& newpreset);
		};

		/// machine parameter preset window.
		class CPresetsDlg : public CDialog
		{
		public:
			void AddPreset(CPreset& preset);
			void AddPreset(const char* name ,float* parameters);
			void AddPreset(const char* name,int* parameters, byte* newdata);
			Machine* _pMachine;
			CPresetsDlg(CWnd* pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CPresetsDlg)
			enum { IDD = IDD_PRESETS };
			CButton	m_exportButton;
			CButton	m_preview;
			CComboBox	m_preslist;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CPresetsDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			int numParameters;
			long int sizeDataStruct;
			int fileversion;
			void TweakMachine(CPreset &preset);
			void SavePresets();
			void ReadPresets();
			void UpdateList();
			CPreset iniPreset;
			CPreset presets[MAX_PRESETS];
			CString fileName;
			bool presetChanged;
			// Generated message map functions
			//{{AFX_MSG(CPresetsDlg)
			afx_msg void OnSave();
			afx_msg void OnDelete();
			afx_msg void OnImport();
			afx_msg void OnSelchangePresetslist();
			afx_msg void OnDblclkPresetslist();
			virtual void OnOK();
			virtual void OnCancel();
			virtual BOOL OnInitDialog();
			afx_msg void OnExport();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	NAMESPACE__END
NAMESPACE__END
