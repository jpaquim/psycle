///\file
///\brief interface file for psycle::host::CBaseParamView.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
namespace psycle { namespace host {

class CFrameMachine;

class CBaseParamView : public CWnd {
	public:
		CBaseParamView(CFrameMachine* frame) : parentFrame(frame) {};
		virtual void Open(){};
		virtual bool GetViewSize(CRect& rect){ return false; }
		virtual void WindowIdle() { Invalidate(false); }
	protected:
		virtual void* WindowPtr(){ return GetSafeHwnd(); }
		CFrameMachine* parentFrame;
};

}}
