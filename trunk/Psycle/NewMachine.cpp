// NewMachine.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "NewMachine.h"
#include "Plugin.h"
#include "VstHost.h"
#include "ProgressDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CNewMachine::pluginOrder = 1;
bool CNewMachine::pluginName = 1;
int CNewMachine::_numPlugins = -1;
int CNewMachine::_numDirs = 0;
int CNewMachine::LastType0=0;
int CNewMachine::LastType1=0;

int CNewMachine::_numBadPlugins;
char * CNewMachine::_BadPluginPath[MAX_BAD_PLUGINS];
FILETIME CNewMachine::_BadPluginFileTime[MAX_BAD_PLUGINS];

PluginInfo* CNewMachine::_pPlugsInfo[MAX_BROWSER_PLUGINS];
//char *CNewMachine::_dirArray[MAX_BROWSER_NODES];
CMapStringToString CNewMachine::dllNames(64);


/////////////////////////////////////////////////////////////////////////////
// CNewMachine dialog

CNewMachine::CNewMachine(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMachine::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMachine)
	m_orderby = pluginOrder;
	m_showdllName = pluginName;
	//}}AFX_DATA_INIT
	OutBus = false;
	psOutputDll = NULL;
//	pluginOrder = 0; Do NOT uncomment. It would cause the variable to be reseted each time.
//					It is initialized above, where it is declared.
	for (int i = 0; i < MAX_BAD_PLUGINS; i++)
	{
		_BadPluginPath[i] = NULL;
	}
}

CNewMachine::~CNewMachine()
{
	if (psOutputDll != NULL)
	{
		delete psOutputDll;
	}
	for (int i = 0; i < MAX_BAD_PLUGINS; i++)
	{
		if (_BadPluginPath[i])
		{
			delete _BadPluginPath[i];
			_BadPluginPath[i] = NULL;
		}
	}
}

void CNewMachine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMachine)
	DDX_Control(pDX, IDC_NAMELABEL, m_nameLabel);
	DDX_Control(pDX, IDC_BROWSER, m_browser);
	DDX_Control(pDX, IDC_VERSIONLABEL, m_versionLabel);
	DDX_Control(pDX, IDC_DESCLABEL, m_descLabel);
	DDX_Radio(pDX, IDC_BYTYPE, m_orderby);
	DDX_Control(pDX, IDC_DLLNAMELABEL, m_dllnameLabel);
	DDX_Radio(pDX, IDC_SHOWDLLNAME, m_showdllName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
	//{{AFX_MSG_MAP(CNewMachine)
	ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_BYCLASS, OnByclass)
	ON_BN_CLICKED(IDC_BYTYPE, OnBytype)
	ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SHOWDLLNAME, OnShowdllname)
	ON_BN_CLICKED(IDC_SHOWEFFNAME, OnShoweffname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewMachine message handlers

BOOL CNewMachine::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (imgList.GetSafeHandle() != 0)
	{
		imgList.DeleteImageList();
 	}
	imgList.Create(IDB_MACHINETYPE,16,2,1);
	m_browser.SetImageList(&imgList,TVSIL_NORMAL);

	LoadPluginInfo();
	UpdateList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CNewMachine::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (imgList.GetSafeHandle() != 0)	{
		imgList.DeleteImageList();
	}
}

