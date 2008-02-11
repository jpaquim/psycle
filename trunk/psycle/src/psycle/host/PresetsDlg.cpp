/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief implementation file for psycle::host::CPresetsDlg.
#include <psycle/project.private.hpp>
#include "PresetsDlg.hpp"
#include "psycle.hpp"
#include "Plugin.hpp"
#include "VSTHost24.hpp"
#include "FrameMachine.hpp"
#include "FileIO.hpp"
#include <cstring>

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		using namespace seib::vst;

		CPreset::CPreset()
		:
			params(),
			data(),
			numPars(-1),
			dataSize()
		{
			std::memset(name, 0, sizeof name * sizeof *name);
		}

		CPreset::~CPreset()
		{
			delete [] params;
			delete [] data;
		}

		void CPreset::Clear()
		{
			delete [] params; params = 0;
			numPars =-1;
			delete [] data; data = 0;
			dataSize = 0;
			std::memset(name,0,32);
		}

		void CPreset::Init(int num)
		{
			if ( num > 0 )
			{
				delete [] params; params = new int[num];
				numPars=num;
			}
			else
			{
				delete params; params = 0;
				numPars =-1;
			}
			delete [] data; data = 0;
			dataSize = 0;

			std::memset(name,0, sizeof name * sizeof *name);
		}

		void CPreset::Init(int num,const char* newname, int const * parameters,int size, void* newdata)
		{
			if ( num > 0 )
			{
				delete [] params; params = new int[num];
				numPars=num;
				std::memcpy(params,parameters,numPars * sizeof *params);
			}
			else
			{
				delete [] params; params = 0;
				numPars=-1;
			}

			if ( size > 0 )
			{
				delete [] data; data = new unsigned char[size];
				std::memcpy(data,newdata,size);
				dataSize = size;
			}
			else
			{
				delete [] data; data = 0;
				dataSize=0;
			}
			std::strcpy(name,newname);
		}

		void CPreset::Init(int num,const char* newname, float const * parameters)
		{
			if ( num > 0 )
			{
				delete [] params; params = new int[num];
				numPars=num;
				for(int x=0;x<num;x++) params[x]= helpers::math::rounded(parameters[x]*65535.0f);
			}
			else
			{
				delete [] params; params = 0;
				numPars=-1;
			}

			delete [] data; data = 0;
			dataSize = 0;

			std::strcpy(name,newname);
		}

		void CPreset::operator=(CPreset& newpreset)
		{
			if ( newpreset.numPars > 0 )
			{
				numPars=newpreset.numPars;
				delete [] params; params = new int[numPars];
				std::memcpy(params,newpreset.params,numPars * sizeof *params);
			}
			else
			{
				delete [] params; params = 0;
				numPars=-1;
			}

			if ( newpreset.dataSize > 0 )
			{
				dataSize = newpreset.dataSize;
				delete [] data; data = new unsigned char[dataSize];
				std::memcpy(data,newpreset.data,dataSize);
			}
			else
			{
				delete [] data; data = 0;
				dataSize = 0;
			}

			strcpy(name,newpreset.name);
		}

		int CPreset::GetParam(const int n)
		{
			if (( numPars != -1 ) && ( n < numPars )) return params[n];
			return -1;
		}

		void CPreset::SetParam(const int n,int val)
		{
			if (( numPars != -1 ) && ( n < numPars )) params[n] = val;
		}

		CPresetsDlg::CPresetsDlg(CWnd* pParent) : CDialog(CPresetsDlg::IDD, pParent)
		{
			numParameters = -1;
			dataSizeStruct = 0;
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

		BOOL CPresetsDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_preslist.LimitText(32);
			presetChanged = false;
			CString buffer;

			numParameters = _pMachine->GetNumParams();

			if( _pMachine->_type == MACH_PLUGIN)
			{
				try
				{
					dataSizeStruct = ((Plugin *)_pMachine)->proxy().GetDataSize();
				}
				catch(const std::exception &)
				{
					dataSizeStruct = 0;
				}
				if(dataSizeStruct > 0)
				{
					void* pData = new char[dataSizeStruct];
					try
					{
						reinterpret_cast<Plugin *>(_pMachine)->proxy().GetData(pData); // Internal Save
						iniPreset.Init(numParameters , "", reinterpret_cast<Plugin *>(_pMachine)->proxy().Vals(), dataSizeStruct, pData);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // reinterpret_cast sucks
					{
						// o_O`
					}
					delete pData;
				}
				else
				{
					
					try
					{
						iniPreset.Init(numParameters , "", reinterpret_cast<Plugin *>(_pMachine)->proxy().Vals(), 0,  0);
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
				buffer = _pMachine->GetDllName();
			}
			else if( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX)
			{ 
				iniPreset.Init(numParameters);
				int i(0);
				while(i < numParameters)
				{
					try
					{
						iniPreset.SetParam(i, helpers::math::rounded(reinterpret_cast<vst::plugin *>(_pMachine)->GetParameter(i) * vst::quantization));
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // reinterpret_cast sucks
					{
						// o_O`
					}
					++i;
				}
				buffer = _pMachine->GetDllName();

			}
			buffer = buffer.Left(buffer.GetLength()-4);
			buffer += ".prs";
			fileName= buffer;
			ReadPresets();
			UpdateList();
			return TRUE;
		}

		void CPresetsDlg::OnSave() 
		{
			if ( m_preslist.GetCurSel() == CB_ERR )
			{
				if ( m_preslist.GetCount() == MAX_PRESETS ) 
				{
					MessageBox("You have reached the maximum number of Presets. Your operation has Not been procesed.","Preset File Full",MB_OK);
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
			if( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX) return;

			char szFile[MAX_PATH]; // buffer for file name
			szFile[0]='\0';

			///\todo unportable code
			OPENFILENAME ofn; // common dialog box structure
			// Initialize OPENFILENAME
			std::memset(&ofn, 0, sizeof ofn);
			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof szFile;
			if( _pMachine->_type == MACH_VST || _pMachine->_type == MACH_VSTFX)
			{
				ofn.lpstrFilter = "Presets\0*.prs\0VST Banks\0*.fxb\0All\0*.*\0";
			}
			else ofn.lpstrFilter = "Presets\0*.prs\0All\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetPluginDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if(GetOpenFileName(&ofn) == TRUE)
			{
				if(ofn.nFilterIndex == 1)
				{
					int numpresets;
					int filenumpars;
					FILE * hfile;
					if(!(hfile=fopen(szFile,"rb")))
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
						//int init=m_preslist.GetCount();
						char cbuf[32];
						int* ibuf = new int[filenumpars];
						byte* dbuf = new byte[dataSizeStruct];
						iniPreset.GetDataArray(dbuf);
						if (dataSizeStruct)
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
							zapArray(ibuf2);
							*/
						}
						delete [] ibuf; ibuf = 0;
						delete [] dbuf; dbuf = 0;
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
		//				int init=m_preslist.GetCount();
						char cbuf[32];
						int* ibuf = new int[filenumpars];
						byte* dbuf = new byte[dataSizeStruct > filepresetsize ? dataSizeStruct : filepresetsize];

						if ( numParameters == filenumpars )
						{
							if (dataSizeStruct != filepresetsize)
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
									if (dataSizeStruct != filepresetsize)
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
									if (dataSizeStruct != filepresetsize)
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
							zapArray(ibuf2);
							*/
						}
						delete [] ibuf; ibuf = 0;
						delete [] dbuf; dbuf = 0;
					}
					fclose(hfile);
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
			char szFile[MAX_PATH];       // buffer for file name
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
				std::string tmpstr = Global::pConfig->GetPluginDir();
				ofn.lpstrInitialDir = tmpstr.c_str();
			}
			else 
			{
				std::string tmpstr = Global::pConfig->GetVstDir();
				ofn.lpstrInitialDir = tmpstr.c_str();
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
				if ( str2.CompareNoCase(".prs") != 0 ) str.Insert(str.GetLength(),".prs");
				sprintf(szFile,str);

				if(!(hfile=fopen(szFile,"r+b"))) // file does not exist.
				{
					if(!(hfile=fopen(szFile,"a+b"))) // file cannot be created
					{
						MessageBox("Couldn't open File for Writing. Operation Aborted","File Save Error",MB_OK);
						return;
					}
					fclose(hfile);
					hfile=fopen(szFile,"r+b"); // read and write mode
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
					fwrite(&dataSizeStruct,sizeof(int),1,hfile);
					fseek(hfile,2*sizeof(int),SEEK_SET);
					filepresets=-1;
					fileparams=fileversion;
				}

				if (filepresets >= 0)
				{
					if (( fileparams != numParameters ) || (dataSizeStruct))
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
					delete [] ibuf;
				}
				else
				{
					int filedatasize;
					fread(&filepresets,sizeof(int),1,hfile);
					fread(&fileparams,sizeof(int),1,hfile);
					fread(&filedatasize,sizeof(int),1,hfile);

					if (( fileparams != numParameters ) || (filedatasize != dataSizeStruct))
					{
						MessageBox("Number of Parameters does not Match with file!","File Save Error",MB_OK);
						fclose(hfile);
						return;
					}
					char cbuf[32];
					int* ibuf = new int[numParameters];
					unsigned char * dbuf = 0;
					if ( dataSizeStruct > 0 ) dbuf = new unsigned char[dataSizeStruct];
					presets[selpreset].GetName(cbuf);
					presets[selpreset].GetParsArray(ibuf);
					presets[selpreset].GetDataArray(dbuf);
					filepresets++;

					fseek(hfile,2*sizeof(int),SEEK_SET);
					fwrite(&filepresets,sizeof(int),1,hfile);

					fseek(hfile,0,SEEK_END);
					fwrite(cbuf,sizeof(cbuf),1,hfile);
					fwrite(ibuf,numParameters*sizeof(int),1,hfile);
					if ( dataSizeStruct > 0 ) fwrite(dbuf,dataSizeStruct,1,hfile);

					fclose(hfile);
					delete [] ibuf;
					delete [] dbuf;
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

			if(hfile=fopen(fileName,"rb"))
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
					if (( filenumpars != numParameters )  || (dataSizeStruct))
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
					delete [] ibuf;
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
						if (( filenumpars != numParameters )  || (filepresetsize != dataSizeStruct))
						{
							MessageBox("The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
							fclose(hfile);
							return;
						}
						// ok that works, so we should now load the names of all of the presets
						int* ibuf= new int[numParameters];
						unsigned char * dbuf = 0;
						if ( dataSizeStruct > 0 ) dbuf = new unsigned char[dataSizeStruct];

						while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
						{
							fread(cbuf,sizeof(cbuf),1,hfile);
							fread(ibuf,numParameters*sizeof(int),1,hfile);
							if ( dataSizeStruct > 0 )  fread(dbuf,dataSizeStruct,1,hfile);
							AddPreset(cbuf,ibuf,dbuf);
							i++;
						}
						delete [] ibuf;
						delete [] dbuf;
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
		}

		void CPresetsDlg::SavePresets()
		{
			int numpresets=m_preslist.GetCount();

			FILE* hfile;
			if(!(hfile=fopen(fileName,"wb")))
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
				delete [] ibuf;
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
				fwrite(&dataSizeStruct,sizeof(int),1,hfile);
				
				int i=0;
				char cbuf[32];
				int* ibuf= new int[numParameters];
				unsigned char * dbuf = 0;
				if ( dataSizeStruct > 0 ) dbuf = new byte[dataSizeStruct];

				while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
				{
					presets[i].GetName(cbuf);
					presets[i].GetParsArray(ibuf);
					presets[i].GetDataArray(dbuf);
					fwrite(cbuf,sizeof(cbuf),1,hfile);
					fwrite(ibuf,numParameters*sizeof(int),1,hfile);
					if ( dataSizeStruct > 0 ) fwrite(dbuf,dataSizeStruct,1,hfile);
					i++;
				}
				fclose(hfile);
				delete [] ibuf;
				delete [] dbuf;
			}
		}

		void CPresetsDlg::TweakMachine(CPreset &preset)
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



		void CPresetsDlg::AddPreset(const char *name, int *parameters, byte *newdata)
		{
			CPreset preset;
			preset.Init(numParameters,name,parameters,dataSizeStruct,newdata);
			AddPreset(preset);
		}

		void CPresetsDlg::AddPreset(const char *name, float *parameters)
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

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
