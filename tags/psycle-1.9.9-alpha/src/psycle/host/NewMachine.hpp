///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include <psycle/host/resources/resources.hpp>
#include <psycle/engine/machine.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		const int NODE_IDENTIFIER	  = 0xFFF00000;
		const int IS_INTERNAL_MACHINE = 0;
		const int IS_FOLDER           = 1<<20;
		const int IS_PLUGIN			  = 1<<21;
		//const int WHATEVER_ELSE	  = 1<<22; //and so on..
		const int TIMER_INTERVAL = 100;

		/// new machine dialog window.
		class CNewMachine : public CDialog
		{
			enum listimagetype
			{
				foldercategory=0,
				folderuncategorized,
				crashedplugin,
				empty,
				internalgen,
				internalfx,
				nativegen,
				nativefx,
				vstgen,
				vstfx,
			};
		public:
			CNewMachine(CWnd* pParent = 0);
			~CNewMachine();
		public:
			///< Output value indicating the selected machine.
			//\todo: It will not be needed when NewMachine Creates the machine and tells the song to add the new machine.
			Machine::class_type Outputmachine;
			///< Output value indicating its mode.
			//\todo: It will not be needed when NewMachine Creates the machine and tells the song to add the new machine.
			Machine::mode_type Outputmode;
			///< Output value with the full path of the dll, in case that it is a Plugin.
			//\todo: It will not be needed when NewMachine Creates the machine and tells the song to add the new machine.
			std::string psOutputDll;

		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();

			afx_msg void OnDestroy();
			afx_msg void OnRefresh();
			afx_msg void OnCheckAllow();
			afx_msg void OnCbnSelendokListstyle();
			afx_msg void OnCbnSelendokNamestyle();
			afx_msg void OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnCancelMode();
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			// No, it is not an afx_msg. We emulate it.
			void OnEndDrag(UINT nFlags, CPoint point);
			afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
			afx_msg void NMPOPUP_AddSubFolder();
			afx_msg void NMPOPUP_AddFolderSameLevel();
			afx_msg void NMPOPUP_RenameFolder();
			afx_msg void NMPOPUP_DeleteMoveToParent();
			afx_msg void NMPOPUP_DeleteMoveUncat();
			afx_msg void NMPOPUP_MoveToTopLevel();
			afx_msg void BeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void EndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnTvnKeydownBrowser(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnTimer(UINT nIDEvent);

			DECLARE_MESSAGE_MAP()

		protected:
			static bool LoadCategoriesFile();
			static bool SaveCategoriesFile();
			void UpdateList();
			void UpdateWithType();
			void UpdateWithMode();
			void UpdateWithCategories();
			void SetPluginCategories(HTREEITEM hItem, CString Category);
			HTREEITEM CategoryExists (HTREEITEM hParent, CString category);
			void SortChildren (HTREEITEM hParent);
			HTREEITEM MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo, HTREEITEM hItemPos = TVI_SORT, bool bAllowReplace = false);
			void FinishDragging(BOOL bDraggingImageList);
			void DeleteMoveUncat (HTREEITEM hParent);
			void RemoveCatSpaces (HTREEITEM hParent);

			inline int GetType(int nodeindex) { return nodeindex&NODE_IDENTIFIER; }
			inline int GetIndex(int nodeindex) { return nodeindex&~NODE_IDENTIFIER; };

			// Dialog Data
			enum { IDD = IDD_NEWMACHINE };
			CButton	m_Allow;
			CStatic	m_nameLabel;
			CTreeCtrl	m_browser;
			CStatic	m_versionLabel;
			CStatic	m_descLabel;
			CStatic	m_dllnameLabel;
			CComboBox comboTypeStyle;
			CComboBox comboNameStyle;
			CImageList imgList;

			HTREEITEM m_hItemDrag;
			//\todo: to be deleted
			HTREEITEM hNodes[1];
			std::map<HTREEITEM,std::string> plugidentify;

			bool bCacheChanged;
			bool bCategoriesChanged;

			///< Number of Plugins that contain a customized category
			///< (and as such, stored inside the categories file)
			static int _numCategorizedPlugins;
			///< Number of existing Categories, aside of "Uncategorized"
			static int _numCustCategories;
			// ?
//			static std::map<CString, int> customFolders;
			CString CustomFolderName;
			CString tempCustomFolderName;

			RECT UpPos;
			bool bScrollUp;
			bool bScrolling;
			int ScrollCount;
			bool bDragging;
			//Used as a helper for window-scrolling.
			int itemheight;
			bool bEditing;
			CPoint MousePt;
			UINT nFlags;
			CPoint point;
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