void CNewMachine::UpdateList(bool bInit)
{
	int nodeindex;

	m_browser.DeleteAllItems();

	HTREEITEM intFxNode;

	if (pluginOrder == 0 )
	{
		hNodes[0] = m_browser.InsertItem("Internal Plugins",0,0 , TVI_ROOT, TVI_LAST);
		hNodes[1] = m_browser.InsertItem("Native plug-ins",2,2,TVI_ROOT,TVI_LAST);
		hNodes[2] = m_browser.InsertItem("VST2 plug-ins",4,4,TVI_ROOT,TVI_LAST);
		intFxNode = hNodes[0];
		nodeindex=3;

		//The following is unfinished. It is for nested branches.
/*		int i=_numPlugins;	// I Search from the end because when creating the
							// array, the deepest dir is saved first.
		HTREEITEM curnode;
		int currdir = numDirs;
		while (i>=0)
		{
			if ( strcpy(_pPlugsInfo[i]->_pPlugsInfo[i]->dllname,dirArray[currdir]) != 0 )
			{
				currdir--:
				// check if you need to create a new node or what.
				// use m_browser.GetNextItem() to check where you are.
			}
			// do what it is under here, but with the correct "hNodes";

		}*/
		for (int i=_numPlugins-1; i>=0; i--)
		{
			int imgindex;
			HTREEITEM hitem;
			if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
			{
				if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
				{ 
					imgindex = 2; 
					hitem= hNodes[1]; 
				}
				else 
				{ 
					imgindex = 4; 
					hitem=hNodes[2]; 
				}
			}
			else
			{
				if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
				{ 
					imgindex = 3; 
					hitem= hNodes[1];
				}
				else 
				{ 
					imgindex = 5; 
					hitem=hNodes[2];
				}
			}
			if (pluginName)
				hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name, imgindex, imgindex, hitem, TVI_SORT);
			else
				hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname, imgindex, imgindex, hitem, TVI_SORT);

		}
		hInt[0] = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
		hInt[1] = m_browser.InsertItem("'Dist!' Distortion",1,1,intFxNode,TVI_SORT);
		hInt[2] = m_browser.InsertItem("PsychOsc AM",1,1,intFxNode,TVI_SORT);
		hInt[3] = m_browser.InsertItem("Dalay Delay",1,1,intFxNode,TVI_SORT);
		hInt[4] = m_browser.InsertItem("2p Filter",1,1,intFxNode,TVI_SORT);
		hInt[5] = m_browser.InsertItem("Gainer",1,1,intFxNode,TVI_SORT);
		hInt[6] = m_browser.InsertItem("Flanger",1,1,intFxNode,TVI_SORT);
		hInt[7] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
		m_browser.Select(hNodes[LastType0],TVGN_CARET);
	}
	else
	{
		hNodes[0] = m_browser.InsertItem("Generators",0,0 , TVI_ROOT, TVI_LAST);
		hNodes[1] = m_browser.InsertItem("Effects",1,1,TVI_ROOT,TVI_LAST);
		intFxNode = hNodes[1];
		nodeindex=2;
		for (int i=_numPlugins-1; i>=0; i--)// I Search from the end because when creating the
		{									// array, the deepest dir comes first.
			int imgindex;
			HTREEITEM hitem;
			if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
			{
				if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
				{ 
					imgindex = 2; 
					hitem= hNodes[0]; 
				}
				else 
				{ 
					imgindex = 4; 
					hitem=hNodes[0]; 
				}
			}
			else
			{
				if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
				{ 
					imgindex = 3; 
					hitem= hNodes[1]; 
				}
				else 
				{ 
					imgindex = 5; 
					hitem=hNodes[1]; 
				}
			}
			if (pluginName)
				hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name, imgindex, imgindex, hitem, TVI_SORT);
			else
				hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname, imgindex, imgindex, hitem, TVI_SORT);

		}
		hInt[0] = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
		hInt[1] = m_browser.InsertItem("'Dist!' Distortion",1,1,intFxNode,TVI_SORT);
		hInt[2] = m_browser.InsertItem("PsychOsc AM",1,1,intFxNode,TVI_SORT);
		hInt[3] = m_browser.InsertItem("Dalay Delay",1,1,intFxNode,TVI_SORT);
		hInt[4] = m_browser.InsertItem("2p Filter",1,1,intFxNode,TVI_SORT);
		hInt[5] = m_browser.InsertItem("Gainer",1,1,intFxNode,TVI_SORT);
		hInt[6] = m_browser.InsertItem("Flanger",1,1,intFxNode,TVI_SORT);
		hInt[7] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
		m_browser.Select(hNodes[LastType1],TVGN_CARET);
	}

	Outputmachine = -1;

}

