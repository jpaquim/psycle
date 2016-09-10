// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

//#include "stdafx.h"
#include "LockIF.hpp"
#include "MainFrm.hpp"

namespace psycle {
namespace host {  

LockIF& LockIF::Instance() {
  static mfc::WinLock locker;
  return locker;  
}

Timer::Timer() : is_running_(false) { GlobalTimer::instance().AddListener(this); }
Timer::~Timer() { GlobalTimer::instance().RemoveListener(this); }

void GlobalTimer::AddListener(Timer* listener) {                
  listeners_.push_back(listener);
}
    
void GlobalTimer::RemoveListener(Timer* listener) {  
  if (!listeners_.empty()) {
    TimerList::iterator i = std::find(listeners_.begin(), listeners_.end(), listener);
    if (i != listeners_.end()) {
      if (i!=it) {
        listeners_.erase(i);
      } else {
        listeners_.erase(it++);
        removed_ = true;
      }          
    }
  }
}
    
void GlobalTimer::OnViewRefresh() {          
  it = listeners_.begin();
  while (it != listeners_.end()) {
    Timer* timer = *it;
    if (timer->is_running()) {
      timer->OnTimerViewRefresh();      
    }    
    if (!removed_) {          
      ++it;
    } else {
      removed_ = false;
    }
  }
}   

void GlobalTimer::KillTimer() {
  CMainFrame* fr = (CMainFrame*) ::AfxGetMainWnd();
  CChildView* cv = &fr->m_wndView;    
  cv->KillTimer(39);
}

void GlobalTimer::StartTimer() {
  CMainFrame* fr = (CMainFrame*) ::AfxGetMainWnd();
  CChildView* cv = &fr->m_wndView;    
  cv->SetTimer(39,30,NULL);
}

}  // namespace
}  // namespace