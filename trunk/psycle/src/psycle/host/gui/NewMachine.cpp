///\file
///\brief implementation file for psycle::host::CNewMachine.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/gui/psycle.hpp>
#include <psycle/host/gui/NewMachine.hpp>
#include <psycle/host/engine/Plugin.hpp>
#include <psycle/host/engine/VstHost.hpp>
#include <psycle/host/gui/ProgressDialog.hpp>
#undef min //\todo : ???
#undef max //\todo : ???
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm> // std::transform
#include <cctype>	   // std::tolower
#include <cstdint>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		int CNewMachine::pluginOrder = 2;
		bool CNewMachine::pluginName = true;
		int CNewMachine::_numPlugins = -1;
		int CNewMachine::LastType0=0;
		int CNewMachine::LastType1=0;
		int CNewMachine::NumPlugsInCategories = 0;

		PluginInfo* CNewMachine::_pPlugsInfo[MAX_BROWSER_PLUGINS];
		InternalMachineInfo * CNewMachine::_pInternalMachines[NUM_INTERNAL_MACHINES];

		std::map<std::string,std::string> CNewMachine::dllNames;
		std::map<CString, int> CNewMachine::CustomFolders;
		int numCustCategories(1);
		CString CustomFolderName;
		CString tempCustomFolderName;

		const int IS_FOLDER           = 2000000000;
		const int IS_INTERNAL_MACHINE = 1000000000;

		const int TIMER_INTERVAL = 100;

		RECT UpPos;
		bool bScrollUp;
		bool bScrolling = false;
		int ScrollCount = 0;
		int itemheight;
		bool bDragging = false;
		CPoint MousePt;

		///\todo is it for debugging?
		void ShowMessage (int var, CString title)
		{
			char buffer[100];  sprintf (buffer, "%d", var); MessageBox (0,buffer, title,0);
		}

		///\todo is it for debugging?
		void ShowMessage (CString var, CString title)
		{
			MessageBox (0,var, title,0);
		}

		void CNewMachine::learnDllName(const std::string & fullname)
		{
			std::string str=fullname;
			// strip off path
			std::string::size_type pos=str.rfind('\\');
			if(pos != std::string::npos) str=str.substr(pos+1);
			// transform string to lower case
			std::transform(str.begin(),str.end(),str.begin(),std::tolower);
			dllNames[str]=fullname;
		}

		///\todo Important: There can exists dlls with the same name (there is a phantom.dll which is a VST).
		///      what about adding a new parameter indicating if we want a VST or a Psycle plugin?
		///      or maybe if found more than one entry, ask the user which one he wants to use?
		bool CNewMachine::lookupDllName(const std::string & name, std::string & result)
		{
			std::map<std::string,std::string>::iterator iterator
				= dllNames.find(name);
			if(iterator != dllNames.end())
			{
				result=iterator->second;
				return true;
			}
			return false;
		}

		CNewMachine::CNewMachine(CWnd* pParent) : CDialog(CNewMachine::IDD, pParent)
		{
			OutBus = false;
		}

		CNewMachine::~CNewMachine()
		{
		}

		void CNewMachine::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_CHECK_ALLOW, m_Allow);
			DDX_Control(pDX, IDC_NAMELABEL, m_nameLabel);
			DDX_Control(pDX, IDC_BROWSER, m_browser);
			DDX_Control(pDX, IDC_VERSIONLABEL, m_versionLabel);
			DDX_Control(pDX, IDC_DESCLABEL, m_descLabel);
			DDX_Control(pDX, IDC_DLLNAMELABEL, m_dllnameLabel);
			DDX_Control(pDX, IDC_LISTSTYLE, comboListStyle);
			DDX_Control(pDX, IDC_NAMESTYLE, comboNameStyle);
		}

		BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
			ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
			ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
			ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_CHECK_ALLOW, OnCheckAllow)
			ON_CBN_SELENDOK(IDC_LISTSTYLE, OnCbnSelendokListstyle)
			ON_CBN_SELENDOK(IDC_NAMESTYLE, OnCbnSelendokNamestyle)
			ON_NOTIFY(TVN_BEGINDRAG, IDC_BROWSER, BeginDrag)
			ON_WM_MOUSEMOVE()
			ON_WM_CANCELMODE()
			ON_WM_LBUTTONUP()
			ON_WM_CONTEXTMENU()
			ON_COMMAND(ID__ADDSUBFOLDER, NMPOPUP_AddSubFolder)
			ON_COMMAND(ID__ADDFOLDERONSAMELEVEL, NMPOPUP_AddFolderSameLevel)
			ON_COMMAND(ID__RENAMEFOLDER, NMPOPUP_RenameFolder)
			ON_COMMAND(ID_DELETEFOLDER_MOVEPARNT, NMPOPUP_DeleteMoveToParent)
			ON_COMMAND(ID_DELETEFOLDER_MOVEUNCAT, NMPOPUP_DeleteMoveUncat)
			ON_COMMAND(ID__MOVETOTOPLEVEL, NMPOPUP_MoveToTopLevel)
			ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_BROWSER, BeginLabelEdit)
			ON_NOTIFY(TVN_ENDLABELEDIT, IDC_BROWSER, EndLabelEdit)
			ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
			ON_WM_TIMER()
			ON_NOTIFY(TVN_KEYDOWN, IDC_BROWSER, OnTvnKeydownBrowser)
		END_MESSAGE_MAP()
		
		BOOL CNewMachine::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			imgList.Create(IDB_MACHINETYPE,16,2,1);
			m_browser.SetImageList(&imgList,TVSIL_NORMAL);
			bAllowChanged = false;
			bCategoriesChanged = false;
			bEditing = false;
			NumPlugsInCategories = 0;
			LoadPluginInfo();


			// set internal machine properties
			// THESE PROPERTIES SHOULD BE MOVED TO THE ACTUAL INTERNAL MACHINE CLASSES
			_pInternalMachines[0] = new InternalMachineInfo;
			_pInternalMachines[0]->name = "Sampler";
			_pInternalMachines[0]->desc = "Stereo Sampler Unit. Inserts new sampler.";
			_pInternalMachines[0]->version = "V0.5b";
			_pInternalMachines[0]->machtype = false;
			_pInternalMachines[0]->Outputmachine = MACH_SAMPLER;
			_pInternalMachines[0]->OutBus = true;
			_pInternalMachines[0]->LastType0 = Machine::type_type(0);
			_pInternalMachines[0]->LastType1 = Machine::type_type(0);
			
			_pInternalMachines[1] = new InternalMachineInfo;
			_pInternalMachines[1]->name = "Dummy";
			_pInternalMachines[1]->desc = "Replaces inexistent plugins";
			_pInternalMachines[1]->version = "V1.0";
			_pInternalMachines[1]->machtype = true;
			_pInternalMachines[1]->Outputmachine = MACH_DUMMY;
			_pInternalMachines[1]->OutBus = false;
			_pInternalMachines[1]->LastType0 = Machine::type_type(0);
			_pInternalMachines[1]->LastType1 = Machine::type_type(1);
			
			_pInternalMachines[2] = new InternalMachineInfo;
			_pInternalMachines[2]->name = "Sampulse Sampler V2";
			_pInternalMachines[2]->desc = "Sampler with the essence of FastTracker II and Impulse Tracker 2";
			_pInternalMachines[2]->version = "V0.5b";
			_pInternalMachines[2]->machtype = false;
			_pInternalMachines[2]->Outputmachine = MACH_XMSAMPLER;
			_pInternalMachines[2]->OutBus = true;
			_pInternalMachines[2]->LastType0 = Machine::type_type(0);
			_pInternalMachines[2]->LastType1 = Machine::type_type(0);
			
			_pInternalMachines[3] = new InternalMachineInfo;
			_pInternalMachines[3]->name = "Note Duplicator";
			_pInternalMachines[3]->desc = "Repeats the Events received to the selected machines";
			_pInternalMachines[3]->version = "V1.0";
			_pInternalMachines[3]->machtype = false;
			_pInternalMachines[3]->Outputmachine = MACH_DUPLICATOR;
			_pInternalMachines[3]->OutBus = true;
			_pInternalMachines[3]->LastType0 = Machine::type_type(0);
			_pInternalMachines[3]->LastType1 = Machine::type_type(0);

			_pInternalMachines[4] = new InternalMachineInfo;
			_pInternalMachines[4]->name = "Send/Return Mixer";
			_pInternalMachines[4]->desc = "Emulates a Mixer table with send Fx's.";
			_pInternalMachines[4]->version = "V1.0";
			_pInternalMachines[4]->machtype = true;
			_pInternalMachines[4]->Outputmachine = MACH_MIXER;
			_pInternalMachines[4]->OutBus = false;
			_pInternalMachines[4]->LastType0 = Machine::type_type(0);
			_pInternalMachines[4]->LastType1 = Machine::type_type(0);

			_pInternalMachines[5] = new InternalMachineInfo;
			_pInternalMachines[5]->name = "LFO";
			_pInternalMachines[5]->desc = "Modulates selected parameters by an LFO.";
			_pInternalMachines[5]->version = "V0.1";
			_pInternalMachines[5]->machtype = true;
			_pInternalMachines[5]->Outputmachine = MACH_LFO;
			_pInternalMachines[5]->OutBus = false;
			_pInternalMachines[5]->LastType0 = Machine::type_type(0);
			_pInternalMachines[5]->LastType1 = Machine::type_type(0);

			LoadCategoriesFile();
			UpdateList();

			// fill combo boxes
			comboListStyle.AddString ("Type of Plugin");
			comboListStyle.AddString ("Class of Machine");
			comboListStyle.AddString ("Custom Categories");
			comboListStyle.SetCurSel (pluginOrder);

			comboNameStyle.AddString ("Filename and Path");
			comboNameStyle.AddString ("Plugin Name");
			comboNameStyle.SetCurSel ((int)pluginName);

			numCustCategories = 1;

			itemheight = m_browser.GetItemHeight ();
			
			return true;

		}

		void CNewMachine::OnDestroy() 
		{
			CDialog::OnDestroy();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
		}

		void CNewMachine::UpdateList(bool bInit)
		{
			m_browser.DeleteAllItems();
			HTREEITEM intFxNode;
			switch(pluginOrder)
			{
			// sort by plugin type
			case 0:
				hNodes[0] = m_browser.InsertItem("Internal Machines",0,0 , TVI_ROOT, TVI_LAST);
				m_browser.SetItemData (hNodes[0], IS_FOLDER);
				m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
				hNodes[1] = m_browser.InsertItem("Native plug-ins",2,2,TVI_ROOT,TVI_LAST);
				m_browser.SetItemData (hNodes[1], IS_FOLDER);
				m_browser.SetItemState (hNodes[1], TVIS_BOLD, TVIS_BOLD);
				hNodes[2] = m_browser.InsertItem("VST2 plug-ins",4,4,TVI_ROOT,TVI_LAST);
				m_browser.SetItemData (hNodes[2], IS_FOLDER);
				m_browser.SetItemState (hNodes[2], TVIS_BOLD, TVIS_BOLD);
				intFxNode = hNodes[0];
				//nodeindex = 3;

				for(int i(_numPlugins - 1) ; i >= 0 ; --i)
				{
					if(_pPlugsInfo[i]->error.empty())
					{
						int imgindex;
						HTREEITEM hitem;
						if( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if( _pPlugsInfo[i]->type == MACH_PLUGIN) 
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
							if( _pPlugsInfo[i]->type == MACH_PLUGIN) 
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
						HTREEITEM newitem;
						if(pluginName)
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						else
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						//associate node with plugin number
						m_browser.SetItemData (newitem, i);
					}
				}
				HTREEITEM newitem;

				for (int i(0);i<NUM_INTERNAL_MACHINES;i++)
				{
					newitem = m_browser.InsertItem(_pInternalMachines[i]->name.c_str(), _pInternalMachines[i]->machtype, _pInternalMachines[i]->machtype, hNodes[0], TVI_SORT);
					if ( i==0 ) hInt[0]=newitem;
					m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE+i);
				}

				m_browser.Select(hNodes[LastType0],TVGN_CARET);
			break;

			//sort by class of machine
			case 1:
				hNodes[0] = m_browser.InsertItem("Generators",2,2 , TVI_ROOT, TVI_LAST);
				m_browser.SetItemData (hNodes[0], IS_FOLDER);
				m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
				hNodes[1] = m_browser.InsertItem("Effects",3,3,TVI_ROOT,TVI_LAST);
				m_browser.SetItemData (hNodes[1], IS_FOLDER);
				m_browser.SetItemState (hNodes[1], TVIS_BOLD, TVIS_BOLD);
				intFxNode = hNodes[1];
				//nodeindex=2;
				for(int i(_numPlugins - 1) ; i >= 0 ; --i) // I Search from the end because when creating the array, the deepest dir comes first.
				{
					if(_pPlugsInfo[i]->error.empty())
					{
						int imgindex;
						HTREEITEM hitem;
						HTREEITEM newitem;
						if(_pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
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
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
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
						if(pluginName)
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						else
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						//associate the plugin number with the node
						m_browser.SetItemData (newitem,i);
					}

				}
				for (int i(0);i<NUM_INTERNAL_MACHINES;i++)
				{
					newitem = m_browser.InsertItem(_pInternalMachines[i]->name.c_str(), _pInternalMachines[i]->machtype, _pInternalMachines[i]->machtype, _pInternalMachines[i]->machtype?hNodes[1]:hNodes[0], TVI_SORT);
					m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE+i);
				}

				m_browser.Select(hNodes[LastType1],TVGN_CARET);
			break;
			
			//sort into custom folders
			case 2:
				hNodes[0] = m_browser.InsertItem(" Uncategorised",7,7, TVI_ROOT, TVI_LAST);
				m_browser.SetItemData (hNodes[0], IS_FOLDER);
				m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
				numCustCategories = 1;

				for(int i(_numPlugins - 1) ; i >= 0 ; --i) // I Search from the end because when creating the array, the deepest dir comes first.
				{
					if(_pPlugsInfo[i]->error.empty())
					{
						//determine plugin icons
						int imgindex;
						HTREEITEM hitem;
						if(_pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 2; 
							}
							else 
							{ 
								imgindex = 4; 
							}
						}
						else
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 3; 
							}
							else 
							{ 
								imgindex = 5; 
							}
						}
						//determine plugin folder
						if (_pPlugsInfo[i]->category == "")
						{
							hCategory = hNodes[0];
						}
						else
						{
							CString restofpath = _pPlugsInfo[i]->category.c_str ();
							int barpos = restofpath.Find ("|",0);
							CString curcategory = restofpath.Left (barpos);
							curcategory = curcategory.TrimRight ("|");
							hCategory = TVI_ROOT;
							HTREEITEM hParent;
							while (restofpath != "")
							{
								hParent = hCategory;
								hCategory = CategoryExists (hParent, curcategory);

								if (hCategory == NULL)
								{
									//category doesn't exist
									hNodes[numCustCategories] = m_browser.InsertItem (" " + curcategory, 6,6, hParent, TVI_SORT);
									hCategory = hNodes[numCustCategories];
									m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
									m_browser.SetItemState (hNodes[numCustCategories], TVIS_BOLD, TVIS_BOLD);
									numCustCategories++;
								}
								else
								{
									//category exists, just add to tree
									hitem = hCategory;
								}
								restofpath = restofpath.Right (restofpath.GetLength () - barpos - 1);
								barpos = restofpath.Find ("|",0);
								
								if (barpos > -1)
								{
									curcategory = restofpath.Left (barpos);
									curcategory = curcategory.TrimRight ("|");
								}
							}

							
						}
						// add plugin to appropriate node on tree
						if(pluginName)
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hCategory, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hCategory, TVI_SORT);
						
						m_browser.SetItemData (hPlug[i],i);
					}

				}
								
				//add categories + entries for internal machines	
				for(int i = 0 ; i < NUM_INTERNAL_MACHINES ; i++)
				{
					//determine internal machine folder
					if (!_pInternalMachines[i]->category.length())
					{
						hCategory = hNodes[0];
					}
					else
					{
						CString restofpath = _pInternalMachines[i]->category.c_str ();
						int barpos = restofpath.Find ("|",0);
						CString curcategory = restofpath.Left (barpos);
						curcategory = curcategory.TrimRight ("|");
						hCategory = TVI_ROOT;
						HTREEITEM hParent;
						while (restofpath != "")
						{
							hParent = hCategory;
							hCategory = CategoryExists (hParent, curcategory);

							if (hCategory == NULL)
							{
								//category doesn't exist
								hNodes[numCustCategories] = m_browser.InsertItem (" " + curcategory, 6,6, hParent, TVI_SORT);
								hCategory = hNodes[numCustCategories];
								m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
								m_browser.SetItemState (hNodes[numCustCategories], TVIS_BOLD, TVIS_BOLD);
								numCustCategories++;
							}
							restofpath = restofpath.Right (restofpath.GetLength () - barpos - 1);
							barpos = restofpath.Find ("|",0);
							
							if (barpos > -1)
							{
								curcategory = restofpath.Left (barpos);
								curcategory = curcategory.TrimRight ("|");
							}
						}
					
					}
					// add internal machine to appropriate node on tree
					hInt[i] = m_browser.InsertItem(_pInternalMachines[i]->name.c_str(), _pInternalMachines[i]->machtype, _pInternalMachines[i]->machtype, hCategory, TVI_SORT);
					m_browser.SetItemData (hInt[i],IS_INTERNAL_MACHINE + i);
				}
				
				// remove leading space from all folders 
				// (space was added at front of folder name to automatically sort folders to the top)
				RemoveCatSpaces(NULL);
			break;
			}
			Outputmachine = Machine::type_type(-1);
		}

		HTREEITEM CNewMachine::CategoryExists (HTREEITEM hParent, CString category)
		{
			bool bCatFound = false;
			HTREEITEM hChild = m_browser.GetChildItem (hParent);
			HTREEITEM hReturn = NULL;
			while ((hChild != NULL) && (bCatFound == false))
			{
				if ((m_browser.GetItemText (hChild) == (" " + category)) && (m_browser.GetItemData (hChild) == IS_FOLDER))
				{
					hReturn = hChild;
					bCatFound = true;
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}

			// item not found, so return nothing.
			if (!bCatFound)
				return NULL;
			else
				return hReturn;
		}

		void CNewMachine::RemoveCatSpaces (HTREEITEM hParent)
		{
			HTREEITEM hChild;
			if (hParent == NULL)
                hChild = m_browser.GetChildItem (TVI_ROOT);
			else
				hChild = m_browser.GetChildItem (hParent);
			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					CString tempstring = m_browser.GetItemText (hChild);
					tempstring.Delete (0,1);
					m_browser.SetItemText (hChild, tempstring);
					RemoveCatSpaces (hChild);
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}

		}

		void CNewMachine::SortChildren (HTREEITEM hParent)
		{
			HTREEITEM hChild = m_browser.GetChildItem (hParent);
			//add space to front of folders, to sort
			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					CString tempstring = m_browser.GetItemText (hChild);
					m_browser.SetItemText (hChild, " " + tempstring);
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}
			//sort alphabetically - folders will be at top because of the space
			m_browser.SortChildren (hParent);
			//remove the space
			hChild = m_browser.GetChildItem (hParent);
			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					CString tempstring = m_browser.GetItemText (hChild);
					tempstring.Delete (0,1);  //delete first char, which is the space
					m_browser.SetItemText (hChild, tempstring);
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}


		}

		void CNewMachine::OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR; pNMTreeView; // not used
			tHand = m_browser.GetSelectedItem();
			Outputmachine = Machine::type_type(-1);
			OutBus = false;
			int i = m_browser.GetItemData (tHand);
			
			CString name, desc, dll, version;


			if (i >= IS_FOLDER)
			{
				name = ""; desc = ""; dll = ""; version = "";
				m_Allow.SetCheck(false);
				m_Allow.EnableWindow(false);
			}
			else if ((i >= IS_INTERNAL_MACHINE) && (i <= (IS_INTERNAL_MACHINE + NUM_INTERNAL_MACHINES - 1)))
			{
					name = _pInternalMachines[i - IS_INTERNAL_MACHINE]->name.c_str();
					desc = _pInternalMachines[i - IS_INTERNAL_MACHINE]->desc.c_str();
					dll = "None (Internal Machine)";
					version = _pInternalMachines[i - IS_INTERNAL_MACHINE]->version.c_str();
					Outputmachine = _pInternalMachines[i - IS_INTERNAL_MACHINE]->Outputmachine;
					OutBus = _pInternalMachines[i - IS_INTERNAL_MACHINE]->OutBus;
					LastType0 = _pInternalMachines[i - IS_INTERNAL_MACHINE]->LastType0;
					LastType1 = _pInternalMachines[i - IS_INTERNAL_MACHINE]->LastType1;
					m_Allow.SetCheck(false);
					m_Allow.EnableWindow(false);

			}
			else
			{
				//ShowMessage (i, _pPlugsInfo[i]->category.c_str ()) ;
				std::string str = _pPlugsInfo[i]->dllname;
				std::string::size_type pos = str.rfind('\\');
				if(pos != std::string::npos)
					str=str.substr(pos+1);
				dll = str.c_str();
				name = _pPlugsInfo[i]->name.c_str();
				desc = _pPlugsInfo[i]->desc.c_str();
				version = _pPlugsInfo[i]->version.c_str();
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
				psOutputDll = _pPlugsInfo[i]->dllname;
				m_Allow.SetCheck(!_pPlugsInfo[i]->allow);
				m_Allow.EnableWindow(true);
			}
			m_nameLabel.SetWindowText(name);
			m_descLabel.SetWindowText(desc);
			m_dllnameLabel.SetWindowText(dll);
			m_versionLabel.SetWindowText(version);
			*pResult = 0;
		}

		void CNewMachine::OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			OnOK();
			*pResult = 0;
		}

		void CNewMachine::OnRefresh() 
		{
			if (MessageBox ("This operation will re-scan your plugins, and reset all plugin warnings.\n\nDo you wish to continue?","Warning!",MB_YESNO | MB_ICONWARNING) == IDYES)
			{
				DestroyPluginInfo();
				{
					char cache[1 << 10];
					GetModuleFileName(0, cache, sizeof cache);
					char * last(std::strrchr(cache,'\\'));
					std::strcpy(last, "\\psycle.plugin-scan.cache");
					DeleteFile(cache);
				}
				LoadPluginInfo();
				UpdateList();
				m_browser.Invalidate();
				SetFocus();
			}
		}

		void CNewMachine::OnCbnSelendokListstyle()
		{
			if ((bCategoriesChanged) && (pluginOrder == 2))
			{
				SetPluginCategories(NULL, NULL);
			}
			//set view style from entry in combobox
			pluginOrder=comboListStyle.GetCurSel();

			UpdateList();
			m_browser.Invalidate();				  
		}

		void CNewMachine::OnCbnSelendokNamestyle()
		{
			pluginName=comboNameStyle.GetCurSel()?true:false;

			if (bCategoriesChanged)
				{
					SetPluginCategories(NULL, NULL);
				}
			UpdateList();
			m_browser.Invalidate();	
		}

		void CNewMachine::LoadPluginInfo()
		{
			if(_numPlugins == -1)
			{
				host::loggers::info("Scanning plugins ...");

				::AfxGetApp()->DoWaitCursor(1); 
				int plugsCount(0);
				int badPlugsCount(0);
				_numPlugins = 0;
				bool progressOpen = !LoadCacheFile(plugsCount, badPlugsCount);

				class populate_plugin_list
				{
					public:
						populate_plugin_list(std::vector<std::string> & result, std::string directory)
						{
							::CFileFind finder;
							int loop = finder.FindFile(::CString((directory + "\\*").c_str()));
							while(loop)
							{
								loop = finder.FindNextFile();
								if(finder.IsDirectory()) {
									if(!finder.IsDots())
									{
										std::string sfilePath = finder.GetFilePath();
										populate_plugin_list(result,sfilePath);
									}
								}
								else
								{
									CString filePath=finder.GetFilePath();
									filePath.MakeLower();
									if(filePath.Right(4) == ".dll")
									{
										std::string sfilePath = filePath;
										result.push_back(sfilePath);
									}
								}
							}
							finder.Close();
						}
				};

				std::vector<std::string> nativePlugs;
				std::vector<std::string> vstPlugs;

				CProgressDialog Progress;
				{
					char c[1 << 10];
					::GetCurrentDirectory(sizeof c, c);
					std::string s(c);
					host::loggers::info("Scanning plugins ... Current Directory: " + s);
				}
				host::loggers::info("Scanning plugins ... Directory for Natives: " + Global::configuration().GetPluginDir());
				host::loggers::info("Scanning plugins ... Directory for VSTs: " + Global::configuration().GetVstDir());
				host::loggers::info("Scanning plugins ... Listing ...");
				if(progressOpen)
				{
					Progress.Create();
					Progress.SetWindowText("Scanning plugins ... Listing ...");
					Progress.ShowWindow(SW_SHOW);
				}

				populate_plugin_list(nativePlugs,Global::configuration().GetPluginDir());
				populate_plugin_list(vstPlugs,Global::configuration().GetVstDir());

				std::size_t plugin_count = nativePlugs.size() + vstPlugs.size();

				std::ostringstream s; s << "Scanning plugins ... Counted " << plugin_count << " plugins.";
				host::loggers::info(s.str());
				if(progressOpen) {
					Progress.m_Progress.SetStep(static_cast<int>(16384 / std::max(std::size_t(1), plugin_count)));
					Progress.SetWindowText(s.str().c_str());
				}
				std::ofstream out;
				{
					std::string module_directory;
					{
						char module_file_name[MAX_PATH];
						::GetModuleFileName(0, module_file_name, sizeof module_file_name);
						module_directory = module_file_name;
						module_directory = module_directory.substr(0, module_directory.rfind('\\'));
					}
					out.open((module_directory + "/psycle.plugin-scan.log.txt").c_str());
				}
				out
					<< "==========================================" << std::endl
					<< "=== Psycle Plugin Scan Enumeration Log ===" << std::endl
					<< std::endl
					<< "If psycle is crashing on load, chances are it's a bad plugin, "
					<< "specifically the last item listed, if it has no comment after the library file name." << std::endl;
				
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing Natives ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::loggers::info("Scanning plugins ... Testing Natives ...");
				out
					<< std::endl
					<< "======================" << std::endl
					<< "=== Native Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPlugins(plugsCount, badPlugsCount, nativePlugs, MACH_PLUGIN, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing VSTs ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::loggers::info("Scanning plugins ... Testing VSTs ...");
				out
					<< std::endl
					<< "===================" << std::endl
					<< "=== VST Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPlugins(plugsCount, badPlugsCount, vstPlugs, MACH_VST, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanned " << plugin_count << " plugins.";
					host::loggers::info(s.str().c_str());
					Progress.SetWindowText(s.str().c_str());
				}
				out.close();
				_numPlugins = plugsCount;
				if(progressOpen)
				{
					Progress.m_Progress.SetPos(16384);
					Progress.SetWindowText("Saving scan cache file ...");
				}
				host::loggers::info("Saving scan cache file ...");
				SaveCacheFile();
				if(progressOpen)
					Progress.OnCancel();
				::AfxGetApp()->DoWaitCursor(-1); 
				host::loggers::info("Done.");
			}
		}

		void CNewMachine::FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress)
		{
			for(unsigned fileIdx=0;fileIdx<list.size();fileIdx++)
			{
				if(pProgress)
				{
					pProgress->m_Progress.StepIt();
					::Sleep(1);
				}
				std::string fileName = list[fileIdx];

				out << fileName << " ... ";
				out.flush();
				FILETIME time;
				ZeroMemory(&time,sizeof FILETIME);
				HANDLE hFile=CreateFile(fileName.c_str(),
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if(hFile!=INVALID_HANDLE_VALUE) {
					GetFileTime(hFile,0,0,&time);
					CloseHandle(hFile);
				}
				bool exists(false);
				for(int i(0) ; i < _numPlugins; ++i)
				{
					if(_pPlugsInfo[i])
					{
						if
							(
								_pPlugsInfo[i]->FileTime.dwHighDateTime == time.dwHighDateTime &&
								_pPlugsInfo[i]->FileTime.dwLowDateTime == time.dwLowDateTime
							)
						{
							if(_pPlugsInfo[i]->dllname == fileName)
							{
								exists = true;
								const std::string error(_pPlugsInfo[i]->error);
								std::stringstream s;
								if(error.empty())
									s << "found in cache.";
								else
									s << "cache says it has previously been disabled because:" << std::endl << error << std::endl;
								out << s.str();
								out.flush();
								host::loggers::info(fileName + '\n' + s.str());
								break;
							}
						}
					}
				}
				if(!exists)
				{
					try
					{
						out << "new plugin added to cache ; ";
						out.flush();
						host::loggers::info(fileName + "\nnew plugin added to cache.");
						_pPlugsInfo[currentPlugsCount]= new PluginInfo;
						_pPlugsInfo[currentPlugsCount]->dllname = fileName;
						_pPlugsInfo[currentPlugsCount]->FileTime = time;
						if(type == MACH_PLUGIN)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;
							Plugin plug(0);
							try
							{
								 plug.Instance(fileName);
								 plug.Init(); // [bohan] hmm, we should get rid of two-stepped constructions.
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s << typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							catch(...)
							{
								std::ostringstream s;
								s << "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << _pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								_pPlugsInfo[currentPlugsCount]->name = "???";
								_pPlugsInfo[currentPlugsCount]->desc = "???";
								_pPlugsInfo[currentPlugsCount]->version = "???";
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = plug.GetName();
								{
									std::ostringstream s; s << (plug.IsSynth() ? "Psycle instrument" : "Psycle effect") << " by " << plug.GetAuthor();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s; s << plug.GetInfo()->Version; // API VERSION
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								if(plug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
								out << plug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// [bohan] plug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling plug.Free(); explicitly.
							try
							{
								plug.Free();
							}
							catch(const std::exception & e)
							{
								std::stringstream s;
								s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title << "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
							catch(...)
							{
								std::stringstream s;
								s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< "Type of exception is unknown, no further information available.";
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title << "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
						}
						else if(type == MACH_VST)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_VST;
							vst::plugin vstPlug(MACH_VST, MACHMODE_UNDEFINED, Machine::id_type());
							try
							{
								 vstPlug.Instance(fileName);
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s << typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							catch(...)
							{
								std::ostringstream s; s << "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << _pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								_pPlugsInfo[currentPlugsCount]->name = "???";
								_pPlugsInfo[currentPlugsCount]->desc = "???";
								_pPlugsInfo[currentPlugsCount]->version = "???";
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = vstPlug.GetName();
								{
									std::ostringstream s;
									s << (vstPlug.IsSynth() ? "VST2 instrument" : "VST2 effect") << " by " << vstPlug.GetVendorName();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s;
									s << vstPlug.GetVersion();
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								
								if(vstPlug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;

								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
								out << vstPlug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// [bohan] vstPlug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling vstPlug.Free(); explicitly.
							try
							{
								vstPlug.Free();
								// <bohan> phatmatik crashes here...
								// <magnus> so does PSP Easyverb, in FreeLibrary
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title << "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
							catch(...)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< "Type of exception is unknown, no further information available.";
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title << "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
						}
					}
					catch(const std::exception & e)
					{
						{
							std::stringstream s;
							s << std::endl << "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl;
							out << s.str().c_str(); out.flush();
							host::loggers::crash(s.str());
						}
						{
							std::stringstream s;
							s << typeid(e).name() << std::endl;
							if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
							out << s.str().c_str(); out.flush();
							host::loggers::crash(s.str());
						}
					}
					catch(...)
					{
						std::stringstream s; s
							<< std::endl
							<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl
							<< "Type of exception is unknown, no further information available.";
						out << s.str().c_str(); out.flush();
						host::loggers::crash(s.str());
					}
				}
				out << std::endl;
				out.flush();
			}
			out.flush();
		}

		void CNewMachine::DestroyPluginInfo()
		{
			for (int i=0; i<_numPlugins; i++)
			{
				zapObject(_pPlugsInfo[i]);
			}
			dllNames.clear();
			_numPlugins = -1;
			//clear internal machines class
			for (int h=0; h<NUM_INTERNAL_MACHINES; h++)
			{
				zapObject(_pInternalMachines[h]);
			}
		}

		bool CNewMachine::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount)
		{
			//load plugin cache
			char modulefilename[_MAX_PATH];
			GetModuleFileName(NULL,modulefilename,_MAX_PATH);
			std::string path=modulefilename;
			std::string::size_type pos=path.rfind('\\');
			if(pos != std::string::npos)
				path=path.substr(0,pos);
			std::string cache=path + "\\psycle.plugin-scan.cache";

			RiffFile file;
			CFileFind finder;

			if (!file.Open(cache.c_str())) 
			{
				return false;
			}

			char Temp[MAX_PATH];
			file.ReadChunk(Temp,8);
			Temp[8]=0;
			if (strcmp(Temp,"PSYCACHE")!=0)
			{
				file.Close();
				DeleteFile(cache.c_str());
				return false;
			}

			UINT version;
			file.Read(version);
			if (version != CURRENT_CACHE_MAP_VERSION)
			{
				file.Close();
				DeleteFile(cache.c_str());
				return false;
			}

			file.Read(_numPlugins);
			for (int i = 0; i < _numPlugins; i++)
			{
				PluginInfo p;
				file.ReadString(Temp,sizeof(Temp));
				file.ReadChunk(&p.FileTime,sizeof(_pPlugsInfo[currentPlugsCount]->FileTime));
				{
					UINT size;
					file.Read(size);
					if(size)
					{
						char *chars(new char[size + 1]);
						file.ReadChunk(chars, size);
						chars[size] = '\0';
						p.error = (const char*)chars;
						zapArray(chars);
					}
				}
				file.Read(p.allow);
				file.Read(p.mode);
				file.Read(p.type);
				file.ReadString(p.name);
				file.ReadString(p.desc);
				file.ReadString(p.version);
				//file.ReadString(p.category);
				if(finder.FindFile(Temp))
				{
					FILETIME time;
					finder.FindNextFile();
					if (finder.GetLastWriteTime(&time))
					{
						if
							(
								p.FileTime.dwHighDateTime == time.dwHighDateTime &&
								p.FileTime.dwLowDateTime == time.dwLowDateTime
							)
						{
							_pPlugsInfo[currentPlugsCount]= new PluginInfo;

							_pPlugsInfo[currentPlugsCount]->dllname = Temp;
							_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;

							///\todo this could be better handled
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								_pPlugsInfo[currentPlugsCount]->error = "";
							}
							if(!p.error.empty())
							{
								_pPlugsInfo[currentPlugsCount]->error = p.error;
							}

							_pPlugsInfo[currentPlugsCount]->allow = p.allow;

							_pPlugsInfo[currentPlugsCount]->mode = p.mode;
							_pPlugsInfo[currentPlugsCount]->type = p.type;
							_pPlugsInfo[currentPlugsCount]->name = p.name;
							_pPlugsInfo[currentPlugsCount]->desc = p.desc;
							_pPlugsInfo[currentPlugsCount]->version = p.version;
							//_pPlugsInfo[currentPlugsCount]->category = p.category;

							if(p.error.empty())
							{
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
							}
							++currentPlugsCount;
						}
					}
				}
			}
			file.Close();

			return true;
		}

		bool CNewMachine::LoadCategoriesFile ()
		{
			//load categories cache
			char catfilename[_MAX_PATH];
			GetModuleFileName(NULL,catfilename,_MAX_PATH);
			std::string path=catfilename;
			std::string::size_type pos=path.rfind('\\');
			if(pos != std::string::npos)
				path=path.substr(0,pos);
			std::string catcache=path + "\\psycle.custom-categories.cache";

			RiffFile file;
			
			if (!file.Open(catcache.c_str())) 
			{
				return false;
			}
			
			char Temp[MAX_PATH];
			file.ReadChunk(Temp,13);
			Temp[13]=0;
			if (strcmp(Temp,"PSYCATEGORIES")!=0)
			{
				file.Close();
				DeleteFile(catcache.c_str());
				return false;
			}

			UINT version;
			file.Read(version);
			if (version != CURRENT_CACHE_MAP_VERSION)
			{
				file.Close();
				DeleteFile(catcache.c_str());
				return false;
			}
			
			for (int h=0; h < NUM_INTERNAL_MACHINES;h++)
			{
				file.Read (h);
				file.ReadString (_pInternalMachines[h]->category);
			}

			file.Read(NumPlugsInCategories);
			PluginInfo p;

			for (int i = 0; i < NumPlugsInCategories; i++)
			{
				//match up dll to _pPlugsInfo entry
				bool bDllFound = false;

				file.ReadString(p.dllname);
				file.ReadString(p.category);
				int j = 0;
				
				while ((bDllFound == false) && (j < _numPlugins))
				{
					//compare dll names - if dll is not in plugin cache, ignore its category entry.
					if (p.dllname == _pPlugsInfo[j]->dllname)
					{
						_pPlugsInfo[j]->category = p.category;  //match up category to dll name
						bDllFound = true;
					}
					j++;
				}
			}
			file.Close();
			return true;
		}

		bool CNewMachine::SaveCacheFile()
		{
			char cache[_MAX_PATH];
			GetModuleFileName(NULL,cache,_MAX_PATH);
			char * last = strrchr(cache,'\\');
			std::strcpy(last,"\\psycle.plugin-scan.cache");
			DeleteFile(cache);
			RiffFile file;
			if(!file.Create(cache,true)) return false;
			file.WriteChunk("PSYCACHE",8);
			std::uint32_t version = CURRENT_CACHE_MAP_VERSION;
			file.Write(version);
			file.Write(_numPlugins);
			for (int i=0; i<_numPlugins; i++ )
			{
				file.WriteChunk(_pPlugsInfo[i]->dllname.c_str(),_pPlugsInfo[i]->dllname.length()+1);
				file.Write(_pPlugsInfo[i]->FileTime);
				{
					const std::string error(_pPlugsInfo[i]->error);
					std::uint32_t size(static_cast<int>(error.size()));
					file.Write(size);
					if(size) file.WriteChunk(error.data(), size);
				}
				file.Write(_pPlugsInfo[i]->allow);
				file.Write(_pPlugsInfo[i]->mode);
				file.Write(_pPlugsInfo[i]->type);
				file.WriteChunk(_pPlugsInfo[i]->name.c_str(),_pPlugsInfo[i]->name.length()+1);
				file.WriteChunk(_pPlugsInfo[i]->desc.c_str(),_pPlugsInfo[i]->desc.length()+1);
				file.WriteChunk(_pPlugsInfo[i]->version.c_str(),_pPlugsInfo[i]->version.length()+1);
			}
			file.Close();
			return true;
		}

		bool CNewMachine::SaveCategoriesFile()
		{
			char cache[_MAX_PATH];
			GetModuleFileName(NULL,cache,_MAX_PATH);
			char * last = strrchr(cache,'\\');
			std::strcpy(last,"\\psycle.custom-categories.cache");
			DeleteFile(cache);
			RiffFile file;
			if (!file.Create(cache,true)) 
			{
				return false;
			}
			file.WriteChunk("PSYCATEGORIES",13);
			std::uint32_t version = CURRENT_CACHE_MAP_VERSION;
			file.Write(version);
	
			//write categories of internal machines
			for (std::uint32_t h=0; h < NUM_INTERNAL_MACHINES;h++)
			{
				file.Write (h);
				file.WriteChunk(_pInternalMachines[h]->category.c_str (), _pInternalMachines[h]->category.length()+1);
			}
			
			file.Write(NumPlugsInCategories);
			
			for (int i=0; i<_numPlugins; i++ )
			{
				if (_pPlugsInfo[i]->category != "")
				{
					file.WriteChunk(_pPlugsInfo[i]->dllname.c_str(),_pPlugsInfo[i]->dllname.length()+1);
					file.WriteChunk(_pPlugsInfo[i]->category.c_str(),_pPlugsInfo[i]->category.length()+1);
				}
			}
			file.Close();
			return true;
		}

		void CNewMachine::SetPluginCategories (HTREEITEM hItem, CString Category)
		{
			//traverse plugin tree recursively, saving plugin categories to _pPlugsInfo[i]
			// call this function with hItem = NULL to start right from the highest level
			if(!hItem)
			{
				//deal with "Uncategorised" folder
				HTREEITEM hChild = m_browser.GetChildItem (hNodes[0]);

				while (hChild != NULL)
				{
					int i = m_browser.GetItemData (hChild);
					if (i < IS_INTERNAL_MACHINE)
						_pPlugsInfo[i]->category = "";
					else  //is an internal machine
						_pInternalMachines[i - IS_INTERNAL_MACHINE]->category = "";
					hChild = m_browser.GetNextSiblingItem (hChild);
				}
				//begin recursive saving
				Category = "";
				SetPluginCategories (TVI_ROOT, Category);
			}
			else if (hItem != hNodes [0])
			{
				HTREEITEM hChild = m_browser.GetChildItem (hItem);
				CString NewCategory;
				while (hChild != NULL)
				{
					if (m_browser.GetItemData (hChild) == IS_FOLDER)
					{
						//continue recursion since it's a folder
						NewCategory = Category + m_browser.GetItemText (hChild) + "|";
						//MessageBox (Category, "2");
						SetPluginCategories (hChild, NewCategory);
					}
					else
					{
						//save plugin's position in tree to _pPlugsInfo[i]
						int i = m_browser.GetItemData (hChild);
						if (i < IS_INTERNAL_MACHINE)
							_pPlugsInfo[i]->category = Category;
						else  //is an internal machine
						{
							_pInternalMachines[i - IS_INTERNAL_MACHINE]->category = Category;
						}
						
						if (m_browser.GetParentItem (hChild) == hNodes[0])
							Category = "";

						if (Category != "")
							NumPlugsInCategories++;


					}
					hChild = m_browser.GetNextSiblingItem (hChild);
				}
				// everything else, recursive searching
			}
		}

		void CNewMachine::OnOK() 
		{
			CEdit* pEdit = m_browser.GetEditControl();
			if(pEdit)
			{
				// save folder name - set focus to make tree contrl send end label edit message
				m_browser.SetFocus ();
			}
			else
			{			
				if (Outputmachine > -1) // Necessary so that you cannot doubleclick a Node
				{
					//do normal closing/saving action
					if (bCategoriesChanged) SetPluginCategories(NULL, NULL);
					if (bAllowChanged) SaveCacheFile();
					if (bCategoriesChanged) SaveCategoriesFile();
					if (Outputmachine == MACH_XMSAMPLER ) MessageBox("This version of the machine is for demonstration purposes. It is unusable except for Importing Modules","Sampulse Warning");
					CDialog::OnOK();

				}
			}
		}

		void CNewMachine::OnCheckAllow() 
		{
			for (int i=0; i<_numPlugins; i++)
			{
				if (tHand == hPlug[i])
				{
					_pPlugsInfo[i]->allow = !m_Allow.GetCheck();
					bAllowChanged = true;
				}
			}
		}

		bool CNewMachine::TestFilename(const std::string & name)
		{
			for(int i(0) ; i < _numPlugins ; ++i)
			{
				if(name == _pPlugsInfo[i]->dllname)
				{
					// bad plugins always have allow = false
					if(_pPlugsInfo[i]->allow) return true;
					std::ostringstream s; s
						<< "Plugin " << name << " is disabled because:" << std::endl
						<< _pPlugsInfo[i]->error << std::endl
						<< "Try to load anyway?";
					return ::MessageBox(0, s.str().c_str(), "Plugin Warning!", MB_YESNO | MB_ICONWARNING) == IDYES;
				}
			}
			return false;
		}

		void CNewMachine::BeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
		{
			//LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
			// edit folder name
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem ();
			//make sure item is a folder that is allowed to be edited
			if ((m_browser.GetItemData (hSelectedItem) >= IS_FOLDER) && (m_browser.GetSelectedItem () != hNodes[0]))
			{			
				bEditing = true;
				*pResult = 0;
			}
			else *pResult = true;
		}

		void CNewMachine::EndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
		{
			//LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
			CEdit* pEdit = m_browser.GetEditControl();
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem ();
			int intNameCount = 0;
			if(pEdit)
			{	
				CString tempstring;
				CString currenttext = m_browser.GetItemText (hSelectedItem);
				pEdit->GetWindowText(tempstring);
				*pResult = false;
				m_browser.SetItemText (hSelectedItem, tempstring);
				// make sure user has entered a category name			
				if (tempstring != "")
				{
					// make sure category name doesn't exist
					HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
					HTREEITEM hChild = m_browser.GetChildItem (hParent);
					while ((hChild != NULL) && (intNameCount < 3))
					{
						if (tempstring == m_browser.GetItemText (hChild))
							intNameCount++;
						hChild = m_browser.GetNextSiblingItem (hChild);
						
					}
					if (intNameCount >= 2)
					{
						// user has entered an invalid name
						MessageBox ("This folder already exists!", "Error!");
						intNameCount = 0;
						// let user continue editing
						m_browser.EditLabel (hSelectedItem);
						CEdit* pEdit2 = m_browser.GetEditControl();
						pEdit2->SetWindowText(tempstring);
						pEdit2->SetSel (0,-1,0);
					}
					else
					{
						if ((tempstring.Find("\\") == -1) && (tempstring.Find("/",0) == -1) && (tempstring.Find(":",0) == -1) && (tempstring.Find("*",0) == -1) && (tempstring.Find("\?",0) == -1) && (tempstring.Find("\"",0) == -1) && (tempstring.Find("\'",0) == -1) && (tempstring.Find("<",0) == -1)  && (tempstring.Find(">",0) == -1)  && (tempstring.Find("|",0) == -1))
						{
							// set folder name
							m_browser.SetItemText (hSelectedItem, tempstring);
							bEditing = false;
							bCategoriesChanged = true;
							SortChildren (hParent);
						}
						else
						{
							MessageBox ("Incorrect Character Entered!  The following characters are not allowed: \n \\ / : * ? \" \' < > | ", "Error!");
							m_browser.EditLabel (hSelectedItem);
							CEdit* pEdit2 = m_browser.GetEditControl();
							pEdit2->SetWindowText(tempstring);
							pEdit2->SetSel (0,-1,0);
						}
					}
				}
				else
				{
						MessageBox ("You must enter a category name!", "Error!");
						m_browser.EditLabel (hSelectedItem);
						CEdit* pEdit2 = m_browser.GetEditControl();
						pEdit2->SetWindowText(tempstring);
						pEdit2->SetSel (0,-1,0);
				}
			}
			else 
				*pResult = false;
		}

		void CNewMachine::BeginDrag(NMHDR *pNMHDR, LRESULT *pResult)
		{
			if (pluginOrder == 2)  //make sure plugins are being displayed by custom category
			{
				LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)pNMHDR;
				*pResult = 0;	// allow drag
	
				CImageList* piml = NULL;    /* handle of image list */
				POINT ptOffset;
				RECT rcItem;
				  
				if ((piml = m_browser.CreateDragImage(lpnmtv->itemNew.hItem)) == NULL) return;
				
				/* get the bounding rectangle of the item being dragged (rel to top-left of control) */
				if (m_browser.GetItemRect(lpnmtv->itemNew.hItem, &rcItem, true))
				{
					CPoint ptDragBegin;
					int nX, nY;
					/* get offset into image that the mouse is at */
					/* item rect doesn't include the image */
					ptDragBegin = lpnmtv->ptDrag;
					ImageList_GetIconSize(piml->GetSafeHandle(), &nX, &nY);
					ptOffset.x = (ptDragBegin.x - rcItem.left) + (nX - (rcItem.right - rcItem.left));
					ptOffset.y = (ptDragBegin.y - rcItem.top) + (nY - (rcItem.bottom - rcItem.top));
					/* convert the item rect to screen co-ords, for use later */
					MapWindowPoints(NULL, &rcItem);
				}
				else
				{
					GetWindowRect(&rcItem);
					ptOffset.x = ptOffset.y = 8;
				}
				
				BOOL bDragBegun = piml->BeginDrag(0, ptOffset);
				if (! bDragBegun)
				{
					delete piml;
					return;
				}
				
				CPoint ptDragEnter = lpnmtv->ptDrag;
				ClientToScreen(&ptDragEnter);
				if (!piml->DragEnter(NULL, ptDragEnter))
				{
					delete piml;
					return;
				}
				
				delete piml;
				
				// set the focus here, so we get a WM_CANCELMODE if needed
				SetFocus();
				
				// redraw item being dragged, otherwise it remains (looking) selected
				InvalidateRect(&rcItem, true);
				UpdateWindow();
				
				// Hide the mouse cursor, and direct mouse input to this window
				SetCapture(); 
				m_hItemDrag = lpnmtv->itemNew.hItem;
				bDragging = true;
			}

		}



		HTREEITEM CNewMachine::MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo, HTREEITEM hItemPos,  bool bAllowReplace)
		{		
  			// check if destination is a plugin, if so, make parent folder the destination
			if (hItemTo != TVI_ROOT)
			{
				if (m_browser.GetItemData (hItemTo) != IS_FOLDER)
					hItemTo = m_browser.GetParentItem (hItemTo);	
			}
			if (hItem == NULL || hItemTo == NULL) return NULL;
			if (!bAllowReplace)
			if (hItem == hItemTo || hItemTo == m_browser.GetParentItem(hItem)) return hItem;

  			// check we're not trying to move to a descendant
			HTREEITEM hItemParent = hItemTo;
			if (hItemParent != TVI_ROOT)
			{
				while (hItemParent != TVI_ROOT && (hItemParent = m_browser.GetParentItem(hItemParent)) != NULL)
	    			if (hItemParent == hItem) return NULL;
			}
			
			HTREEITEM hItemNew;
			
			//check if item already exists
			bool bExists = false;
			HTREEITEM hChild = m_browser.GetChildItem (hItemTo);
			CString ItemText =  m_browser.GetItemText (hItem);
			while ((hChild != NULL) && (!bExists))
			{
				if (m_browser.GetItemText(hChild) == ItemText)
				{
					bExists = true;
					hItemNew = hChild;
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}
			if (!bExists)
			{
				// copy items to new location, recursively, then delete old heirarchy
  				// get text, and other info
  				CString sText = m_browser.GetItemText(hItem);
				int itemdata = m_browser.GetItemData (hItem);
  				TVINSERTSTRUCT tvis;
  				tvis.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | 
	     					TVIF_SELECTEDIMAGE | TVIF_STATE;
  				tvis.item.hItem = hItem;
  				// we don't want to copy selection/expanded state etc
  				tvis.item.stateMask = (UINT)-1 & ~(TVIS_DROPHILITED | TVIS_EXPANDED | 
							TVIS_EXPANDEDONCE | TVIS_EXPANDPARTIAL | TVIS_SELECTED);
  				m_browser.GetItem(&tvis.item);
  				tvis.hParent = hItemTo;
  				tvis.hInsertAfter = hItemPos;
  				// if we're only copying, then ask for new data
  				//if (bCopyOnly && pfnCopyData != NULL)
	    			//tvis.item.lParam = pfnCopyData(tree, hItem, tvis.item.lParam);
  				hItemNew = m_browser.InsertItem(&tvis);
				m_browser.SetItemData (hItemNew, itemdata);
  				m_browser.SetItemText(hItemNew, sText);
			}
			
			// now move children to under new item
 			HTREEITEM hItemChild = m_browser.GetChildItem(hItem);
 			while (hItemChild != NULL)
  			{
	   			HTREEITEM hItemNextChild = m_browser.GetNextSiblingItem(hItemChild);
				///\todo warning C4701: potentially uninitialized local variable 'hItemNew' used
				MoveTreeItem(hItemChild, hItemNew, TVI_SORT, false);
				hItemChild = hItemNextChild;
  			}
			
			// clear old item data
			m_browser.SetItemData(hItem, 0);
			// no (more) children, so we can safely delete top item
			m_browser.DeleteItem(hItem);
			
  			return hItemNew; 
		}

		void CNewMachine::OnMouseMove(UINT nFlags, CPoint point)
		{
			if (m_hItemDrag)
			{
				CPoint pt;
			
    				// drag the item to the current position
    				pt = point;
    				ClientToScreen(&pt);

    				CImageList::DragMove(pt);
    				CImageList::DragShowNolock(false);			
			    
				if (CWnd::WindowFromPoint(pt) == &m_browser)
				{
      				//SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
      				TVHITTESTINFO tvhti;
      				tvhti.pt = pt;
      				m_browser.ScreenToClient(&tvhti.pt);
      				/*HTREEITEM hItemSel =*/ m_browser.HitTest(&tvhti);
      				m_browser.SelectDropTarget(tvhti.hItem);
    				}

    				CImageList::DragShowNolock(true);
  			}

			CPoint curpoint = point;
			ClientToScreen (&curpoint);
			m_browser.GetWindowRect (&UpPos);
			
			// sometime, fix this code to do automatic scrolling when user is dragging.
			MousePt = point;
			ClientToScreen (&MousePt);
			if ((UpPos.left < curpoint.x) && (UpPos.right > curpoint.x))
			{
				if ((UpPos.top < curpoint.y) && ((UpPos.top + 20) > curpoint.y))
				{
					bScrollUp = true;
					if (!bScrolling)
					{
						SetTimer (IDT_NEW_MACHINES_BROWSER_TIMER, TIMER_INTERVAL,NULL);
						bScrolling = true;
						ScrollCount = 0;
					}
				}
				else if (((UpPos.bottom - 20) < curpoint.y) && (UpPos.bottom > curpoint.y))
				{
					bScrollUp = false;
					if (!bScrolling)
					{
						SetTimer (IDT_NEW_MACHINES_BROWSER_TIMER, TIMER_INTERVAL,NULL);
						bScrolling = true;
						ScrollCount = 0;
					}
				}
				else 
				{
					//if (bScrolling)
						KillTimer (IDT_NEW_MACHINES_BROWSER_TIMER);
					bScrolling = false;
					
				}
				

			}
			else
			{
				bScrolling = false;
				KillTimer (IDT_NEW_MACHINES_BROWSER_TIMER);
			}
			CDialog::OnMouseMove(nFlags, point);
		}

		void CNewMachine::OnCancelMode()
		{
			CDialog::OnCancelMode();
			if (m_hItemDrag) OnEndDrag(nFlags, point);
			else CDialog::OnLButtonUp(nFlags, point);
		}

		void CNewMachine::OnLButtonUp(UINT nFlags, CPoint point)
		{
			if (m_hItemDrag) OnEndDrag(nFlags, point);
			else CDialog::OnLButtonUp(nFlags, point);
		}

		void CNewMachine::OnEndDrag(UINT nFlags, CPoint point)
		{
			bDragging = false;
			bScrolling = false;
			KillTimer (IDT_NEW_MACHINES_BROWSER_TIMER);
			if (!m_hItemDrag) return;

			CPoint pt;

			pt = point;
			ClientToScreen(&pt);

			/*BOOL bCopy =*/ (GetKeyState(VK_CONTROL) & 0x10000000);

  			// do drop

  			HTREEITEM hItemDrop = m_browser.GetDropHilightItem();

  			m_browser.SelectDropTarget(NULL);
  			
			if (hItemDrop)
			{
				if (!((m_hItemDrag == hNodes[0]) || ((m_browser.GetItemData(m_hItemDrag) == IS_FOLDER) &&((hItemDrop != hNodes[0]) || (m_browser.GetParentItem (hItemDrop) != hNodes[0])))))
				{
					HTREEITEM hItemDropped = hItemDrop;
					MoveTreeItem (m_hItemDrag, hItemDrop == NULL ? TVI_ROOT : hItemDrop, TVI_SORT, false);
				
					bCategoriesChanged = true;
					if (m_browser.GetItemData (hItemDropped) == IS_FOLDER) SortChildren(hItemDropped);
					else SortChildren(m_browser.GetParentItem (hItemDropped));
				}
			}

  			FinishDragging(true);
			
  			RedrawWindow();
		}

		void CNewMachine::FinishDragging(BOOL bDraggingImageList)
		{
  			if (m_hItemDrag)
  			{
    				if (bDraggingImageList)
    				{
      				CImageList::DragLeave(NULL);
      				CImageList::EndDrag();
    				}
    				ReleaseCapture();
    				ShowCursor(true);
    				m_hItemDrag = NULL;
    				m_browser.SelectDropTarget(NULL);
  			}
		}

		void CNewMachine::OnContextMenu(CWnd* pWnd, CPoint point)
		{
			if (m_hItemDrag) OnEndDrag(nFlags, point);
			else
			{
				UINT nFlags;
				CPoint newpoint = point;
				ScreenToClient (&newpoint);
				newpoint.y = newpoint.y - 13;   //compensate for shift (not sure why this happens, but it does)
				//  ^^  this number should be fixed to be something relative to the height of an 
				//      item/title bar/something like that, so it's accurate on ALL windows themes.
				
				HTREEITEM hItem = m_browser.HitTest(newpoint, &nFlags);
				if (hItem) m_browser.SelectItem(hItem);

				if (hItem && (pluginOrder == 2)/*&& (TVHT_ONITEM & nFlags)*/)
				{				
					//check if selected item is a folder
					if (m_browser.GetParentItem (hItem) != hNodes[0])
					{

						CMenu popupmenu;
						VERIFY(popupmenu.LoadMenu(IDR_NEWMACHINESPOPUP));
				
						// TO DO:  Delete/Add menu items depending on selection.
						if (m_browser.GetItemData(hItem) == IS_FOLDER)
						{
							//Check if the "Uncategorised" folder has been selected
							if (hItem == hNodes[0])
							{
								//grey out items that can't be used.
								popupmenu.EnableMenuItem (ID__RENAMEFOLDER, MF_GRAYED);
								popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEPARNT, MF_GRAYED);
								popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEUNCAT, MF_GRAYED);
								popupmenu.EnableMenuItem (ID__ADDSUBFOLDER, MF_GRAYED);
								popupmenu.EnableMenuItem (ID__MOVETOTOPLEVEL, MF_GRAYED);
							}

							//check if selected item is a root folder
							if (m_browser.GetParentItem (hItem) == NULL)
							{
								//prevent plugins from being moved to the root
								popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEPARNT, MF_GRAYED);
								popupmenu.EnableMenuItem (ID__MOVETOTOPLEVEL, MF_GRAYED);
							}
						}
						else
						{
							popupmenu.EnableMenuItem (ID__ADDSUBFOLDER, MF_GRAYED);
							popupmenu.EnableMenuItem (ID__RENAMEFOLDER, MF_GRAYED);
							popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEPARNT, MF_GRAYED);
							popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEUNCAT, MF_GRAYED);
							popupmenu.EnableMenuItem (ID__MOVETOTOPLEVEL, MF_GRAYED);
						}
						CMenu* pPopup = popupmenu.GetSubMenu(0);
						assert(pPopup);
				
						pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, CNewMachine::GetActiveWindow());
				
						popupmenu.DestroyMenu();
					}
				}
			}

		}

		void CNewMachine::NMPOPUP_AddSubFolder()
		{
			HTREEITEM hSelectedItem;
			hSelectedItem = m_browser.GetSelectedItem ();
			//make sure folders aren't added to the "Uncategorised" folder
			hNodes[numCustCategories] = m_browser.InsertItem ("A New Category", 6,6,hSelectedItem, TVI_SORT);
			m_browser.SelectItem (hNodes[numCustCategories]);
			m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
			m_browser.SetItemState (hNodes[numCustCategories], TVIS_BOLD, TVIS_BOLD);
			//let user edit the name of the new category
			/*CEdit* EditNewFolder =*/ m_browser.EditLabel (m_browser.GetSelectedItem ());
			numCustCategories++;
			bEditing = true;
		}



		void CNewMachine::NMPOPUP_AddFolderSameLevel()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
			hNodes[numCustCategories] = m_browser.InsertItem ("A New Category", 6,6,hParent, TVI_SORT);
			m_browser.SelectItem (hNodes[numCustCategories]);
			m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
			m_browser.SetItemState (hNodes[numCustCategories], TVIS_BOLD, TVIS_BOLD);
			/*CEdit* EditNewFolder =*/ m_browser.EditLabel (m_browser.GetSelectedItem ());
			numCustCategories++;
			bEditing = true;
		}

		void CNewMachine::NMPOPUP_RenameFolder()
		{	
			/*CEdit* EditNewFolder =*/ m_browser.EditLabel (m_browser.GetSelectedItem());
		}

		void CNewMachine::NMPOPUP_DeleteMoveToParent()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			//add code to move items contained in the subfolder to the parent folder
			//ALSO must check if sub folders being moved up a level already exist;  if they do,
			// ask the user when to move plugins or not.
			HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
			HTREEITEM hChild = m_browser.GetChildItem (hSelectedItem);
			while (hChild)
			{
				//Add code to check if item already exists.
				MoveTreeItem (hChild, hParent, TVI_SORT, false);
				hChild = m_browser.GetChildItem (hSelectedItem);
			}
			//delete category
			m_browser.DeleteItem (hSelectedItem);
			SortChildren(hParent);
		}

		void CNewMachine::NMPOPUP_MoveToTopLevel()
		{
			HTREEITEM hMovedItem = MoveTreeItem (m_browser.GetSelectedItem(), TVI_ROOT, TVI_SORT);
			m_browser.SelectItem (hMovedItem);
			SortChildren (hMovedItem);  //sort items in case two categories are merged
			SortChildren (m_browser.GetParentItem (hMovedItem));  //sort category that item was moved to
			bCategoriesChanged = true;
		}


		void CNewMachine::NMPOPUP_DeleteMoveUncat()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			//add code to move items contained in the subfolder to the "Uncategorised" folder.
			DeleteMoveUncat (hSelectedItem);
			//delete category
			SortChildren(hNodes[0]);
		}

		void CNewMachine::DeleteMoveUncat (HTREEITEM hParent)
		{
			HTREEITEM hChild = m_browser.GetChildItem (hParent);
			while (hChild)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					//recurse into folder
					DeleteMoveUncat (hChild);
				}
				else
				{
					//hChild is a plugin, move it
					MoveTreeItem (hChild, hNodes[0], TVI_SORT, false);
				}
				hChild = m_browser.GetChildItem (hParent);
			}
			m_browser.DeleteItem (hParent);
		}

		void CNewMachine::OnBnClickedCancel()
		{
			CEdit* pEdit = m_browser.GetEditControl();
			if (pEdit)
			{
				// save folder name - set focus to make tree contrl send end label edit message
				HTREEITEM Sel = m_browser.GetSelectedItem ();
				CString text = m_browser.GetItemText (Sel);
				m_browser.SetFocus ();
				m_browser.SetItemText (Sel, text);
			}
			else
			{
				if (bCategoriesChanged)
				{
					SetPluginCategories(NULL, NULL);
				}
				if (bAllowChanged)
					SaveCacheFile();

				if (bCategoriesChanged)
					SaveCategoriesFile();

				OnCancel();
			}
		}

		void CNewMachine::OnTimer(UINT nIDEvent)
		{
			ScrollCount++;
			if (ScrollCount > 0)
			{
				//scroll view, if possible
				int ScrollPos;
				ScrollPos = m_browser.GetScrollPos (SB_VERT);
				if (bScrollUp)
				{
					if (ScrollPos > 0)
					{
						m_browser.ScrollWindow (0, itemheight, 0, 0);
						HTREEITEM hTemp = m_browser.GetPrevVisibleItem(m_browser.GetFirstVisibleItem ());
						m_browser.SelectSetFirstVisible (hTemp);
						m_browser.SelectDropTarget (hTemp);
						m_browser.Invalidate ();
					}
				}
				else
				{
					if (ScrollPos < m_browser.GetScrollLimit (SB_VERT))
						m_browser.ScrollWindow (0,  - itemheight, 0, 0);
						HTREEITEM hTemp = m_browser.GetNextVisibleItem(m_browser.GetFirstVisibleItem ());
						m_browser.SelectSetFirstVisible (hTemp);

						//NEED TO ADD A METHOD FOR FINDING THE LAST VISIBLE ITEM
						
						//
						
						HTREEITEM hNext = m_browser.GetNextVisibleItem (hTemp);
						HTREEITEM hCurrent;
						for (unsigned int inttemp = 1; inttemp < m_browser.GetVisibleCount (); inttemp++)
						{
							hCurrent = hNext;
							hNext = m_browser.GetNextVisibleItem (hCurrent);
							m_versionLabel.SetWindowText (m_browser.GetItemText(hCurrent));
						}
						///\todo warning C4701: potentially uninitialized local variable 'hCurrent' used
						m_browser.SelectDropTarget (hCurrent);
						m_browser.Invalidate ();
				}
			}
			CDialog::OnTimer(nIDEvent);
		}
	
		void CNewMachine::OnTvnKeydownBrowser(NMHDR *pNMHDR, LRESULT *pResult)
		{
			#if 0 ///\todo THIS DOESN'T WORK - SOMETHING TO FIX ONE DAY
				LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
				if (pTVKeyDown->wVKey == VK_APPS)
				{
					//deal with "right click " key on those modern fancy windows keyboards!
					HTREEITEM hSelected = m_browser.GetSelectedItem();
					if (hSelected != NULL)
					{
						RECT selrect;
						m_browser.GetItemRect (hSelected, &selrect, 0);
						CPoint point;  
						point.x = int((selrect.left + selrect.right ) / 2);
						point.y = int((selrect.bottom + selrect.top) / 2);
						m_browser.OnContextMenu (0,point);
					}
				}
			#endif
			*pResult = 0;
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

