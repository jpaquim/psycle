// PresetsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "PresetsDlg.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "FrameMachine.h"
#include "FileIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// CPreset Class

// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPreset::CPreset()
{
	params=NULL;
	data=NULL;
	numPars = -1;
	sizeData = 0;
	strcpy(name,"");
}

CPreset::~CPreset()
{
	if ( params != NULL ) delete params;
}

void CPreset::Clear()
{
	if ( params != NULL ) 
	{
		delete params;
		params=NULL;
	}
	numPars =-1;
	data=NULL;
	sizeData = 0;
	strcpy(name,"");

}

void CPreset::Init(int num)
{
	if ( num > 0 )
	{
		if ( params != NULL ) delete params;
		params= new int[num];
		numPars=num;
		data=NULL;
		sizeData = 0;
		strcpy(name,"");
	}
}

void CPreset::Init(int num,char* newname,int* parameters,int size, byte* newdata)
{
	if ( num > 0 )
	{
		if ( params != NULL ) delete params;
		params= new int[num];
		numPars=num;
		memcpy(params,parameters,numPars*sizeof(int));
		strcpy(name,newname);
		if (data) delete data;
		if (sizeData = size)
		{
			data= new byte[sizeData];
			memcpy(data,newdata,size);
		}
		else
		{
			data = NULL;
		}
	}
}

void CPreset::Init(char* newname)
{
	params = NULL;
	numPars = -1;
	data=NULL;
	sizeData = 0;
	strcpy(name,newname);
}

void CPreset::Init(int num,char* newname,float* parameters)
{
	if ( num > 0 )
	{
		if ( params != NULL ) delete params;
		params= new int[num];
		numPars=num;
		data=NULL;
		sizeData = 0;
		for(int x=0;x<num;x++) params[x]= (int)(parameters[x]*65535.0f);
		strcpy(name,newname);
	}
}

void CPreset::operator=(CPreset& newpreset)
{
	if ( params != NULL) delete params;
	numPars=newpreset.numPars;
	params= new int[numPars];
	memcpy(params,newpreset.params,numPars*sizeof(int));
	strcpy(name,newpreset.name);
	sizeData = newpreset.sizeData;
	if (data) delete data;
	if (sizeData)
	{
		data= new byte[sizeData];
		memcpy(data,newpreset.data,sizeData);
	}
	else
	{
		data=NULL;
	}
}

int CPreset::GetParam(int n)
{
	if (( numPars != -1 ) && ( n < numPars )) return params[n];
	return -1;
}

void CPreset::SetParam(int n,int val)
{
	if (( numPars != -1 ) && ( n < numPars )) params[n] = val;
}




/////////////////////////////////////////////////////////////////////////////
// CPresetsDlg dialog


CPresetsDlg::CPresetsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPresetsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPresetsDlg)
	//}}AFX_DATA_INIT
}


void CPresetsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPresetsDlg)
	DDX_Control(pDX, IDC_EXPORT, m_exportButton);
	DDX_Control(pDX, IDC_PREVIEW, m_preview);
	DDX_Control(pDX, IDC_PRESETSLIST, m_preslist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPresetsDlg, CDialog)
	//{{AFX_MSG_MAP(CPresetsDlg)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_CBN_SELCHANGE(IDC_PRESETSLIST, OnSelchangePresetslist)
	ON_CBN_DBLCLK(IDC_PRESETSLIST, OnDblclkPresetslist)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPresetsDlg message handlers

