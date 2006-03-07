/** @file
 *  @brief implementation file 
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.6 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#include "cstdlib"
#include "crtdbg.h"
#define _CRTDBG_MAP_ALLOC
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//#include "psycle2.h"
#include "Plugin.h"
#include "VstHost.h"
#include "ProgressDialog.h"
#include ".\newmachine.h"



/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
int CNewMachine::pluginOrder = 1;
bool CNewMachine::pluginName = 1;
int CNewMachine::_numPlugins = -1;
int CNewMachine::LastType0=0;
int CNewMachine::LastType1=0;

PluginInfo* CNewMachine::_pPlugsInfo[MAX_BROWSER_PLUGINS];

CMapStringToString CNewMachine::dllNames;//MAX_BROWSER_PLUGINS);


/////////////////////////////////////////////////////////////////////////////
// CNewMachine dialog

CNewMachine::CNewMachine()
{
	m_orderby = pluginOrder;
	m_showdllName = pluginName;
	OutBus = false;
	psOutputDll = NULL;
}

CNewMachine::~CNewMachine()
{
	if (psOutputDll != NULL)
	{
		delete psOutputDll;
	}
}
/*
void CNewMachine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMachine)
	DDX_Control(pDX, IDC_CHECK_ALLOW, m_Allow);
	DDX_Control(pDX, IDC_NAMELABEL, m_nameLabel);
	DDX_Control(pDX, IDC_BROWSER, m_browser);
	DDX_Control(pDX, IDC_VERSIONLABEL, m_versionLabel);
	DDX_Control(pDX, IDC_DESCLABEL, m_descLabel);
	DDX_Radio(pDX, IDC_BYTYPE, m_orderby);
	DDX_Control(pDX, IDC_DLLNAMELABEL, m_dllnameLabel);
	DDX_Radio(pDX, IDC_SHOWDLLNAME, m_showdllName);
	//}}AFX_DATA_MAP
}
*/


void CNewMachine::UpdateList(bool bInit)
{
	int nodeindex;

	m_browser.DeleteAllItems();

	HTREEITEM intFxNode;

	if (pluginOrder == 0 )
	{
		hNodes[0] = m_browser.InsertItem(SF::CResourceString(IDS_MSG0082),0,0 , TVI_ROOT, TVI_LAST);
		hNodes[1] = m_browser.InsertItem(SF::CResourceString(IDS_MSG0083),2,2,TVI_ROOT,TVI_LAST);
		hNodes[2] = m_browser.InsertItem(SF::CResourceString(IDS_MSG0084),4,4,TVI_ROOT,TVI_LAST);
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
			// do what it is under here, but with the correct _T("hNodes");

		}*/
		for (int i=_numPlugins-1; i>=0; i--)
		{
			if (_pPlugsInfo[i]->error_type == VSTINSTANCE_NO_ERROR)
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
		}
		hInt[0] = m_browser.InsertItem(_T("Sampler"),0, 0, hNodes[0], TVI_SORT);
		hInt[2] = m_browser.InsertItem(_T("XMSampler"),0, 0, hNodes[0], TVI_SORT);

//		hInt[1] = m_browser.InsertItem(_T("'Dist!' Distortion"),1,1,intFxNode,TVI_SORT);
//		hInt[2] = m_browser.InsertItem(_T("PsychOsc AM"),1,1,intFxNode,TVI_SORT);
//		hInt[3] = m_browser.InsertItem(_T("Dalay Delay"),1,1,intFxNode,TVI_SORT);
//		hInt[4] = m_browser.InsertItem(_T("2p Filter"),1,1,intFxNode,TVI_SORT);
//		hInt[5] = m_browser.InsertItem(_T("Gainer"),1,1,intFxNode,TVI_SORT);
//		hInt[6] = m_browser.InsertItem(_T("Flanger"),1,1,intFxNode,TVI_SORT);
//		hInt[7] = m_browser.InsertItem(_T("Dummy plug"),1,1,intFxNode,TVI_SORT);
		hInt[1] = m_browser.InsertItem(SF::CResourceString(IDS_MSG0085),1,1,intFxNode,TVI_SORT);
		m_browser.Select(hNodes[LastType0],TVGN_CARET);
	}
	else
	{
		hNodes[0] = m_browser.InsertItem(_T("Generators"),0,0 , TVI_ROOT, TVI_LAST);
		hNodes[1] = m_browser.InsertItem(_T("Effects"),1,1,TVI_ROOT,TVI_LAST);
		intFxNode = hNodes[1];
		nodeindex=2;
		for (int i=_numPlugins-1; i>=0; i--)// I Search from the end because when creating the
		{									// array, the deepest dir comes first.
			if (_pPlugsInfo[i]->error_type == VSTINSTANCE_NO_ERROR)
			{
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

		}
		hInt[0] = m_browser.InsertItem(_T("Sampler"),0, 0, hNodes[0], TVI_SORT);
//		hInt[1] = m_browser.InsertItem(_T("'Dist!' Distortion"),1,1,intFxNode,TVI_SORT);
//		hInt[2] = m_browser.InsertItem(_T("PsychOsc AM"),1,1,intFxNode,TVI_SORT);
//		hInt[3] = m_browser.InsertItem(_T("Dalay Delay"),1,1,intFxNode,TVI_SORT);
//		hInt[4] = m_browser.InsertItem(_T("2p Filter"),1,1,intFxNode,TVI_SORT);
//		hInt[5] = m_browser.InsertItem(_T("Gainer"),1,1,intFxNode,TVI_SORT);
//		hInt[6] = m_browser.InsertItem(_T("Flanger"),1,1,intFxNode,TVI_SORT);
//		hInt[7] = m_browser.InsertItem(_T("Dummy plug"),1,1,intFxNode,TVI_SORT);
		hInt[1] = m_browser.InsertItem(SF::CResourceString(IDS_MSG0085),1,1,intFxNode,TVI_SORT);
		hInt[2] = m_browser.InsertItem(_T("XMSampler"),1,1,hNodes[0],TVI_SORT);
		m_browser.Select(hNodes[LastType1],TVGN_CARET);
	}

	Outputmachine = -1;

}




