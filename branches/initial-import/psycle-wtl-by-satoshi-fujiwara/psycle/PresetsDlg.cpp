/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.6 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "PresetsDlg.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "FrameMachine.h"
#include "FileIO.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
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
	memset(name,0,32);
}

CPreset::~CPreset()
{
	if ( params != NULL ) delete params;
	if ( data != NULL ) delete data;
}

void CPreset::Clear()
{
	if ( params != NULL ) delete params;
	params=NULL;
	numPars =-1;

	if ( data != NULL ) delete data;
	data=NULL;
	sizeData = 0;

	memset(name,0,32);
}

void CPreset::Init(const int num)
{
	if ( params != NULL ) delete params;
	if ( num > 0 )
	{
		params= new int[num];
		numPars=num;
	}
	else
	{
		params=NULL;
		numPars =-1;
	}
	if ( data != NULL ) delete data;
	data=NULL;
	sizeData = 0;

	memset(name,0,32);
}

void CPreset::Init(const int num,const TCHAR* newname,int* parameters,int size, byte* newdata)
{
	if ( params != NULL ) delete params;
	if ( num > 0 )
	{
		params= new int[num];
		numPars=num;
		memcpy(params,parameters,numPars*sizeof(int));
	}
	else
	{
		params=NULL;
		numPars=-1;
	}

	if ( data != NULL )	delete data;
	if ( size > 0 )
	{
		data= new byte[size];
		memcpy(data,newdata,size);
		sizeData = size;
	}
	else
	{
		data=NULL;
		sizeData=0;
	}
	_tcscpy(name,newname);
}

void CPreset::Init(const int num,const TCHAR* newname,float* parameters)
{
	if ( params != NULL ) delete params;
	if ( num > 0 )
	{
		params= new int[num];
		numPars=num;
		for(int x=0;x<num;x++) params[x]= f2i(parameters[x]*65535.0f);
	}
	else
	{
		params=NULL;
		numPars=-1;
	}

	if ( data != NULL )	delete data;
	data=NULL;
	sizeData = 0;

	_tcscpy(name,newname);
}

