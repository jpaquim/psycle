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

class PsyFilterBase;
class CoreSong;
class MachineFactory;

class SongSerializer
{
	public:
		SongSerializer();
		~SongSerializer();

		bool loadSong(const std::string & fileName, CoreSong& song);
		bool saveSong(const std::string & fileName, const CoreSong& song, int version);

	private:
		std::vector<PsyFilterBase*>  filters;
};

}}
#endif // SONGFACTORY_HPP