void CNewMachine::OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM tHand = m_browser.GetSelectedItem();

	Outputmachine = -1;
	OutBus = false;

	if (tHand == hInt[0])
	{
		m_nameLabel.SetWindowText("Sampler");
		m_descLabel.SetWindowText("Stereo Sampler Unit. Inserts new sampler.");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V0.5b");
		Outputmachine = MACH_SAMPLER;
		OutBus = true;
		LastType0 = 0;
		LastType1 = 0;
	}
	
	// Effects
	if (tHand == hInt[1])
	{
		m_nameLabel.SetWindowText("'Dist!' Distortion");
		m_descLabel.SetWindowText("Threshold/Clamp digital distortion/booster machine");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V1.0");
		Outputmachine = MACH_DIST;
		LastType0 = 0;
		LastType1 = 1;
	}
	
	if (tHand == hInt[2])
	{
		m_nameLabel.SetWindowText("PsychOsc Theremin/AM");
		m_descLabel.SetWindowText("Amplitude Modulator/Analog Theremin Emulator");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V0.3b");
		Outputmachine = MACH_SINE;
		LastType0 = 0;
		LastType1 = 1;
	}
	
	if (tHand == hInt[3])
	{
		m_nameLabel.SetWindowText("Dalay Delay");
		m_descLabel.SetWindowText("Digital Delay Unit");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V1.1");
		Outputmachine = MACH_DELAY;
		LastType0 = 0;
		LastType1 = 1;
	}
	
	if (tHand == hInt[4])
	{
		m_nameLabel.SetWindowText("2p Filter");
		m_descLabel.SetWindowText("Digital 2-Pole Filter");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V1.1");
		Outputmachine = MACH_2PFILTER;
		LastType0 = 0;
		LastType1 = 1;
	}
	
	if (tHand == hInt[5])
	{
		m_nameLabel.SetWindowText("Gainer");
		m_descLabel.SetWindowText("Volume Gainer");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V1.0");
		Outputmachine = MACH_GAIN;
		LastType0 = 0;
		LastType1 = 1;
	}
	
	if (tHand == hInt[6])
	{
		m_nameLabel.SetWindowText("Flanger");
		m_descLabel.SetWindowText("Variable delay flanger unit");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V1.0");
		Outputmachine = MACH_FLANGER;
		LastType0 = 0;
		LastType1 = 1;
	}
	
	if (tHand == hInt[7])
	{
		m_nameLabel.SetWindowText("Dummy");
		m_descLabel.SetWindowText("Replaces inexistent plugins");
		m_dllnameLabel.SetWindowText("Internal Machine");
		m_versionLabel.SetWindowText("V1.0");
		Outputmachine = MACH_DUMMY;
		LastType0 = 0;
		LastType1 = 1;
	}

	for (int i=0; i<_numPlugins; i++)
	{
		if (tHand == hPlug[i])
		{
			CString str = _pPlugsInfo[i]->dllname;
			char str2[256];
			strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
			m_dllnameLabel.SetWindowText(str2);
			m_nameLabel.SetWindowText(_pPlugsInfo[i]->name);
			m_descLabel.SetWindowText(_pPlugsInfo[i]->desc);
			m_versionLabel.SetWindowText(_pPlugsInfo[i]->version);
			if ( _pPlugsInfo[i]->type == MACH_PLUGIN )
			{
				Outputmachine = MACH_PLUGIN;
				LastType0 = 1;
				if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
				{
					OutBus = true;
					LastType1 = 0;
				}
				else
				{
					LastType1 = 1;
				}
			}
			else
			{
				LastType0 = 2;
				if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
				{
					Outputmachine = MACH_VST;
					OutBus = true;
					LastType1 = 0;
				}
				else
				{
					Outputmachine = MACH_VSTFX;
					LastType1 = 1;
				}
			}

			if (psOutputDll != NULL) delete psOutputDll;
			psOutputDll = new char[strlen(_pPlugsInfo[i]->dllname)+1];
			strcpy(psOutputDll,_pPlugsInfo[i]->dllname);
		}
	}

	*pResult = 0;
}

