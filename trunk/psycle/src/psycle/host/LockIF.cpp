// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "LockIF.hpp"

namespace psycle {
namespace host {  

LockIF& LockIF::Instance() {
  static mfc::WinLock locker;
  return locker;  
}


}  // namespace
}  // namespace