void CNewMachine::LoadPluginInfo()
{
	if (_numPlugins == -1)
	{

		HCURSOR oldcursor = SetCursor(LoadCursor(NULL,IDC_WAIT)); 

		int plugsCount=0;
		int badPlugsCount =0;
		_numPlugins = 0;
		BOOL bProgressOpen = !LoadCacheFile(plugsCount,badPlugsCount);
		CProgressDialog Progress;

		if (bProgressOpen)
		{
			Progress.Create(NULL);
			Progress.SetWindowText(SF::CResourceString(IDS_MSG0086));
			Progress.ShowWindow(SW_SHOW);
		}

		TCHAR logname[MAX_PATH];
		GetModuleFileName(NULL,logname,MAX_PATH);
		TCHAR *last = _tcsrchr(logname,_T('\\'));
		_tcscpy(last,_T("\\pluginlog.txt"));

		DeleteFile(logname);
		FILE* hfile;
		hfile=_tfopen(logname,_T("a"));  
		_ftprintf(hfile,_T("[Psycle Plugin Enumeration Log]\n\nIf psycle is crashing on load, chances are it's a bad plugin, specifically the last item listed\n(if it has no comment after the -)\n\n"));
		_ftprintf(hfile,_T("[Native Plugins]\n\n"));
		fclose(hfile);
		if (bProgressOpen)
		{
			FindPluginsInDir(plugsCount,badPlugsCount,CString(Global::pConfig->GetPluginDir()),MACH_PLUGIN,&Progress);
		}
		else
		{
			FindPluginsInDir(plugsCount,badPlugsCount,CString(Global::pConfig->GetPluginDir()),MACH_PLUGIN);
		}

		if (bProgressOpen)
		{
			Progress.m_Progress.SetPos(8192);
		}

		hfile=_tfopen(logname,_T("a"));  
		_ftprintf(hfile,_T("\n[VST Plugins]\n\n"));
		fclose(hfile);
		if (bProgressOpen)
		{
			FindPluginsInDir(plugsCount,badPlugsCount,CString(Global::pConfig->GetVstDir()),MACH_VST,&Progress);
		}
		else
		{
			FindPluginsInDir(plugsCount,badPlugsCount,CString(Global::pConfig->GetVstDir()),MACH_VST);
		}
		_numPlugins = plugsCount;

		if (bProgressOpen)
		{
			Progress.m_Progress.SetPos(16384);
		}

		SaveCacheFile();

		if (bProgressOpen)
		{
			Progress.OnCancel();
		}

		SetCursor(oldcursor); 
	}
}

