///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
#include "Psycle.hpp"
namespace psycle {
namespace host {

		class CFrameMachine;

		class CBaseParamView : public CWnd
		{
		public:
			CBaseParamView(CFrameMachine* frame) : parentFrame(frame) {};
			virtual void Open(){};
			virtual bool GetViewSize(CRect& rect){ return false; }
			virtual void WindowIdle() { Invalidate(false); }
		protected:
			virtual void* WindowPtr(){ return GetSafeHwnd(); }
			CFrameMachine* parentFrame;
		};
	}   // namespace host
}   // namespace psycle