void CNewMachine::OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOK();	
	*pResult = 0;
}

void CNewMachine::OnRefresh() 
{
	char cache[_MAX_PATH];
	GetModuleFileName(NULL,cache,_MAX_PATH);
	
	char *last = strrchr(cache,'\\');
	strcpy(last,"\\cache.map");

	DestroyPluginInfo();
	DeleteFile(cache);
	LoadPluginInfo();
	UpdateList();
	m_browser.Invalidate();
}

void CNewMachine::OnBytype() 
{
	pluginOrder=0;
	UpdateList();
	m_browser.Invalidate();
}
void CNewMachine::OnByclass() 
{
	pluginOrder=1;
	UpdateList();
	m_browser.Invalidate();
}

void CNewMachine::OnShowdllname() 
{
	pluginName=false;	
	UpdateList();
	m_browser.Invalidate();
}

void CNewMachine::OnShoweffname() 
{
	pluginName=true;
	UpdateList();
	m_browser.Invalidate();
}

void CNewMachine::LoadPluginInfo()
{
	if (_numPlugins == -1)
	{
		AfxGetApp()->DoWaitCursor(1); 

		int plugsCount=0;
		int badPlugsCount =0;
		_numPlugins = 0;
		_numBadPlugins = 0;
		LoadCacheFile(plugsCount,badPlugsCount);

		char logname[MAX_PATH];
		GetModuleFileName(NULL,logname,MAX_PATH);
		char *last = strrchr(logname,'\\');
		strcpy(last,"\\pluginlog.txt");

		DeleteFile(logname);
		FILE* hfile;
		hfile=fopen(logname,"wa");  
		fprintf(hfile,"[Psycle Plugin Enumeration Log]\n\nIf psycle is crashing on load, chances are it's a bad plugin, specifically the last item listed\n(if it has no comment after the -)\n\n");
		fprintf(hfile,"[Native Plugins]\n\n");
		fclose(hfile);
		FindPluginsInDir(plugsCount,badPlugsCount,CString(Global::pConfig->GetPluginDir()),MACH_PLUGIN);

		hfile=fopen(logname,"a");  
		fprintf(hfile,"\n[VST Plugins]\n\n");
		fclose(hfile);
		FindPluginsInDir(plugsCount,badPlugsCount,CString(Global::pConfig->GetVstDir()),MACH_VST);
		_numPlugins = plugsCount;
		_numBadPlugins = badPlugsCount;

		SaveCacheFile();

		AfxGetApp()->DoWaitCursor(-1); 
	}
}