void CNewMachine::FindPluginsInDir(int& currentPlugsCount,int& currentBadPlugsCount,CString findDir,MachineType type,CProgressDialog * pProgress)
{
	WTL::CFindFile finder;
	int loop = finder.FindFile(findDir + _T("\\*"));	// check for subfolders.
	while (loop) 
	{								
		if (pProgress)
		{
			pProgress->m_Progress.SetStep(16);
			pProgress->m_Progress.StepIt();
			::Sleep(1);
		}
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindPluginsInDir(currentPlugsCount,currentBadPlugsCount,finder.GetFilePath(),type);
		}
		loop = finder.FindNextFile();
	}
	finder.Close();

	TCHAR logname[MAX_PATH];
	GetModuleFileName(NULL,logname,MAX_PATH);
	TCHAR *last = _tcsrchr(logname,_T('\\'));
	_tcscpy(last,_T("\\pluginlog.txt"));
	FILE* hfile;
	hfile = _tfopen(logname,_T("a"));  
	int tmpCount = currentPlugsCount;

	loop = finder.FindFile(findDir + _T("\\*.dll")); // check if the directory is empty
	while (loop)
	{
		if (pProgress)
		{
			pProgress->m_Progress.SetStep(32);
			pProgress->m_Progress.StepIt();
			::Sleep(1);
		}

		CString sDllName, tmpPath;
		sDllName = finder.GetFileName();
		sDllName.MakeLower();
		// Meaning : If it is not a directory and it is not in the cache
		if (!finder.IsDirectory())
		{
			_ftprintf(hfile,finder.GetFilePath());
			_ftprintf(hfile,_T(" - "));
			fclose(hfile);
			hfile = _tfopen(logname,_T("a"));  

			FILETIME time;
			finder.GetLastWriteTime(&time);
			BOOL bExist = FALSE;
			for (int i = 0; i < _numPlugins; i++)
			{
				if (_pPlugsInfo[i])
				{
					if ((_pPlugsInfo[i]->FileTime.dwHighDateTime == time.dwHighDateTime)
						&& (_pPlugsInfo[i]->FileTime.dwLowDateTime == time.dwLowDateTime))
					{
						if (_tcscmp(finder.GetFilePath(),_pPlugsInfo[i]->dllname)==0)
						{
							bExist = TRUE;
							switch (_pPlugsInfo[i]->error_type)
							{
							case VSTINSTANCE_ERR_NO_VALID_FILE:
								_ftprintf(hfile,_T("* Previously Disabled - No File Error *"));
								break;
							case VSTINSTANCE_ERR_NO_VST_PLUGIN:
								_ftprintf(hfile,_T("* Previously Disabled - Not Plugin Error *"));
								break;
							case VSTINSTANCE_ERR_REJECTED:
								_ftprintf(hfile,_T("* Previously Disabled - Rejected Error *"));
								break;
							case VSTINSTANCE_ERR_EXCEPTION:
								_ftprintf(hfile,_T("* Previously Disabled - Exception Error *"));
								break;
							case VSTINSTANCE_NO_ERROR:
								_ftprintf(hfile,_T("* Previously Mapped *"));
								break;
							default:
								_ftprintf(hfile,_T("* Disabled - Unknown Reason *"));
								break;
							}
							break;
						}
					}
				}
			}
			if (!bExist)
			{
				if (type == MACH_PLUGIN )
				{
					Plugin plug(0);

					_pPlugsInfo[currentPlugsCount]= new PluginInfo();
					ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

					_pPlugsInfo[currentPlugsCount]->dllname = new TCHAR[finder.GetFilePath().GetLength()+1];
					_tcscpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());

					FILETIME time;
					finder.GetLastWriteTime(&time);
					_pPlugsInfo[currentPlugsCount]->FileTime = time;

					_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;

					if (plug.Instance(finder.GetFilePath().GetBuffer()))
					{
						_pPlugsInfo[currentPlugsCount]->error_type = VSTINSTANCE_NO_ERROR;
						_pPlugsInfo[currentPlugsCount]->allow = true;

						_tcscpy(_pPlugsInfo[currentPlugsCount]->name,plug.GetName());
						_stprintf(_pPlugsInfo[currentPlugsCount]->desc, _T("%s by %s"),
								(plug.IsSynth()) ? _T("Psycle instrument") : _T("Psycle effect"),
								plug.GetAuthor());
						_tcscpy(_pPlugsInfo[currentPlugsCount]->version,_T("PsycleAPI 1.0"));

						if ( plug.IsSynth() ) 
						{
							_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
						}
						else 
						{
							_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
						}

						TCHAR str2[256];
						CString str = _pPlugsInfo[currentPlugsCount]->dllname;
						str.MakeLower();
						_tcscpy(str2,str.Mid(str.ReverseFind(_T('\\'))+1));
						dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

						_ftprintf(hfile,_T("*** NEW *** - "));
						_ftprintf(hfile,plug.GetName());
					}
					else
					{
						_pPlugsInfo[currentPlugsCount]->error_type = VSTINSTANCE_ERR_NO_VST_PLUGIN;
						_pPlugsInfo[currentPlugsCount]->allow = false;

						_stprintf(_pPlugsInfo[currentPlugsCount]->name,_T("???"));
						_stprintf(_pPlugsInfo[currentPlugsCount]->desc,_T("???"));
						_stprintf(_pPlugsInfo[currentPlugsCount]->version,_T("???"));
						_ftprintf(hfile,_T("*** ERROR - NOT NATIVE PLUGIN OR BUGGED ***"));
					}
					currentPlugsCount++;
				}
				else if (type == MACH_VST)
				{
					VSTPlugin vstPlug;

					_pPlugsInfo[currentPlugsCount]= new PluginInfo;
					ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

					_pPlugsInfo[currentPlugsCount]->dllname = new TCHAR[finder.GetFilePath().GetLength()+1];
					_tcscpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());

					FILETIME time;
					finder.GetLastWriteTime(&time);
					_pPlugsInfo[currentPlugsCount]->FileTime = time;

					_pPlugsInfo[currentPlugsCount]->type = MACH_VST;

					_pPlugsInfo[currentPlugsCount]->error_type = vstPlug.Instance(finder.GetFilePath().GetBuffer());

					if (_pPlugsInfo[currentPlugsCount]->error_type == VSTINSTANCE_NO_ERROR)
					{
						_pPlugsInfo[currentPlugsCount]->allow = true;

						_tcscpy(_pPlugsInfo[currentPlugsCount]->name,vstPlug.GetName());
						_stprintf(_pPlugsInfo[currentPlugsCount]->desc, _T("%s by %s"),
							(vstPlug.IsSynth()) ? _T("VST2 instrument") : _T("VST2 effect"),
							vstPlug.GetVendorName());
						_stprintf(_pPlugsInfo[currentPlugsCount]->version,_T("%d"),vstPlug.GetVersion());
						_pPlugsInfo[currentPlugsCount]->version[15]=_T('\0');
						if ( vstPlug.IsSynth() ) 
						{
							_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
						}
						else 
						{
							_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
						}

						TCHAR str2[256];
						CString str = _pPlugsInfo[currentPlugsCount]->dllname;
						str.MakeLower();
						_tcscpy(str2,str.Mid(str.ReverseFind(_T('\\'))+1));
						dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

						_ftprintf(hfile,_T("*** NEW *** - "));
						_ftprintf(hfile,vstPlug.GetName());
					}
					else
					{
						_pPlugsInfo[currentPlugsCount]->allow = false;

						_stprintf(_pPlugsInfo[currentPlugsCount]->name,_T("???"));
						_stprintf(_pPlugsInfo[currentPlugsCount]->desc,_T("???"));
						_stprintf(_pPlugsInfo[currentPlugsCount]->version,_T("???"));

						switch (_pPlugsInfo[currentPlugsCount]->error_type)
						{
						case VSTINSTANCE_ERR_NO_VALID_FILE:
							_ftprintf(hfile,_T("*** ERROR - NO FILE ***"));
							break;
						case VSTINSTANCE_ERR_NO_VST_PLUGIN:
							_ftprintf(hfile,_T("*** ERROR - NOT VST PLUGIN ***"));
							break;
						case VSTINSTANCE_ERR_REJECTED:
							_ftprintf(hfile,_T("*** ERROR - REJECTED ***"));
							break;
						case VSTINSTANCE_ERR_EXCEPTION:
							_ftprintf(hfile,_T("*** ERROR - EXCEPTION ***"));
							break;
						default:
							_ftprintf(hfile,_T("*** ERROR - NOT VST PLUGIN OR BUGGED ***"));
							break;
						}
					}
					currentPlugsCount++;
				}
			}
			_ftprintf(hfile,_T("\n"));
		}
		loop = finder.FindNextFile();
	}
	fclose(hfile);
	finder.Close();
}