BOOL CPresetsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_preslist.LimitText(32);

	presetChanged=false;
	CString buffer;
	if ( _pMachine->_type == MACH_PLUGIN )
	{
		numParameters = ((Plugin*)_pMachine)->GetInfo()->numParameters;
		sizeDataStruct = ((Plugin *)_pMachine)->GetInterface()->GetDataSize();
		if (sizeDataStruct)
		{
			byte* pData = new byte[sizeDataStruct];
			((Plugin *)_pMachine)->GetInterface()->GetData(pData); // Internal save
			iniPreset.Init(numParameters , "", ((Plugin*)_pMachine)->GetInterface()->Vals, sizeDataStruct,  pData);
			delete pData;
		}
		else
		{
			iniPreset.Init(numParameters , "", ((Plugin*)_pMachine)->GetInterface()->Vals, sizeDataStruct,  NULL);
		}

		buffer = ((Plugin *)_pMachine)->GetDllName();
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		numParameters = ((VSTPlugin*)_pMachine)->NumParameters();
		iniPreset.Init(numParameters);
		int i=0;
		while (i < numParameters)
		{
			iniPreset.SetParam(i,(int)(((VSTPlugin*)_pMachine)->GetParameter(i)*65535));
			i++;
		}

		buffer = ((VSTPlugin *)_pMachine)->GetDllName();
	}

	buffer = buffer.Left(buffer.GetLength()-4);
	buffer += ".prs";
	fileName= buffer;

	ReadPresets();
	UpdateList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPresetsDlg::OnSave() 
{
	if ( m_preslist.GetCurSel() == CB_ERR )
	{
		if ( m_preslist.GetCount() == MAX_PRESETS ) 
		{
			MessageBox("You have reached the maximum number of Presets. Your operation has \
Not been procesed.","Preset File Full",MB_OK);
			return;
		}
		char str[32];
		m_preslist.GetWindowText(str,32);
		if ( str[0] == '\0' )
		{
			MessageBox("You have not specified any name. Operation Aborted.","Preset Save Error",MB_OK);
			return;
		}
		iniPreset.SetName(str);
		AddPreset(iniPreset);
	}
	else
	{
		char str[32];
		int i2 = m_preslist.GetCurSel();
		presets[i2].GetName(str); // If we don't do this,
		presets[i2]=iniPreset;		
		presets[i2].SetName(str);	// the name gets lost.
	}
	SavePresets();	
}

void CPresetsDlg::OnDelete() 
{
	int i = m_preslist.GetCurSel();
	if ( i != CB_ERR )
	{
		m_preslist.DeleteString(i);
		int i2=m_preslist.GetCount();
		while( i < i2)
		{
			presets[i]=presets[i+1];
			i++;
		}
		presets[i].Clear();
	}	
	SavePresets();
}

