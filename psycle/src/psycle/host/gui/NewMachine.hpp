///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include <psycle/host/gui/resources/resources.hpp>
#include <psycle/host/engine/machine.hpp>
#include <afxcoll.h>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		const int MAX_BROWSER_NODES = 64;
		const int MAX_BROWSER_PLUGINS = 2048;
		const int NUM_INTERNAL_MACHINES = 7;
		const int IS_FOLDER           = 2000000000;
		const int IS_INTERNAL_MACHINE = 1000000000;
		const int TIMER_INTERVAL = 100;

		class CProgressDialog;

		/// new machine dialog window.
		class CNewMachine : public CDialog
		{
		public:
			CNewMachine(CWnd* pParent = 0);
			~CNewMachine();
		public:
			///< Output value indicating the selected machine.
			//\todo: It will not be needed when NewMachine Creates the machine and tells the song to add the new machine.
			Machine::type_type Outputmachine;
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
			// No, it is not an afx_msg....
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
			void UpdateList(bool bInit = false);
			void SetPluginCategories(HTREEITEM hItem, CString Category);
			HTREEITEM CategoryExists (HTREEITEM hParent, CString category);
			void SortChildren (HTREEITEM hParent);
			HTREEITEM MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo, HTREEITEM hItemPos = TVI_SORT, bool bAllowReplace = false);
			void FinishDragging(BOOL bDraggingImageList);
			void DeleteMoveUncat (HTREEITEM hParent);
			void RemoveCatSpaces (HTREEITEM hParent);

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

			HTREEITEM hNodes[MAX_BROWSER_NODES];
			HTREEITEM hInt[NUM_INTERNAL_MACHINES];
			HTREEITEM hPlug[MAX_BROWSER_PLUGINS];
			HTREEITEM tHand;
			HTREEITEM m_hItemDrag;
			HTREEITEM hCategory;

			bool updateCache;
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