void CNewMachine::DestroyPluginInfo()
{
	for (int i=0; i<_numPlugins; i++)
	{
		delete _pPlugsInfo[i];
	}
	dllNames.RemoveAll();
	_numPlugins = -1;
}

bool CNewMachine::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount)
{
	TCHAR cache[_MAX_PATH];
	GetModuleFileName(NULL,cache,_MAX_PATH);
	
	TCHAR *last = _tcsrchr(cache,_T('\\'));
	_tcscpy(last,_T("\\cache.map"));

	RiffFile file;
	CFindFile finder;

	if (!file.Open(cache)) 
	{
		return false;
	}

	TCHAR Temp[MAX_PATH];
	file.Read(Temp,8 * sizeof(TCHAR));
	Temp[8]=0;
	if (_tcscmp(Temp,_T("PSYCACHE"))!=0)
	{
		file.Close();
		DeleteFile(cache);
		return false;
	}

	UINT version;
	file.Read(&version,sizeof(version));
	if (version > CURRENT_CACHE_MAP_VERSION)
	{
		file.Close();
		DeleteFile(cache);
		return false;
	}
	file.Read(&_numPlugins,sizeof(_numPlugins));

	for (int i = 0; i < _numPlugins; i++)
	{
		PluginInfo p;

		file.ReadString(Temp,sizeof(Temp) / sizeof(TCHAR));
		file.Read(&p.FileTime,sizeof(_pPlugsInfo[currentPlugsCount]->FileTime));
		file.Read(&p.error_type,sizeof(p.error_type));
		file.Read(&p.allow,sizeof(p.allow));
		file.Read(&p.mode,sizeof(p.mode));
		file.Read(&p.type,sizeof(p.type));
		file.ReadString(p.name,sizeof(p.name) / sizeof(TCHAR));
		file.ReadString(p.desc,sizeof(p.desc) / sizeof(TCHAR));
		file.ReadString(p.version,sizeof(p.version) / sizeof(TCHAR));

		if ( finder.FindFile(Temp) )
		{
			FILETIME time;
			if (finder.GetLastWriteTime(&time))
			{
				if ((p.FileTime.dwHighDateTime == time.dwHighDateTime)
					&& (p.FileTime.dwLowDateTime == time.dwLowDateTime))
				{
					_pPlugsInfo[currentPlugsCount]= new PluginInfo;
					ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

					_pPlugsInfo[currentPlugsCount]->dllname = new TCHAR[_tcslen(Temp)+1];
					_tcscpy(_pPlugsInfo[currentPlugsCount]->dllname,Temp);

					_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;

					_pPlugsInfo[currentPlugsCount]->error_type = p.error_type;
					_pPlugsInfo[currentPlugsCount]->allow = p.allow;

					_pPlugsInfo[currentPlugsCount]->mode = p.mode;
					_pPlugsInfo[currentPlugsCount]->type = p.type;
					_tcscpy(_pPlugsInfo[currentPlugsCount]->name,p.name);
					_tcscpy(_pPlugsInfo[currentPlugsCount]->desc,p.desc);
					_tcscpy(_pPlugsInfo[currentPlugsCount]->version,p.version);

					if (p.error_type == VSTINSTANCE_NO_ERROR)
					{
						TCHAR str2[MAX_PATH];
						CString str = _pPlugsInfo[currentPlugsCount]->dllname;
						str.MakeLower();
						_tcscpy(str2,str.Mid(str.ReverseFind(_T('\\'))+1));
						dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);
					}

					currentPlugsCount++;
				}
			}
			finder.FindNextFile();

		}
	}

	file.Close();
	
	return true;
}

