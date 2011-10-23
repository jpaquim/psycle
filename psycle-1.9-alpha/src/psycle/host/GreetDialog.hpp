///\file
///\brief interface file for psycle::host::CGreetDialog.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {

/// greeting window.
class CGreetDialog : public CDialog	{
	public:
		CGreetDialog(CWnd* pParent = NULL);

	protected:
		virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support		
		virtual BOOL OnInitDialog();
		DECLARE_MESSAGE_MAP()

	private:
		enum { IDD = IDD_GREETS };
		CListBox m_greetz;
};		

}}