void CNewMachine::FindPluginsInDir(int& currentPlugsCount,int& currentBadPlugsCount,CString findDir,MachineType type)
{
	CFileFind finder;
	VSTPlugin* vstPlug;
	Plugin* plug;

	int loop = finder.FindFile(findDir + "\\*");	// check for subfolders.
	while (loop) 
	{								
		loop = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindPluginsInDir(currentPlugsCount,currentBadPlugsCount,finder.GetFilePath(),type);
		}
	}
	finder.Close();

	char logname[MAX_PATH];
	GetModuleFileName(NULL,logname,MAX_PATH);
	char *last = strrchr(logname,'\\');
	strcpy(last,"\\pluginlog.txt");
	FILE* hfile;
	hfile=fopen(logname,"a");  
	int tmpCount = currentPlugsCount;

	loop = finder.FindFile(findDir + "\\*.dll"); // check if the directory is empty
	while (loop)
	{
		loop = finder.FindNextFile();

		CString sDllName, tmpPath;
		sDllName = finder.GetFileName();
		sDllName.MakeLower();

		// Meaning : If it is not a directory and it is not in the cache
		if (!finder.IsDirectory())
		{
			fprintf(hfile,finder.GetFilePath());
			fprintf(hfile," - ");
			fclose(hfile);
			hfile=fopen(logname,"a");  

			FILETIME time;
			finder.GetLastWriteTime(&time);
			BOOL bExist = FALSE;
			for (int i = 0; i < _numPlugins; i++)
			{
				if ((_pPlugsInfo[i]->FileTime.dwHighDateTime == time.dwHighDateTime)
					&& (_pPlugsInfo[i]->FileTime.dwLowDateTime == time.dwLowDateTime))
				{
					if (strcmp(finder.GetFilePath(),_pPlugsInfo[i]->dllname)==0)
					{
						bExist = TRUE;
						fprintf(hfile,"already mapped");
						break;
					}
				}
			}
			if (!bExist)
			{
				bExist = FALSE;
				for (int i = 0; i < _numBadPlugins; i++)
				{
					if ((_BadPluginFileTime[i].dwHighDateTime == time.dwHighDateTime)
						&& (_BadPluginFileTime[i].dwLowDateTime == time.dwLowDateTime))
					{
						if (strcmp(finder.GetFilePath(),_BadPluginPath[i])==0)
						{
							bExist = TRUE;
							fprintf(hfile,"already mapped - not plugin or bugged");
							break;
						}
					}
				}
				if (!bExist)
				{
					if (type == MACH_PLUGIN )
					{
						plug = new Plugin;
						if (plug->Instance((char*)(const char*)finder.GetFilePath()))
						{
							_pPlugsInfo[currentPlugsCount]= new PluginInfo;
							ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));
							strcpy(_pPlugsInfo[currentPlugsCount]->name,plug->GetName());
							sprintf(_pPlugsInfo[currentPlugsCount]->desc, "%s by %s",
									(plug->IsSynth()) ? "Psycle instrument" : "Psycle effect",
									plug->GetAuthor());
							strcpy(_pPlugsInfo[currentPlugsCount]->version,"PsycleAPI 1.0");
							if ( plug->IsSynth() ) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
							else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
							_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;
							_pPlugsInfo[currentPlugsCount]->dllname = new char[finder.GetFilePath().GetLength()+1];
							strcpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());

							char str2[256];
							CString str = _pPlugsInfo[currentPlugsCount]->dllname;
							str.MakeLower();
							strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
							dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

							FILETIME time;
							finder.GetLastWriteTime(&time);
							_pPlugsInfo[currentPlugsCount]->FileTime = time;

							currentPlugsCount++;
							fprintf(hfile,"*** NEW *** - ");
							fprintf(hfile,plug->GetName());
						}
						else
						{
							_BadPluginPath[currentBadPlugsCount] = new char[strlen(finder.GetFilePath())+1];
							strcpy(_BadPluginPath[currentBadPlugsCount],finder.GetFilePath());
							FILETIME time;
							finder.GetLastWriteTime(&time);
							_BadPluginFileTime[currentBadPlugsCount] = time;
							currentBadPlugsCount++;
							fprintf(hfile,"*** NOT NATIVE PLUGIN OR BUGGED ***");
						}
						delete plug;
					}
					else if (type == MACH_VST)
					{
						vstPlug = new VSTPlugin;
						if (vstPlug->Instance((char*)(const char*)finder.GetFilePath()) == VSTINSTANCE_NO_ERROR)
						{
							_pPlugsInfo[currentPlugsCount]= new PluginInfo;
							strcpy(_pPlugsInfo[currentPlugsCount]->name,vstPlug->GetName());
							sprintf(_pPlugsInfo[currentPlugsCount]->desc, "%s by %s",
								(vstPlug->IsSynth()) ? "VST2 instrument" : "VST2 effect",
								vstPlug->GetVendorName());
							sprintf(_pPlugsInfo[currentPlugsCount]->version,"%d",vstPlug->GetVersion());
							_pPlugsInfo[currentPlugsCount]->version[15]='\0';
							if ( vstPlug->IsSynth() ) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
							else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
							_pPlugsInfo[currentPlugsCount]->type = MACH_VST;
							_pPlugsInfo[currentPlugsCount]->dllname = new char[finder.GetFilePath().GetLength()+1];
							strcpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());

							char str2[256];
							CString str = _pPlugsInfo[currentPlugsCount]->dllname;
							str.MakeLower();
							strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
							dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

							FILETIME time;
							finder.GetLastWriteTime(&time);
							_pPlugsInfo[currentPlugsCount]->FileTime = time;

							currentPlugsCount++;
							fprintf(hfile,"*** NEW *** - ");
							fprintf(hfile,vstPlug->GetName());
						}
						else
						{
							_BadPluginPath[currentBadPlugsCount] = new char[strlen(finder.GetFilePath())+1];
							strcpy(_BadPluginPath[currentBadPlugsCount],finder.GetFilePath());
							FILETIME time;
							finder.GetLastWriteTime(&time);
							_BadPluginFileTime[currentBadPlugsCount] = time;
							currentBadPlugsCount++;
							fprintf(hfile,"*** NOT VST PLUGIN OR BUGGED ***");
						}
						delete vstPlug;
					}
				}
			}
			fprintf(hfile,"\n");
		}
	}
	fclose(hfile);
	finder.Close();

