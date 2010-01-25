///\file
///\interface psycle::host::CMidiInputDlg.
#pragma once
#include "Psycle.hpp"
#include <vector>
namespace psycle {
	namespace host {

		/// midi input config window.
		class CMidiInputDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(CMidiInputDlg)
			DECLARE_MESSAGE_MAP()
		public:
			virtual void OnOK();
		protected:
			virtual BOOL OnInitDialog();
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);
		public:
			CMidiInputDlg();
			virtual ~CMidiInputDlg();
			int const static IDD = IDD_MIDI_INPUT;
			class group_with_message;
			class group
			{
				public:
					CButton record;
					CComboBox type;
					CEdit command;
					CEdit from;
					CEdit to;
					typedef group_with_message with_message;
			};
			class group_with_message : public group
			{
				public:
					CEdit message;
			};
			group velocity;
			group pitch;
			typedef std::vector<group::with_message*> groups_type;
			groups_type groups;
			CButton raw;
		};

	}   // namespace
}   // namespace