void CPresetsDlg::OnImport() 
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		ofn.lpstrFilter = "Presets\0*.prs\0VST Banks\0*.fxb\0All\0*.*\0";
	}
	else ofn.lpstrFilter = "Presets\0*.prs\0All\0*.*\0";

	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if ( _pMachine->_type == MACH_PLUGIN )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetPluginDir();
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetVstDir();
	}
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		if ( ofn.nFilterIndex == 1 )
		{
			int numpresets;
			int filenumpars;
			FILE* hfile;
			if ((hfile=fopen(szFile,"rb")) == NULL )
			{
				MessageBox("Couldn't open File. Operation Aborted","File Open Error",MB_OK);
				return;
			}
			if ( fread(&numpresets,sizeof(int),1,hfile) != 1 ||
				 fread(&filenumpars,sizeof(int),1,hfile) != 1 )
			{
				MessageBox("Couldn't read from File. Operation Aborted","File Open Error",MB_OK);
				fclose(hfile);
				return;
			}
			if (numpresets >= 0)
			{
				int i=0;
				int init=m_preslist.GetCount();
				char cbuf[32];
				int* ibuf = new int[filenumpars];
				byte* dbuf = new byte[sizeDataStruct];
				iniPreset.GetDataArray(dbuf);
				if (sizeDataStruct)
				{
					MessageBox("Files do not match! - this file is old format and this machine requires new format.","File Import Error",MB_OK);
				}
				else if ( numParameters == filenumpars )
				{
					while (i < numpresets && !feof(hfile) && !ferror(hfile) )
					{
						fread(cbuf,sizeof(cbuf),1,hfile);
						fread(ibuf,filenumpars*sizeof(int),1,hfile);
						AddPreset(cbuf,ibuf,dbuf);
						i++;
					}
				}
				else 
				{
					// should be warning
					MessageBox("Files do not match!","File Import Error",MB_OK);
					/*
					int* ibuf2 = new int[numParameters];
					iniPreset.GetParsArray(ibuf2);
					while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
					{
						if ( i<numParameters )
						{
							fread(cbuf,sizeof(cbuf),1,hfile);
							fread(ibuf,filenumpars*sizeof(int),1,hfile);
							if (numParameters<filenumpars)
							{
								memcpy(ibuf2,ibuf,numParameters*sizeof(int));
							}
							else
							{
								memcpy(ibuf2,ibuf,filenumpars*sizeof(int));
							}
							AddPreset(cbuf,ibuf,dbuf);
							i++;
						}
						else
						{
							fseek(hfile,sizeof(cbuf)+(filenumpars*sizeof(int)),SEEK_CUR);
							i++;
						}
					}
					delete ibuf2;
					*/
				}
				delete ibuf;
				delete dbuf;
			}
			else
			{
				int filepresetsize;
				// new preset format version 1
				fread(&numpresets,sizeof(int),1,hfile);
				fread(&filenumpars,sizeof(int),1,hfile);
				fread(&filepresetsize,sizeof(int),1,hfile);
				// we are importing new format! this should be fun!
				int i=0;
				int init=m_preslist.GetCount();
				char cbuf[32];
				int* ibuf = new int[filenumpars];
				byte* dbuf = new byte[sizeDataStruct > filepresetsize ? sizeDataStruct : filepresetsize];

				if ( numParameters == filenumpars )
				{
					if (sizeDataStruct != filepresetsize)
					{
						// should be warning
						MessageBox("Files do not match!","File Import Error",MB_OK);
					}
					else
					{
						while (i < numpresets && !feof(hfile) && !ferror(hfile) )
						{
							fread(cbuf,sizeof(cbuf),1,hfile);
							fread(ibuf,filenumpars*sizeof(int),1,hfile);
							fread(dbuf,filepresetsize,1,hfile);
							/*
							if (sizeDataStruct != filepresetsize)
							{
								// there should be a warning for this
								iniPreset.GetDataArray(dbuf);
							}
							*/
							AddPreset(cbuf,ibuf,dbuf);
							i++;
						}
					}
				}
				else
				{
					// should be warning
					MessageBox("Files do not match!","File Import Error",MB_OK);
					/*
					int* ibuf2 = new int[numParameters];
					iniPreset.GetParsArray(ibuf2);
					while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
					{
						if ( i<numParameters )
						{
							fread(cbuf,sizeof(cbuf),1,hfile);
							fread(ibuf,filenumpars*sizeof(int),1,hfile);
							fread(dbuf,filepresetsize,1,hfile);
							if (sizeDataStruct != filepresetsize)
							{
								// should be warning
								iniPreset.GetDataArray(dbuf);
							}
							if (numParameters<filenumpars)
							{
								memcpy(ibuf2,ibuf,numParameters*sizeof(int));
							}
							else
							{
								memcpy(ibuf2,ibuf,filenumpars*sizeof(int));
							}
							AddPreset(cbuf,ibuf,dbuf);
							i++;
						}
						else
						{
							fseek(hfile,sizeof(cbuf)+(filenumpars*sizeof(int))+filepresetsize,SEEK_CUR);
							i++;
						}
					}
					delete ibuf2;
					*/
				}
				delete ibuf;
				delete dbuf;
			}
			fclose(hfile);
		}
		else
		{	// Check later for Fileformat explanation
			RiffFile fxb;
			if ( !fxb.Open(szFile) )
			{
				MessageBox("Couldn't open File! Operation Aborted.",".fxb File Load Error",MB_OK);
				return;
			}
			fxb.Skip(16);

			ULONGINV numpresets;
			fxb.Read(&numpresets,4);
			int intpresets = (numpresets.lohi*256) + numpresets.lolo;

			ULONGINV filenumpars;
			fxb.Skip(128);
			fxb.Read(&filenumpars,0);

			int i=0;
			int init=m_preslist.GetCount();
			char cbuf[29]; cbuf[28]='\0';
			float* fbuf;
			fbuf= new float[numParameters];
			
			while ( i < intpresets)
			{
				fxb.Skip(24);
				fxb.Read(&filenumpars,4);

				if ( (filenumpars.lohi*256)+filenumpars.lolo != numParameters)
				{
					MessageBox("Number of Parameters does not Match with file!",".fxb File Load Error",MB_OK);
					fxb.Close();
					return;
				}
				fxb.Read(cbuf,28);
				fxb.Read(fbuf,numParameters*sizeof(float));
				for (int y=0;y<numParameters;y++)
				{
					float temp=fbuf[y];
					((char*)&fbuf[y])[0]=((char*)&temp)[3];
					((char*)&fbuf[y])[1]=((char*)&temp)[2];
					((char*)&fbuf[y])[2]=((char*)&temp)[1];
					((char*)&fbuf[y])[3]=((char*)&temp)[0];

				}
				AddPreset(cbuf,fbuf);
				i++;
			}
			delete fbuf;
			fxb.Close();
		}

		UpdateList();
		SavePresets();
	}
}

