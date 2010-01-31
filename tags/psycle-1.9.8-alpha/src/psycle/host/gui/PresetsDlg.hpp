///\file
///\brief interface file for psycle::host::CPresetsDlg.
#pragma once
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		class Machine;
		class CFrameMachine;


		/// machine parameter preset
		class CPreset  
		{
			private:
				/// happy?
				typedef unsigned char byte;
				int numPars;
				int* params;
				long int sizeData;
				byte* data;
				char name[32];
			public:
				void Clear();
				void Init(int num);
				void Init(int num,char* newname,int* parameters,int size, byte* newdata);
				void Init(int num,char* newname, float* parameters); // for VST .fxb's
				int GetNumPars() { return numPars; }
				void GetParsArray(int* destarray) { if(numPars>0){ memcpy(destarray,params,numPars*sizeof(int)); }}
				void GetDataArray(byte* destarray) {if(sizeData>0){ memcpy(destarray,data,sizeData); }}
				byte* GetData() {return data;}
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
				/// happy?
				typedef unsigned char byte;
				int const static MAX_PRESETS = 256;
			public:
				void AddPreset(CPreset& preset);
				void AddPreset(char* name ,float* parameters);
				void AddPreset(char* name,int* parameters, byte* newdata);
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
				CString fileName;
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