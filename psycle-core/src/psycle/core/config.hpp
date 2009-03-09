// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2009-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__CONFIG__INCLUDED
#define PSYCLE__CORE__CONFIG__INCLUDED
#pragma once

#ifdef _MSC_VER
	#ifdef PSYCLE__CORE__SOURCE
		#define PSYCLE__CORE__DECL __declspec(dllexport)
	#else
		#define PSYCLE__CORE__DECL __declspec(dllimport)
		#pragma comment(lib, "psycle-core")
	#endif
#else
	#define PSYCLE__CORE__DECL
#endif

#endif
