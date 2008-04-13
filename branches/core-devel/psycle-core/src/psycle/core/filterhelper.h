// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef PSYCLE__CORE__PSYFILTER__INCLUDED
#define PSYCLE__CORE__PSYFILTER__INCLUDED
#pragma once

#include <string>

namespace psy { namespace core {

///\todo: Have a way to allow to save program-specific data to the song filters.
/// This will allow extended Songs (for example, save some windows specific options that the linux version doesn't need to know about, etc..)
//  Idea: LoaderHelpers:
//  The extended class from CoreSong (passed as class T), could provide a FilterHelper class, which will be called in each load/save state.
//  psycle-core will provide two FilterHelpers: Psy2FilterHelper and Psy3FilterHelper, in order to ease loading the data that these formats
//  already store in a non-packed way.

class FilterHelper {
	public:
		virtual ~FilterHelper(){}
		///\todo: Add helper classes
};

}}

#endif