void CPreset::operator=(CPreset& newpreset)
{
	if ( params != NULL) delete params;
	if ( newpreset.numPars > 0 )
	{
		numPars=newpreset.numPars;
		params= new int[numPars];
		memcpy(params,newpreset.params,numPars*sizeof(int));
	}
	else
	{
		params=NULL;
		numPars=-1;
	}
	if ( data != NULL ) delete data;
	if ( newpreset.sizeData > 0 )
	{
		sizeData = newpreset.sizeData;
		data= new byte[sizeData];
		memcpy(data,newpreset.data,sizeData);
	}
	else
	{
		data=NULL;
		sizeData = 0;
	}

	_tcscpy(name,newpreset.name);
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


CPresetsDlg::CPresetsDlg(Machine* pMachine,CWindow* pWndFrame)
{
	m_pMachine = pMachine;
	m_pWndFrame = pWndFrame;

	numParameters = -1;
	sizeDataStruct = 0;
}



/////////////////////////////////////////////////////////////////////////////
// CPresetsDlg message handlers

void CPresetsDlg::UpdateList()
{
	m_preslist.ResetContent();
	int i=0;
	TCHAR cbuf[32];

	while ( i < MAX_PRESETS)
	{
		presets[i].GetName(cbuf);
		if ( cbuf[0] != _T('\0'))
		{
			m_preslist.AddString(cbuf);
			i++;
		}
		else break;
	}
}

void CPresetsDlg::ReadPresets()
{
	// TODO: プリセットファイルのリードライトをチェックする
	fileversion = 1; // unless otherwise notified

	FILE* hfile;
	int i=0; // Number of Read Presets

//  PSYCLE .prs FILE

	if ((hfile=_tfopen(fileName,_T("rb"))) != NULL )
	{
		int numpresets;
		int filenumpars;
		if ( fread(&numpresets,sizeof(int),1,hfile) != 1 ||
			 fread(&filenumpars,sizeof(int),1,hfile) != 1 )
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0076),
				SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
			fclose(hfile);
			return;
		}
		// ok so we still support old file format by checking for a positive numpresets
		char cbuf[32];
		if (numpresets >= 0)
		{
			if (( filenumpars != numParameters )  || (sizeDataStruct))
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0077),
					SF::CResourceString(IDS_ERR_MSG0078),MB_OK);
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
				AddPreset(CA2T(cbuf),ibuf,0);
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
					MessageBox(SF::CResourceString(IDS_ERR_MSG0079),SF::CResourceString(IDS_ERR_MSG0078),MB_OK);
					fclose(hfile);
					return;
				}
				// ok that works, so we should now load the names of all of the presets
				int* ibuf= new int[numParameters];
				byte* dbuf=NULL;
				if ( sizeDataStruct > 0 ) dbuf = new byte[sizeDataStruct];

				while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
				{
					fread(cbuf,sizeof(cbuf),1,hfile);
					fread(ibuf,numParameters*sizeof(int),1,hfile);
					if ( sizeDataStruct > 0 )  fread(dbuf,sizeDataStruct,1,hfile);
					AddPreset(CA2T(cbuf),ibuf,dbuf);
					i++;
				}
				delete ibuf;
				delete dbuf;
			}
			else
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0080),
					SF::CResourceString(IDS_ERR_MSG0078),MB_OK);
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
//	long ChunkMagic;	// _T("CcnK")
//	long byteSize;		// size of this chunk, excluding ChunkMagic and byteSize
//
//	long fxMagic;		// _T("FxBk")
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
//	long ChunkMagic;	// _T("CcnK")
//	long byteSize;		// size of this chunk, excluding ChunkMagic and byteSize
//
//	long fxMagic;		// _T("FxCk")
//	long version;
//	long fxID;			// Fx ID
//	long fxVersion;
//	long numParams;
//	char prgName[28];
//	float *params		 //variable no. of params
// }

	if ( m_pMachine->_type == MACH_VST || m_pMachine->_type == MACH_VSTFX )
	{
		RiffFile fxb;
		CString buffer= ((VSTPlugin *)m_pMachine)->GetDllName();
		buffer = buffer.Left(buffer.GetLength()-4);
		buffer += _T(".fxb");
		TCHAR filename[_MAX_PATH];
		_stprintf(filename,buffer);

		if ( !fxb.Open(filename) ) return; // here it is read _T("CcnK") and its _T("size") (it is 0)
		if ( fxb._header._id != fxb.FourCC("CcnK") ) return;
		RiffChunkHeader tmp;
		fxb.Read(&tmp,8);
		if ( tmp._id == fxb.FourCC("FBCh") ) // Bank Chunk
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0081),SF::CResourceString(IDS_ERR_MSG0078),MB_OK);
			// Possible structure
			// CcnK, size, fxID, fxversion, numprograms, future[128], chunksize, chunk
			// Can't be read program by program, so it has to be loaded directly.
		}
		else if ( tmp._id == fxb.FourCC("FxBk") )
		{
			fxb.Skip(8); // VST ID + VSTVersion
			ULONGINV numpresets;
			fxb.Read(&numpresets,4);
			int intpresets = (numpresets.lohi*256) + numpresets.lolo;
			// well, I don't expect any file with more than 65535 presets.
			fxb.Skip(128);

			ULONGINV filenumpars;
			fxb.Read(&filenumpars,0); // Just because it seems that one _T("Skip") after another
										// cause problems.

			char cbuf[29]; cbuf[28] = L'\0';
			float* fbuf;
			fbuf= new float[numParameters];
			
			intpresets+=i; // Global _T("i") to prevent deleting presets previously read from .prs file
			while ( i < intpresets)
			{
				fxb.Skip(24); // CcnK header + _T("size") +  FxCk header + fxbkVersion + VST ID + VSTVersion
				fxb.Read(&filenumpars,4);

				if ( (filenumpars.lohi*256)+filenumpars.lolo != numParameters) // same here...
				{
					MessageBox(SF::CResourceString(IDS_ERR_MSG0083),SF::CResourceString(IDS_ERR_MSG0084),MB_OK);
					fxb.Close();
					return;
				}
				fxb.Read(cbuf,28); // Read Name
				fxb.Read(fbuf,numParameters*sizeof(float)); // Read All params.
				for (int y=0;y<numParameters;y++)
				{
					const float temp=fbuf[y];
					((char*)&fbuf[y])[0]=((char*)&temp)[3];
					((char*)&fbuf[y])[1]=((char*)&temp)[2];
					((char*)&fbuf[y])[2]=((char*)&temp)[1];
					((char*)&fbuf[y])[3]=((char*)&temp)[0];

				}
				AddPreset(CA2T(cbuf),fbuf);
				i++;
			}
			delete fbuf;
		}
		fxb.Close();
	}
}

