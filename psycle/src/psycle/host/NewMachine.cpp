///\file
///\brief implementation file for psycle::host::CNewMachine.

#include "NewMachine.hpp"

#include "Configuration.hpp"
#include "ProgressDialog.hpp"

#include <psycle/core/machinefactory.h>
#include <psycle/core/machinehost.hpp>
#include <psycle/core/plugincatcher.h>
#include <psycle/core/machinekey.hpp>
#include <psycle/core/internalkeys.hpp>
#include <universalis/os/fs.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

#include <direct.h>

namespace psycle { namespace host {

		int CNewMachine::machineGrouping = groupRole;
		int CNewMachine::displayName = displayName;
		int CNewMachine::selectedGroup = Hosts::INTERNAL;
		int CNewMachine::selectedRole = MachineRole::GENERATOR;

		CNewMachine::CNewMachine(CWnd* pParent)
			: CDialog(CNewMachine::IDD, pParent)
			, outputMachine(InternalKeys::invalid)
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
			DDX_Control(pDX, IDC_DESCRLABEL, m_descLabel);
			DDX_Radio(pDX, IDC_BYTYPE, machineGrouping);
			DDX_Control(pDX, IDC_DLLNAMELABEL, m_dllnameLabel);
			DDX_Radio(pDX, IDC_SHOWDLLNAME, displayName);
			DDX_Control(pDX, IDC_APIVERSIONLABEL, m_APIversionLabel);
		}

		BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
			ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
			ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
			ON_BN_CLICKED(IDC_BYCLASS, OnByclass)
			ON_BN_CLICKED(IDC_BYTYPE, OnBytype)
			ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_SHOWDLLNAME, OnShowdllname)
			ON_BN_CLICKED(IDC_SHOWEFFNAME, OnShoweffname)
			ON_BN_CLICKED(IDC_CHECK_ALLOW, OnCheckAllow)
			ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
		END_MESSAGE_MAP()

		BOOL CNewMachine::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			imgList.Create(IDB_MACHINETYPE,16,2,1);
			m_browser.SetImageList(&imgList,TVSIL_NORMAL);
			UpdateList();
			return TRUE;
		}

		void CNewMachine::OnDestroy() 
		{
			CDialog::OnDestroy();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			m_browser.DeleteAllItems();
			treeToInfo.clear();
		}

		void CNewMachine::UpdateList(bool bInit)
		{
			m_browser.DeleteAllItems();
			treeToInfo.clear();
			HTREEITEM* hNodes = 0;
			HTREEITEM fx[Hosts::NUM_HOSTS];
			HTREEITEM gen[Hosts::NUM_HOSTS];
			HTREEITEM crashedNode;

			std::vector<MachineHost*> hosts = MachineFactory::getInstance().getHosts();
			if(machineGrouping == groupHost)
			{
				hNodes = new HTREEITEM[hosts.size()];
				unsigned int i=0;
				for ( ; i < hosts.size(); i++) {
					hNodes[i] = m_browser.InsertItem(hosts[i]->hostName().c_str() ,i*2, i*2 , TVI_ROOT, TVI_LAST);
					gen[i] = hNodes[i];
					fx[i] = hNodes[i];
					treeToInfo[hNodes[i]] = InternalKeys::invalid;
				}
				crashedNode = m_browser.InsertItem("Crashed or invalid plugins", 6, 6, TVI_ROOT,TVI_LAST);
				treeToInfo[crashedNode] = InternalKeys::invalid;
			}
			else {
				hNodes = new HTREEITEM[2];
				hNodes[0] = m_browser.InsertItem("Generators",0,0 , TVI_ROOT, TVI_LAST);
				hNodes[1] = m_browser.InsertItem("Effects",1,1,TVI_ROOT,TVI_LAST);
				crashedNode = m_browser.InsertItem("Crashed or invalid plugins",6,6,TVI_ROOT,TVI_LAST);
				treeToInfo[hNodes[0]] = InternalKeys::invalid;
				treeToInfo[hNodes[1]] = InternalKeys::invalid;
				treeToInfo[crashedNode] = InternalKeys::invalid;
				for (unsigned int i=0; i < Hosts::NUM_HOSTS; i++ ) {
					gen[i] = hNodes[0];
					fx[i] = hNodes[1];
				}
			}
			const PluginFinder& catcher = MachineFactory::getInstance().getFinder();
			int imgindex;
			HTREEITEM hPlug, hNode;
			for( int j = 0 ; j < hosts.size(); j++ ) {
				for(PluginFinder::const_iterator ite = catcher.begin(Hosts::type(j)); ite != catcher.end(Hosts::type(j)); ite++) {
					if ( ite->second.role() == MachineRole::MASTER ) { continue; }
					else if ( ite->second.role() == MachineRole::GENERATOR || ite->second.role() == MachineRole::CONTROLLER ) { imgindex = j*2 ; hNode = gen[j]; }
					else if ( ite->second.role() == MachineRole::EFFECT ) { imgindex = j*2 +1; hNode = fx[j]; }
					else { imgindex = 6; hNode = crashedNode; }
					if (ite->second.error().empty() && ite->second.allow()) {
						if ( displayName == displayDesc || ite->first.host() == Hosts::INTERNAL) {
							hPlug = m_browser.InsertItem(ite->second.name().c_str(), imgindex, imgindex, hNode, TVI_SORT);
						} else {
							hPlug = m_browser.InsertItem(ite->second.libName().c_str(), imgindex, imgindex, hNode, TVI_SORT);
						}
					} else {
						hPlug = m_browser.InsertItem(ite->second.libName().c_str(), 6, 6, crashedNode, TVI_SORT);
					}
					treeToInfo[hPlug] = ite->first;
				}
			}


			if(machineGrouping == groupHost ) { m_browser.Select(hNodes[selectedGroup],TVGN_CARET); }
			else { m_browser.Select(hNodes[selectedRole],TVGN_CARET); }
			outputMachine = InternalKeys::invalid;
			delete[] hNodes;
		}

		void CNewMachine::OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			//NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR; pNMTreeView; // not used
			tHand = m_browser.GetSelectedItem();
			//Do not do. OnSelchangedBrowser is called when destroying the elements, so the selection is lost
			//outputMachine = InternalKeys::invalid;
			MachineKey key = treeToInfo[tHand];
			if (key == InternalKeys::invalid ) {
				return;
			}
			const PluginInfo& info = MachineFactory::getInstance().getFinder().info(key);


			{ //  Strip the directory and put just the dll name.
				std::string str = info.libName();
				std::string::size_type pos = str.rfind('\\');
				if(pos != std::string::npos)
					str=str.substr(pos+1);
				m_dllnameLabel.SetWindowText(str.c_str());
			}

			m_nameLabel.SetWindowText(info.name().c_str());

			if ( info.error().empty()) {
				m_descLabel.SetWindowText(info.desc().c_str());
			}
			else
			{	// Strip the function, and show only the error.
				std::string str = info.error();
				std::ostringstream s; s << std::endl;
				std::string::size_type pos = str.find(s.str());
				if(pos != std::string::npos)
					str=str.substr(pos+1);

				m_descLabel.SetWindowText(str.c_str());
			}
			m_versionLabel.SetWindowText(info.plugVersion().c_str());
			m_APIversionLabel.SetWindowText(info.apiVersion().c_str());
			selectedGroup = key.host();
			if ( info.role() == MachineRole::EFFECT ) {
				selectedRole = MachineRole::EFFECT;
			}
			else {
				selectedRole = MachineRole::GENERATOR;
			}
			outputMachine = key;

			m_Allow.SetCheck(!info.allow());
			m_Allow.EnableWindow(TRUE);

			*pResult = 0;
		}

		void CNewMachine::OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			OnOK();	
			*pResult = 0;
		}

		void CNewMachine::OnRefresh() 
		{
			MachineFactory& factory = MachineFactory::getInstance();
			factory.RegenerateFinderData(true);
			UpdateList();
			m_browser.Invalidate();
			SetFocus();
		}
		void CNewMachine::OnBnClickedButton1()
		{
			MachineFactory& factory = MachineFactory::getInstance();
			factory.RegenerateFinderData(false);
			UpdateList();
			m_browser.Invalidate();
			SetFocus();
		}

		void CNewMachine::OnBytype() 
		{
			machineGrouping=groupHost;
			UpdateList();
			m_browser.Invalidate();
		}
		void CNewMachine::OnByclass() 
		{
			machineGrouping=groupRole;
			UpdateList();
			m_browser.Invalidate();
		}

		void CNewMachine::OnShowdllname() 
		{
			displayName=displayDll;	
			UpdateList();
			m_browser.Invalidate();
		}

		void CNewMachine::OnShoweffname() 
		{
			displayName = displayDesc;
			UpdateList();
			m_browser.Invalidate();
		}

		void CNewMachine::OnOK() 
		{
			if (outputMachine != InternalKeys::invalid ) // Necessary so that you cannot doubleclick a Node
			{
				CDialog::OnOK();
			}
		}

		void CNewMachine::OnCheckAllow() 
		{
			MachineKey key = treeToInfo[tHand];
			if (key != InternalKeys::invalid ) {
				PluginFinder& finder = MachineFactory::getInstance().getFinder();
				const PluginInfo& info = MachineFactory::getInstance().getFinder().info(key);
				finder.EnablePlugin(key, !info.allow());
			}
		}
	}   // namespace
}   // namespace