void CPresetsDlg::OnExport() 
{
	if ( m_preslist.GetCurSel() == CB_ERR )
	{
		MessageBox("You have to select a preset first.","File Save Error",MB_OK);
	}
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Presets\0*.prs\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if ( _pMachine->_type == MACH_PLUGIN )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetPluginDir();
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetVstDir();
	}
	ofn.Flags = OFN_PATHMUSTEXIST;	

	// Display the Open dialog box. 
	
	if (GetSaveFileName(&ofn)==TRUE)
	{
		int selpreset=m_preslist.GetCurSel();
		int filepresets;
		int fileparams;
		FILE* hfile;
		CString str = szFile;
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(".prs") != 0 ) str.Insert(str.GetLength(),".prs");
		sprintf(szFile,str);

		if ((hfile=fopen(szFile,"r+b")) == NULL ) // file does not exist.
		{
			if ((hfile=fopen(szFile,"a+b")) == NULL ) // file cannot be created
			{
				MessageBox("Couldn't open File. Operation Aborted","File Save Error",MB_OK);
				return;
			}
			fclose(hfile);
			hfile=fopen(szFile,"r+b");
			// we have to create a new file

		}
		if ( fread(&filepresets,sizeof(int),1,hfile) != 1 ||
			 fread(&fileparams,sizeof(int),1,hfile) != 1 )
		{
			// create a new file in new format of course
			fseek(hfile,0,SEEK_SET);
			int temp1 = -1;
			if ( fwrite(&temp1,sizeof(int),1,hfile) != 1 ||
				 fwrite(&fileversion,sizeof(int),1,hfile) != 1 )
			{
				MessageBox("Couldn't write to File. Operation Aborted","File Save Error",MB_OK);
				fclose(hfile);
				return;
			}
			temp1 = 0;
			fwrite(&temp1,sizeof(int),1,hfile);
			fwrite(&numParameters,sizeof(int),1,hfile);
			fwrite(&sizeDataStruct,sizeof(int),1,hfile);
			fseek(hfile,2*sizeof(int),SEEK_SET);
			filepresets=-1;
			fileparams=fileversion;
		}

		if (filepresets >= 0)
		{
			if (( fileparams != numParameters ) || (sizeDataStruct))
			{
				MessageBox("Number of Parameters does not Match with file!","File Save Error",MB_OK);
				fclose(hfile);
				return;
			}
			char cbuf[32];
			int* ibuf = new int[numParameters];
			presets[selpreset].GetName(cbuf);
			presets[selpreset].GetParsArray(ibuf);
			filepresets++;

			fseek(hfile,0,SEEK_SET);
			fwrite(&filepresets,sizeof(int),1,hfile);
			fwrite(&fileparams,sizeof(int),1,hfile); // This is for the case that the file doesn't exist

			fseek(hfile,0,SEEK_END);
			fwrite(cbuf,sizeof(cbuf),1,hfile);
			fwrite(ibuf,numParameters*sizeof(int),1,hfile);

			fclose(hfile);
			delete ibuf;
		}
		else
		{
			int filedatasize;
			fread(&filepresets,sizeof(int),1,hfile);
			fread(&fileparams,sizeof(int),1,hfile);
			fread(&filedatasize,sizeof(int),1,hfile);

			if (( fileparams != numParameters ) || (filedatasize != sizeDataStruct))
			{
				MessageBox("Number of Parameters does not Match with file!","File Save Error",MB_OK);
				fclose(hfile);
				return;
			}
			char cbuf[32];
			int* ibuf = new int[numParameters];
			byte* dbuf = new byte[sizeDataStruct];
			presets[selpreset].GetName(cbuf);
			presets[selpreset].GetParsArray(ibuf);
			presets[selpreset].GetDataArray(dbuf);
			filepresets++;

			fseek(hfile,2*sizeof(int),SEEK_SET);
			fwrite(&filepresets,sizeof(int),1,hfile);

			fseek(hfile,0,SEEK_END);
			fwrite(cbuf,sizeof(cbuf),1,hfile);
			fwrite(ibuf,numParameters*sizeof(int),1,hfile);
			fwrite(dbuf,sizeDataStruct,1,hfile);

			fclose(hfile);
			delete ibuf;
			delete dbuf;
		}
	}
}