bool CNewMachine::SaveCacheFile()
{
	TCHAR cache[_MAX_PATH];
	GetModuleFileName(NULL,cache,_MAX_PATH);
	
	TCHAR *last = _tcsrchr(cache,_T('\\'));
	_tcscpy(last,_T("\\cache.map"));
	DeleteFile(cache);

	RiffFile file;
	if (!file.Create(cache,true)) 
	{
		return false;
	}

	file.Write(_T("PSYCACHE"),8 * sizeof(TCHAR));
	UINT version = CURRENT_CACHE_MAP_VERSION;
	file.Write(&version,sizeof(version));
	file.Write(&_numPlugins,sizeof(_numPlugins));

	for (int i = 0; i < _numPlugins; i++ )
	{
		file.Write(_pPlugsInfo[i]->dllname,(_tcslen(_pPlugsInfo[i]->dllname) + 1) * sizeof(TCHAR));
		file.Write(&_pPlugsInfo[i]->FileTime,sizeof(_pPlugsInfo[i]->FileTime));
		file.Write(&_pPlugsInfo[i]->error_type,sizeof(_pPlugsInfo[i]->error_type));
		file.Write(&_pPlugsInfo[i]->allow,sizeof(_pPlugsInfo[i]->allow));
		file.Write(&_pPlugsInfo[i]->mode,sizeof(_pPlugsInfo[i]->mode));
		file.Write(&_pPlugsInfo[i]->type,sizeof(_pPlugsInfo[i]->type));
		file.Write(_pPlugsInfo[i]->name,(_tcslen(_pPlugsInfo[i]->name) + 1) * sizeof(TCHAR));
		file.Write(_pPlugsInfo[i]->desc,(_tcslen(_pPlugsInfo[i]->desc) + 1) * sizeof(TCHAR));
		file.Write(_pPlugsInfo[i]->version,(_tcslen(_pPlugsInfo[i]->version) + 1) * sizeof(TCHAR));
	}

	file.Close();
	return true;
}