void CPresetsDlg::SavePresets()
{
	int numpresets=m_preslist.GetCount();

	FILE* hfile;
	if ((hfile=_tfopen(fileName,_T("wb"))) == NULL )
	{
		MessageBox(SF::CResourceString(IDS_ERR_MSG0074),
			SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
		return;
	}
	if (fileversion == 0)
	{
		if ( fwrite(&numpresets,sizeof(int),1,hfile) != 1 ||
			 fwrite(&numParameters,sizeof(int),1,hfile) != 1 )
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0085),
				SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
			fclose(hfile);
			return;
		}
		
		int i=0;
		char cbuf[32];
		TCHAR _cbuf[32];

		int* ibuf;
		ibuf= new int[numParameters];

		while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
		{
			presets[i].GetName(_cbuf);
			strcpy(cbuf,CT2A(_cbuf));
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
			MessageBox(SF::CResourceString(IDS_ERR_MSG0085),
				SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
			fclose(hfile);
			return;
		}
		fwrite(&numpresets,sizeof(int),1,hfile);
		fwrite(&numParameters,sizeof(int),1,hfile);
		fwrite(&sizeDataStruct,sizeof(int),1,hfile);
		
		int i=0;
		char cbuf[32];
		TCHAR _cbuf[32];

		int* ibuf= new int[numParameters];
		byte* dbuf=NULL;
		if ( sizeDataStruct > 0 ) dbuf = new byte[sizeDataStruct];

		while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
		{
			presets[i].GetName(_cbuf);
			strcpy(cbuf,CT2A(_cbuf));
			presets[i].GetParsArray(ibuf);
			presets[i].GetDataArray(dbuf);
			fwrite(cbuf,sizeof(cbuf),1,hfile);
			fwrite(ibuf,numParameters*sizeof(int),1,hfile);
			if ( sizeDataStruct > 0 ) fwrite(dbuf,sizeDataStruct,1,hfile);
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
	if ( m_pMachine->_type == MACH_PLUGIN )
	{
		for (int i=0;i<num;i++)
		{
			((Plugin *)m_pMachine)->GetInterface()->ParameterTweak(i,preset.GetParam(i));
		}
		if (preset.GetData())
		{
			((Plugin *)m_pMachine)->GetInterface()->PutData(preset.GetData()); // Internal save
		}
		m_pWndFrame->Invalidate(false);
	}
	else if ( m_pMachine->_type == MACH_VST || m_pMachine->_type == MACH_VSTFX )
	{
		for (int i=0;i<num;i++)
		{
			((VSTPlugin *)m_pMachine)->SetParameter(i,preset.GetParam(i)/65535.0f);
		}
	}
	presetChanged=true;
}



void CPresetsDlg::AddPreset(const TCHAR *name, int *parameters, byte *newdata)
{
	CPreset preset;
	preset.Init(numParameters,name,parameters,sizeDataStruct,newdata);
	AddPreset(preset);
}

void CPresetsDlg::AddPreset(const TCHAR *name, float *parameters)
{
	CPreset preset;
	preset.Init(numParameters,name,parameters);
	AddPreset(preset);
}

void CPresetsDlg::AddPreset(CPreset &preset)
{
	TCHAR name[32];
	preset.GetName(name);
	int i = m_preslist.AddString(name);
	int i2 = m_preslist.GetCount()-1;
	while( i2 > i)
	{
		presets[i2]=presets[i2-1];
		i2--;
	}
	presets[i2]=preset;;
}

LRESULT CPresetsDlg::OnBnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( m_preslist.GetCurSel() == CB_ERR )
	{
		if ( m_preslist.GetCount() == MAX_PRESETS ) 
		{
			MessageBox(
				SF::CResourceString(IDS_ERR_MSG0086)
				,SF::CResourceString(IDS_ERR_MSG0087),MB_OK);
			return TRUE;
		}
		TCHAR str[32];
		m_preslist.GetWindowText(str,32);
		if ( str[0] == _T('\0') )
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0088),
				SF::CResourceString(IDS_ERR_MSG0089),MB_OK);
			return TRUE;
		}
		iniPreset.SetName(str);
		AddPreset(iniPreset);
	}
	else
	{
		TCHAR str[32];
		int i2 = m_preslist.GetCurSel();
		presets[i2].GetName(str); // If we don't do this,
		presets[i2]=iniPreset;		
		presets[i2].SetName(str);	// the name gets lost.
	}
	SavePresets();	

	return 0;
}

LRESULT CPresetsDlg::OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
	return 0;
}

