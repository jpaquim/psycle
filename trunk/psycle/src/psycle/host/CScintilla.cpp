// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include <psycle/host/detail/project.private.hpp>
#include "CScintilla.hpp"

namespace psycle {
namespace host  {

IMPLEMENT_DYNAMIC(CScintilla, CWnd)

BEGIN_MESSAGE_MAP(CScintilla, CWnd)
ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
END_MESSAGE_MAP()

} // namespace host
} // namespace psycle