bool CNewMachine::TestFilename(TCHAR* name)
{
	for (int i=0; i<_numPlugins; i++)
	{
		if (_tcscmp(name,_pPlugsInfo[i]->dllname)==0)
		{
			// bad plugins always have allow = false
			if (_pPlugsInfo[i]->allow)
			{
				return TRUE;
			}
			TCHAR buf[256];
			switch (_pPlugsInfo[i]->error_type)
			{
			case VSTINSTANCE_ERR_NO_VALID_FILE:
				_stprintf(buf,_T("%s is Disabled - No File Error.  Try to Load Anyway?"),name);
				break;
			case VSTINSTANCE_ERR_NO_VST_PLUGIN:
				_stprintf(buf,_T("%s is Disabled - Not Plugin Error.  Try to Load Anyway?"),name);
				break;
			case VSTINSTANCE_ERR_REJECTED:
				_stprintf(buf,_T("%s is Disabled - Rejected Error  Try to Load Anyway?"),name);
				break;
			case VSTINSTANCE_ERR_EXCEPTION:
				_stprintf(buf,_T("%s is Disabled - Exception Error  Try to Load Anyway?"),name);
				break;
			case VSTINSTANCE_NO_ERROR:
				_stprintf(buf,_T("%s is Disabled by User.  Try to Load Anyway?"),name);
				break;
			default:
				_stprintf(buf,_T("%s is Disabled - Unknown Reason  Try to Load Anyway?"),name);
				break;
			}
			return (::MessageBox(NULL,buf,SF::CResourceString(IDS_ERR_MSG0112),MB_YESNO) == IDYES);

		}
	}
	return FALSE;
}

