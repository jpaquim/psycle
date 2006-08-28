///\file
///\brief implementation file for psycle::host::CNewMachine.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/NewMachine.hpp>
#include <psycle/host/ProgressDialog.hpp>
#include <psycle/host/cacheddllfinder.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/internal_machine_package.hpp>
#undef min //\todo : ???
#undef max //\todo : ???
//#include <sstream>
//#include <fstream>
#include <cstdint>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		int CNewMachine::_numCategorizedPlugins = 0;
		int CNewMachine::_numCustCategories = 1;
//		std::map<CString, int> CNewMachine::CustomFolders;


		CNewMachine::CNewMachine(CWnd* pParent)
			: CDialog(CNewMachine::IDD, pParent)
			, Outputmachine(MACH_UNDEFINED)
			, Outputmode(MACHMODE_UNDEFINED)
			, bCacheChanged(false)
		{
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
			DDX_Control(pDX, IDC_LISTSTYLE, comboTypeStyle);
			DDX_Control(pDX, IDC_NAMESTYLE, comboNameStyle);
		}

		BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
			ON_BN_CLICKED(IDC_CHECK_ALLOW, OnCheckAllow)
			ON_CBN_SELENDOK(IDC_LISTSTYLE, OnCbnSelendokListstyle)
			ON_CBN_SELENDOK(IDC_NAMESTYLE, OnCbnSelendokNamestyle)
			ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
			ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
			ON_NOTIFY(TVN_BEGINDRAG, IDC_BROWSER, OnBeginDrag)
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
			ON_WM_TIMER()
			ON_NOTIFY(TVN_KEYDOWN, IDC_BROWSER, OnTvnKeydownBrowser)
		END_MESSAGE_MAP()
		
		BOOL CNewMachine::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			imgList.Create(IDB_MACHINETYPE,16,2,1);
			m_browser.SetImageList(&imgList,TVSIL_NORMAL);
			
			// fill combo boxes
			comboTypeStyle.AddString ("Type of Plugin");
			comboTypeStyle.AddString ("Class of Machine");
			comboTypeStyle.AddString ("Custom Categories");
			comboTypeStyle.SetCurSel (Global::configuration()._comboTypeSel);

			comboNameStyle.AddString ("Filename and Path");
			comboNameStyle.AddString ("Plugin Name");
			comboNameStyle.SetCurSel (Global::configuration()._comboNameSel);