LRESULT CPresetsDlg::OnBnClickedImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]=_T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if ( m_pMachine->_type == MACH_VST || m_pMachine->_type == MACH_VSTFX )
	{
		ofn.lpstrFilter = _T("Presets\0*.prs\0VST Banks\0*.fxb\0All\0*.*\0");
	}
	else ofn.lpstrFilter = _T("Presets\0*.prs\0All\0*.*\0");

	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if ( m_pMachine->_type == MACH_PLUGIN )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetPluginDir();
	}
	else if ( m_pMachine->_type == MACH_VST || m_pMachine->_type == MACH_VSTFX )
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
			if ((hfile=_tfopen(szFile,_T("rb"))) == NULL )
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0090),
					SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
				return 0;
			}
			if ( fread(&numpresets,sizeof(int),1,hfile) != 1 ||
				 fread(&filenumpars,sizeof(int),1,hfile) != 1 )
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0076),
					SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
				fclose(hfile);
				return 0;
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
					MessageBox(SF::CResourceString(IDS_ERR_MSG0091),SF::CResourceString(IDS_ERR_MSG0092),MB_OK);
				}
				else if ( numParameters == filenumpars )
				{
					while (i < numpresets && !feof(hfile) && !ferror(hfile) )
					{
						fread(cbuf,sizeof(cbuf),1,hfile);
						fread(ibuf,filenumpars*sizeof(int),1,hfile);
						AddPreset(CA2T(cbuf),ibuf,dbuf);
						i++;
					}
				}
				else 
				{
					// should be warning
					MessageBox(SF::CResourceString(IDS_ERR_MSG0093),SF::CResourceString(IDS_ERR_MSG0092),MB_OK);
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
						MessageBox(SF::CResourceString(IDS_ERR_MSG0113),
							SF::CResourceString(IDS_ERR_MSG0114),MB_OK);
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
							AddPreset(CA2T(cbuf),ibuf,dbuf);
							i++;
						}
					}
				}
				else
				{
					// should be warning
					MessageBox(SF::CResourceString(IDS_ERR_MSG0093),SF::CResourceString(IDS_ERR_MSG0092),MB_OK);
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
				MessageBox(SF::CResourceString(IDS_ERR_MSG0090),
					SF::CResourceString(IDS_ERR_MSG0084),MB_OK);
				return 0;
			}
			if ( fxb._header._id != fxb.FourCC("CcnK") ) return 0;
			RiffChunkHeader tmp;
			fxb.Read(&tmp,8);
			if ( tmp._id == fxb.FourCC("FBCh") ) // Bank Chunk
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0081),SF::CResourceString(IDS_ERR_MSG0082),MB_OK);
				// Possible structure
				// CcnK, size, fxID, fxversion, numprograms, future[128], chunksize, chunk
				// Can't be read program by program, so it has to be loaded directly.
				// 注：Chunk Bank とはたぶんこれのこと。2003/08/05 WATANABE@NEUE-Engineering
				// http://ygrabit.steinberg.de/users/ygrabit/public_html/vstsdk/OnlineDoc/vstsdk2.3/html/plug/others.html
			}
			else if ( tmp._id == fxb.FourCC("FxBk") )
			{
				fxb.Skip(8); // VST ID + VSTVersion

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
						MessageBox(SF::CResourceString(IDS_ERR_MSG0083),
							SF::CResourceString(IDS_ERR_MSG0084),MB_OK);
						fxb.Close();
						return 0;
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
					AddPreset(CA2T(cbuf),fbuf);
					i++;
				}
				delete fbuf;
			}
			fxb.Close();
		}

		UpdateList();
		SavePresets();
	}
	return 0;
}

