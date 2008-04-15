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
#ifndef SONGFACTORY_HPP
#define SONGFACTORY_HPP 
#pragma once

#include <string>
#include <vector>

namespace psy { namespace core {

class MachineFactory;

template <class T>
class PsyFilterBase;

template <class T>
class SongFactory
{
	public:
		SongFactory(MachineFactory& factory1);
		~SongFactory();

		T* createEmptySong();
		T* loadSong(const std::string & fileName);
		bool saveSong(const std::string & fileName, const T& song, int version);
		

		//signals
		//sigslot::signal2<const std::string &, const std::string &> report;
		//sigslot::signal3<const std::int32_t& , const std::int32_t& , const std::string& > progress;

	private:
		std::vector<PsyFilterBase<T>*> filters;
		MachineFactory& factory;
};

}}
#endif // SONGFACTORY_HPP
