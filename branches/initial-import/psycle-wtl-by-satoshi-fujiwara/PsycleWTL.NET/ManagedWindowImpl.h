#pragma once
/** @file 
 *  @brief 
 *  $Date: 2005/01/15 22:44:35 $
 *  $Revision: 1.2 $
 */

#pragma push_macro("new")
#undef new

#using <mscorlib.dll>
#using "System.Windows.Forms.dll"
#using "..\XMSamplerUI\obj\Debug\XMSamplerUI.dll"
#include <vcclr.h>

#pragma push_macro("GetCurrentDirectory")
#undef GetCurrentDirectory
#include "..\iwindow.h"

namespace SF {

__gc class NetForm;

__nogc class ManagedWindowImpl :
	public IWindow
{
public:
	#pragma managed
	ManagedWindowImpl(const HWND parentHwnd,const TCHAR *pDllName,const TCHAR *pClassName);
	
	~ManagedWindowImpl(void)
	{
	}
	virtual void Show(){m_pForm->Show();};
	virtual void Close(){m_pForm->Close();};

	virtual void OnClose()
	{ 
		delete this;
	};

private:
	gcroot< System::Windows::Forms::Form* > m_pForm;
	gcroot< NetForm* > m_pNetForm;
};


__gc class NetForm 
{
public:
	#pragma managed
	NetForm(ManagedWindowImpl *p,System::Windows::Forms::Form *pForm)
	{
		m_pManagedWindowImpl = p;
		m_pForm = pForm;
		m_pForm->add_Closed(new System::EventHandler(this,OnClose));
	};

	#pragma managed
	void OnClose(System::Object* sender,System::EventArgs* e)
	{
		m_pManagedWindowImpl->OnClose();
	};
private:
	ManagedWindowImpl *m_pManagedWindowImpl;
	System::Windows::Forms::Form *m_pForm;
};


#pragma managed
ManagedWindowImpl::ManagedWindowImpl(const HWND parentHwnd,const TCHAR *pDllName,const TCHAR *pClassName)
{
	System::Text::StringBuilder* _path = new System::Text::StringBuilder();
	
	_path->Append(System::IO::Directory::GetCurrentDirectory());
	_path->Append(_T("\\"));
	_path->Append(pDllName);
	System::Reflection::Assembly* _asmb;
	_asmb = System::Reflection::Assembly::LoadFile(_path->ToString());
	//System::Type* _type = _asmb->GetType(new(pClassName));
	
	System::Object *_pobj = _asmb->CreateInstance(new System::String(pClassName));

	m_pForm = dynamic_cast<System::Windows::Forms::Form*>(_pobj);

	::SetParent(static_cast<HWND>(m_pForm->Handle.ToPointer()),parentHwnd);
	//System::Windows::Forms::Form::FromHandle(parentHwnd);

//		m_pForm->ShowDialog(System::Windows::Forms::Control::FromHandle(parentHwnd));
	m_pForm->Show();
	m_pForm->Text = L"Managed Window";
	m_pNetForm = new NetForm(this,m_pForm);
	
//	m_pForm->add_Closed(new System::EventHandler(new this,NetForm::OnClose));

	//m_pForm->TopMost = false;
};

}
#pragma pop_macro("GetCurrentDirectory")
#pragma pop_macro("new")


