///\file
///\brief implementation file for psycle::host::CPresetsDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/PresetsDlg.hpp>
#include <psycle/engine/Plugin.hpp>
#include <psycle/engine/VSTHost.hpp>
#include <psycle/host/FrameMachine.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/FileIO.hpp>

#define use_boost 0 // causes an exception with paths that contain spaces
#if use_boost
	#include <boost/filesystem/path.hpp>
	#include <boost/filesystem/convenience.hpp>
#endif

#if !defined DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
	#error "sorry, only works on little endian machines"
#endif

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

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
			zapArray(params);
			zapArray(data);
		}

		void CPreset::Clear()
		{
			zapArray(params);
			numPars =-1;
			zapArray(data);
			sizeData = 0;
			std::memset(name,0,32);
		}

		void CPreset::Init(int num)
		{
			if ( num > 0 )
			{
				zapArray(params,new int[num]);
				numPars=num;
			}
			else
			{
				zapArray(params);
				numPars =-1;
			}
			zapArray(data);
			sizeData = 0;

			memset(name,0,32);
		}

		void CPreset::Init(int num, char* newname, int const * parameters, int size, unsigned char * newdata)
		{
			if ( num > 0 )
			{
				zapArray(this->params, new int[num]);
				numPars=num;
				std::memcpy(this->params,parameters,numPars*sizeof(int));
			}
			else
			{
				zapArray(this->params);
				numPars=-1;
			}

			if ( size > 0 )
			{
				zapArray(data, new unsigned char[size]);
				std::memcpy(data,newdata,size);
				sizeData = size;
			}
			else
			{
				zapArray(data);
				sizeData=0;
			}
			std::strcpy(name,newname);
		}

		void CPreset::Init(int num, char * newname, float const * parameters)
		{
			if ( num > 0 )
			{
				zapArray(this->params,new int[num]);
				numPars=num;
				for(int x=0;x<num;x++) params[x]= f2i(parameters[x]*65535.0f);
			}
			else
			{
				zapArray(params);
				numPars=-1;
			}

			zapArray(data);
			sizeData = 0;

			std::strcpy(name,newname);
		}

		void CPreset::operator=(CPreset& newpreset)
		{
			if ( newpreset.numPars > 0 )
			{
				numPars=newpreset.numPars;
				zapArray(params, new int[numPars]);
				memcpy(params,newpreset.params,numPars*sizeof(int));
			}
			else
			{
				zapArray(params);
				numPars=-1;
			}

			if ( newpreset.sizeData > 0 )
			{
				sizeData = newpreset.sizeData;
				zapArray(data,new unsigned char[sizeData]);
				memcpy(data,newpreset.data,sizeData);
			}
			else
			{
				zapArray(data);
				sizeData = 0;
			}

			strcpy(name,newpreset.name);
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

		CPresetsDlg::CPresetsDlg(CWnd* pParent) : CDialog(CPresetsDlg::IDD, pParent)
		{
			numParameters = -1;
			sizeDataStruct = 0;
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
			if( _pMachine->subclass() == MACH_PLUGIN)
			{
				numParameters = _pMachine->GetNumParams();
				try
				{
					sizeDataStruct = ((Plugin *)_pMachine)->proxy().GetDataSize();
				}
				catch(const std::exception &)
				{
					sizeDataStruct = 0;
				}
				if(sizeDataStruct > 0)
				{
					unsigned char * pData = new unsigned char[sizeDataStruct];
					try
					{
						static_cast<Plugin *>(_pMachine)->proxy().GetData(pData); // Internal Save
						iniPreset.Init(numParameters , "", static_cast<Plugin*>(_pMachine)->proxy().Vals(), sizeDataStruct, pData);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // static_cast sucks
					{
						// o_O`
					}
					zapArray(pData);
				}
				else
				{
					try
					{
						iniPreset.Init(numParameters , "", static_cast<Plugin *>(_pMachine)->proxy().Vals(), 0,  0);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // static_cast sucks
					{
						// o_O`
					}
				}
				
			}
			else if( _pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX)
			{
				try
				{
					numParameters = _pMachine->GetNumParams();
				}
				catch(const std::exception &)
				{
					numParameters = 0;
				}
				catch(...) // static_cast sucks
				{
					numParameters = 0;
				}
				iniPreset.Init(numParameters);
				int i(0);
				while(i < numParameters)
				{
					try
					{
						iniPreset.SetParam(i, f2i(static_cast<vst::plugin *>(_pMachine)->proxy().getParameter(i) * 65535));
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // static_cast sucks
					{
						// o_O`
					}
					++i;
				}
			}

			// change extension of the plugin's library file name to .prs
			{
				#if use_boost // causes an exception with paths that contain spaces
					boost::filesystem::path path(_pMachine->GetDllName(), boost::filesystem::native);
					path = boost::filesystem::change_extension(path, ".prs");
					file_name = path.string();
				#else
					file_name = _pMachine->GetDllName();
					std::string::size_type pos = file_name.rfind('.');
					if(pos != std::string::npos) file_name = file_name.substr(0, pos);
					file_name += ".prs";
				#endif
			}

			ReadPresets();
			UpdateList();
			return true;
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
			OPENFILENAME ofn; // common dialog box structure
			char szFile[MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			if( _pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX)
			{
				ofn.lpstrFilter = "Presets\0*.prs\0VST Banks\0*.fxb\0All\0*.*\0";
			}
			else ofn.lpstrFilter = "Presets\0*.prs\0All\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			if( _pMachine->subclass() == MACH_PLUGIN)
			{
				std::string tmpstr = Global::configuration().GetPluginDir();
				ofn.lpstrInitialDir = tmpstr.c_str();
			}
			else if( _pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX)
			{
				std::string tmpstr = Global::configuration().GetVstDir();
				ofn.lpstrInitialDir = tmpstr.c_str();
			}
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if(GetOpenFileName(&ofn) == true)
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
						unsigned char * dbuf = new unsigned char[sizeDataStruct];
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
							zapArray(ibuf2);
							*/
						}
						zapArray(ibuf);
						zapArray(dbuf);
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
						unsigned char * dbuf = new unsigned char[sizeDataStruct > filepresetsize ? sizeDataStruct : filepresetsize];

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
							zapArray(ibuf2);
							*/
						}
						zapArray(ibuf);
						zapArray(dbuf);
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
					if ( fxb._header._id != fxb.FourCC("CcnK") ) return;
					RiffChunkHeader tmp;
					fxb.Read(tmp);
					if ( tmp._id == fxb.FourCC("FBCh") ) // Bank Chunk
					{
						MessageBox("Chunk Banks not supported yet","Preset File Error",MB_OK);
						// Possible structure
						// CcnK, size, fxID, fxversion, numprograms, future[128], chunksize, chunk
						// Can't be read program by program, so it has to be loaded directly.
					}
					else if ( tmp._id == fxb.FourCC("FxBk") )
					{
						fxb.Skip(8); // VST ID + VSTVersion

						endian::big::uint32_t numpresets;
						fxb.ReadChunk(&numpresets,4);
						int intpresets = (numpresets.lohi*256) + numpresets.lolo;

						endian::big::uint32_t filenumpars;
						fxb.Skip(128);
						//\todo: Investigate the following line. I assume it to be wrong, and the next fxb.Skip be 28.
						fxb.ReadChunk(&filenumpars,0);

						int i=0;
						//int init=m_preslist.GetCount();
						char cbuf[29]; cbuf[28]='\0';
						float* fbuf;
						fbuf= new float[numParameters];
						
						while ( i < intpresets)
						{
							fxb.Skip(24);
							fxb.ReadChunk(&filenumpars,4);

							if ( (filenumpars.lohi*256)+filenumpars.lolo != numParameters)
							{
								MessageBox("Number of Parameters does not Match with file!",".fxb File Load Error",MB_OK);
								fxb.Close();
								return;
							}
							fxb.ReadChunk(cbuf,28); cbuf[27]=0;
							fxb.ReadChunk(fbuf,numParameters*sizeof(float));
							for (int y=0;y<numParameters;y++)
							{
								float temp=fbuf[y];
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								((char*)&fbuf[y])[0]=((char*)&temp)[3];
								((char*)&fbuf[y])[1]=((char*)&temp)[2];
								((char*)&fbuf[y])[2]=((char*)&temp)[1];
								((char*)&fbuf[y])[3]=((char*)&temp)[0];
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
								/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							}
							AddPreset(cbuf,fbuf);
							i++;
						}
						zapArray(fbuf);
					}
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
			if ( _pMachine->subclass() == MACH_PLUGIN )
			{
				std::string tmpstr = Global::configuration().GetPluginDir();
				ofn.lpstrInitialDir = tmpstr.c_str();
			}
			else if ( _pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX )
			{
				std::string tmpstr = Global::configuration().GetVstDir();
				ofn.lpstrInitialDir = tmpstr.c_str();
			}
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	

			// Display the Open dialog box. 
			
			if (GetSaveFileName(&ofn)==true)
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
					zapArray(ibuf);
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
					unsigned char * dbuf(0);
					if ( sizeDataStruct > 0 ) dbuf = new unsigned char[sizeDataStruct];
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
					zapArray(ibuf);
					zapArray(dbuf);
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

			if(hfile=fopen(file_name.c_str(), "rb"))
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
					zapArray(ibuf);
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
						unsigned char * dbuf(0);
						if ( sizeDataStruct > 0 ) dbuf = new unsigned char[sizeDataStruct];

						while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
						{
							fread(cbuf,sizeof(cbuf),1,hfile);
							fread(ibuf,numParameters*sizeof(int),1,hfile);
							if ( sizeDataStruct > 0 )  fread(dbuf,sizeDataStruct,1,hfile);
							AddPreset(cbuf,ibuf,dbuf);
							i++;
						}
						zapArray(ibuf);
						zapArray(dbuf);
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

			if( _pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX)
			{
				RiffFile fxb;

				// change extension of the plugin's library file name to .fxb
				{
					#if use_boost // causes an exception with paths that contain spaces
						boost::filesystem::path path(_pMachine->GetDllName(), boost::filesystem::native);
						path = boost::filesystem::change_extension(path, ".fxb");
						if(!fxb.Open(path.string())) return; // here it is read "CcnK" and its "size" (it is 0)
					#else
						std::string file_name = _pMachine->GetDllName();
						std::string::size_type pos = file_name.rfind('.');
						if(pos != std::string::npos) file_name = file_name.substr(0, pos);
						file_name += ".fxb";
						if(!fxb.Open(file_name)) return; // here it is read "CcnK" and its "size" (it is 0)
					#endif
				}

				if(fxb._header._id != fxb.FourCC("CcnK")) return;
				RiffChunkHeader tmp;
				fxb.Read(tmp);
				if(tmp._id == fxb.FourCC("FBCh")) // Bank Chunk
				{
					MessageBox("Chunk Banks not supported yet","Preset File Error",MB_OK);
					// Possible structure
					// CcnK, size, fxID, fxversion, numprograms, future[128], chunksize, chunk
					// Can't be read program by program, so it has to be loaded directly.
				}
				else if( tmp._id == fxb.FourCC("FxBk"))
				{
					fxb.Skip(8); // VST ID + VSTVersion
					endian::big::uint32_t numpresets;
					fxb.ReadChunk(&numpresets,4);
					int intpresets = (numpresets.lohi*256) + numpresets.lolo;
					// well, I don't expect any file with more than 65535 presets.
					fxb.Skip(128);

					endian::big::uint32_t filenumpars;
					fxb.ReadChunk(&filenumpars,0); // Just because it seems that one "Skip" after another cause problems.

					char cbuf[29]; cbuf[28]='\0';
					float* fbuf;
					fbuf= new float[numParameters];
					
					intpresets+=i; // Global "i" to prevent deleting presets previously read from .prs file
					while( i < intpresets)
					{
						fxb.Skip(24); // CcnK header + "size" +  FxCk header + fxbkVersion + VST ID + VSTVersion
						fxb.ReadChunk(&filenumpars,4);
						if(filenumpars.lohi * 256 + filenumpars.lolo != numParameters) // same here...
						{
							MessageBox("Number of Parameters does not Match with file!",".fxb File Load Error",MB_OK);
							fxb.Close();
							return;
						}
						fxb.ReadChunk(cbuf,28); cbuf[27]=0;// Read Name
						fxb.ReadChunk(fbuf,numParameters*sizeof(float)); // Read All params.
						for (int y=0;y<numParameters;y++)
						{
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							const float temp=fbuf[y];
							((char*)&fbuf[y])[0]=((char*)&temp)[3];
							((char*)&fbuf[y])[1]=((char*)&temp)[2];
							((char*)&fbuf[y])[2]=((char*)&temp)[1];
							((char*)&fbuf[y])[3]=((char*)&temp)[0];
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
							/////////// TODO THIS IS HARDCODED FOR LITTLE ENDIAND MACHINES ////////
						}
						AddPreset(cbuf, fbuf);
						++i;
					}
					zapArray(fbuf);
				}
				fxb.Close();
			}
		}

		void CPresetsDlg::SavePresets()
		{
			int numpresets=m_preslist.GetCount();

			FILE * hfile;
			if(!(hfile=fopen(file_name.c_str(), "wb")))
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
				zapArray(ibuf);
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
				unsigned char * dbuf(0);
				if ( sizeDataStruct > 0 ) dbuf = new unsigned char[sizeDataStruct];

				while ( i< numpresets && !feof(hfile) && !ferror(hfile) )
				{
					presets[i].GetName(cbuf);
					presets[i].GetParsArray(ibuf);
					presets[i].GetDataArray(dbuf);
					fwrite(cbuf,sizeof(cbuf),1,hfile);
					fwrite(ibuf,numParameters*sizeof(int),1,hfile);
					if ( sizeDataStruct > 0 ) fwrite(dbuf,sizeDataStruct,1,hfile);
					i++;
				}
				fclose(hfile);
				zapArray(ibuf);
				zapArray(dbuf);
			}
		}

		void CPresetsDlg::TweakMachine(CPreset &preset)
		{
			int num=preset.GetNumPars();
			if(_pMachine->subclass() == MACH_PLUGIN)
			{
				for(int i(0) ; i < num ; ++i)
				{
					try
					{
						static_cast<Plugin *>(_pMachine)->proxy().ParameterTweak(i, preset.GetParam(i));
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // static_cast sucks
					{
						// o_O`
					}
				}
				if(preset.GetData())
				{
					try
					{
						static_cast<Plugin *>(_pMachine)->proxy().PutData(preset.GetData()); // Internal save
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // static_cast sucks
					{
						// o_O`
					}
				}
				m_wndFrame->Invalidate(false);
			}
			else if(_pMachine->subclass() == MACH_VST || _pMachine->subclass() == MACH_VSTFX)
			{
				for(int i(0) ; i < num ; ++i)
				{
					try
					{
						static_cast<vst::plugin *>(_pMachine)->proxy().setParameter(i, preset.GetParam(i) / 65535.0f);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					catch(...) // static_cast sucks
					{
						// o_O`
					}
				}
			}
			presetChanged = true;
		}



		void CPresetsDlg::AddPreset(char *name, int *parameters, unsigned char * newdata)
		{
			CPreset preset;
			preset.Init(numParameters,name,parameters,sizeDataStruct,newdata);
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

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