/*	if ( tmpCount != currentPlugsCount ) // If a plugin has been found & loaded, add this directory.
	{
		if ( _numDirs < 32 ) // Limit of dirs.
		{
			_dirArray[_numDirs] = new char[findDir.GetLength()+1];
			strcpy(_dirArray[_numDirs],findDir);
			_numDirs++;
		}
	}
*/
}

void CNewMachine::DestroyPluginInfo()
{
	for (int i=0; i<_numPlugins; i++)
	{
		delete _pPlugsInfo[i];
	}
	for (i = 0; i <_numBadPlugins; i++)
	{
		delete _BadPluginPath[i];
		_BadPluginPath[i] = 0;
	}
/*	for (i=0;i<_numDirs; i++)
	{
		delete _dirArray[i];
	}*/
	dllNames.RemoveAll();
	_numPlugins = -1;
	_numDirs=0;
}

bool CNewMachine::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount)
{
	char cache[_MAX_PATH];
	GetModuleFileName(NULL,cache,_MAX_PATH);
	
	char *last = strrchr(cache,'\\');
	strcpy(last,"\\cache.map");

	RiffFile file;
	CFileFind finder;

	if (!file.Open(cache)) 
	{
		return false;
	}

	char Temp[MAX_PATH];
	file.Read(Temp,8);
	Temp[8]=0;
	if (strcmp(Temp,"PSYCACHE"))
	{
		return false;
	}

	file.Read(&_numPlugins,sizeof(_numPlugins));

	for (int i = 0; i < _numPlugins; i++)
	{
		PluginInfo p;

		file.ReadString(Temp,sizeof(Temp));
		file.Read(&p.FileTime,sizeof(_pPlugsInfo[currentPlugsCount]->FileTime));
		file.Read(&p.mode,sizeof(p.mode));
		file.Read(&p.type,sizeof(p.type));
		file.ReadString(p.name,sizeof(p.name));
		file.ReadString(p.desc,sizeof(p.desc));
		file.ReadString(p.version,sizeof(p.version));

		if ( finder.FindFile(Temp) )
		{
			FILETIME time;
			finder.FindNextFile();
			if (finder.GetLastWriteTime(&time))
			{
				if ((p.FileTime.dwHighDateTime == time.dwHighDateTime)
					&& (p.FileTime.dwLowDateTime == time.dwLowDateTime))
				{
					_pPlugsInfo[currentPlugsCount]= new PluginInfo;

					_pPlugsInfo[currentPlugsCount]->dllname = new char[strlen(Temp)+1];
					strcpy(_pPlugsInfo[currentPlugsCount]->dllname,Temp);

					_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;
					_pPlugsInfo[currentPlugsCount]->mode = p.mode;
					_pPlugsInfo[currentPlugsCount]->type = p.type;
					strcpy(_pPlugsInfo[currentPlugsCount]->name,p.name);
					strcpy(_pPlugsInfo[currentPlugsCount]->desc,p.desc);
					strcpy(_pPlugsInfo[currentPlugsCount]->version,p.version);

					char str2[MAX_PATH];
					CString str = _pPlugsInfo[currentPlugsCount]->dllname;
					str.MakeLower();
					strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
					dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

					currentPlugsCount++;
				}
			}
		}
	}

	file.Read(&_numBadPlugins,sizeof(_numBadPlugins));
	for (i = 0; i < _numBadPlugins; i++)
	{
		file.ReadString(Temp,sizeof(Temp));
		file.Read(&_BadPluginFileTime[currentBadPlugsCount],sizeof(_BadPluginFileTime[currentBadPlugsCount]));
		if ( finder.FindFile(Temp) )
		{
			FILETIME time;
			finder.FindNextFile();
			if (finder.GetLastWriteTime(&time))
			{
				if ((_BadPluginFileTime[currentBadPlugsCount].dwHighDateTime == time.dwHighDateTime)
					&& (_BadPluginFileTime[currentBadPlugsCount].dwLowDateTime == time.dwLowDateTime))
				{
					_BadPluginPath[currentBadPlugsCount] = new char[strlen(Temp)+1];
					strcpy(_BadPluginPath[currentBadPlugsCount],Temp);
					currentBadPlugsCount++;
				}
			}
		}
	}

	file.Close();
	
	return true;
}