LRESULT CPresetsDlg::OnBnClickedExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( m_preslist.GetCurSel() == CB_ERR )
	{
		MessageBox(SF::CResourceString(IDS_ERR_MSG0094),
			SF::CResourceString(IDS_ERR_MSG0089),MB_OK);
	}
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	szFile[0] = _T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Presets\0*.prs\0All\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if ( m_pMachine->_type == MACH_PLUGIN )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetPluginDir();
	}
	else if ( m_pMachine->_type == MACH_VST || m_pMachine->_type == MACH_VSTFX )
	{
		ofn.lpstrInitialDir = Global::pConfig->GetVstDir();
	}
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	

	// Display the Open dialog box. 
	
	if (GetSaveFileName(&ofn)==TRUE)
	{
		int selpreset=m_preslist.GetCurSel();
		int filepresets;
		int fileparams;
		FILE* hfile;
		CString str = szFile;
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(_T(".prs")) != 0 ) str.Insert(str.GetLength(),_T(".prs"));
		_stprintf(szFile,str);

		if ((hfile=_tfopen(szFile,_T("r+b"))) == NULL ) // file does not exist.
		{
			if ((hfile=_tfopen(szFile,_T("a+b"))) == NULL ) // file cannot be created
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0074),
					SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
				return 0;
			}
			fclose(hfile);
			hfile=_tfopen(szFile,_T("r+b"));
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
				MessageBox(SF::CResourceString(IDS_ERR_MSG0085),
					SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
				fclose(hfile);
				return 0;
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
				MessageBox(SF::CResourceString(IDS_ERR_MSG0083),
					SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
				fclose(hfile);
				return 0;
			}
			char cbuf[32];
			TCHAR _cbuf[32];

			int* ibuf = new int[numParameters];

			presets[selpreset].GetName(_cbuf);
			strcpy(cbuf,CT2A(_cbuf));
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
				MessageBox(SF::CResourceString(IDS_ERR_MSG0083),
					SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
				fclose(hfile);
				return 0;
			}
			TCHAR cbuf[32];
			int* ibuf = new int[numParameters];
			byte* dbuf = NULL;
			if ( sizeDataStruct > 0 ) dbuf = new byte[sizeDataStruct];
			presets[selpreset].GetName(cbuf);
			presets[selpreset].GetParsArray(ibuf);
			presets[selpreset].GetDataArray(dbuf);
			filepresets++;

			fseek(hfile,2*sizeof(int),SEEK_SET);
			fwrite(&filepresets,sizeof(int),1,hfile);

			fseek(hfile,0,SEEK_END);
			fwrite(cbuf,sizeof(cbuf),1,hfile);
			fwrite(ibuf,numParameters*sizeof(int),1,hfile);
			if ( sizeDataStruct > 0 ) fwrite(dbuf,sizeDataStruct,1,hfile);

			fclose(hfile);
			delete ibuf;
			delete dbuf;
		}
	}
	return 0;
}

LRESULT CPresetsDlg::OnBnClickedPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。

	return 0;
}

LRESULT CPresetsDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int sel=m_preslist.GetCurSel();
	TweakMachine(presets[sel]);
	EndDialog(0);
	return 0;
}

LRESULT CPresetsDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (presetChanged ) // Restore old preset if changed.
	{
		TweakMachine(iniPreset);
	}

	EndDialog(0);
	return 0;
}

LRESULT CPresetsDlg::OnCbnDblclkPresetslist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int sel=m_preslist.GetCurSel();
	TweakMachine(presets[sel]);
	EndDialog(0);
	return 0;
}

LRESULT CPresetsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_exportButton.Attach(GetDlgItem(IDC_EXPORT));
	m_preview.Attach(GetDlgItem(IDC_PREVIEW));
	m_preslist.Attach(GetDlgItem(IDC_PRESETSLIST));
	
	m_preslist.LimitText(32);

	presetChanged=false;
	CString buffer;
	if ( m_pMachine->_type == MACH_PLUGIN )
	{
		numParameters = ((Plugin*)m_pMachine)->GetInfo()->numParameters;
		sizeDataStruct = ((Plugin *)m_pMachine)->GetInterface()->GetDataSize();
		if (sizeDataStruct > 0)
		{
			byte* pData = new byte[sizeDataStruct];
			((Plugin *)m_pMachine)->GetInterface()->GetData(pData); // Internal Save
			iniPreset.Init(numParameters , _T(""), ((Plugin*)m_pMachine)->GetInterface()->Vals, sizeDataStruct,  pData);
			delete pData;
		}
		else
		{
			iniPreset.Init(numParameters , _T(""), ((Plugin*)m_pMachine)->GetInterface()->Vals, 0,  NULL);
		}

		buffer = ((Plugin *)m_pMachine)->GetDllName();
	}
	else if ( m_pMachine->_type == MACH_VST || m_pMachine->_type == MACH_VSTFX )
	{
		numParameters = ((VSTPlugin*)m_pMachine)->NumParameters();
		iniPreset.Init(numParameters);
		int i=0;
		while (i < numParameters)
		{
			iniPreset.SetParam(i,f2i(((VSTPlugin*)m_pMachine)->GetParameter(i)*65535));
			i++;
		}

		buffer = ((VSTPlugin *)m_pMachine)->GetDllName();
	}

	buffer = buffer.Left(buffer.GetLength()-4);
	buffer += _T(".prs");
	fileName= buffer;

	ReadPresets();
	UpdateList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CPresetsDlg::OnCbnSelchangePresetslist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( m_preview.GetCheck())
	{
		int sel=m_preslist.GetCurSel();
		TweakMachine(presets[sel]);
	}
	return 0;
}
