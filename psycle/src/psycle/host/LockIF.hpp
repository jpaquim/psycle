// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once

#include <list>

namespace psycle {
namespace host {  

class LockIF {
  public:
  LockIF() {}

  static LockIF& Instance();

  virtual ~LockIF() = 0;
  virtual void lock() const = 0;
  virtual void unlock() const = 0;
};

inline LockIF::~LockIF() { }

namespace mfc {

  class WinLock : public LockIF {
   public:     
     WinLock() {  InitializeCriticalSection(&cs); }
     virtual ~WinLock() { DeleteCriticalSection(&cs); }

    virtual void lock() const { ::EnterCriticalSection(&cs); }
    virtual void unlock() const { ::LeaveCriticalSection(&cs); }

   private:
    mutable CRITICAL_SECTION cs;  
  };

}

class Timer { 
 public:
  Timer();
  virtual ~Timer();
       
  virtual void OnTimerViewRefresh() = 0;
  void StartTimer() { is_running_ = true; }
  void StopTimer() { is_running_ = false; }
  bool is_running() const { return is_running_; }

 private:
  bool is_running_;
};
            
typedef std::list<Timer*> TimerList;

class GlobalTimer {
  friend class CChildView;
  friend Timer;
 public:
  static GlobalTimer& instance() {
    static GlobalTimer instance;
    return instance;
  }       
	void OnViewRefresh();
 private:       
  GlobalTimer() : removed_(false), it(listeners_.end()) { }
  ~GlobalTimer() { }                
  
  void AddListener(Timer*);
  void RemoveListener(Timer*);
  void Clear() {
    listeners_.clear(); 
    it = listeners_.end();
    removed_ = true;
  }
  TimerList listeners_;
  TimerList::iterator it;
  bool removed_;
};

}  // namespace
}  // namespace