LRESULT CNewMachine::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DoDataExchange(FALSE);

	m_Allow.Attach(GetDlgItem(IDC_CHECK_ALLOW));
	m_nameLabel.Attach(GetDlgItem(IDC_NAMELABEL));
	m_browser.Attach(GetDlgItem(IDC_BROWSER));
	m_versionLabel.Attach(GetDlgItem(IDC_VERSIONLABEL));
	m_descLabel.Attach(GetDlgItem(IDC_DESCLABEL));
	m_dllnameLabel.Attach(GetDlgItem(IDC_DLLNAMELABEL));

	if ((HIMAGELIST)imgList != 0)
	{
		imgList.RemoveAll();
 	}
	imgList.Create(IDB_MACHINETYPE,16,2,1);
	m_browser.SetImageList((HIMAGELIST)imgList,TVSIL_NORMAL);

	bAllowChanged = FALSE;

	LoadPluginInfo();
	CenterWindow();
	UpdateList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CNewMachine::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	DoDataExchange(TRUE);
	if (Outputmachine > -1) // Necessary so that you cannot doubleclick a Node
	{
		if (bAllowChanged)
		{
			SaveCacheFile();
		}
	}
	bHandled = FALSE;
	EndDialog(wID);
	return 0;
}

LRESULT CNewMachine::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	bHandled = FALSE;
	EndDialog(wID);
	return 0;
}

LRESULT CNewMachine::OnBnClickedShowdllname(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pluginName=false;	
	UpdateList();
	m_browser.Invalidate();
	return 0;
}

LRESULT CNewMachine::OnBnClickedShoweffname(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pluginName=true;
	UpdateList();
	m_browser.Invalidate();
	return 0;
}