//			bCategoriesChanged = false;	//\todo: verify that it is used enough/correctly.
//			bEditing = false;			//\todo: verify that it is used enough/correctly.
//			_numCustCategories = 1;		//\todo: the way it is used could be improved.
//			bScrolling = false;
//			ScrollCount = 0;
//			bDragging = false;
			LoadCategoriesFile();
			UpdateList();

			itemheight = m_browser.GetItemHeight ();
			
			return true;
		}

		void CNewMachine::OnOK() 
		{
			CEdit* pEdit = m_browser.GetEditControl();
			if(pEdit)
			{
				// save folder name - set focus to make tree contrl send end label edit message
				m_browser.SetFocus ();
			}
			else if (Outputmachine > -1) // This is to ensure that the selection is a machine, and not a folder
			{			
				//do normal closing/saving action
				if (Outputmachine == MACH_XMSAMPLER ) MessageBox("This version of the machine is for demonstration purposes. It is unusable except for Importing Modules","Sampulse Warning");
				CDialog::OnOK();
			}
		}

		void CNewMachine::OnCancel()
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
				CDialog::OnCancel();
			}
		}

		void CNewMachine::OnDestroy() 
		{
			if (bCacheChanged) Global::dllfinder().ForceSaveOnDestroy();
			//\todo: 
			//if (bCategoriesChanged) SetPluginCategories(NULL, NULL);
			//if (bCategoriesChanged) SaveCategoriesFile();

			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			CDialog::OnDestroy();
		}

		void CNewMachine::OnRefresh() 
		{
			if (MessageBox ("This operation will re-scan your plugins, and reset all plugin warnings.\n\nDo you wish to continue?","Warning!",MB_YESNO | MB_ICONWARNING) == IDYES)
			{
				//\todo: could there be a better way of doing this? ( concretely about the need to use Global::configuration() )
				Global::dllfinder().ResetFinder();
				CProgressCtrl progress;
				//\todo:
//				Global::dllfinder().progress.connect(this,&onSignalProgress);
//				Global::dllfinder().logger.connect(this,&onSignalLogger);

				Global::dllfinder().AddPath(Global::configuration().GetPluginDir(),MACH_PLUGIN);
				Global::dllfinder().AddPath(Global::configuration().GetVstDir(),MACH_VST);
				UpdateList();
			}
			SetFocus();
		}

		void CNewMachine::OnCbnSelendokListstyle()
		{
			//\todo:
//			if ((bCategoriesChanged) && (pluginOrder == 2))
//			{
//				SetPluginCategories(NULL, NULL);
//			}
			//set view style from entry in combobox
			Global::configuration()._comboTypeSel=comboTypeStyle.GetCurSel();
			UpdateList();
			m_browser.Invalidate();				  
		}

		void CNewMachine::OnCbnSelendokNamestyle()
		{
			Global::configuration()._comboNameSel=comboNameStyle.GetCurSel();
			//\todo:
//			if (bCategoriesChanged)
//				{
//					SetPluginCategories(NULL, NULL);
//				}
			UpdateList();
			m_browser.Invalidate();	
		}

		void CNewMachine::OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			CString name, desc, dll, version;

			HTREEITEM tHand = m_browser.GetSelectedItem();
			int i = m_browser.GetItemData (tHand);
			
			switch(GetType(i))
			{
			case IS_FOLDER:
				{
					Outputmachine = MACH_UNDEFINED;
					Outputmode = MACHMODE_UNDEFINED;
					name = ""; desc = ""; dll = ""; version = "";
					m_Allow.SetCheck(false);
					m_Allow.EnableWindow(false);
				}
				break;
			case IS_INTERNAL_MACHINE:
				{
					const InternalMachineInfo* iinfo = Machine::infopackage().GetInfo(Machine::class_type(GetIndex(i)));
					if ( iinfo )
					{
						Outputmachine = iinfo->mclass;
						Outputmode = iinfo->mode;
						name = iinfo->brandname;
						desc = "Internal Machine by "; desc.Append(iinfo->vendor);
						dll = "None (Internal Machine)";
						version.Format("%d",iinfo->version);
						m_Allow.SetCheck(true);
						m_Allow.EnableWindow(false);
					}
				}
				break;
			case IS_PLUGIN:
				{
					std::map<HTREEITEM,std::string>::iterator iditer = plugidentify.find(tHand);
					const PluginInfo& minfo = Global::dllfinder().GetPluginInfo(iditer->second);
					//\todo: temporary hack.
					if (minfo.subclass == MACH_VST && minfo.mode == MACHMODE_FX) Outputmachine = MACH_VSTFX;
					else Outputmachine = minfo.subclass;
					Outputmode = minfo.mode;
					dll = iditer->second.c_str();
					name = minfo.name.c_str();
					desc = minfo.desc.c_str();
					version = minfo.version.c_str();
					m_Allow.SetCheck(!minfo.allow);
					m_Allow.EnableWindow(true);
					psOutputDll = minfo.dllname;
				}
				break;
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

		void CNewMachine::OnCheckAllow() 
		{
			HTREEITEM tHand = m_browser.GetSelectedItem();
			int i = m_browser.GetItemData (tHand);
			switch(GetType(i))
			{
			case IS_PLUGIN:
				{
					std::map<HTREEITEM,std::string>::iterator iditer = plugidentify.find(tHand);
					PluginInfo& minfo = Global::dllfinder().GetPluginInfo(iditer->second);
					minfo.allow = !m_Allow.GetCheck();
					bCacheChanged = true;
				}
				break;
			}
		}

		void CNewMachine::OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult)
		{
			if (comboTypeStyle.GetCurSel() == 2)  //make sure plugins are being displayed by custom category
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
			//\todo: necessary?
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

				if (hItem && (comboTypeStyle.GetCurSel() == 2)/*&& (TVHT_ONITEM & nFlags)*/)
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
			hNodes[_numCustCategories] = m_browser.InsertItem ("A New Category", 6,6,hSelectedItem, TVI_SORT);
			m_browser.SelectItem (hNodes[_numCustCategories]);
			m_browser.SetItemData (hNodes[_numCustCategories], IS_FOLDER);
			m_browser.SetItemState (hNodes[_numCustCategories], TVIS_BOLD, TVIS_BOLD);
			//let user edit the name of the new category
			/*CEdit* EditNewFolder =*/ m_browser.EditLabel (m_browser.GetSelectedItem ());
			_numCustCategories++;
			bEditing = true;
		}



		void CNewMachine::NMPOPUP_AddFolderSameLevel()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
			hNodes[_numCustCategories] = m_browser.InsertItem ("A New Category", 6,6,hParent, TVI_SORT);
			m_browser.SelectItem (hNodes[_numCustCategories]);
			m_browser.SetItemData (hNodes[_numCustCategories], IS_FOLDER);
			m_browser.SetItemState (hNodes[_numCustCategories], TVIS_BOLD, TVIS_BOLD);
			/*CEdit* EditNewFolder =*/ m_browser.EditLabel (m_browser.GetSelectedItem ());
			_numCustCategories++;
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


		bool CNewMachine::LoadCategoriesFile ()
		{
			//load categories cache
/*			char catfilename[_MAX_PATH];
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
			//\todo : When the number of internal machines changes, this causes an error with the existing cache.
			for (int h=0; h < NUM_INTERNAL_MACHINES;h++)
			{
				file.Read (h);
				file.ReadString (_pInternalMachines[h]->category);
			}

			file.Read(_numCategorizedPlugins);
			PluginInfo p;

			for (int i = 0; i < _numCategorizedPlugins; i++)
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
*/
			return true;
		}

		bool CNewMachine::SaveCategoriesFile()
		{
/*			char cache[_MAX_PATH];
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

			file.Write(_numCategorizedPlugins);

			for (int i=0; i<_numPlugins; i++ )
			{
				if (_pPlugsInfo[i]->category != "")
				{
					file.WriteChunk(_pPlugsInfo[i]->dllname.c_str(),_pPlugsInfo[i]->dllname.length()+1);
					file.WriteChunk(_pPlugsInfo[i]->category.c_str(),_pPlugsInfo[i]->category.length()+1);
				}
			}
			file.Close();
*/
			return true;
		}

		void CNewMachine::UpdateList()
		{
			m_browser.DeleteAllItems();
			switch(comboTypeStyle.GetCurSel())
			{
			case 0: // sort by plugin type
				UpdateWithType(); break;
				
			case 1:	//sort by class of machine
				UpdateWithMode(); break;
				
			case 2: //sort into custom folders
				UpdateWithCategories(); break;
			}
		}

		void CNewMachine::UpdateWithType()
		{
			// 1.Creates a branch for Internal machines, and then iterates through the internalmachinepackage, adding the machines to
			//   this branch, or creating new branches for hosts.

			std::vector<HTREEITEM> itemhosts;

			HTREEITEM newitem = m_browser.InsertItem("Internal Machines",internalgen,internalgen, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (newitem, IS_FOLDER);
			m_browser.SetItemState (newitem, TVIS_BOLD, TVIS_BOLD);
			itemhosts.push_back(newitem);

			int modetoicon[]={internalgen,internalfx,internalfx,internalgen};
			Machine::infopackage().MoveFirst();
			while (!Machine::infopackage().end())
			{
				const InternalMachineInfo* iminfo = Machine::infopackage().GetInfoAtPos();
				if ( !iminfo->host )
				{
					if (iminfo->mode != MACHMODE_MASTER && !iminfo->deprecated)
					{
						newitem = m_browser.InsertItem(iminfo->brandname, modetoicon[iminfo->mode], modetoicon[iminfo->mode], itemhosts[0], TVI_SORT);
						m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE+iminfo->mclass);
					}
				}
				else if (!iminfo->deprecated)
				{
					newitem = m_browser.InsertItem(iminfo->brandname,internalgen+(itemhosts.size()*2),internalgen+(itemhosts.size()*2), TVI_ROOT, TVI_LAST);
					m_browser.SetItemData (newitem, IS_FOLDER+iminfo->mclass);
					m_browser.SetItemState (newitem, TVIS_BOLD, TVIS_BOLD);
					itemhosts.push_back(newitem);
				}
				Machine::infopackage().MoveNext();
			}
			HTREEITEM nonfunctional = m_browser.InsertItem("Non Functional",crashedplugin,crashedplugin, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (nonfunctional, IS_FOLDER);
			m_browser.SetItemState (nonfunctional, TVIS_BOLD, TVIS_BOLD);

			// 2.Iterates through the detected hosts, in order to add the machines of each host to the list.

			int index=2;
			std::vector<HTREEITEM>::iterator iterhosts = itemhosts.begin();
			iterhosts++;// skip internal host.
			while(iterhosts != itemhosts.end())
			{
				Machine::class_type hosttype = static_cast<Machine::class_type>(GetIndex(m_browser.GetItemData (*iterhosts)));
				Global::dllfinder().MoveFirstOf(hosttype);
				while (!Global::dllfinder().end())
				{
					HTREEITEM newitem;
					const PluginInfo* minfo = Global::dllfinder().GetInfoAtPos();
					if (minfo->error.empty())
					{
						if (comboNameStyle.GetCurSel() == 0)
							newitem = m_browser.InsertItem(minfo->dllname.c_str(), index+modetoicon[minfo->mode], index+modetoicon[minfo->mode], *iterhosts, TVI_SORT);
						else
							newitem = m_browser.InsertItem(minfo->name.c_str(), index+modetoicon[minfo->mode], index+modetoicon[minfo->mode], *iterhosts, TVI_SORT);
					}
					else
						newitem = m_browser.InsertItem(minfo->dllname.c_str(), crashedplugin, crashedplugin, nonfunctional, TVI_SORT);
					m_browser.SetItemData (newitem, IS_PLUGIN+minfo->subclass);
					//In order to be able to identify the selection in the plugin cache
					plugidentify[newitem]=Global::dllfinder().FileFromFullpath(minfo->dllname);
					Global::dllfinder().MoveNextOf(hosttype);
				}
				index+=2;
				iterhosts++;
			}
//			m_browser.Select(hNodes[LastType0],TVGN_CARET);
		}
		void CNewMachine::UpdateWithMode()
		{
			// 1.Creates branches for generators and effects, and then iterates through the internalmachinepackage, adding the machines to
			//   each branch

			std::vector<InternalMachineInfo> itemhosts;

			HTREEITEM itemgens = m_browser.InsertItem("Generators",internalgen,internalgen, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (itemgens, IS_FOLDER);
			m_browser.SetItemState (itemgens, TVIS_BOLD, TVIS_BOLD);
			HTREEITEM itemfx = m_browser.InsertItem("Effects",internalfx,internalfx, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (itemfx, IS_FOLDER);
			m_browser.SetItemState (itemfx, TVIS_BOLD, TVIS_BOLD);

			int modetoicon[]={internalgen,internalfx,internalfx,internalgen};
			HTREEITEM modetoitem[]={itemgens,itemfx,itemfx,itemgens};
			Machine::infopackage().MoveFirst();
			while (!Machine::infopackage().end())
			{
				HTREEITEM newitem;
				const InternalMachineInfo* iminfo = Machine::infopackage().GetInfoAtPos();
				if ( !iminfo->host && !iminfo->deprecated)
				{
					newitem = m_browser.InsertItem(iminfo->brandname, modetoicon[iminfo->mode], modetoicon[iminfo->mode], modetoitem[iminfo->mode], TVI_SORT);
					m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE+iminfo->mclass);
				}
				else if (!iminfo->deprecated)
				{
					itemhosts.push_back(*iminfo);
				}

				Machine::infopackage().MoveNext();
			}
			HTREEITEM nonfunctional = m_browser.InsertItem("Non Functional",crashedplugin,crashedplugin, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (nonfunctional, IS_FOLDER);
			m_browser.SetItemState (nonfunctional, TVIS_BOLD, TVIS_BOLD);

			// 2.Iterates through the Plugin Cache, in order to add the machines of each host to the list.
			//   (probably not the best way, but I do it this way to know the icon.
			int index=2;
			std::vector<InternalMachineInfo>::iterator iterhosts = itemhosts.begin();
			while(iterhosts != itemhosts.end())
			{
				Machine::class_type hosttype = iterhosts->mclass;
				Global::dllfinder().MoveFirstOf(hosttype);
				while (!Global::dllfinder().end())
				{
					HTREEITEM newitem;
					const PluginInfo* minfo = Global::dllfinder().GetInfoAtPos();
					if (minfo->error.empty())
					{
						if (comboNameStyle.GetCurSel() == 0)
							newitem = m_browser.InsertItem(minfo->dllname.c_str(), index+modetoicon[minfo->mode], index+modetoicon[minfo->mode], modetoitem[minfo->mode], TVI_SORT);
						else
							newitem = m_browser.InsertItem(minfo->name.c_str(), index+modetoicon[minfo->mode], index+modetoicon[minfo->mode], modetoitem[minfo->mode], TVI_SORT);
					}
					else 
						newitem = m_browser.InsertItem(minfo->dllname.c_str(), crashedplugin, crashedplugin, nonfunctional, TVI_SORT);
					m_browser.SetItemData (newitem, IS_PLUGIN+minfo->subclass);
					//In order to be able to identify the selection in the plugin cache
					plugidentify[newitem]=Global::dllfinder().FileFromFullpath(minfo->dllname);

					Global::dllfinder().MoveNextOf(hosttype);
				}
				index+=2;
				iterhosts++;
			}
//			m_browser.Select(hNodes[LastType0],TVGN_CARET);

		}
		void CNewMachine::UpdateWithCategories()
		{
			// 1.Creates branches for generators and effects, and then iterates through the internalmachinepackage, adding the machines to
			//   each branch
/*
			std::vector<HTREEITEM> itemcateg;

			HTREEITEM hCategory = m_browser.InsertItem(" Uncategorised",7,7, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (hNodes[0], IS_FOLDER);
			m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
			itemcateg.push_back(hCategory);

			int modetoicon[]={internalgen,internalfx,internalfx,internalgen};
			Machine::infopackage().MoveFirst();
			while (!Machine::infopackage().end())
			{
				HTREEITEM newitem;
				const InternalMachineInfo* iminfo = Machine::infopackage().GetInfoAtPos();
				if ( !iminfo->host && !iminfo->deprecated)
				{
					newitem = m_browser.InsertItem(iminfo->brandname, modetoicon[iminfo->mode], modetoicon[iminfo->mode], modetoitem[iminfo->mode], TVI_SORT);
					m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE+iminfo->mclass);
				}
				else if (!iminfo->deprecated)
				{
					itemhosts.push_back(*iminfo);
				}

				Machine::infopackage().MoveNext();
			}
			HTREEITEM nonfunctional = m_browser.InsertItem("Non Functional",crashedplugin,crashedplugin, TVI_ROOT, TVI_LAST);
			m_browser.SetItemData (nonfunctional, IS_FOLDER);
			m_browser.SetItemState (nonfunctional, TVIS_BOLD, TVIS_BOLD);

			// 2.Iterates through the Plugin Cache, in order to add the machines of each host to the list.
			//   (probably not the best way, but I do it this way to know the icon.
			int index=2;
			std::vector<InternalMachineInfo>::iterator iterhosts = itemhosts.begin();
			while(iterhosts != itemhosts.end())
			{
				Machine::class_type hosttype = iterhosts->mclass;
				Global::dllfinder().MoveFirstOf(hosttype);
				while (!Global::dllfinder().end())
				{
					HTREEITEM newitem;
					const PluginInfo* minfo = Global::dllfinder().GetInfoAtPos();
					if (minfo->error.empty())
					{
						if (comboNameStyle.GetCurSel() == 0)
							newitem = m_browser.InsertItem(minfo->dllname.c_str(), index+modetoicon[minfo->mode], index+modetoicon[minfo->mode], modetoitem[minfo->mode], TVI_SORT);
						else
							newitem = m_browser.InsertItem(minfo->name.c_str(), index+modetoicon[minfo->mode], index+modetoicon[minfo->mode], modetoitem[minfo->mode], TVI_SORT);
					}
					else 
						newitem = m_browser.InsertItem(minfo->dllname.c_str(), crashedplugin, crashedplugin, nonfunctional, TVI_SORT);
					m_browser.SetItemData (newitem, IS_PLUGIN+minfo->subclass);
					//In order to be able to identify the selection in the plugin cache
					plugidentify[newitem]=Global::dllfinder().FileFromFullpath(minfo->dllname);

					Global::dllfinder().MoveNextOf(hosttype);
				}
				index+=2;
				iterhosts++;
			}
*/
			//			m_browser.Select(hNodes[LastType0],TVGN_CARET);

/*
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
								hNodes[_numCustCategories] = m_browser.InsertItem (" " + curcategory, 6,6, hParent, TVI_SORT);
								hCategory = hNodes[_numCustCategories];
								m_browser.SetItemData (hNodes[_numCustCategories], IS_FOLDER);
								m_browser.SetItemState (hNodes[_numCustCategories], TVIS_BOLD, TVIS_BOLD);
								_numCustCategories++;
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
					HTREEITEM newitem;
					if(pluginName)
						newitem = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hCategory, TVI_SORT);
					else
						newitem = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hCategory, TVI_SORT);

					m_browser.SetItemData (newitem,IS_PLUGIN+i);
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
							hNodes[_numCustCategories] = m_browser.InsertItem (" " + curcategory, 6,6, hParent, TVI_SORT);
							hCategory = hNodes[_numCustCategories];
							m_browser.SetItemData (hNodes[_numCustCategories], IS_FOLDER);
							m_browser.SetItemState (hNodes[_numCustCategories], TVIS_BOLD, TVIS_BOLD);
							_numCustCategories++;
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
				HTREEITEM newitem = m_browser.InsertItem(_pInternalMachines[i]->name.c_str(), _pInternalMachines[i]->machtype, _pInternalMachines[i]->machtype, hCategory, TVI_SORT);
				m_browser.SetItemData (newitem,IS_INTERNAL_MACHINE + i);
				//hInt[i] = m_browser.InsertItem(_pInternalMachines[i]->name.c_str(), _pInternalMachines[i]->machtype, _pInternalMachines[i]->machtype, hCategory, TVI_SORT);
				//m_browser.SetItemData (hInt[i],IS_INTERNAL_MACHINE + i);
			}

			// remove leading space from all folders 
			// (space was added at front of folder name to automatically sort folders to the top)
			RemoveCatSpaces(NULL);
			break;
*/		}

		void CNewMachine::SetPluginCategories (HTREEITEM hItem, CString Category)
		{
/*
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
							_numCategorizedPlugins++;


					}
					hChild = m_browser.GetNextSiblingItem (hChild);
				}
				// everything else, recursive searching
			}
*/
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

		UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