void CPresetsDlg::OnSelchangePresetslist() 
{
	if ( m_preview.GetCheck())
	{
		int sel=m_preslist.GetCurSel();
		TweakMachine(presets[sel]);
	}
}

void CPresetsDlg::OnDblclkPresetslist() 
{
	CPresetsDlg::OnOK();
	
}

void CPresetsDlg::OnOK() 
{
	int sel=m_preslist.GetCurSel();
	TweakMachine(presets[sel]);
	
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
	m_preslist.ResetContent();
	int i=0;
	char cbuf[32];

	while ( i < MAX_PRESETS)
	{
		presets[i].GetName(cbuf);
		if ( cbuf[0] != '\0')
		{
			m_preslist.AddString(cbuf);
			i++;
		}
		else break;
	}
}

void CPresetsDlg::ReadPresets()
{
	fileversion = 1; // unless otherwise notified

	FILE* hfile;
	int i=0; // Number of Read Presets

//  PSYCLE .prs FILE

	if ((hfile=fopen(fileName,"rb")) != NULL )
	{
		int numpresets;
		int filenumpars;
		if ( fread(&numpresets,sizeof(int),1,hfile) != 1 ||
			 fread(&filenumpars,sizeof(int),1,hfile) != 1 )
		{
			MessageBox("Couldn't read from File. Operation Aborted","File Open Error",MB_OK);
			fclose(hfile);
			return;
		}
		// ok so we still support old file format by checking for a positive numpresets
		char cbuf[32];
		if (numpresets >= 0)
		{
			if (( filenumpars != numParameters )  || (sizeDataStruct))
			{
				MessageBox("The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
				fclose(hfile);
				return;
			}
			fileversion = 0;
			
			int* ibuf;
			ibuf= new int[numParameters];

			while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
			{
				fread(cbuf,sizeof(cbuf),1,hfile);
				fread(ibuf,numParameters*sizeof(int),1,hfile);
				AddPreset(cbuf,ibuf,0);
				i++;
			}
			delete ibuf;
		}
		else
		{
			// new preset file format since numpresets was < 0
			// we will use filenumpars already read as version #
			if (filenumpars == 1)
			{
				int filepresetsize;
				// new preset format version 1
				fread(&numpresets,sizeof(int),1,hfile);
				fread(&filenumpars,sizeof(int),1,hfile);
				fread(&filepresetsize,sizeof(int),1,hfile);
				// now it is time to check our file for compatability
				if (( filenumpars != numParameters )  || (filepresetsize != sizeDataStruct))
				{
					MessageBox("The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
					fclose(hfile);
					return;
				}
				// ok that works, so we should now load the names of all of the presets
				int* ibuf= new int[numParameters];
				byte* dbuf= new byte[sizeDataStruct];

				while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
				{
					fread(cbuf,sizeof(cbuf),1,hfile);
					fread(ibuf,numParameters*sizeof(int),1,hfile);
					fread(dbuf,sizeDataStruct,1,hfile);
					AddPreset(cbuf,ibuf,dbuf);
					i++;
				}
				delete ibuf;
				delete dbuf;
			}
			else
			{
				MessageBox("The current preset file is from a newer version of psycle than you are currently running.","Preset File Error",MB_OK);
				fclose(hfile);
				return;
			}
		}
		fclose(hfile);
	}

//  VST's fxb FILE.
// fxb Structure:
//
// struct FxSet
// {
//	long ChunkMagic;	// "CcnK"
//	long byteSize;		// size of this chunk, excluding ChunkMagic and byteSize
//
//	long fxMagic;		// "FxBk"
//	long version;
//	long fxID;			// Fx ID
//	long	fxVersion;
//
//	long numPrograms;
//  char	future[128];
//	FxProgram *programs;	// Variable no. of programs
// }
//
// struct FxProgram
// {
//	long ChunkMagic;	// "CcnK"
//	long byteSize;		// size of this chunk, excluding ChunkMagic and byteSize
//
//	long fxMagic;		// "FxCk"
//	long version;
//	long fxID;			// Fx ID
//	long fxVersion;
//	long numParams;
//	char prgName[28];
//	float *params		 //variable no. of params
// }

	if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		RiffFile fxb;
		CString buffer= ((VSTPlugin *)_pMachine)->GetDllName();
		buffer = buffer.Left(buffer.GetLength()-4);
		buffer += ".fxb";
		char filename[_MAX_PATH];
		sprintf(filename,buffer);

		if ( !fxb.Open(filename) ) return; // here it is read "CcnK" and its "size" (it is 0)

		fxb.Skip(16); // FxBk header + fxbkVersion + VST ID + VSTVersion

		ULONGINV numpresets;
		fxb.Read(&numpresets,4);
		int intpresets = (numpresets.lohi*256) + numpresets.lolo;
		// well, I don't expect any file with more than 65535 presets.

		ULONGINV filenumpars;
		fxb.Skip(128);
		fxb.Read(&filenumpars,0); // Just because it seems that one "Skip" after another
									// cause problems.

		char cbuf[29]; cbuf[28]='\0';
		float* fbuf;
		fbuf= new float[numParameters];
		
		intpresets+=i; // Global "i" to prevent deleting presets previously read from .prs file
		while ( i < intpresets)
		{
			fxb.Skip(24); // CcnK header + "size" +  FxCk header + fxbkVersion + VST ID + VSTVersion
			fxb.Read(&filenumpars,4);

			if ( (filenumpars.lohi*256)+filenumpars.lolo != numParameters) // same here...
			{
				MessageBox("Number of Parameters does not Match with file!",".fxb File Load Error",MB_OK);
				fxb.Close();
				return;
			}
			fxb.Read(cbuf,28); // Read Name
			fxb.Read(fbuf,numParameters*sizeof(float)); // Read All params.
			for (int y=0;y<numParameters;y++)
			{
				float temp=fbuf[y];
				((char*)&fbuf[y])[0]=((char*)&temp)[3];
				((char*)&fbuf[y])[1]=((char*)&temp)[2];
				((char*)&fbuf[y])[2]=((char*)&temp)[1];
				((char*)&fbuf[y])[3]=((char*)&temp)[0];

			}
			AddPreset(cbuf,fbuf);
			i++;
		}
		delete fbuf;
		fxb.Close();
	}
}

void CPresetsDlg::SavePresets()
{
	int numpresets=m_preslist.GetCount();

	FILE* hfile;
	if ((hfile=fopen(fileName,"wb")) == NULL )
	{
		MessageBox("The File couldn't be opened for Writing. Operation Aborted","File Save Error",MB_OK);
		return;
	}
	if (fileversion == 0)
	{
		if ( fwrite(&numpresets,sizeof(int),1,hfile) != 1 ||
			 fwrite(&numParameters,sizeof(int),1,hfile) != 1 )
		{
			MessageBox("Couldn't write to File. Operation Aborted","File Save Error",MB_OK);
			fclose(hfile);
			return;
		}
		
		int i=0;
		char cbuf[32];
		int* ibuf;
		ibuf= new int[numParameters];

		while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
		{
			presets[i].GetName(cbuf);
			presets[i].GetParsArray(ibuf);
			fwrite(cbuf,sizeof(cbuf),1,hfile);
			fwrite(ibuf,numParameters*sizeof(int),1,hfile);
			i++;
		}
		fclose(hfile);
		delete ibuf;
	}
	else if (fileversion == 1)
	{
		int temp1 = -1;
		
		if ( fwrite(&temp1,sizeof(int),1,hfile) != 1 ||
			 fwrite(&fileversion,sizeof(int),1,hfile) != 1 )
		{
			MessageBox("Couldn't write to File. Operation Aborted","File Save Error",MB_OK);
			fclose(hfile);
			return;
		}
		fwrite(&numpresets,sizeof(int),1,hfile);
		fwrite(&numParameters,sizeof(int),1,hfile);
		fwrite(&sizeDataStruct,sizeof(int),1,hfile);
		
		int i=0;
		char cbuf[32];
		int* ibuf= new int[numParameters];
		byte* dbuf = new byte[sizeDataStruct];

		while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
		{
			presets[i].GetName(cbuf);
			presets[i].GetParsArray(ibuf);
			presets[i].GetDataArray(dbuf);
			fwrite(cbuf,sizeof(cbuf),1,hfile);
			fwrite(ibuf,numParameters*sizeof(int),1,hfile);
			fwrite(dbuf,sizeDataStruct,1,hfile);
			i++;
		}
		fclose(hfile);
		delete ibuf;
		delete dbuf;
	}
}

void CPresetsDlg::TweakMachine(CPreset &preset)
{
	int num=preset.GetNumPars();
	if ( _pMachine->_type == MACH_PLUGIN )
	{
		for (int i=0;i<num;i++)
		{
			((Plugin *)_pMachine)->GetInterface()->ParameterTweak(i,preset.GetParam(i));
		}
		if (preset.GetData())
		{
			((Plugin *)_pMachine)->GetInterface()->PutData(preset.GetData()); // Internal save
		}
		m_wndFrame->Invalidate(false);
	}
	else if ( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX )
	{
		for (int i=0;i<num;i++)
		{
			((VSTPlugin *)_pMachine)->SetParameter(i,preset.GetParam(i)/65535.0f);
		}
	}
	presetChanged=true;
}



void CPresetsDlg::AddPreset(char *name, int *parameters, byte *newdata)
{
	CPreset preset;
	preset.Init(numParameters,name,parameters,sizeDataStruct,newdata);
	AddPreset(preset);
}

void CPresetsDlg::AddPreset(char *name)
{
	CPreset preset;
	preset.Init(name);
	AddPreset(preset);
}

void CPresetsDlg::AddPreset(char *name, float *parameters)
{
	CPreset preset;
	preset.Init(numParameters,name,parameters);
	AddPreset(preset);
}

void CPresetsDlg::AddPreset(CPreset &preset)
{
	char name[32];
	preset.GetName(name);
	int i=m_preslist.AddString(name);
	int i2=m_preslist.GetCount()-1;
	while( i2> i)
	{
		presets[i2]=presets[i2-1];
		i2--;
	}
	presets[i2]=preset;;
}