LRESULT CNewMachine::OnTvnSelchangedBrowser(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	//NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	tHand = m_browser.GetSelectedItem();

	Outputmachine = -1;
	OutBus = false;

	if (tHand == hInt[0])
	{
		m_nameLabel.SetWindowText(_T("Sampler"));
		m_descLabel.SetWindowText(SF::CResourceString(IDS_MSG0087));
		m_dllnameLabel.SetWindowText(SF::CResourceString(IDS_MSG0088));
		m_versionLabel.SetWindowText(_T("V0.5b"));
		Outputmachine = MACH_SAMPLER;
		OutBus = true;
		LastType0 = 0;
		LastType1 = 0;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	
	
	// XMSampler
	if (tHand == hInt[2])
	{
		m_nameLabel.SetWindowText(_T("XMSampler"));
		m_descLabel.SetWindowText(SF::CResourceString(IDS_MSG0089));
		m_dllnameLabel.SetWindowText(_T("Internal Machine"));
		m_versionLabel.SetWindowText(_T("V0.1"));
		Outputmachine = MACH_XMSAMPLER;
		OutBus = true;
		LastType0 = 0;
		LastType1 = 0;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	/*
	if (tHand == hInt[2])
	{
		m_nameLabel.SetWindowText(_T("PsychOsc Theremin/AM"));
		m_descLabel.SetWindowText(_T("Amplitude Modulator/Analog Theremin Emulator"));
		m_dllnameLabel.SetWindowText(_T("Internal Machine"));
		m_versionLabel.SetWindowText(_T("V0.3b"));
		Outputmachine = MACH_SINE;
		LastType0 = 0;
		LastType1 = 1;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	
	if (tHand == hInt[3])
	{
		m_nameLabel.SetWindowText(_T("Dalay Delay"));
		m_descLabel.SetWindowText(_T("Digital Delay Unit"));
		m_dllnameLabel.SetWindowText(_T("Internal Machine"));
		m_versionLabel.SetWindowText(_T("V1.1"));
		Outputmachine = MACH_DELAY;
		LastType0 = 0;
		LastType1 = 1;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	
	if (tHand == hInt[4])
	{
		m_nameLabel.SetWindowText(_T("2p Filter"));
		m_descLabel.SetWindowText(_T("Digital 2-Pole Filter"));
		m_dllnameLabel.SetWindowText(_T("Internal Machine"));
		m_versionLabel.SetWindowText(_T("V1.1"));
		Outputmachine = MACH_2PFILTER;
		LastType0 = 0;
		LastType1 = 1;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	
	if (tHand == hInt[5])
	{
		m_nameLabel.SetWindowText(_T("Gainer"));
		m_descLabel.SetWindowText(_T("Volume Gainer"));
		m_dllnameLabel.SetWindowText(_T("Internal Machine"));
		m_versionLabel.SetWindowText(_T("V1.0"));
		Outputmachine = MACH_GAIN;
		LastType0 = 0;
		LastType1 = 1;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	
	if (tHand == hInt[6])
	{
		m_nameLabel.SetWindowText(_T("Flanger"));
		m_descLabel.SetWindowText(_T("Variable delay flanger unit"));
		m_dllnameLabel.SetWindowText(_T("Internal Machine"));
		m_versionLabel.SetWindowText(_T("V1.0"));
		Outputmachine = MACH_FLANGER;
		LastType0 = 0;
		LastType1 = 1;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}
	*/
	
	if (tHand == hInt[1])
	{
		m_nameLabel.SetWindowText(SF::CResourceString(IDS_MSG0090));
		m_descLabel.SetWindowText(SF::CResourceString(IDS_MSG0091));
		m_dllnameLabel.SetWindowText(SF::CResourceString(IDS_MSG0088));
		m_versionLabel.SetWindowText(_T("V1.0"));
		Outputmachine = MACH_DUMMY;
		LastType0 = 0;
		LastType1 = 1;
		m_Allow.SetCheck(FALSE);
		m_Allow.EnableWindow(FALSE);
	}

	for (int i=0; i<_numPlugins; i++)
	{
		if (tHand == hPlug[i])
		{
			CString str = _pPlugsInfo[i]->dllname;
			TCHAR str2[256];
			_tcscpy(str2,str.Mid(str.ReverseFind(_T('\\'))+1));
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
			psOutputDll = new TCHAR[_tcslen(_pPlugsInfo[i]->dllname) + 1];
			_tcscpy(psOutputDll,_pPlugsInfo[i]->dllname);

			m_Allow.SetCheck(!_pPlugsInfo[i]->allow);
			m_Allow.EnableWindow(TRUE);
		}
	}

	return 0;
}

LRESULT CNewMachine::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ((HIMAGELIST)imgList != 0)	{
		imgList.RemoveAll();
	}
	return 0;
}

LRESULT CNewMachine::OnBnClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR cache[_MAX_PATH];
	GetModuleFileName(NULL,cache,_MAX_PATH);
	
	TCHAR *last = _tcsrchr(cache,_T('\\'));
	_tcscpy(last,_T("\\cache.map"));

	DestroyPluginInfo();
	DeleteFile(cache);
	LoadPluginInfo();
	UpdateList();
	m_browser.Invalidate();
	SetFocus();

	return 0;
}

LRESULT CNewMachine::OnBnClickedBytype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pluginOrder=0;
	UpdateList();
	m_browser.Invalidate();
	return 0;
}

LRESULT CNewMachine::OnBnClickedByclass(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pluginOrder=1;
	UpdateList();
	m_browser.Invalidate();

	return 0;
}

LRESULT CNewMachine::OnBnClickedCheckAllow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	for (int i=0; i<_numPlugins; i++)
	{
		if (tHand == hPlug[i])
		{
			_pPlugsInfo[i]->allow = !m_Allow.GetCheck();
			bAllowChanged = TRUE;
		}
	}
	return 0;
}

LRESULT CNewMachine::OnNMDblclkBrowser(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	DoDataExchange(TRUE);
	if (Outputmachine > -1) // Necessary so that you cannot doubleclick a Node
	{
		if (bAllowChanged)
		{
			SaveCacheFile();
		}
	}
	bHandled = FALSE;
	EndDialog(IDOK);
	return 0;
}
