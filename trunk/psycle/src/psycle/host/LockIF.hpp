// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once

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


}  // namespace
}  // namespace