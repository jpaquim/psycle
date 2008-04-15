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
#ifndef PSYCLE__CORE__PSYFILTERBASE__INCLUDED
#define PSYCLE__CORE__PSYFILTERBASE__INCLUDED
#pragma once

#include <string>

namespace psy { namespace core {

class CoreSong;

class RiffFile;
class MachineFactory;

/**
@author  Psycledelics  
*/
template <class T>
class PsyFilterBase
{
	public:
		virtual ~PsyFilterBase() {}
		//signals
		//sigslot::signal2<const std::string &, const std::string &> report;
		//sigslot::signal3<const std::int32_t& , const std::int32_t& , const std::string& > progress;

	public:
		virtual int version() const = 0;
		virtual std::string filePostfix() const = 0;
		virtual bool testFormat(const std::string & fileName) = 0;
		virtual bool load(const std::string & fileName, T& song, MachineFactory& factory)= 0;
		virtual bool save(const std::string & fileName, const T& song) = 0;
};

}}
#endif