bool CNewMachine::SaveCacheFile()
{
	char cache[_MAX_PATH];
	GetModuleFileName(NULL,cache,_MAX_PATH);
	
	char *last = strrchr(cache,'\\');
	strcpy(last,"\\cache.map");

	RiffFile file;
	if (!file.Create(cache,true)) 
	{
		return false;
	}

	file.Write("PSYCACHE",8);
	file.Write(&_numPlugins,sizeof(_numPlugins));

	for (int i=0; i<_numPlugins; i++ )
	{
		file.Write(_pPlugsInfo[i]->dllname,strlen(_pPlugsInfo[i]->dllname)+1);
		file.Write(&_pPlugsInfo[i]->FileTime,sizeof(_pPlugsInfo[i]->FileTime));
		file.Write(&_pPlugsInfo[i]->mode,sizeof(_pPlugsInfo[i]->mode));
		file.Write(&_pPlugsInfo[i]->type,sizeof(_pPlugsInfo[i]->type));
		file.Write(_pPlugsInfo[i]->name,strlen(_pPlugsInfo[i]->name)+1);
		file.Write(_pPlugsInfo[i]->desc,strlen(_pPlugsInfo[i]->desc)+1);
		file.Write(_pPlugsInfo[i]->version,strlen(_pPlugsInfo[i]->version)+1);
	}

	file.Write(&_numBadPlugins,sizeof(_numBadPlugins));
	for (i=0; i<_numBadPlugins; i++ )
	{
		file.Write(_BadPluginPath[i],strlen(_BadPluginPath[i])+1);
		file.Write(&_BadPluginFileTime[i],sizeof(_BadPluginFileTime[i]));
	}

	file.Close();
	return true;
}

void CNewMachine::OnOK() 
{
	if (Outputmachine > -1) // Necessary so that you cannot doubleclick a Node
	{
		CDialog::OnOK();
	}
}
